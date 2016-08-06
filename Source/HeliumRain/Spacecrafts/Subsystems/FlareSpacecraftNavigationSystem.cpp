
#include "../../Flare.h"

#include "FlareSpacecraftNavigationSystem.h"
#include "../FlareSpacecraft.h"
#include "../FlareEngine.h"
#include "../../Game/FlareGame.h"
#include "../FlarePilotHelper.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftNavigationSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftNavigationSystem::UFlareSpacecraftNavigationSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
	, Status(EFlareShipStatus::SS_Manual)
	, AngularDeadAngle(0.5)
	, LinearDeadDistance(0.1)
	, LinearMaxDockingVelocity(10)
	, NegligibleSpeedRatio(0.0005)
{
	AnticollisionAngle = FMath::FRandRange(0, 360);
	DockConstraint = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::TickSystem(float DeltaSeconds)
{
	UpdateCOM();

	// Manual pilot
	if (IsManualPilot() && Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		LinearTargetVelocity = Spacecraft->GetStateManager()->GetLinearTargetVelocity();
		AngularTargetVelocity = Spacecraft->GetStateManager()->GetAngularTargetVelocity();
		UseOrbitalBoost = Spacecraft->GetStateManager()->IsUseOrbitalBoost();

		if (Spacecraft->GetStateManager()->IsWantFire())
		{
			Spacecraft->GetWeaponsSystem()->StartFire();
		}
		else
		{
			Spacecraft->GetWeaponsSystem()->StopFire();
		}
	}

	// Autopilot
	else if (IsAutoPilot())
	{
		FFlareShipCommandData CurrentCommand;
		if (CommandData.Peek(CurrentCommand))
		{
			if (CurrentCommand.Type == EFlareCommandDataType::CDT_Location)
			{
				if (UpdateLinearAttitudeAuto(DeltaSeconds, CurrentCommand.LocationTarget, FVector::ZeroVector, (CurrentCommand.PreciseApproach ? LinearMaxDockingVelocity : LinearMaxVelocity), 1.0))
				{
					ClearCurrentCommand();
				}
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_BrakeLocation)
			{
				UpdateLinearBraking(DeltaSeconds, CurrentCommand.VelocityTarget);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_Rotation)
			{
				UpdateAngularAttitudeAuto(DeltaSeconds);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_BrakeRotation)
			{
				UpdateAngularBraking(DeltaSeconds);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_Dock)
			{
				DockingAutopilot(Cast<AFlareSpacecraft>(CurrentCommand.ActionTarget), CurrentCommand.ActionTargetParam, DeltaSeconds);
			}
		}

		// TODO Autopilot anticollision system
	}

	// Physics
	if (!IsDocked())
	{
		// TODO enable physic when docked but attach the ship to the station

		PhysicSubTick(DeltaSeconds);
	}
}

void UFlareSpacecraftNavigationSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetParent()->GetDescription();
	Data = OwnerData;

	// Load data from the ship info
	if (Description)
	{
		LinearMaxVelocity = Description->LinearMaxVelocity;
		AngularMaxVelocity = Description->AngularMaxVelocity;
	}
}

void UFlareSpacecraftNavigationSystem::Start()
{
	UpdateCOM();
}


bool UFlareSpacecraftNavigationSystem::IsManualPilot()
{
	return (Status == EFlareShipStatus::SS_Manual);
}

bool UFlareSpacecraftNavigationSystem::IsAutoPilot()
{
	return (Status == EFlareShipStatus::SS_AutoPilot);
}

bool UFlareSpacecraftNavigationSystem::IsDocked()
{
	return (Status == EFlareShipStatus::SS_Docked);
}

void UFlareSpacecraftNavigationSystem::SetStatus(EFlareShipStatus::Type NewStatus)
{
	if (NewStatus != Status)
	{
		switch (NewStatus)
		{
			case EFlareShipStatus::SS_Manual:
				FLOG("UFlareSpacecraftNavigationSystem::SetStatus : Manual");
				break;

			case EFlareShipStatus::SS_AutoPilot:
				FLOG("UFlareSpacecraftNavigationSystem::SetStatus : AutoPilot");
				break;

			case EFlareShipStatus::SS_Docked:
				FLOG("UFlareSpacecraftNavigationSystem::SetStatus : Docked");
				break;

			default: break;
		}
		if (Spacecraft->GetStateManager())
		{
			// Not available on redock
			Spacecraft->GetStateManager()->OnStatusChanged();
		}
	}

	Status = NewStatus;
}

void UFlareSpacecraftNavigationSystem::SetAngularAccelerationRate(float Acceleration)
{
	AngularAccelerationRate = Acceleration;
}

/*----------------------------------------------------
	Docking
----------------------------------------------------*/

bool UFlareSpacecraftNavigationSystem::DockAt(AFlareSpacecraft* TargetStation)
{
	FLOGV("UFlareSpacecraftNavigationSystem::DockAt : '%s' docking at '%s'",
		*Spacecraft->GetParent()->GetImmatriculation().ToString(),
		*TargetStation->GetParent()->GetImmatriculation().ToString());

	FFlareDockingInfo DockingInfo = TargetStation->GetDockingSystem()->RequestDock(Spacecraft, Spacecraft->GetActorLocation());

	// Docking granted
	if (DockingInfo.Granted)
	{
		if (IsDocked())
		{
			FLOG("UFlareSpacecraftNavigationSystem::DockAt : leaving current dock");
			Undock();
		}

		FLOG("UFlareSpacecraftNavigationSystem::DockAt : access granted");
		PushCommandDock(DockingInfo);
		return true;
	}

	// Failed
	else
	{
		FLOG("UFlareSpacecraftNavigationSystem::DockAt : docking denied");
		return false;
	}
}

void UFlareSpacecraftNavigationSystem::BreakDock()
{
	// Detach from station
	if(DockConstraint)
	{
		DockConstraint->BreakConstraint();
		DockConstraint->DestroyComponent();
		DockConstraint = NULL;
	}
}

