
#include "../Flare.h"

#include "FlareSpacecraftNavigationSystem.h"
#include "FlareSpacecraft.h"

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
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::TickSystem(float DeltaSeconds)
{
	UpdateCOM();

	// Manual pilot
	if (IsManualPilot() && Spacecraft->GetDamageSystem()->IsAlive())
	{
		if (Spacecraft->IsPilotMode())
		{
			LinearTargetVelocity = Spacecraft->GetPilot()->GetLinearTargetVelocity();
			AngularTargetVelocity = Spacecraft->GetPilot()->GetAngularTargetVelocity();
			UseOrbitalBoost = Spacecraft->GetPilot()->IsUseOrbitalBoost();
			if (Spacecraft->GetPilot()->IsWantFire())
			{
				// TODO in weapons system
				Spacecraft->StartFire();
			}
			else
			{
				// TODO in weapons system
				Spacecraft->StopFire();
			}
		}
		else
		{
			UpdateLinearAttitudeManual(DeltaSeconds);
			UpdateAngularAttitudeManual(DeltaSeconds);
			UseOrbitalBoost = Spacecraft->PlayerManualOrbitalBoost;
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
				UpdateLinearAttitudeAuto(DeltaSeconds, (CurrentCommand.PreciseApproach ? LinearMaxDockingVelocity : LinearMaxVelocity));
			}
			else if (CurrentCommand.Type == EFlareCommandDataType::CDT_BrakeLocation)
			{
				UpdateLinearBraking(DeltaSeconds);
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
				ConfirmDock(Cast<IFlareSpacecraftInterface>(CurrentCommand.ActionTarget), CurrentCommand.ActionTargetParam);
			}
		}
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
	Description = Spacecraft->GetDescription();
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
	FLOGV("AFlareSpacecraft::SetStatus %d", NewStatus - EFlareShipStatus::SS_Manual);
	Status = NewStatus;
}

void UFlareSpacecraftNavigationSystem::SetAngularAccelerationRate(float Acceleration)
{
	AngularAccelerationRate = Acceleration;
}

/*----------------------------------------------------
	Docking
----------------------------------------------------*/

bool UFlareSpacecraftNavigationSystem::DockAt(IFlareSpacecraftInterface* TargetStation)
{
	FLOG("AFlareSpacecraft::DockAt");
	FFlareDockingInfo DockingInfo = TargetStation->GetDockingSystem()->RequestDock(Spacecraft);

	// Try to dock
	if (DockingInfo.Granted)
	{
		FLOG("AFlareSpacecraft::DockAt : access granted");
		FVector ShipDockOffset = GetDockLocation();
		DockingInfo.EndPoint += DockingInfo.Rotation.RotateVector(ShipDockOffset * FVector(1, 0, 0)) - ShipDockOffset * FVector(0, 1, 1);
		DockingInfo.StartPoint = DockingInfo.EndPoint + 5000 * DockingInfo.Rotation.RotateVector(FVector(1, 0, 0));

		// Dock
		if (NavigateTo(DockingInfo.StartPoint))
		{
			// Align front to dock axis, ship top to station top, set speed
			PushCommandRotation((DockingInfo.EndPoint - DockingInfo.StartPoint), FVector(1, 0, 0));
			PushCommandRotation(FVector(0,0,1), FVector(0,0,1));

			// Move there
			PushCommandLocation(DockingInfo.EndPoint, true);
			PushCommandDock(DockingInfo);
			FLOG("AFlareSpacecraft::DockAt : navigation sent");
			return true;
		}
	}

	// Failed
	FLOG("AFlareSpacecraft::DockAt failed");
	return false;
}

bool UFlareSpacecraftNavigationSystem::Undock()
{
	FLOG("AFlareSpacecraft::Undock");
	FFlareShipCommandData Head;

	// Try undocking
	if (IsDocked())
	{
		// Enable physics
		Spacecraft->Airframe->SetSimulatePhysics(true);

		// Evacuate
		GetDockStation()->GetDockingSystem()->ReleaseDock(Spacecraft, Data->DockedAt);
		PushCommandLocation(Spacecraft->GetRootComponent()->GetComponentTransform().TransformPositionNoScale(5000 * FVector(-1, 0, 0)));

		// Update data
		SetStatus(EFlareShipStatus::SS_AutoPilot);
		Data->DockedTo = NAME_None;
		Data->DockedAt = -1;

		FLOG("AFlareSpacecraft::Undock successful");
		return true;
	}

	// Failed
	FLOG("AFlareSpacecraft::Undock failed");
	return false;
}

