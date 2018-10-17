
#include "FlareSpacecraftNavigationSystem.h"
#include "../../Flare.h"

#include "../FlareSpacecraft.h"
#include "../FlareEngine.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlarePlayerController.h"
#include "../FlareOrbitalEngine.h"
#include "../FlarePilotHelper.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem Tick"), STAT_NavigationSystem_Tick, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem Manual"), STAT_NavigationSystem_Manual, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem Auto"), STAT_NavigationSystem_Auto, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem Physics"), STAT_NavigationSystem_Physics, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem CheckCollision"), STAT_NavigationSystem_CheckCollision, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem DockingAuto"), STAT_NavigationSystem_DockingAuto, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem GetNearestShip"), STAT_NavigationSystem_GetNearestShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem UpdateLinearAttitudeAuto"), STAT_NavigationSystem_UpdateLinearAttitudeAuto, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem UpdateAngularAttitudeAuto"), STAT_NavigationSystem_UpdateAngularAttitudeAuto, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem GetAngularVelocityToAlignAxis"), STAT_NavigationSystem_GetAngularVelocityToAlignAxis, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem GetTotalMaxThrustInAxis"), STAT_NavigationSystem_GetTotalMaxThrustInAxis, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareNavigationSystem GetTotalMaxTorqueInAxis"), STAT_NavigationSystem_GetTotalMaxTorqueInAxis, STATGROUP_Flare);

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
	, HasUsedOrbitalBoost(false)
{
	AnticollisionAngle = FMath::FRandRange(0, 360);
	DockConstraint = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::TickSystem(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_Tick);

	UpdateCOM();

	// Manual pilot
	if (IsManualPilot() && Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_Manual);

		LinearEngineTarget = Spacecraft->GetStateManager()->GetLinearEngineTarget();
		AngularTargetVelocity = Spacecraft->GetStateManager()->GetAngularTargetVelocity();
		UseOrbitalBoost = Spacecraft->GetStateManager()->IsUseOrbitalBoost();
	}

	// Autopilot
	else if (IsAutoPilot())
	{
		SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_Auto);

		FFlareShipCommandData CurrentCommand;
		if (CommandData.Peek(CurrentCommand))
		{
			bool KeepCommand = true;
			if (CurrentCommand.Type == EFlareCommandDataType::CDT_Location)
			{
				KeepCommand = UpdateLinearAttitudeAuto(DeltaSeconds, CurrentCommand.LocationTarget, FVector::ZeroVector, (CurrentCommand.PreciseApproach ? LinearMaxDockingVelocity : LinearMaxVelocity), 1.0);

				//TODO
				CurrentCommand.VelocityTarget = PilotHelper::AnticollisionCorrection(Spacecraft, CurrentCommand.VelocityTarget, Spacecraft->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), 0.f);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_BrakeLocation)
			{
				KeepCommand = UpdateLinearBraking(CurrentCommand, DeltaSeconds, CurrentCommand.VelocityTarget);

				FVector RotationTarget = -Spacecraft->Airframe->GetPhysicsLinearVelocity();

				if (RotationTarget.SizeSquared() > FMath::Square(10000))
				{
					CurrentCommand.RotationTarget = RotationTarget;
					CurrentCommand.LocalShipAxis = FVector(1, 0, 0);
					KeepCommand |= UpdateAngularAttitudeAuto(CurrentCommand, DeltaSeconds);
				}
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_Rotation)
			{
				KeepCommand = UpdateAngularAttitudeAuto(CurrentCommand, DeltaSeconds);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_BrakeRotation)
			{
				KeepCommand = UpdateAngularBraking(DeltaSeconds);
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_Dock)
			{
				DockingAutopilot(Cast<AFlareSpacecraft>(CurrentCommand.ActionTarget), CurrentCommand.ActionTargetParam, DeltaSeconds);
			}

			if(!KeepCommand)
			{
				ClearCurrentCommand();
			}
		}

		// TODO Autopilot anticollision system
	}

	// Physics
	HasUsedOrbitalBoost = false;
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

	// Initialize
	XEngines.Key.Empty();
	YEngines.Key.Empty();
	ZEngines.Key.Empty();
	XEngines.Value.Empty();
	YEngines.Value.Empty();
	ZEngines.Value.Empty();

	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);

		FVector LocalThrustAxis = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(Engine->GetThrustAxis());

		float XAlignement = FVector::DotProduct(LocalThrustAxis, FVector(1.f, 0.f, 0.f));
		float YAlignement = FVector::DotProduct(LocalThrustAxis, FVector(0.f, 1.f, 0.f));
		float ZAlignement = FVector::DotProduct(LocalThrustAxis, FVector(0.f, 0.f, 1.f));

		int MatchCount = 0;
		if(XAlignement > 0.8)
		{
			XEngines.Value.Add(EngineIndex);
			MatchCount++;
		}
		if(XAlignement < -0.8)
		{
			XEngines.Key.Add(EngineIndex);
			MatchCount++;
		}
		if(YAlignement > 0.8)
		{
			YEngines.Value.Add(EngineIndex);
			MatchCount++;
		}
		if(YAlignement < -0.8)
		{
			YEngines.Key.Add(EngineIndex);
			MatchCount++;
		}
		if(ZAlignement > 0.8)
		{
			ZEngines.Value.Add(EngineIndex);
			MatchCount++;
		}
		if(ZAlignement < -0.8)
		{
			ZEngines.Key.Add(EngineIndex);
			MatchCount++;
		}

		if(MatchCount != 1)
		{
			FLOGV("WARNING: engine #%d for %s as %d axis match", EngineIndex, *Spacecraft->GetImmatriculation().ToString(), MatchCount);
		}
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
		AbortAllCommands();

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
		if (Spacecraft->IsPlayerShip())
		{
			FVector UndockDestination = 1000 * FVector(-1, 0, 0);
			PushCommandLocation(Spacecraft->GetRootComponent()->GetComponentTransform().TransformPositionNoScale(UndockDestination));
		}
		else
		{
			FVector UndockDestination = 10000 * FVector(-1, 0, 0) + 5000 * FMath::VRand();
			PushCommandLocation(Spacecraft->GetRootComponent()->GetComponentTransform().TransformPositionNoScale(UndockDestination));
		}

		// Hack for bug #195: for ue4 to reweld all.
		Spacecraft->Airframe->SetSimulatePhysics(false);
		Spacecraft->Airframe->SetSimulatePhysics(true);

		// Successful
		FLOG("UFlareSpacecraftNavigationSystem::Undock : successful");
		Spacecraft->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("undock").PutName("target", Spacecraft->GetImmatriculation()));
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
	return (Distance / 2.5 + 460) * 100;
}