bool UFlareSpacecraftNavigationSystem::Undock()
{

	// Try undocking
	if (IsDocked())
	{
		if(Spacecraft->GetParent()->IsTrading())
		{
			FLOGV("UFlareSpacecraftNavigationSystem::Undock : '%s' is trading", *Spacecraft->GetParent()->GetImmatriculation().ToString());
			return false;
		}

		FLOGV("UFlareSpacecraftNavigationSystem::Undock : '%s' undocking from '%s'",
			*Spacecraft->GetParent()->GetImmatriculation().ToString(),
			*Data->DockedTo.ToString());

		// Detach from station
		if(DockConstraint)
		{
			DockConstraint->BreakConstraint();
			DockConstraint->DestroyComponent();
			DockConstraint = NULL;
		}

		AFlareSpacecraft* DockStation = GetDockStation();
		DockStation->GetDockingSystem()->ReleaseDock(Spacecraft, Data->DockedAt);

		// Update data
		SetStatus(EFlareShipStatus::SS_AutoPilot);
		Data->DockedTo = NAME_None;
		Data->DockedAt = -1;

		// Update Angular acceleration rate : when it's docked the mass is the ship mass + the station mass
		Spacecraft->SetRCSDescription(Spacecraft->GetRCSDescription());
		Spacecraft->OnUndocked(DockStation);

		// Leave
		PushCommandLocation(Spacecraft->GetRootComponent()->GetComponentTransform().TransformPositionNoScale(5000 * FVector(-1, 0, 0)));
		FLOG("UFlareSpacecraftNavigationSystem::Undock : successful");

		// Hack for bug #195: for ue4 to reweld all.
		Spacecraft->Airframe->SetSimulatePhysics(false);
		Spacecraft->Airframe->SetSimulatePhysics(true);

		return true;
	}

	// Failed
	else
	{
		FLOGV("UFlareSpacecraftNavigationSystem::Undock : '%s' is not docked", *Spacecraft->GetParent()->GetImmatriculation().ToString());
		return false;
	}
}

AFlareSpacecraft* UFlareSpacecraftNavigationSystem::GetDockStation()
{
	if (IsDocked())
	{
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Spacecraft->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Station = Spacecraft->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

			if (Station && Station->GetParent()->GetImmatriculation() == Data->DockedTo)
			{
				return Station;
			}
		}
	}
	return NULL;
}

static float GetApproachDockToDockLateralDistanceLimit(float Distance)
{
	// Approch cone :
	//  At 1 m -> 1 m
	//  At 100 m -> 5 m
	return Distance / 20 + 95;
}

static float GetApproachVelocityLimit(float Distance)
{
	// Approch cone :
	//  At 1 m -> 5 m/s
	//  At 100 m -> 40 m/s
	return Distance / 2.5 + 460;
}

void UFlareSpacecraftNavigationSystem::CheckCollisionDocking(AFlareSpacecraft* DockingCandidate)
{
	if (IsAutoPilot())
	{
		FFlareShipCommandData CurrentCommand;
		if (CommandData.Peek(CurrentCommand))
		{
			if (CurrentCommand.Type == EFlareCommandDataType::CDT_Dock)
			{
				// We are in a automatic docking process
				AFlareSpacecraft* DockStation = CurrentCommand.ActionTarget;
				if (DockStation != DockingCandidate)
				{
					// The hit spacecraft is not the docking target station
					return;
				}

				// Check dock alignement

				// TODO Put to external constants
				float DockingAngleLimit = 2; // 1° of angle error to dock
				float DockingVelocityLimit = 200; // 1 m/s
				float DockingLateralVelocityLimit = 20; // 10 cm/s
				float DockingAngularVelocityLimit = 10; // 5 °/s

				int32 DockId = CurrentCommand.ActionTargetParam;

				FVector ShipAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector ShipDockLocation = GetDockLocation();

				FFlareDockingInfo StationDockInfo = DockStation->GetDockingSystem()->GetDockInfo(DockId);
				FVector StationCOM = DockStation->Airframe->GetBodyInstance()->GetCOMPosition();

				FVector StationAngularVelocity = DockStation->Airframe->GetPhysicsAngularVelocity();
				FVector StationDockLocation =  DockStation->Airframe->GetComponentTransform().TransformPosition(StationDockInfo.LocalLocation);
				FVector StationDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(StationAngularVelocity, StationDockLocation- StationCOM);

				FVector ShipDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(ShipAngularVelocity, ShipDockLocation-COM);

				FVector StationDockLinearVelocity = StationDockSelfRotationInductedLinearVelocity + DockStation->GetLinearVelocity() * 100;
				FVector ShipDockLinearVelocity = ShipDockSelfRotationInductedLinearVelocity + Spacecraft->GetLinearVelocity() * 100;



				FVector ShipDockAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1, 0, 0)); // Ship docking port are always at front
				FVector StationDockAxis = DockStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(StationDockInfo.LocalAxis);
				float DockToDockAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-ShipDockAxis, StationDockAxis)));
				FVector RelativeDockToDockLinearVelocity = StationDockLinearVelocity - ShipDockLinearVelocity;


				// Angular velocity must be the same
				FVector RelativeDockAngularVelocity = ShipAngularVelocity - StationAngularVelocity;


				// Check if dockable
				bool OkForDocking = true;
				if (DockToDockAngle > DockingAngleLimit)
				{
					//FLOG("OkForDocking ? Not aligned");
					// Not aligned
					OkForDocking = false;
				}
				else if (RelativeDockToDockLinearVelocity.Size() > DockingVelocityLimit)
				{
					//FLOG("OkForDocking ? Too fast");
					// Too fast
					OkForDocking = false;
				}
				else if (FVector::VectorPlaneProject(RelativeDockToDockLinearVelocity, StationDockAxis).Size()  > DockingLateralVelocityLimit)
				{
					//FLOG("OkForDocking ? Too much lateral velocity");
					// Too much lateral velocity
					OkForDocking = false;
				}
				else if (RelativeDockAngularVelocity.Size() > DockingAngularVelocityLimit)
				{
					//FLOG("OkForDocking ? Too much angular velocity");
					// Too much angular velocity
					OkForDocking = false;
				}

				if (OkForDocking)
				{
					// All is ok for docking, perform dock
					//FLOG("  Ok for docking after hit");
					ConfirmDock(DockStation, DockId);
					return;
				}

			}
		}
	}
}