IFlareSpacecraftInterface* UFlareSpacecraftNavigationSystem::GetDockStation()
{
	if (IsDocked())
	{
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* Station = Cast<AFlareSpacecraft>(*ActorItr);
			if (Station && *Station->GetName() == Data->DockedTo)
			{
				return Station;
			}
		}
	}
	return NULL;
}

void UFlareSpacecraftNavigationSystem::ConfirmDock(IFlareSpacecraftInterface* DockStation, int32 DockId)
{
	FLOG("AFlareSpacecraft::ConfirmDock");
	ClearCurrentCommand();

	// Set as docked
	DockStation->GetDockingSystem()->Dock(Spacecraft, DockId);
	SetStatus(EFlareShipStatus::SS_Docked);
	Data->DockedTo = *DockStation->_getUObject()->GetName();
	Data->DockedAt = DockId;

	// Disable physics, reset speed
	LinearMaxVelocity = Description->LinearMaxVelocity;
	Spacecraft->Airframe->SetSimulatePhysics(false);

	// Cut engines
	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		Engine->SetAlpha(0.0f);
	}

	Spacecraft->OnDocked();
}

/*----------------------------------------------------
	Navigation commands and helpers
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::PushCommandLinearBrake()
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Location;
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
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Rotation;
	Command.RotationTarget = RotationTarget;
	Command.LocalShipAxis = LocalShipAxis;
	FLOGV("PushCommandRotation RotationTarget '%s'", *RotationTarget.ToString());
	FLOGV("PushCommandRotation LocalShipAxis '%s'", *LocalShipAxis.ToString());
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommandDock(const FFlareDockingInfo& DockingInfo)
{
	FFlareShipCommandData Command;
	Command.Type = EFlareCommandDataType::CDT_Dock;
	Command.ActionTarget = Cast<AFlareSpacecraft>(DockingInfo.Station);
	Command.ActionTargetParam = DockingInfo.DockId;
	PushCommand(Command);
}

void UFlareSpacecraftNavigationSystem::PushCommand(const FFlareShipCommandData& Command)
{
	SetStatus(EFlareShipStatus::SS_AutoPilot);
	CommandData.Enqueue(Command);

	FLOGV("Pushed command '%s'", *EFlareCommandDataType::ToString(Command.Type));
}

void UFlareSpacecraftNavigationSystem::ClearCurrentCommand()
{
	FFlareShipCommandData Command;
	CommandData.Dequeue(Command);

	FLOGV("Cleared command '%s'", *EFlareCommandDataType::ToString(Command.Type));

	if (!CommandData.Peek(Command))
	{
		SetStatus(EFlareShipStatus::SS_Manual);
	}
}

void UFlareSpacecraftNavigationSystem::AbortAllCommands()
{
	FFlareShipCommandData Command;

	while (CommandData.Dequeue(Command))
	{
		FLOGV("Abort command '%s'", *EFlareCommandDataType::ToString(Command.Type));
		if (Command.Type == EFlareCommandDataType::CDT_Dock)
		{
			// Release dock grant
			IFlareSpacecraftInterface* Station = Cast<IFlareSpacecraftInterface>(Command.ActionTarget);
			Station->GetDockingSystem()->ReleaseDock(Spacecraft, Command.ActionTargetParam);
		}
	}
	SetStatus(EFlareShipStatus::SS_Manual);
}

FVector UFlareSpacecraftNavigationSystem::GetDockLocation()
{
	FVector WorldLocation = Spacecraft->GetRootComponent()->GetSocketLocation(FName("Dock"));
	return Spacecraft->GetRootComponent()->GetComponentTransform().InverseTransformPosition(WorldLocation);
}

bool UFlareSpacecraftNavigationSystem::ComputePath(TArray<FVector>& Path, TArray<AActor*>& PossibleColliders, FVector OriginLocation, FVector TargetLocation, float ShipSize)
{
	// Travel information
	float TravelLength;
	FVector TravelDirection;
	FVector Travel = TargetLocation - OriginLocation;
	Travel.ToDirectionAndLength(TravelDirection, TravelLength);

	for (int32 i = 0; i < PossibleColliders.Num(); i++)
	{
		// Get collider info
		FVector ColliderLocation;
		FVector ColliderExtent;
		PossibleColliders[i]->GetActorBounds(true, ColliderLocation, ColliderExtent);
		float ColliderSize = ShipSize + ColliderExtent.Size();

		// Colliding : split the travel
		if (FMath::LineSphereIntersection(OriginLocation, TravelDirection, TravelLength, ColliderLocation, ColliderSize))
		{
			//DrawDebugSphere(GetWorld(), ColliderLocation, ColliderSize, 12, FColor::Blue, true);

			// Get an orthogonal plane
			FPlane TravelOrthoPlane = FPlane(ColliderLocation, TargetLocation - ColliderLocation);
			FVector IntersectedLocation = FMath::LinePlaneIntersection(OriginLocation, TargetLocation, TravelOrthoPlane);

			// Relocate intersection inside the sphere
			FVector Intersector = IntersectedLocation - ColliderLocation;
			Intersector.Normalize();
			IntersectedLocation = ColliderLocation + Intersector * ColliderSize;

			// Collisions
			bool IsColliding = IsPointColliding(IntersectedLocation, PossibleColliders[i]);
			//DrawDebugPoint(GetWorld(), IntersectedLocation, 8, IsColliding ? FColor::Red : FColor::Green, true);

			// Dead end, go back
			if (IsColliding)
			{
				return false;
			}

			// Split travel
			else
			{
				Path.Add(IntersectedLocation);
				PossibleColliders.RemoveAt(i, 1);
				bool FirstPartOK = ComputePath(Path, PossibleColliders, OriginLocation, IntersectedLocation, ShipSize);
				bool SecondPartOK = ComputePath(Path, PossibleColliders, IntersectedLocation, TargetLocation, ShipSize);
				return FirstPartOK && SecondPartOK;
			}

		}
	}

	// No collision found
	return true;
}

void UFlareSpacecraftNavigationSystem::UpdateColliders()
{
	PathColliders.Empty();
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		FVector Unused;
		FVector ColliderExtent;
		ActorItr->GetActorBounds(true, Unused, ColliderExtent);

		if (ColliderExtent.Size() < 100000 && ActorItr->IsRootComponentMovable())
		{
			PathColliders.Add(*ActorItr);
		}
	}
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

bool UFlareSpacecraftNavigationSystem::NavigateTo(FVector TargetLocation)
{
	// Pathfinding data
	TArray<FVector> Path;
	FVector Unused;
	FVector ShipExtent;
	FVector Temp = Spacecraft->GetActorLocation();

	// Prepare data
	FLOG("AFlareSpacecraft::NavigateTo");
	Spacecraft->GetActorBounds(true, Unused, ShipExtent);
	UpdateColliders();

	// Compute path
	if (ComputePath(Path, PathColliders, Temp, TargetLocation, ShipExtent.Size()))
	{
		FLOGV("AFlareSpacecraft::NavigateTo : generating path (%d stops)", Path.Num());

		// Generate commands for travel
		for (int32 i = 0; i < Path.Num(); i++)
		{
			PushCommandRotation((Path[i] - Temp), FVector(1,0,0)); // Front
			PushCommandLocation(Path[i]);
			Temp = Path[i];
		}

		// Move toward objective for pre-final approach
		PushCommandRotation((TargetLocation - Temp), FVector(1,0,0));
		PushCommandLocation(TargetLocation);
		return true;
	}

	// Failed
	FLOG("AFlareSpacecraft::NavigateTo failed : no path found");
	return false;
}

/*----------------------------------------------------
	Attitude control : linear version
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::UpdateLinearAttitudeManual(float DeltaSeconds)
{
	// Manual orbital boost
	if (Spacecraft->PlayerManualOrbitalBoost)
	{
		Spacecraft->PlayerManualLinearVelocity = GetLinearMaxBoostingVelocity() * FVector(1, 0, 0);
	}

	// Add velocity command to current velocity
	LinearTargetVelocity = Spacecraft->GetLinearVelocity() + Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(Spacecraft->PlayerManualLinearVelocity);
}

void UFlareSpacecraftNavigationSystem::UpdateLinearAttitudeAuto(float DeltaSeconds, float MaxVelocity)
{
	// Location data
	FFlareShipCommandData Command;
	CommandData.Peek(Command);

	TArray<UActorComponent*> Engines = Spacecraft->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector DeltaPosition = (Command.LocationTarget - Spacecraft->GetActorLocation()) / 100; // Distance in meters
	FVector DeltaPositionDirection = DeltaPosition;
	DeltaPositionDirection.Normalize();
	float Distance = FMath::Max(0.0f, DeltaPosition.Size() - LinearDeadDistance);

	FVector DeltaVelocity = -Spacecraft->GetLinearVelocity();
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else
	{

		FVector Acceleration = GetTotalMaxThrustInAxis(Engines, DeltaVelocityAxis, false) / Spacecraft->Airframe->GetMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, DeltaPositionDirection));

		TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float DistanceToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	FVector RelativeResultSpeed;

	if (DistanceToStop > Distance)
	{
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else
	{

		float MaxPreciseSpeed = FMath::Min((Distance - DistanceToStop) / DeltaSeconds, MaxVelocity);

		RelativeResultSpeed = DeltaPositionDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	// Under this distance we consider the variation negligible, and ensure null delta + null speed
	if (Distance < LinearDeadDistance && DeltaVelocity.Size() < NegligibleSpeedRatio * MaxVelocity)
	{
		Spacecraft->Airframe->SetPhysicsLinearVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
		RelativeResultSpeed = FVector::ZeroVector;

	}
	LinearTargetVelocity = RelativeResultSpeed;
}

void UFlareSpacecraftNavigationSystem::UpdateLinearBraking(float DeltaSeconds)
{
	LinearTargetVelocity = FVector::ZeroVector;
	FVector LinearVelocity = Spacecraft->WorldToLocal(Spacecraft->Airframe->GetPhysicsLinearVelocity());

	// Null speed detection
	if (LinearVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Spacecraft->Airframe->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		ClearCurrentCommand();
	}
}


/*----------------------------------------------------
	Attitude control : angular version
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::UpdateAngularAttitudeManual(float DeltaSeconds)
{
	AngularTargetVelocity = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(Spacecraft->PlayerManualAngularVelocity);
}

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

	if (AngleToStop > angle) {
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else {

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
		// Clamp speed
		float MaxVelocity = LinearMaxVelocity;
		if (UseOrbitalBoost)
		{
			FVector FrontDirection = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));
			MaxVelocity = FVector::DotProduct(LinearTargetVelocity.GetUnsafeNormal(), FrontDirection) * GetLinearMaxBoostingVelocity();
		}
		LinearTargetVelocity = LinearTargetVelocity.GetClampedToMaxSize(MaxVelocity);

		// Linear physics
		FVector DeltaV = LinearTargetVelocity - Spacecraft->GetLinearVelocity();
		FVector DeltaVAxis = DeltaV;
		DeltaVAxis.Normalize();

		if (!DeltaV.IsNearlyZero())
		{
			FVector Acceleration = DeltaVAxis * GetTotalMaxThrustInAxis(Engines, -DeltaVAxis, UseOrbitalBoost).Size() / Spacecraft->Airframe->GetMass();
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
			float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, true) / GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, false);
			FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;
			FVector ClampedSimplifiedAcceleration = DamagedSimpleAcceleration.GetClampedToMaxSize(DeltaAngularV.Size() / DeltaSeconds);

			Spacecraft->Airframe->SetPhysicsAngularVelocity(ClampedSimplifiedAcceleration  * DeltaSeconds, true);
		}

		// Update engine alpha
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			FVector ThrustAxis = Engine->GetThrustAxis();
			float LinearAlpha = 0;
			float AngularAlpha = 0;

			if (Spacecraft->IsPresentationMode()) {
				LinearAlpha = true;
			} else if (!DeltaV.IsNearlyZero()) {
				if (!(!UseOrbitalBoost && Engine->IsA(UFlareOrbitalEngine::StaticClass()))) {
					LinearAlpha = -FVector::DotProduct(ThrustAxis, DeltaVAxis);
				}
			}

			FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
			FVector TorqueDirection = FVector::CrossProduct(EngineOffset, ThrustAxis);
			TorqueDirection.Normalize();

			if (!DeltaAngularV.IsNearlyZero() && !Engine->IsA(UFlareOrbitalEngine::StaticClass())) {
				AngularAlpha = -FVector::DotProduct(TorqueDirection, DeltaAngularVAxis);
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

		if (Engine->IsA(UFlareOrbitalEngine::StaticClass()) && !WithOrbitalEngines)
		{
			continue;
		}

		FVector WorldThrustAxis = Engine->GetThrustAxis();

		float Ratio = FVector::DotProduct(WorldThrustAxis, Axis);
		if (Ratio > 0)
		{
			TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * Ratio;
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
		if (Engine->IsA(UFlareOrbitalEngine::StaticClass())) {
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

		float ratio = FVector::DotProduct(TorqueAxis, TorqueDirection);

		if (ratio > 0) {
			TotalMaxTorque += FVector::CrossProduct(EngineOffset, WorldThrustAxis).Size() * MaxThrust * ratio;
		}

	}

	return TotalMaxTorque;
}




#undef LOCTEXT_NAMESPACE