void UFlareSpacecraftNavigationSystem::CheckCollisionDocking(AFlareSpacecraft* DockingCandidate)
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_CheckCollision);

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

				int32 DockId = CurrentCommand.ActionTargetParam;

				FFlareDockingInfo StationDockInfo = DockStation->GetDockingSystem()->GetDockInfo(DockId);
				FFlareDockingParameters DockingParameters = GetDockingParameters(StationDockInfo, FVector::ZeroVector);
				if (DockingParameters.DockingPhase == EFlareDockingPhase::Dockable)
				{
					// All is ok for docking, perform dock
					ConfirmDock(DockStation, DockId);
					return;
				}

			}
		}
	}
}

static const float DockingDockToDockDistanceLimit = 20; // 20 cm of linear distance to dock
static const float FinalApproachDockToDockDistanceLimit = 100; // 1 m of linear distance
static const float ApproachDockToDockDistanceLimit = 20000; // 200 m approch distance

static const float FinalApproachDockToDockLateralDistanceLimit = 100; // 50 cm of linear lateral distance

static const float DockingAngleLimit = 2; // 2° of angle error to dock
static const float FinalApproachAngleLimit = 10;// 10° of angle error to dock
static const float ApproachAngleLimit = 90;

static const float DockingVelocityLimit = 200; // 2 m/s
static const float FinalApproachVelocityLimit = 500; // 5 m/s

static const float DockingLateralVelocityLimit = 20; // 10 cm/s
static const float FinalApproachLateralVelocityLimit = 50; // 0.5 m/s

static const float DockingAngularVelocityLimit = 10; // 5 °/s
static const float FinalApproachAngularVelocityLimit = 10; // 10 °/s

FText UFlareSpacecraftNavigationSystem::GetDockingPhaseName(EFlareDockingPhase::Type Phase)
{
	switch (Phase)
	{
	case EFlareDockingPhase::Docked:        return LOCTEXT("Docked", "Docked");
	case EFlareDockingPhase::Dockable:      return LOCTEXT("Dockable", "Ready to dock");
	case EFlareDockingPhase::FinalApproach: return LOCTEXT("FinalApproach", "On final approach");
	case EFlareDockingPhase::Approach:      return LOCTEXT("Approach", "On close approach");
	case EFlareDockingPhase::RendezVous:    return LOCTEXT("RendezVous", "On approach");
	case EFlareDockingPhase::Distant:       return LOCTEXT("Distant", "Distant");
	case EFlareDockingPhase::Locked:        return LOCTEXT("Locked", "Locked on");
	}

	return FText();
}

float UFlareSpacecraftNavigationSystem::GetDockingPhaseSpeed(EFlareDockingPhase::Type Phase)
{
	switch (Phase)
	{
	case EFlareDockingPhase::Docked:        return DockingVelocityLimit;
	case EFlareDockingPhase::Dockable:      return DockingVelocityLimit;
	case EFlareDockingPhase::FinalApproach: return FinalApproachVelocityLimit;
	case EFlareDockingPhase::Approach:      return 2000;  // 20m/s
	case EFlareDockingPhase::RendezVous:    return 10000; // 100 m/s
	case EFlareDockingPhase::Distant:       return 50000; // 500 m/s
	case EFlareDockingPhase::Locked:        return 50000; // 500 m/s
	}

	return 0;
}