void UFlareSpacecraftNavigationSystem::DockingAutopilot(AFlareSpacecraft* DockStation, int32 DockId, float DeltaSeconds)
{
	// The dockin has multiple phase
	// - Rendez-vous : go at the entrance of the docking corridor.
	//     Its a large sphere  in front of the dock with a speed limit. During
	//     the approch phase the ship can go as phase is as he want.
	// 
	// - Approch (500 m): The ship advance in the approch corridor and try to keep itself
	//    near the dock axis an align th ship.
	//    The approch corridor is a cone with speed limitation more and more strict
	//    approching the station. There is a prefered  speed and a limit speed. If
	//    the limit speed is reach, the ship must abord and return to the entrance
	//    of the docking corridor. The is also a angular limit.
	// 
	// - Final Approch (5 m) : The ship slowly advance and wait for the docking trying to keep
	//    the speed and alignement.
	// 
	// - Docking : As soon as the ship is in the docking limits, the ship is attached to the station

	if (!DockStation)
	{
		// TODO LOG
		return;
	}
	//FLOG("==============DockingAutopilot==============");

	float DockingDockToDockDistanceLimit = 20; // 20 cm of linear distance to dock
	float FinalApproachDockToDockDistanceLimit = 100; // 1 m of linear distance
	float ApproachDockToDockDistanceLimit = 40000; // 400 m approch distance

	float FinalApproachDockToDockLateralDistanceLimit = 100; // 50 cm of linear lateral distance

	float DockingAngleLimit = 2; // 1° of angle error to dock
	float FinalApproachAngleLimit = 10;// 10° of angle error to dock

	float DockingVelocityLimit = 200; // 1 m/s
	float FinalApproachVelocityLimit = 500; // 5 m/s

	float DockingLateralVelocityLimit = 20; // 10 cm/s
	float FinalApproachLateralVelocityLimit = 50; // 0.5 m/s
	float ApproachLateralVelocityLimit = 1000; // 10 m/s

	float DockingAngularVelocityLimit = 10; // 5 °/s
	float FinalApproachAngularVelocityLimit = 10; // 10 °/s


	FVector ShipDockAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1, 0, 0)); // Ship docking port are always at front
	FVector ShipDockLocation = GetDockLocation();
	FVector ShipDockOffset = ShipDockLocation - Spacecraft->GetActorLocation();
	FVector ShipAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();




	FFlareDockingInfo StationDockInfo = DockStation->GetDockingSystem()->GetDockInfo(DockId);
	FVector StationCOM = DockStation->Airframe->GetBodyInstance()->GetCOMPosition();
	FVector StationDockAxis = DockStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(StationDockInfo.LocalAxis);
	FVector StationDockLocation =  DockStation->Airframe->GetComponentTransform().TransformPosition(StationDockInfo.LocalLocation);
	FVector StationDockOffset = StationDockLocation - DockStation->GetActorLocation();
	FVector StationAngularVelocity = DockStation->Airframe->GetPhysicsAngularVelocity();

	// Compute docking infos
	FVector DockToDockDeltaLocation = StationDockLocation - ShipDockLocation;
	float DockToDockDistance = DockToDockDeltaLocation.Size();

	// The linear velocity of the docking port induced by the station or ship rotation
	FVector ShipDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(ShipAngularVelocity, ShipDockLocation-COM);
	FVector ShipDockLinearVelocity = ShipDockSelfRotationInductedLinearVelocity + Spacecraft->GetLinearVelocity() * 100;

	FVector StationDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(StationAngularVelocity, StationDockLocation- StationCOM);
	FVector StationDockLinearVelocity = StationDockSelfRotationInductedLinearVelocity + DockStation->GetLinearVelocity() * 100;

	float InAxisDistance = FVector::DotProduct(DockToDockDeltaLocation, -StationDockAxis);
	FVector RotationInductedLinearVelocityAtShipDistance = (PI /  180.f) * FVector::CrossProduct(StationAngularVelocity, (StationDockLocation - StationCOM).GetUnsafeNormal() * (InAxisDistance + (StationDockLocation - StationCOM).Size()));
	FVector LinearVelocityAtShipDistance = RotationInductedLinearVelocityAtShipDistance + DockStation->GetLinearVelocity() * 100;


	AFlareSpacecraft* AnticollisionDockStation = DockStation;
	FVector RelativeDockToDockLinearVelocity = StationDockLinearVelocity - ShipDockLinearVelocity;
	float DockToDockAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-ShipDockAxis, StationDockAxis)));

	// Angular velocity must be the same
	FVector RelativeDockAngularVelocity = ShipAngularVelocity - StationAngularVelocity;