FFlareDockingParameters UFlareSpacecraftNavigationSystem::GetDockingParameters(FFlareDockingInfo StationDockInfo, FVector CameraLocation)
{
	FFlareDockingParameters Params;

	AFlareSpacecraft* DockStation = StationDockInfo.Station;

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


	// Compute ship infos
	Params.ShipDockLocation = GetDockLocation();
	Params.ShipDockAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1, 0, 0)).GetUnsafeNormal(); // Ship docking port are always at front
	Params.ShipDockOffset = Params.ShipDockLocation - Spacecraft->GetActorLocation();

	// Compute station infos
	Params.StationDockLocation =  DockStation->Airframe->GetComponentTransform().TransformPosition(StationDockInfo.LocalLocation);
	Params.StationDockAxis = DockStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(StationDockInfo.LocalAxis).GetUnsafeNormal();
	Params.StationAngularVelocity = DockStation->Airframe->GetPhysicsAngularVelocityInDegrees();
	Params.StationDockTopAxis = DockStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(StationDockInfo.LocalTopAxis).GetUnsafeNormal();


	// Compute approch infos
	Params.DockToDockDeltaLocation = Params.StationDockLocation - Params.ShipDockLocation;
	Params.DockToDockDistance = Params.DockToDockDeltaLocation.Size();

	// Compute camera params
	FVector DockToCamera = CameraLocation - Params.ShipDockLocation;
	if (DockToCamera.IsNearlyZero())
	{
		Params.ShipHorizontalCameraOffset = 0.f;
		Params.ShipVerticalCameraOffset = 0.f;
		Params.ShipCameraTargetLocation = Params.StationDockLocation;
	}
	else
	{
		FVector LocalDockToCamera = FVector::VectorPlaneProject(DockToCamera, Params.ShipDockAxis);
		FVector StationLeftDockVector = FVector::CrossProduct(Params.StationDockTopAxis, Params.StationDockAxis);


		FVector ShipTopAxis = Spacecraft->GetActorRotation().RotateVector(FVector(0, 0, 1));
		FVector ShipLeftAxis = FVector::CrossProduct(Params.ShipDockAxis, ShipTopAxis);

		/*float DockToCameraDistance = DockToCamera.Size();
		FVector DockToCameraAxis = DockToCamera.GetUnsafeNormal();
		float Dot = FVector::DotProduct(Params.ShipDockAxis, DockToCameraAxis);
		float Angle = FMath::Acos(FMath::Abs(Dot));*/

		Params.ShipHorizontalCameraOffset = FVector::DotProduct(LocalDockToCamera, ShipLeftAxis);
		Params.ShipVerticalCameraOffset = FVector::DotProduct(LocalDockToCamera, ShipTopAxis);
		Params.ShipCameraTargetLocation = Params.StationDockLocation + Params.ShipHorizontalCameraOffset * StationLeftDockVector + Params.ShipVerticalCameraOffset *  Params.StationDockTopAxis;

		/*FLOG("-------------");
		FLOGV("DockToCamera %s", *DockToCamera.ToString());
		FLOGV("LocalDockToCamera %s", *LocalDockToCamera.ToString());
		FLOGV("Params.ShipDockAxis %s", *Params.ShipDockAxis.ToString());
		FLOGV("ShipTopAxis %s", *ShipTopAxis.ToString());
		FLOGV("ShipLeftAxis %s", *ShipLeftAxis.ToString());
		FLOGV("Params.StationDockTopAxis %s", *Params.StationDockTopAxis.ToString());
		FLOGV("StationLeftDockVector %s", *StationLeftDockVector.ToString());
		FLOGV("Params.ShipHorizontalCameraOffset %f", Params.ShipHorizontalCameraOffset);
		FLOGV("Params.ShipVerticalCameraOffset %f", Params.ShipVerticalCameraOffset);
		FLOGV("Params.ShipCameraTargetLocatio %s", *Params.ShipCameraTargetLocation.ToString());

		UKismetSystemLibrary::DrawDebugPoint(Spacecraft->GetWorld(), Params.ShipCameraTargetLocation, 10, FColor::White, 5.f);

		UKismetSystemLibrary::DrawDebugPoint(Spacecraft->GetWorld(), Params.StationDockLocation, 10, FColor::Magenta, 5.f);
		UKismetSystemLibrary::DrawDebugPoint(Spacecraft->GetWorld(), Params.StationDockLocation + Params.ShipDockAxis * 100, 10, FColor::Red, 5.f);
		UKismetSystemLibrary::DrawDebugPoint(Spacecraft->GetWorld(), Params.StationDockLocation + Params.StationDockTopAxis *100, 10, FColor::Green, 5.f);
		UKismetSystemLibrary::DrawDebugPoint(Spacecraft->GetWorld(), Params.StationDockLocation + StationLeftDockVector * 100, 10, FColor::Blue, 5.f);
*/
	}

	if(DockStation->GetDockingSystem()->IsDockedShip(Spacecraft))
	{
		Params.DockingPhase = EFlareDockingPhase::Docked;
		return Params;
	}


	if (StationDockInfo.Granted && StationDockInfo.Ship != Spacecraft)
	{
		Params.DockingPhase = EFlareDockingPhase::Locked;
		return Params;
	}

	float DockToDockAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-Params.ShipDockAxis, Params.StationDockAxis)));
	float ApproachAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-Params.DockToDockDeltaLocation.GetUnsafeNormal(), Params.StationDockAxis)));


	//FVector ShipDockAxis = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1, 0, 0)); // Ship docking port are always at front
	//FVector ShipDockLocation = GetDockLocation();

	FVector ShipAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();

	FVector StationCOM = DockStation->Airframe->GetBodyInstance()->GetCOMPosition();
	//FVector StationDockAxis = DockStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(StationDockInfo.LocalAxis);

	//FVector StationDockOffset = StationDockLocation - DockStation->GetActorLocation();


	// Compute docking infos
	//FVector DockToDockDeltaLocation = StationDockLocation - ShipDockLocation;
	//float DockToDockDistance = DockToDockDeltaLocation.Size();


	float InAxisDistance = FVector::DotProduct(Params.DockToDockDeltaLocation, -Params.StationDockAxis);
	FVector RotationInductedLinearVelocityAtShipDistance = (PI /  180.f) * FVector::CrossProduct(Params.StationAngularVelocity, (Params.StationDockLocation - StationCOM).GetUnsafeNormal() * (InAxisDistance + (Params.StationDockLocation - StationCOM).Size()));
	Params.LinearVelocityAtShipDistance = RotationInductedLinearVelocityAtShipDistance + DockStation->GetLinearVelocity() * 100;


	// Angular velocity must be the same
	FVector RelativeDockAngularVelocity = ShipAngularVelocity - Params.StationAngularVelocity;


	// The linear velocity of the docking port induced by the station or ship rotation
	Params.ShipDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(ShipAngularVelocity, Params.ShipDockLocation-COM);
	FVector ShipDockLinearVelocity = Params.ShipDockSelfRotationInductedLinearVelocity + Spacecraft->GetLinearVelocity() * 100;


	FVector StationDockSelfRotationInductedLinearVelocity = (PI /  180.f) * FVector::CrossProduct(Params.StationAngularVelocity, Params.StationDockLocation- StationCOM);
	FVector StationDockLinearVelocity = StationDockSelfRotationInductedLinearVelocity + DockStation->GetLinearVelocity() * 100;

	FVector RelativeDockToDockLinearVelocity = StationDockLinearVelocity - ShipDockLinearVelocity;



	// Check if dockable
	bool OkForDocking = true;
	if (Params.DockToDockDistance > DockingDockToDockDistanceLimit)
	{
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
	else if (FVector::VectorPlaneProject(RelativeDockToDockLinearVelocity, Params.StationDockAxis).Size()  > DockingLateralVelocityLimit)
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
		Params.DockingPhase = EFlareDockingPhase::Dockable;
		return Params;
	}

	bool InFinalApproach = true;
	// Not dockable, check if in final approch
	if (Params.DockToDockDistance > FinalApproachDockToDockDistanceLimit)
	{
		//FLOG("InFinalApproach ? Too far");
		// Too far
		InFinalApproach = false;
	}
	else if (FVector::VectorPlaneProject(Params.DockToDockDeltaLocation, Params.StationDockAxis).Size() > FinalApproachDockToDockLateralDistanceLimit)
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
		// Too fast
		InFinalApproach = false;
	}
	else if (FVector::VectorPlaneProject(RelativeDockToDockLinearVelocity, Params.StationDockAxis).Size()  > FinalApproachLateralVelocityLimit)
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
		Params.DockingPhase = EFlareDockingPhase::FinalApproach;
		return Params;
	}

	bool InApproach = true;
	// Not in final approch, check if in approch
	if (Params.DockToDockDistance > ApproachDockToDockDistanceLimit)
	{
		//FLOG("InApproch ? Too far");
		// Too far
		InApproach = false;
	}
	else if (FVector::VectorPlaneProject(Params.DockToDockDeltaLocation, Params.StationDockAxis).Size() > GetApproachDockToDockLateralDistanceLimit(Params.DockToDockDistance))
	{
		// Too far in lateral axis
		InApproach = false;
	}
	else if (ApproachAngle > ApproachAngleLimit)
	{
		//FLOG("InFinalApproach ? Not aligned");
		// Not aligned
		InApproach = false;
	}
	else if (RelativeDockToDockLinearVelocity.Size() > GetApproachVelocityLimit(Params.DockToDockDistance))
	{
		/*FLOG("InApproch ? Too fast");
		FLOGV("  - limit= %f",  GetApproachVelocityLimit(DockToDockDistance));
		FLOGV("  - velocity= %f", RelativeDockToDockLinearVelocity.Size());*/

		// Too fast
		InApproach = false;
	}

	if (InApproach)
	{
		Params.DockingPhase = EFlareDockingPhase::Approach;
		return Params;
	}

	if (ApproachAngle < ApproachAngleLimit && Params.DockToDockDistance < ApproachDockToDockDistanceLimit * 2)
	{
		//FLOG("InFinalApproach ? Not aligned");
		// Not aligned
		Params.DockingPhase = EFlareDockingPhase::RendezVous;
	}
	else
	{
		Params.DockingPhase = EFlareDockingPhase::Distant;
	}


	return Params;
}

void UFlareSpacecraftNavigationSystem::DockingAutopilot(AFlareSpacecraft* DockStation, int32 DockId, float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_DockingAuto);

	if (!DockStation)
	{
		// TODO LOG
		return;
	}
	//FLOG("==============DockingAutopilot==============");


	FFlareDockingInfo StationDockInfo = DockStation->GetDockingSystem()->GetDockInfo(DockId);
	FFlareDockingParameters DockingParameters = GetDockingParameters(StationDockInfo, FVector::ZeroVector);

	float MaxVelocity = 0;
	FVector LocationTarget = DockingParameters.StationDockLocation - DockingParameters.ShipDockOffset;
	FVector AngularVelocityTarget = DockingParameters.StationAngularVelocity;
	FVector AxisTarget = -DockingParameters.StationDockAxis;
	AFlareSpacecraft* AnticollisionDockStation = DockStation;
	bool Anticollision = false;
	FVector VelocityTarget = DockingParameters.LinearVelocityAtShipDistance - DockingParameters.ShipDockSelfRotationInductedLinearVelocity;

	switch (DockingParameters.DockingPhase)
	{
		case EFlareDockingPhase::RendezVous:
		case EFlareDockingPhase::Distant:
			// Rendez-vous
			MaxVelocity = LinearMaxVelocity * 100;
			LocationTarget += DockingParameters.StationDockAxis * (ApproachDockToDockDistanceLimit / 2);
			if (DockingParameters.DockToDockDistance > ApproachDockToDockDistanceLimit)
			{
				AxisTarget = LocationTarget - DockingParameters.ShipDockLocation;
				AngularVelocityTarget = FVector::ZeroVector;
			}

			//FLOGV("Anticollision test ignore FVector::DotProduct(DockToDockDeltaLocation.GetUnsafeNormal(), StationDockAxis)=%f", FVector::DotProduct(DockToDockDeltaLocation.GetUnsafeNormal(), StationDockAxis));

			// During rendez-vous avoid the station if not in axis
			if (FVector::DotProduct(DockingParameters.DockToDockDeltaLocation.GetUnsafeNormal(), DockingParameters.StationDockAxis) > -0.9)
			{
				AnticollisionDockStation = NULL;
			}
			Anticollision = true;
		break;
		case EFlareDockingPhase::Approach:
			MaxVelocity = GetApproachVelocityLimit(DockingParameters.DockToDockDistance) /*/ 200*/ ;
			LocationTarget += DockingParameters.StationDockAxis * (FinalApproachDockToDockDistanceLimit / 2);
		break;

		case EFlareDockingPhase::FinalApproach:
			MaxVelocity = DockingVelocityLimit / 200;
		break;
		case EFlareDockingPhase::Dockable:
			ConfirmDock(DockStation, DockId);
			return;
		break;
		case EFlareDockingPhase::Docked:
		case EFlareDockingPhase::Locked:
		default:
		// Do nothink
		LinearEngineTarget.SetVelocity(FVector::ZeroVector);
		AngularTargetVelocity = FVector::ZeroVector;
		break;
	}

	// Not in approach, just go to the docking entrance point
	UpdateLinearAttitudeAuto(DeltaSeconds, LocationTarget, VelocityTarget/100, MaxVelocity, 0.1);
	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), AxisTarget, AngularVelocityTarget, DeltaSeconds);

	if (Anticollision)
	{
		//FLOGV("Docking Anticollision ignore=%p", AnticollisionDockStation);

		// During docking, lets the others avoid me
		PilotHelper::AnticollisionConfig IgnoreConfig;
		IgnoreConfig.SpacecraftToIgnore = AnticollisionDockStation;

		FVector Global = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LinearEngineTarget.Target);
		FVector GlobalFixed = PilotHelper::AnticollisionCorrection(Spacecraft, Global, Spacecraft->GetPreferedAnticollisionTime(), IgnoreConfig, 0.f);

		LinearEngineTarget.SetVelocity(Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(GlobalFixed));
	}
}