/*
	FLOGV("ShipLocation=%s", *(Spacecraft->GetActorLocation().ToString()));

	FLOGV("ShipDockAxis=%s", *ShipDockAxis.ToString());
	FLOGV("ShipDockOffset=%s", *ShipDockOffset.ToString());
	FLOGV("ShipDockLocation=%s", *ShipDockLocation.ToString());
	FLOGV("ShipAngularVelocity=%s", *ShipAngularVelocity.ToString());

	FLOGV("StationLocation=%s", *(DockStation->GetActorLocation().ToString()));
	FLOGV("StationCOM=%s", *StationCOM.ToString());
	FLOGV("LocalAxis=%s", *(StationDockInfo.LocalAxis.ToString()));
	FLOGV("StationDockAxis=%s", *StationDockAxis.ToString());
	FLOGV("StationDockOffset=%s", *StationDockOffset.ToString());
	FLOGV("StationDockLocation=%s", *StationDockLocation.ToString());
	FLOGV("StationAngularVelocity=%s", *StationAngularVelocity.ToString());

	FLOGV("DockToDockDeltaLocation=%s", *DockToDockDeltaLocation.ToString());
	FLOGV("DockToDockDistance=%f", DockToDockDistance);

	FLOGV("ShipDockSelfRotationInductedLinearVelocity=%s", *ShipDockSelfRotationInductedLinearVelocity.ToString());
	FLOGV("ShipDockLinearVelocity=%s", *ShipDockLinearVelocity.ToString());
	FLOGV("StationDockSelfRotationInductedLinearVelocity=%s", *StationDockSelfRotationInductedLinearVelocity.ToString());
	FLOGV("StationDockLinearVelocity=%s", *StationDockLinearVelocity.ToString());

	FLOGV("RelativeDockToDockLinearVelocity=%s", *RelativeDockToDockLinearVelocity.ToString());


	FLOGV("RotationInductedLinearVelocityAtShipDistance=%s", *RotationInductedLinearVelocityAtShipDistance.ToString());
	FLOGV("LinearVelocityAtShipDistance=%s", *LinearVelocityAtShipDistance.ToString());
	FLOGV("InAxisDistance=%f", InAxisDistance);

	FLOGV("DockToDockAngle=%f", DockToDockAngle);
*/
	// DrawDebugSphere(Spacecraft->GetWorld(), ShipDockLocation, 100, 12, FColor::Red, false,0.03);
	// DrawDebugSphere(Spacecraft->GetWorld(), StationDockLocation, 100, 12, FColor::Blue, false,0.03);
	// Output
	float MaxVelocity = 0;
	FVector LocationTarget = StationDockLocation - ShipDockOffset;
	FVector AxisTarget = -StationDockAxis;
	FVector AngularVelocityTarget = StationAngularVelocity;
	FVector VelocityTarget = LinearVelocityAtShipDistance - ShipDockSelfRotationInductedLinearVelocity;
	bool Anticollision = false;
	/*FLOGV("Initial LocationTarget=%s", *LocationTarget.ToString());
	FLOGV("Initial AxisTarget=%s", *AxisTarget.ToString());
	FLOGV("Initial AngularVelocityTarget=%s", *AngularVelocityTarget.ToString());
	FLOGV("Initial VelocityTarget=%s", *VelocityTarget.ToString());
*/
	// First find the current docking phase

	// Check if dockable
	bool OkForDocking = true;
	if (DockToDockDistance > DockingDockToDockDistanceLimit)
	{
		/*FLOG("OkForDocking ? Too far");
		FLOGV("  - limit= %f", DockingDockToDockDistanceLimit);
		FLOGV("  - distance= %f", DockToDockDistance);*/
		// Too far
		OkForDocking = false;
	}
	else if (DockToDockAngle > DockingAngleLimit)
	{
		//FLOG("OkForDocking ? Not aligned");
		// Not aligned
		OkForDocking = false;
	}
	else if (RelativeDockToDockLinearVelocity.Size() > DockingVelocityLimit)
	{
		//FLOG("OkForDocking ? Too fast");
		// Too fast
		OkForDocking = false;
	}
	else if (FVector::VectorPlaneProject(RelativeDockToDockLinearVelocity, StationDockAxis).Size()  > DockingLateralVelocityLimit)
	{
		//FLOG("OkForDocking ? Too much lateral velocity");
		// Too much lateral velocity
		OkForDocking = false;
	}
	else if (RelativeDockAngularVelocity.Size() > DockingAngularVelocityLimit)
	{
		//FLOG("OkForDocking ? Too much angular velocity");
		// Too much angular velocity
		OkForDocking = false;
	}

	if (OkForDocking)
	{
		/*FLOG("-> OK for docking");*/
		ConfirmDock(DockStation, DockId);
		return;
	}

	bool InFinalApproach = true;
	// Not dockable, check if in final approch
	if (DockToDockDistance > FinalApproachDockToDockDistanceLimit)
	{
		//FLOG("InFinalApproach ? Too far");
		// Too far
		InFinalApproach = false;
	}
	else if (FVector::VectorPlaneProject(DockToDockDeltaLocation, StationDockAxis).Size() > FinalApproachDockToDockLateralDistanceLimit)
	{
		//FLOG("InFinalApproach ? Too far in lateral axis");
		// Too far in lateral axis
		InFinalApproach = false;
	}
	else if (DockToDockAngle > FinalApproachAngleLimit)
	{
		//FLOG("InFinalApproach ? Not aligned");
		// Not aligned
		InFinalApproach = false;
	}
	else if (RelativeDockToDockLinearVelocity.Size() > FinalApproachVelocityLimit)
	{
		/*FLOG("InFinalApproach ? Too fast");
		FLOGV("  - limit= %f", FinalApproachVelocityLimit);
		FLOGV("  - velocity= %f", RelativeDockToDockLinearVelocity.Size());*/
		// Too fast
		InFinalApproach = false;
	}
	else if (FVector::VectorPlaneProject(RelativeDockToDockLinearVelocity, StationDockAxis).Size()  > FinalApproachLateralVelocityLimit)
	{
		//FLOG("InFinalApproach ? Too much lateral velocity");
		// Too much lateral velocity
		InFinalApproach = false;
	}
	else if (RelativeDockAngularVelocity.Size() > FinalApproachAngularVelocityLimit)
	{
		//FLOG("InFinalApproach ? Too much angular velocity");
		// Too much angular velocity
		InFinalApproach = false;
	}

	if (InFinalApproach)
	{
		/*FLOG("-> In final approach");*/
		MaxVelocity = DockingVelocityLimit / 200;
	}
	else
	{
		bool InApproach = true;
		// Not in final approch, check if in approch
		if (DockToDockDistance > ApproachDockToDockDistanceLimit)
		{
			//FLOG("InApproch ? Too far");
			// Too far
			InApproach = false;
		}
		else if (FVector::VectorPlaneProject(DockToDockDeltaLocation, StationDockAxis).Size() > GetApproachDockToDockLateralDistanceLimit(DockToDockDistance))
		{
			/*FLOG("InApproch ? Too far in lateral axis");
			FLOGV("  - limit= %f", GetApproachDockToDockLateralDistanceLimit(DockToDockDistance));
			FLOGV("  - distance= %f", FVector::VectorPlaneProject(DockToDockDeltaLocation, StationDockAxis).Size());*/

			// Too far in lateral axis
			InApproach = false;
		}
		else if (RelativeDockToDockLinearVelocity.Size() > GetApproachVelocityLimit(DockToDockDistance))
		{
			/*FLOG("InApproch ? Too fast");
			FLOGV("  - limit= %f",  GetApproachVelocityLimit(DockToDockDistance));
			FLOGV("  - velocity= %f", RelativeDockToDockLinearVelocity.Size());*/

			// Too fast
			InApproach = false;
		}

		if (InApproach)
		{
			/*FLOG("-> In approach");*/
			MaxVelocity = GetApproachVelocityLimit(DockToDockDistance) / 200 ;
			LocationTarget += StationDockAxis * (FinalApproachDockToDockDistanceLimit / 2);
			//FLOGV("Location offset=%s", *((StationDockAxis * (FinalApproachDockToDockDistanceLimit / 2)).ToString()));
		}
		else
		{
			/*FLOG("-> Rendez-vous");*/
			MaxVelocity = LinearMaxVelocity;
			LocationTarget += StationDockAxis * (ApproachDockToDockDistanceLimit / 2);
			if (DockToDockDistance > ApproachDockToDockDistanceLimit)
			{
				AxisTarget = LocationTarget - ShipDockLocation;
				AngularVelocityTarget = FVector::ZeroVector;
			}

			//FLOGV("Anticollision test ignore FVector::DotProduct(DockToDockDeltaLocation.GetUnsafeNormal(), StationDockAxis)=%f", FVector::DotProduct(DockToDockDeltaLocation.GetUnsafeNormal(), StationDockAxis));

			// During rendez-vous avoid the station if not in axis
			if (FVector::DotProduct(DockToDockDeltaLocation.GetUnsafeNormal(), StationDockAxis) > -0.9)
			{
				AnticollisionDockStation = NULL;
			}
			Anticollision = true;

			//FLOGV("Location offset=%s", *((StationDockAxis * (ApproachDockToDockDistanceLimit / 2)).ToString()));
		}
	}

	/*FLOGV("MaxVelocity=%f", MaxVelocity);
	FLOGV("LocationTarget=%s", *LocationTarget.ToString());
	FLOGV("AxisTarget=%s", *AxisTarget.ToString());
	FLOGV("AngularVelocityTarget=%s", *AngularVelocityTarget.ToString());
	FLOGV("VelocityTarget=%s", *VelocityTarget.ToString());



	DrawDebugSphere(Spacecraft->GetWorld(), LocationTarget, 100, 12, FColor::Green, false,0.03);
*/
	// UpdateLinearAttitudeAuto(DeltaSeconds, (CurrentCommand.PreciseApproach ? LinearMaxDockingVelocity : LinearMaxVelocity));
	// UpdateAngularAttitudeAuto(DeltaSeconds);

	// Not in approach, just go to the docking entrance point
	UpdateLinearAttitudeAuto(DeltaSeconds, LocationTarget, VelocityTarget/100, MaxVelocity, 0.25);
	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), AxisTarget, AngularVelocityTarget, DeltaSeconds);

	if (Anticollision)
	{
		//FLOGV("Docking Anticollision ignore=%p", AnticollisionDockStation);

		// During docking, lets the others avoid me
		LinearTargetVelocity = PilotHelper::AnticollisionCorrection(Spacecraft, LinearTargetVelocity, AnticollisionDockStation);
	}

	/*FLOGV("AngularTargetVelocity=%s", *AngularTargetVelocity.ToString());
	FLOGV("LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
*/
	// TODO refactor to get the position in parameter
}