void UFlareSpacecraftNavigationSystem::ConfirmDock(AFlareSpacecraft* DockStation, int32 DockId, bool TellUser)
{
	FLOGV("UFlareSpacecraftNavigationSystem::ConfirmDock : '%s' is now docked to %s", *Spacecraft->GetParent()->GetImmatriculation().ToString(), *DockStation->GetParent()->GetImmatriculation().ToString());
	ClearCurrentCommand();

	FFlareDockingInfo DockingPort = DockStation->GetDockingSystem()->GetDockInfo(DockId);
	FFlareDockingParameters DockingParameters = GetDockingParameters(DockingPort, FVector::ZeroVector);
	FVector ShipTopVector = Spacecraft->GetActorTransform().GetRotation().RotateVector(FVector(0,0,1));
	FVector StationTopVector =  DockingParameters.StationDockTopAxis;
	float Dot = FVector::DotProduct(ShipTopVector, StationTopVector);
	float AngleAbs = FMath::Acos(Dot);
	FVector CrossVector = FVector::CrossProduct(StationTopVector, ShipTopVector);
	float OutputAngle = FMath::RadiansToDegrees(AngleAbs * FMath::Sign( FVector::DotProduct(DockingParameters.StationDockAxis, CrossVector)));

	// Set as docked
	SetStatus(EFlareShipStatus::SS_Docked);
	Data->DockedTo = DockStation->GetImmatriculation();
	Data->DockedAt = DockId;
	Data->DockedAngle = OutputAngle;
	DockStation->GetDockingSystem()->Dock(Spacecraft, DockId);

	if(DockConstraint)
	{
		DockConstraint->BreakConstraint();
		DockConstraint->DestroyComponent();
		DockConstraint = NULL;
	}

	// Attach to station
	FConstraintInstance ConstraintInstance;
	ConstraintInstance.ProfileInstance.bDisableCollision = true;
	ConstraintInstance.ProfileInstance.ConeLimit.Swing1Motion = ACM_Locked;
	ConstraintInstance.ProfileInstance.ConeLimit.Swing2Motion = ACM_Locked;
	ConstraintInstance.ProfileInstance.ConeLimit.Swing1LimitDegrees = 0;
	ConstraintInstance.ProfileInstance.TwistLimit.TwistMotion = ACM_Locked;
	ConstraintInstance.ProfileInstance.TwistLimit.TwistLimitDegrees = 0;
	ConstraintInstance.ProfileInstance.LinearLimit.XMotion = LCM_Locked;
	ConstraintInstance.ProfileInstance.LinearLimit.YMotion = LCM_Locked;
	ConstraintInstance.ProfileInstance.LinearLimit.ZMotion = LCM_Locked;
	ConstraintInstance.ProfileInstance.LinearLimit.Limit = 0;
	ConstraintInstance.ProfileInstance.bLinearBreakable = 0;
	ConstraintInstance.ProfileInstance.bAngularBreakable = 0;
	ConstraintInstance.AngularRotationOffset = FRotator::ZeroRotator;

	DockConstraint = NewObject<UPhysicsConstraintComponent>(Spacecraft->Airframe);
	DockConstraint->ConstraintInstance = ConstraintInstance;
	DockConstraint->SetWorldLocation(Spacecraft->GetActorLocation());
	DockConstraint->AttachToComponent(Spacecraft->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, false), NAME_None);

	AFlareSpacecraft* AttachStation = DockStation;

	if (DockStation->IsComplexElement())
	{
		AttachStation = DockStation->GetComplex();
	}

	DockConstraint->SetConstrainedComponents(Spacecraft->Airframe, NAME_None, AttachStation->Airframe,NAME_None);

	// Cut engines
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		Engine->SetAlpha(0.0f);
	}


	Spacecraft->OnDocked(DockStation, TellUser);
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
	if(IsDocked())
	{
		Undock();
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
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_GetNearestShip);

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
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_UpdateLinearAttitudeAuto);

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
		LinearEngineTarget.SetVelocity(Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetVelocity));
		return false;
	}
	//FLOGV("RelativeResultSpeed %s", *RelativeResultSpeed.ToString());
	LinearEngineTarget.SetVelocity(Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(RelativeResultSpeed));
	return true;
}