void UFlareSpacecraftNavigationSystem::ConfirmDock(AFlareSpacecraft* DockStation, int32 DockId)
{
	FLOGV("UFlareSpacecraftNavigationSystem::ConfirmDock : '%s' is now docked", *Spacecraft->GetParent()->GetImmatriculation().ToString());
	ClearCurrentCommand();

	// Set as docked
	DockStation->GetDockingSystem()->Dock(Spacecraft, DockId);
	SetStatus(EFlareShipStatus::SS_Docked);
	Data->DockedTo = DockStation->GetImmatriculation();
	Data->DockedAt = DockId;

	if(DockConstraint)
	{
		DockConstraint->BreakConstraint();
		DockConstraint->DestroyComponent();
		DockConstraint = NULL;
	}

	// Attach to station
	FConstraintInstance ConstraintInstance;
	ConstraintInstance.bDisableCollision = true;
	ConstraintInstance.AngularSwing1Motion = ACM_Locked;
	ConstraintInstance.AngularSwing2Motion = ACM_Locked;
	ConstraintInstance.AngularTwistMotion = ACM_Locked;
	ConstraintInstance.LinearXMotion = LCM_Locked;
	ConstraintInstance.LinearYMotion = LCM_Locked;
	ConstraintInstance.LinearZMotion = LCM_Locked;
	ConstraintInstance.AngularRotationOffset = FRotator::ZeroRotator;
	ConstraintInstance.bSwingLimitSoft = 0;
	ConstraintInstance.bTwistLimitSoft = 0;
	ConstraintInstance.bLinearLimitSoft = 0;
	ConstraintInstance.bLinearBreakable = 0;
	ConstraintInstance.bAngularBreakable = 0;


	DockConstraint = NewObject<UPhysicsConstraintComponent>(Spacecraft->Airframe);
	DockConstraint->ConstraintInstance = ConstraintInstance;
	DockConstraint->SetWorldLocation(Spacecraft->GetActorLocation());
	DockConstraint->AttachToComponent(Spacecraft->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, false), NAME_None);

	DockConstraint->SetConstrainedComponents(Spacecraft->Airframe, NAME_None, DockStation->Airframe,NAME_None);

	// Cut engines
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		Engine->SetAlpha(0.0f);
	}


	Spacecraft->OnDocked(DockStation);
}


/*----------------------------------------------------
	Navigation commands and helpers
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::PushCommandLinearBrake(const FVector& VelocityTarget)
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_BrakeLocation;
	Command.VelocityTarget = VelocityTarget;
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommandAngularBrake()
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_BrakeRotation;
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommandLocation(const FVector& Location, bool Precise)
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Location;
	Command.LocationTarget = Location;
	Command.PreciseApproach = Precise;
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommandRotation(const FVector& RotationTarget, const FVector& LocalShipAxis)
{
	if (RotationTarget.IsNearlyZero() || LocalShipAxis.IsNearlyZero())
	{
		return;
	}

	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Rotation;
	Command.RotationTarget = RotationTarget;
	Command.LocalShipAxis = LocalShipAxis;
	FLOGV("UFlareSpacecraftNavigationSystem::PushCommandRotation RotationTarget '%s'", *RotationTarget.ToString());
	FLOGV("UFlareSpacecraftNavigationSystem::PushCommandRotation LocalShipAxis '%s'", *LocalShipAxis.ToString());
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommandDock(const FFlareDockingInfo& DockingInfo)
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Dock;
	Command.ActionTarget = DockingInfo.Station;
	Command.ActionTargetParam = DockingInfo.DockId;
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommand(const FFlareShipCommandData& Command)
{
	SetStatus(EFlareShipStatus::SS_AutoPilot);
	CommandData.Enqueue(Command);

	FLOGV("UFlareSpacecraftNavigationSystem::PushCommand : '%s' has new command '%s'",
		*Spacecraft->GetParent()->GetImmatriculation().ToString(),
		*EFlareCommandDataType::ToString(Command.Type));
}

void UFlareSpacecraftNavigationSystem::ClearCurrentCommand()
{
	FFlareShipCommandData Command;
	CommandData.Dequeue(Command);

	FLOGV("UFlareSpacecraftNavigationSystem::ClearCurrentCommand : '%s' has cleared command '%s'",
		*Spacecraft->GetParent()->GetImmatriculation().ToString(),
		*EFlareCommandDataType::ToString(Command.Type));

	if (!CommandData.Peek(Command))
	{
		SetStatus(EFlareShipStatus::SS_Manual);
	}
}

FFlareShipCommandData UFlareSpacecraftNavigationSystem::GetCurrentCommand()
{
	FFlareShipCommandData CurrentCommand;
	CurrentCommand.Type = EFlareCommandDataType::CDT_None;
	CommandData.Peek(CurrentCommand);
	return CurrentCommand;
}

void UFlareSpacecraftNavigationSystem::AbortAllCommands()
{
	FFlareShipCommandData Command;

	while (CommandData.Dequeue(Command))
	{
		FLOGV("UFlareSpacecraftNavigationSystem::AbortAllCommands : '%s' has aborted command '%s'",
			*Spacecraft->GetParent()->GetImmatriculation().ToString(),
			*EFlareCommandDataType::ToString(Command.Type));

		if (Command.Type == EFlareCommandDataType::CDT_Dock)
		{
			// Release dock grant
			AFlareSpacecraft* Station = Command.ActionTarget;
			Station->GetDockingSystem()->ReleaseDock(Spacecraft, Command.ActionTargetParam);
		}
	}
	SetStatus(EFlareShipStatus::SS_Manual);
}

FVector UFlareSpacecraftNavigationSystem::GetDockLocation()
{
	return Spacecraft->GetRootComponent()->GetSocketLocation(FName("Dock"));
}

FVector UFlareSpacecraftNavigationSystem::GetDockOffset()
{
	return Spacecraft->GetRootComponent()->GetComponentTransform().InverseTransformPosition(GetDockLocation());
}

bool UFlareSpacecraftNavigationSystem::IsPointColliding(FVector Candidate, AActor* Ignore)
{
	for (int32 i = 0; i < PathColliders.Num(); i++)
	{
		FVector ColliderLocation;
		FVector ColliderExtent;
		PathColliders[i]->GetActorBounds(true, ColliderLocation, ColliderExtent);

		if ((Candidate - ColliderLocation).Size() < ColliderExtent.Size() && PathColliders[i] != Ignore)
		{
			return true;
		}
	}

	return false;
}

AFlareSpacecraft* UFlareSpacecraftNavigationSystem::GetNearestShip(AFlareSpacecraft* DockingStation) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive or not
	// - From any company
	// - Is the nearest
	// - Is not me

	FVector PilotLocation = Spacecraft->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestShip = NULL;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Spacecraft->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Spacecraft->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

		if (ShipCandidate != Spacecraft && ShipCandidate != DockingStation)
		{

			if (DockingStation && (DockingStation->GetDockingSystem()->IsGrantedShip(ShipCandidate) || DockingStation->GetDockingSystem()->IsDockedShip(ShipCandidate)))
			{
				// Ignore ship docked or docking at the same station
				continue;
			}

			float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();

			if (NearestShip == NULL || DistanceSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistanceSquared;
				NearestShip = ShipCandidate;
			}
		}
	}
	return NearestShip;
}

/*----------------------------------------------------
	Attitude control : linear version
----------------------------------------------------*/