bool UFlareSpacecraftNavigationSystem::UpdateLinearBraking(FFlareShipCommandData& Command, float DeltaSeconds, FVector TargetVelocity)
{
	LinearEngineTarget.SetVelocity(Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetVelocity));
	FVector DeltaLinearVelocity = TargetVelocity*100 - Spacecraft->Airframe->GetPhysicsLinearVelocity();

	bool KeepCommand = true;
	// Null speed detection
	if (DeltaLinearVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Spacecraft->Airframe->SetAllPhysicsLinearVelocity(TargetVelocity*100);
		KeepCommand = false;
	}
	return KeepCommand;
}


/*----------------------------------------------------
	Attitude control : angular version
----------------------------------------------------*/

bool UFlareSpacecraftNavigationSystem::UpdateAngularAttitudeAuto(FFlareShipCommandData& Command, float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_UpdateAngularAttitudeAuto);

	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	// Rotation data
	FVector TargetAxis = Command.RotationTarget;
	FVector LocalShipAxis = Command.LocalShipAxis;

	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
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

	bool KeepCommand = true;

	// Under this angle we consider the variation negligible, and ensure null delta + null speed
	if (angle < AngularDeadAngle && DeltaVelocity.Size() < AngularDeadAngle)
	{
		Spacecraft->Airframe->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false); // TODO remove
		KeepCommand = false;
		RelativeResultSpeed = FVector::ZeroVector;
	}
	AngularTargetVelocity = RelativeResultSpeed;

	return KeepCommand;
}


FVector UFlareSpacecraftNavigationSystem::GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_GetAngularVelocityToAlignAxis);

	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
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

bool UFlareSpacecraftNavigationSystem::UpdateAngularBraking(float DeltaSeconds)
{
	AngularTargetVelocity = FVector::ZeroVector;
	FVector AngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
	// Null speed detection

	bool KeepCommand = true;

	if (AngularVelocity.Size() < NegligibleSpeedRatio * AngularMaxVelocity)
	{
		AngularTargetVelocity = FVector::ZeroVector;
		Spacecraft->Airframe->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false); // TODO remove
		KeepCommand = false;
	}

	return KeepCommand;
}