bool UFlareSpacecraftNavigationSystem::UpdateLinearAttitudeAuto(float DeltaSeconds, FVector TargetLocation, FVector TargetVelocity, float MaxVelocity, float SecurityRatio)
{
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector DeltaPosition = (TargetLocation - Spacecraft->GetActorLocation()) / 100; // Distance in meters
	FVector DeltaPositionDirection = DeltaPosition;
	DeltaPositionDirection.Normalize();
	float Distance = FMath::Max(0.0f, DeltaPosition.Size() - LinearDeadDistance);

	FVector DeltaVelocity = TargetVelocity - Spacecraft->GetLinearVelocity();
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else
	{

		FVector Acceleration = GetTotalMaxThrustInAxis(Engines, DeltaVelocityAxis, false) / Spacecraft->GetSpacecraftMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, DeltaPositionDirection));

		// TODO: Fix security ratio engine flickering
		TimeToFinalVelocity = (DeltaVelocity.Size() / (SecurityRatio * AccelerationInAngleAxis));
	}

	float DistanceToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	FVector RelativeResultSpeed;

	if (DistanceToStop > Distance)
	{
		RelativeResultSpeed = TargetVelocity;
	}
	else
	{

		float MaxPreciseSpeed = FMath::Min((Distance - DistanceToStop) / DeltaSeconds, MaxVelocity);

		if (DistanceToStop * 1.1 > Distance)
		{
			MaxPreciseSpeed = FMath::Min(MaxPreciseSpeed, Spacecraft->GetLinearVelocity().Size());

		}

		RelativeResultSpeed = DeltaPositionDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
		RelativeResultSpeed += TargetVelocity;
		/*FLOGV("DeltaPositionDirection %s", *DeltaPositionDirection.ToString());
		FLOGV("MaxPreciseSpeed %f", MaxPreciseSpeed);
		FLOGV("TargetVelocity %s", *TargetVelocity.ToString());*/
	}

	// Under this distance we consider the variation negligible, and ensure null delta + null speed
	if (Distance < LinearDeadDistance && DeltaVelocity.Size() < NegligibleSpeedRatio * MaxVelocity)
	{
		LinearTargetVelocity = TargetVelocity;
		return true;
	}
	//FLOGV("RelativeResultSpeed %s", *RelativeResultSpeed.ToString());
	LinearTargetVelocity = RelativeResultSpeed;
	return false;
}

void UFlareSpacecraftNavigationSystem::UpdateLinearBraking(float DeltaSeconds, FVector TargetVelocity)
{
	LinearTargetVelocity = TargetVelocity;
	FVector DeltaLinearVelocity = TargetVelocity*100 - Spacecraft->Airframe->GetPhysicsLinearVelocity();

	// Null speed detection
	if (DeltaLinearVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Spacecraft->Airframe->SetAllPhysicsLinearVelocity(TargetVelocity*100);
		ClearCurrentCommand();
	}
}


/*----------------------------------------------------
	Attitude control : angular version
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::UpdateAngularAttitudeAuto(float DeltaSeconds)
{
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	// Rotation data
	FFlareShipCommandData Command;
	CommandData.Peek(Command);
	FVector TargetAxis = Command.RotationTarget;
	FVector LocalShipAxis = Command.LocalShipAxis;

	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
	FVector WorldShipAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalShipAxis);

	WorldShipAxis.Normalize();
	TargetAxis.Normalize();

	FVector RotationDirection = FVector::CrossProduct(WorldShipAxis, TargetAxis);
	if (RotationDirection.IsNearlyZero())
	{
		RotationDirection=FVector(0,0,1);
	}

	RotationDirection.Normalize();
	float Dot = FVector::DotProduct(WorldShipAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	FVector DeltaVelocity = -AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
		FVector SimpleAcceleration = DeltaVelocityAxis * AngularAccelerationRate;
		// Scale with damages
		float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, true) / GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, false);
		FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;

		FVector Acceleration = DamagedSimpleAcceleration;
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(DamagedSimpleAcceleration, RotationDirection));

		TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	FVector RelativeResultSpeed;

	if (AngleToStop > angle)
	{
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else
	{

		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / DeltaSeconds, AngularMaxVelocity);

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	// Under this angle we consider the variation negligible, and ensure null delta + null speed
	if (angle < AngularDeadAngle && DeltaVelocity.Size() < AngularDeadAngle)
	{
		Spacecraft->Airframe->SetPhysicsAngularVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
		RelativeResultSpeed = FVector::ZeroVector;
	}
	AngularTargetVelocity = RelativeResultSpeed;
}


FVector UFlareSpacecraftNavigationSystem::GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const
{
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
	FVector WorldShipAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalShipAxis);

	WorldShipAxis.Normalize();
	TargetAxis.Normalize();

	FVector RotationDirection = FVector::CrossProduct(WorldShipAxis, TargetAxis);
	RotationDirection.Normalize();
	float Dot = FVector::DotProduct(WorldShipAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	FVector DeltaVelocity = TargetAngularVelocity - AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
		FVector SimpleAcceleration = DeltaVelocityAxis * GetAngularAccelerationRate();
		// Scale with damages
		float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, true) / GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, false);
		FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;

		FVector Acceleration = DamagedSimpleAcceleration;
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(DamagedSimpleAcceleration, RotationDirection));

		TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (FMath::Max(TimeToFinalVelocity,DeltaSeconds));

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = TargetAngularVelocity;
	}
	else
	{
		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / (DeltaSeconds * 0.75f), GetAngularMaxVelocity());

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
}

void UFlareSpacecraftNavigationSystem::UpdateAngularBraking(float DeltaSeconds)
{
	AngularTargetVelocity = FVector::ZeroVector;
	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
	// Null speed detection
	if (AngularVelocity.Size() < NegligibleSpeedRatio * AngularMaxVelocity)
	{
		AngularTargetVelocity = FVector::ZeroVector;
		Spacecraft->Airframe->SetPhysicsAngularVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
	}
}


/*----------------------------------------------------
	Physics
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::PhysicSubTick(float DeltaSeconds)
{
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
	if (Spacecraft->GetDamageSystem()->IsPowered())
	{
		// Linear physics
		FVector DeltaV = LinearTargetVelocity - Spacecraft->GetLinearVelocity();
		FVector DeltaVAxis = DeltaV;
		DeltaVAxis.Normalize();

		bool Log = false;
		if (Spacecraft->GetParent()->GetCompany() == Spacecraft->GetGame()->GetPC()->GetCompany())
		{
			//Log = true;
		}

		bool HasUsedOrbitalBoost = false;
		float LinearMasterAlpha = 0.f;
		float LinearMasterBoostAlpha = 0.f;

		if(Log) {
			FLOGV("PhysicSubTick DeltaSeconds=%f", DeltaSeconds);

			FLOGV("PhysicSubTick LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
			FLOGV("PhysicSubTick Spacecraft->GetLinearVelocity()=%s", *Spacecraft->GetLinearVelocity().ToString());

			FLOGV("PhysicSubTick DeltaV=%s", *DeltaV.ToString());
		}

		if (!DeltaV.IsNearlyZero())
		{
			// First, try without using the boost
			FVector Acceleration = DeltaVAxis * GetTotalMaxThrustInAxis(Engines, -DeltaVAxis, false).Size() / Spacecraft->GetSpacecraftMass();

			float AccelerationDeltaV = Acceleration.Size() * DeltaSeconds;

			LinearMasterAlpha = FMath::Clamp(DeltaV.Size()/ AccelerationDeltaV, 0.0f, 1.0f);
			// Second, if the not enought trust check with the boost
			if (UseOrbitalBoost && AccelerationDeltaV < DeltaV.Size() )
			{
				FVector AccelerationWithBoost = DeltaVAxis * GetTotalMaxThrustInAxis(Engines, -DeltaVAxis, true).Size() / Spacecraft->GetSpacecraftMass();
				if (AccelerationWithBoost.Size() > Acceleration.Size())
				{
					HasUsedOrbitalBoost = true;


					float BoostDeltaV = (AccelerationWithBoost.Size() - Acceleration.Size()) * DeltaSeconds;
					float DeltaVAfterClassicalAcceleration = DeltaV.Size() - AccelerationDeltaV;

					LinearMasterBoostAlpha = FMath::Clamp(DeltaVAfterClassicalAcceleration/ BoostDeltaV, 0.0f, 1.0f);

					Acceleration = AccelerationWithBoost;
				}
			}

			FVector ClampedAcceleration = Acceleration.GetClampedToMaxSize(DeltaV.Size() / DeltaSeconds);



			Spacecraft->Airframe->SetPhysicsLinearVelocity(ClampedAcceleration * DeltaSeconds * 100, true); // Multiply by 100 because UE4 works in cm
		}

		// Angular physics
		FVector DeltaAngularV = AngularTargetVelocity - Spacecraft->Airframe->GetPhysicsAngularVelocity();
		FVector DeltaAngularVAxis = DeltaAngularV;
		DeltaAngularVAxis.Normalize();

		if (!DeltaAngularV.IsNearlyZero())
		{
			FVector SimpleAcceleration = DeltaAngularVAxis * AngularAccelerationRate;

			// Scale with damages
			float TotalMaxTorqueInAxis = GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, false);
			if (!FMath::IsNearlyZero(TotalMaxTorqueInAxis))
			{
				float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, true) / TotalMaxTorqueInAxis;
				FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;
				FVector ClampedSimplifiedAcceleration = DamagedSimpleAcceleration.GetClampedToMaxSize(DeltaAngularV.Size() / DeltaSeconds);

				Spacecraft->Airframe->SetPhysicsAngularVelocity(ClampedSimplifiedAcceleration  * DeltaSeconds, true);
			}
		}

		// Update engine alpha
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			FVector ThrustAxis = Engine->GetThrustAxis();
			float LinearAlpha = 0;
			float AngularAlpha = 0;

			if (Spacecraft->IsPresentationMode())
			{
				LinearAlpha = true;
			}
			else if (!DeltaV.IsNearlyZero() || !DeltaAngularV.IsNearlyZero())
			{
				if(Engine->IsA(UFlareOrbitalEngine::StaticClass()))
				{
					if(HasUsedOrbitalBoost)
					{
						LinearAlpha = (-FVector::DotProduct(ThrustAxis, DeltaVAxis) + 0.2) * LinearMasterBoostAlpha;
					}
					AngularAlpha = 0;
				}
				else
				{
					LinearAlpha = -FVector::DotProduct(ThrustAxis, DeltaVAxis) * LinearMasterAlpha;
					FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
					FVector TorqueDirection = FVector::CrossProduct(EngineOffset, ThrustAxis);
					TorqueDirection.Normalize();

					if (!DeltaAngularV.IsNearlyZero() && !Engine->IsA(UFlareOrbitalEngine::StaticClass()))
					{
						AngularAlpha = -FVector::DotProduct(TorqueDirection, DeltaAngularVAxis);
					}
				}
			}

			Engine->SetAlpha(FMath::Clamp(LinearAlpha + AngularAlpha, 0.0f, 1.0f));
		}
	}
	else
	{
		// Shutdown engines
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			Engine->SetAlpha(0);
		}
	}
}

void UFlareSpacecraftNavigationSystem::UpdateCOM()
{
	COM = Spacecraft->Airframe->GetBodyInstance()->GetCOMPosition();
}


/*----------------------------------------------------
		Getters (Attitude)
----------------------------------------------------*/

FVector UFlareSpacecraftNavigationSystem::GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, bool WithOrbitalEngines) const
{
	Axis.Normalize();
	FVector TotalMaxThrust = FVector::ZeroVector;
	for (int32 i = 0; i < Engines.Num(); i++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		FVector WorldThrustAxis = Engine->GetThrustAxis();
		float Ratio = FVector::DotProduct(WorldThrustAxis, Axis);

		if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
		{
			if(WithOrbitalEngines && Ratio + 0.2 > 0)
			{
				TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * (Ratio + 0.2);
			}
		}
		else
		{
			if (Ratio > 0)
			{
				TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * Ratio;
			}
		}
	}

	return TotalMaxThrust;
}

float UFlareSpacecraftNavigationSystem::GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueAxis, bool WithDamages) const
{
	TorqueAxis.Normalize();
	float TotalMaxTorque = 0;

	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		// Ignore orbital engines for torque computation
		if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
		{
		  continue;
		}

		float MaxThrust = (WithDamages ? Engine->GetMaxThrust() : Engine->GetInitialMaxThrust());

		if (MaxThrust == 0)
		{
			// Not controlable engine
			continue;
		}

		FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;

		FVector WorldThrustAxis = Engine->GetThrustAxis();
		WorldThrustAxis.Normalize();
		FVector TorqueDirection = FVector::CrossProduct(EngineOffset, WorldThrustAxis);
		TorqueDirection.Normalize();

		float Ratio = FVector::DotProduct(TorqueAxis, TorqueDirection);

		if (Ratio > 0)
		{
			TotalMaxTorque += FVector::CrossProduct(EngineOffset, WorldThrustAxis).Size() * MaxThrust * Ratio;
		}

	}

	return TotalMaxTorque;
}




#undef LOCTEXT_NAMESPACE