/*----------------------------------------------------
	Physics
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::PhysicSubTick(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_Physics);

	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	if(Spacecraft->GetParent()->GetDamageSystem()->IsUncontrollable())
	{
		// Shutdown engines
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			Engine->SetAlpha(0);
		}

		return;
	}

	TArray<float> EnginesAlpha;

	for(int i = 0 ; i < Engines.Num() ; i++)
	{
		EnginesAlpha.Add(0.f);
	}

	bool Log = false;
	if (false && Spacecraft == Spacecraft->GetGame()->GetPC()->GetShipPawn())
	{
		Log = true;
	}



	FVector LocalLinearVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(Spacecraft->GetLinearVelocity());


	FVector LocalDeltaV = FVector::ZeroVector;


	if(Log) {
		FLOG("==========================");
		FLOGV("PhysicSubTick DeltaSeconds=%f", DeltaSeconds);

		FLOGV("PhysicSubTick Spacecraft->GetLinearVelocity()=%s", *Spacecraft->GetLinearVelocity().ToString());
		FLOGV("PhysicSubTick LocalLinearVelocity=%s", *LocalLinearVelocity.ToString());

		FLOGV("PhysicSubTick LinearEngineTarget.XVelocityControl=%d", LinearEngineTarget.XVelocityControl);
		FLOGV("PhysicSubTick LinearEngineTarget.YVelocityControl=%d", LinearEngineTarget.YVelocityControl);
		FLOGV("PhysicSubTick LinearEngineTarget.ZVelocityControl=%d", LinearEngineTarget.ZVelocityControl);
		FLOGV("PhysicSubTick LinearEngineTarget.Target=%s", *LinearEngineTarget.Target.ToString());
		FLOGV("PhysicSubTick UseOrbitalBoost=%d", UseOrbitalBoost);
	}



	float SharableBoostAcceleration = 0.f;

	auto ProcessVelocityEngineAxis = [&](float VelocityTargetInAxis, FVector Axis, TPair<TArray<int>, TArray<int>> AxisEngines)
	{
		float LocalLinearVelocityInAxis = FVector::DotProduct(Axis, LocalLinearVelocity);
		float DeltaV = VelocityTargetInAxis - LocalLinearVelocityInAxis;

		/*FLOGV("ProcessVelocityEngineAxis VelocityTargetInAxis=%f Axis=%s", VelocityTargetInAxis, *Axis.ToString());
		FLOGV("    - LocalLinearVelocityInAxis=%f", LocalLinearVelocityInAxis);
		FLOGV("    - DeltaV=%f", DeltaV);*/

		TArray<int>& UsefulEngines = DeltaV > 0 ? AxisEngines.Key: AxisEngines.Value;

		//FLOGV("    - UsefulEngines=%d", UsefulEngines.Num());

		float LinearMasterAlpha = 0.f;
		float LinearMasterBoostAlpha = 0.f;


		if (!FMath::IsNearlyZero(DeltaV))
		{
			// First, try without using the boost
			float Acceleration = SharableBoostAcceleration + GetTotalMaxThrustWithEngines(Engines, UsefulEngines, false) / Spacecraft->GetSpacecraftMass();

			//FLOGV("    - Acceleration=%f", Acceleration);


			float AccelerationDeltaV = Acceleration * DeltaSeconds;
			//FLOGV("    - AccelerationDeltaV=%f", AccelerationDeltaV);

			LinearMasterAlpha = FMath::Clamp(FMath::Abs(DeltaV)/ AccelerationDeltaV, 0.0f, 1.0f);
			// Second, if the not enought trust check with the boost
			if (UseOrbitalBoost && AccelerationDeltaV < FMath::Abs(DeltaV) )
			{
				float AccelerationWithBoost = GetTotalMaxThrustWithEngines(Engines, UsefulEngines, true) / Spacecraft->GetSpacecraftMass();

				if (AccelerationWithBoost > Acceleration)
				{
					HasUsedOrbitalBoost = true;

					float BoostDeltaV = (AccelerationWithBoost - Acceleration) * DeltaSeconds;
					float DeltaVAfterClassicalAcceleration = FMath::Abs(DeltaV) - AccelerationDeltaV;

					LinearMasterBoostAlpha = FMath::Clamp(DeltaVAfterClassicalAcceleration/ BoostDeltaV, 0.0f, 1.0f);

					SharableBoostAcceleration += (AccelerationWithBoost - Acceleration) * 1;

					Acceleration = AccelerationWithBoost;

					/*FLOGV("    - boost Acceleration=%f", Acceleration);
					FLOGV("    - boost AccelerationWithBoost=%f", AccelerationWithBoost);
					FLOGV("    - boost LinearMasterBoostAlpha=%f", LinearMasterBoostAlpha);*/
				}
			}

			// Apply alpha

			for(int EngineIndex : UsefulEngines)
			{
				UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);


				if (!Engine->IsA(UFlareOrbitalEngine::StaticClass()))
				{
					EnginesAlpha[EngineIndex] += LinearMasterAlpha;
				}
				else
				{
					if(HasUsedOrbitalBoost)
					{
						EnginesAlpha[EngineIndex] += LinearMasterBoostAlpha;
					}
				}

			}

			float ClampedAcceleration = FMath::Clamp(Acceleration, 0.f, FMath::Abs(DeltaV) / DeltaSeconds);



			FVector Increment = Axis * (DeltaV > 0 ? 1.f : -1.f) * ClampedAcceleration * DeltaSeconds * 100; // Multiply by 100 because UE4 works in cm

			/*FLOGV("    - Acceleration=%f", Acceleration);
			FLOGV("    - ClampedAcceleration=%f", ClampedAcceleration);
			FLOGV("    - LinearMasterAlpha=%f", LinearMasterAlpha);
			FLOGV("    - Increment=%s", *Increment.ToString());*/


			LocalDeltaV += Increment;
		}
	};

	auto ProcessAccelerationEngineAxis = [&](float AccelerationTargetInAxis, FVector Axis, TPair<TArray<int>, TArray<int>> AxisEngines)
	{
		float ClampedAccelerationTargetInAxis = FMath::Clamp(AccelerationTargetInAxis, -1.f, 1.f);

		TArray<int>& UsefulEngines = ClampedAccelerationTargetInAxis > 0 ? AxisEngines.Key: AxisEngines.Value;

		if (!FMath::IsNearlyZero(ClampedAccelerationTargetInAxis))
		{
			// First, try without using the boost
			// TODO
			float MaxAcceleration = SharableBoostAcceleration + GetTotalMaxThrustWithEngines(Engines, UsefulEngines, true) / Spacecraft->GetSpacecraftMass();

			if(Axis.X == 1.f)
			{
				SharableBoostAcceleration += MaxAcceleration * 1;
			}

			//FLOGV("ProcessAccelerationEngineAxis SharableBoostAcceleration=%f MaxAcceleration=%f", SharableBoostAcceleration, MaxAcceleration);


			// Apply alpha

			for(int EngineIndex : UsefulEngines)
			{
				EnginesAlpha[EngineIndex] += FMath::Abs(ClampedAccelerationTargetInAxis);
			}

			LocalDeltaV += Axis * ClampedAccelerationTargetInAxis * MaxAcceleration * DeltaSeconds * 100; // Multiply by 100 because UE4 works in cm
		}
	};


	if(LinearEngineTarget.XVelocityControl)
	{
		ProcessVelocityEngineAxis(LinearEngineTarget.Target.X, FVector(1, 0, 0), XEngines);
	}
	else
	{
		ProcessAccelerationEngineAxis(LinearEngineTarget.Target.X, FVector(1, 0, 0), XEngines);
	}

	if(LinearEngineTarget.YVelocityControl)
	{
		ProcessVelocityEngineAxis(LinearEngineTarget.Target.Y, FVector(0, 1, 0), YEngines);
	}
	else
	{
		ProcessAccelerationEngineAxis(LinearEngineTarget.Target.Y, FVector(0, 1, 0), YEngines);
	}

	if(LinearEngineTarget.ZVelocityControl)
	{
		ProcessVelocityEngineAxis(LinearEngineTarget.Target.Z, FVector(0, 0, 1), ZEngines);
	}
	else
	{
		ProcessAccelerationEngineAxis(LinearEngineTarget.Target.Z, FVector(0, 0, 1), ZEngines);
	}

	FVector GlobalDeltaV = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalDeltaV);
	Spacecraft->Airframe->SetPhysicsLinearVelocity(GlobalDeltaV, true);

	if(Log) {

		FLOGV("PhysicSubTick LocalDeltaV=%s", *LocalDeltaV.ToString());
		FLOGV("PhysicSubTick GlobalDeltaV=%s", *GlobalDeltaV.ToString());
		FLOGV("PhysicSubTick SharableBoostAcceleration=%f", SharableBoostAcceleration);
	}

	// Angular physics
	FVector DeltaAngularV = AngularTargetVelocity - Spacecraft->Airframe->GetPhysicsAngularVelocityInDegrees();
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

			Spacecraft->Airframe->SetPhysicsAngularVelocityInDegrees(ClampedSimplifiedAcceleration  * DeltaSeconds, true);
		}
	}

	// Update engine alpha
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		FVector ThrustAxis = Engine->GetThrustAxis();
		float LinearAlpha = EnginesAlpha[EngineIndex];
		float AngularAlpha = 0;

		if (Spacecraft->IsPresentationMode())
		{
			LinearAlpha = true;
		}
		else if (!DeltaAngularV.IsNearlyZero() && !Engine->IsA(UFlareOrbitalEngine::StaticClass()))
		{
				FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
				FVector TorqueDirection = FVector::CrossProduct(EngineOffset, ThrustAxis);
				TorqueDirection.Normalize();

				AngularAlpha = -FVector::DotProduct(TorqueDirection, DeltaAngularVAxis);
		}

		/*if(Log) {
			FLOGV("PhysicSubTick Engine %d alpha=%f + %f", EngineIndex , LinearAlpha, AngularAlpha);
		}*/


		Engine->SetAlpha(FMath::Clamp(LinearAlpha + AngularAlpha, 0.0f, 1.0f));
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
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_GetTotalMaxThrustInAxis);

	Axis.Normalize();
	FVector TotalMaxThrust = FVector::ZeroVector;
	for (int32 i = 0; i < Engines.Num(); i++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		FVector WorldThrustAxis = Engine->GetThrustAxis();
		float Ratio = FVector::DotProduct(WorldThrustAxis, Axis);

		/*if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
		{
			if(WithOrbitalEngines && Ratio + 0.2 > 0)
			{
				TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * (Ratio + 0.2);
			}
		}
		else
		{*/
			if (Ratio > 0)
			{
				TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * Ratio;
			}
		/*}*/
	}

	return TotalMaxThrust;
}

float UFlareSpacecraftNavigationSystem::GetTotalMaxThrustWithEngines(TArray<UActorComponent*>& Engines, TArray<int>& UsefulEngines, bool WithOrbitalEngines)
{
	float TotalMaxThrust = 0.f;
	for (int i : UsefulEngines)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
		{
			if(WithOrbitalEngines)
			{
				TotalMaxThrust += Engine->GetMaxThrust();
			}
		}
		else
		{
			TotalMaxThrust += Engine->GetMaxThrust();
		}
	}

	return TotalMaxThrust;
}


float UFlareSpacecraftNavigationSystem::GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueAxis, bool WithDamages) const
{
	SCOPE_CYCLE_COUNTER(STAT_NavigationSystem_GetTotalMaxTorqueInAxis);

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
