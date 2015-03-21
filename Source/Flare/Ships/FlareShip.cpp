
#include "../Flare.h"

#include "FlareShip.h"
#include "FlareAirframe.h"
#include "FlareOrbitalEngine.h"
#include "FlareRCS.h"
#include "FlareWeapon.h"

#include "Particles/ParticleSystemComponent.h"

#include "../Stations/FlareStation.h"
#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareShip::AFlareShip(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, AngularDeadAngle(0.5)
	, AngularInputDeadRatio(0.025)
	, LinearDeadDistance(0.1)
	, LinearMaxDockingVelocity(10)
	, NegligibleSpeedRatio(0.0005)
	, Status(EFlareShipStatus::SS_Manual)
{	
	// Create static mesh component
	Airframe = PCIP.CreateDefaultSubobject<UFlareAirframe>(this, TEXT("Airframe"));
	Airframe->SetSimulatePhysics(true);
	RootComponent = Airframe;

	// Camera settings
	CameraContainerYaw->AttachTo(Airframe);
	CameraMaxPitch = 80;
	CameraPanSpeed = 2;

	// Dock info
	ShipData.DockedTo = NAME_None;
	ShipData.DockedAt = -1;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareShip::BeginPlay()
{
	Super::BeginPlay();
	TArray<UActorComponent*> ActorComponents;
	GetComponents(ActorComponents);
	
	UpdateCOM();
}

void AFlareShip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Attitude control
	if (Airframe && !IsPresentationMode())
	{
		UpdateCOM();
	  
		// Manual pilot
		if (IsManualPilot())
		{
			UpdateLinearAttitudeManual(DeltaSeconds);
			UpdateAngularAttitudeManual(DeltaSeconds);
		}

		// Autopilot
		else if (IsAutoPilot())
		{
			FFlareShipCommandData Temp;
			if (CommandData.Peek(Temp))
			{
				if (Temp.Type == EFlareCommandDataType::CDT_Location)
				{
					UpdateLinearAttitudeAuto(DeltaSeconds);
				}
				else if (Temp.Type == EFlareCommandDataType::CDT_BrakeLocation)
				{
					UpdateLinearBraking(DeltaSeconds);
				}
				else if (Temp.Type == EFlareCommandDataType::CDT_Rotation)
				{
					UpdateAngularAttitudeAuto(DeltaSeconds);
				}
				else if (Temp.Type == EFlareCommandDataType::CDT_BrakeRotation)
				{
					UpdateAngularBraking(DeltaSeconds);
				}
				else if (Temp.Type == EFlareCommandDataType::CDT_Dock)
				{
					ConfirmDock(Cast<IFlareStationInterface>(Temp.ActionTarget), Temp.ActionTargetParam);
				}
			}
		}

		// Physics
		if (!IsDocked())
		{
			// Tick components
			TArray<UActorComponent*> Components = GetComponentsByClass(UFlareShipComponent::StaticClass());
			for (int32 i = 0; i < Components.Num(); i++)
			{
				UFlareShipComponent* Component = Cast<UFlareShipComponent>(Components[i]);
				Component->ShipTickComponent(DeltaSeconds);
			}
			PhysicSubTick(DeltaSeconds);
		}
	}
}

void AFlareShip::ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void AFlareShip::Destroyed()
{
	if (Company)
	{
		Company->Unregister(this);
	}
}


/*----------------------------------------------------
	Player interface
----------------------------------------------------*/

void AFlareShip::SetExternalCamera(bool NewState)
{
	// Stop firing
	if (NewState)
	{
		StopFire();
	}

	// Reset rotations
	ExternalCamera = NewState;
	SetCameraPitch(0);
	SetCameraYaw(0);

	// Reset controls
	ManualLinearVelocity = FVector::ZeroVector;
	ManualAngularVelocity = FVector::ZeroVector;

	// Put the camera at the right spot
	if (ExternalCamera)
	{
		SetCameraLocalPosition(FVector::ZeroVector);
		SetCameraDistance(CameraMaxDistance * GetMeshScale());
	}
	else
	{
		FVector CameraOffset = WorldToLocal(Airframe->GetSocketLocation(FName("Camera")) - GetActorLocation());
		SetCameraDistance(0);
		SetCameraLocalPosition(CameraOffset);
	}
}


/*----------------------------------------------------
	Ship interface
----------------------------------------------------*/

void AFlareShip::Load(const FFlareShipSave& Data)
{
	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();
	
	// Update local data
	ShipData = Data;
	ShipData.Name = FName(*GetName());

	// Look for parent company
	SetOwnerCompany(GetGame()->FindCompany(Data.CompanyIdentifier));

	// Load ship description
	UFlareShipPartsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	FFlareShipDescription* Desc = GetGame()->GetShipCatalog()->Get(Data.Identifier);
	SetShipDescription(Desc);

	// Initialize components
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareShipComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{	
		UFlareShipComponent* Component = Cast<UFlareShipComponent>(Components[ComponentIndex]);
		FFlareShipComponentSave ComponentData;
		
		// Find component data
		bool found = false;
		for (int32 i = 0; i < Data.Components.Num(); i++)
		{
			if (Component->SlotIdentifier == Data.Components[i].ShipSlotIdentifier)
			{
				ComponentData = Data.Components[i];
				found = true;
				break;
			}
		}
		
		// Reload the component
		if (!found)
		{
			continue;
		}
		ReloadPart(Component, &ComponentData);
		
		// Set RCS description
		FFlareShipComponentDescription* ComponentDescription = Catalog->Get(ComponentData.ComponentIdentifier);
		if (ComponentDescription->Type == EFlarePartType::RCS)
		{
			SetRCSDescription(ComponentDescription);
		}

		// Set orbital engine description
		else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
		{
			SetOrbitalEngineDescription(ComponentDescription);
		}
		
		// If this is a weapon, add to weapon list.
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Component);
		if (Weapon)
		{
			WeaponList.Add(Weapon);
		}
	}
	
	// Load weapon descriptions
	for (int32 i = 0; i < Data.Components.Num(); i++)
	{
		FFlareShipComponentDescription* ComponentDescription = Catalog->Get(Data.Components[i].ComponentIdentifier);
		if (ComponentDescription->Type == EFlarePartType::Weapon)
		{
			WeaponDescriptionList.Add(ComponentDescription);
		}
	}

	// Customization
	UpdateCustomization();

	// Re-dock if we were docked
	if (ShipData.DockedTo != NAME_None)
	{
		FLOGV("AFlareShip::Load : Looking for station '%s'", *ShipData.DockedTo.ToString());
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareStation* Station = Cast<AFlareStation>(*ActorItr);
			if (Station && *Station->GetName() == ShipData.DockedTo)
			{
				FLOGV("AFlareShip::Load : Found dock station '%s'", *Station->GetName());
				ConfirmDock(Station, ShipData.DockedAt);
				break;
			}
		}
	}
}

FFlareShipSave* AFlareShip::Save()
{
	// Physical data
	ShipData.Location = GetActorLocation();
	ShipData.Rotation = GetActorRotation();
	ShipData.LinearVelocity = Airframe->GetPhysicsLinearVelocity();
	ShipData.AngularVelocity = Airframe->GetPhysicsAngularVelocity();

	// Engines
	//ShipData.OrbitalEngineIdentifier = OrbitalEngineDescription->Identifier;
	//ShipData.RCSIdentifier = RCSDescription->Identifier;

	// Weapons
	/*ShipData.WeaponIdentifiers.Empty();
	for (int32 i = 0; i < WeaponDescriptionList.Num(); i++)
	{
		ShipData.WeaponIdentifiers.Add(WeaponDescriptionList[i]->Identifier);
	}*/
	
	//TODO save all components

	return &ShipData;
}

void AFlareShip::SetOwnerCompany(UFlareCompany* NewCompany)
{
	SetCompany(NewCompany);
	ShipData.CompanyIdentifier = NewCompany->GetIdentifier();
	Airframe->Initialize(NULL, Company, this);
	NewCompany->Register(this);
}

UFlareCompany* AFlareShip::GetCompany()
{
	return Company;
}

bool AFlareShip::NavigateTo(FVector TargetLocation)
{
	// Pathfinding data
	TArray<FVector> Path;
	FVector Unused;
	FVector ShipExtent;
	FVector Temp = GetActorLocation();

	// Prepare data
	FLOG("AFlareShip::NavigateTo");
	GetActorBounds(true, Unused, ShipExtent);
	UpdateColliders();

	// Compute path
	if (ComputePath(Path, PathColliders, Temp, TargetLocation, ShipExtent.Size()))
	{
		FLOGV("AFlareShip::NavigateTo : generating path (%d stops)", Path.Num());

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
	FLOG("AFlareShip::NavigateTo failed : no path found");
	return false;
}

bool AFlareShip::IsManualPilot()
{
	return (Status == EFlareShipStatus::SS_Manual || Status == EFlareShipStatus::SS_Gliding);
}

bool AFlareShip::IsGliding()
{
	return (Status == EFlareShipStatus::SS_Gliding);
}

bool AFlareShip::IsAutoPilot()
{
	return (Status == EFlareShipStatus::SS_AutoPilot);
}

bool AFlareShip::IsDocked()
{
	return (Status == EFlareShipStatus::SS_Docked);
}


/*----------------------------------------------------
	Docking
----------------------------------------------------*/

bool AFlareShip::DockAt(IFlareStationInterface* TargetStation)
{
	FLOG("AFlareShip::DockAt");
	FFlareDockingInfo DockingInfo = TargetStation->RequestDock(this);

	// Try to dock
	if (DockingInfo.Granted)
	{
		FLOG("AFlareShip::DockAt : access granted");
		FVector ShipDockOffset = GetDockLocation();
		DockingInfo.EndPoint += DockingInfo.Rotation.RotateVector(ShipDockOffset * FVector(1, 0, 0)) - ShipDockOffset * FVector(0, 1, 1);
		DockingInfo.StartPoint = DockingInfo.EndPoint + 5000 * DockingInfo.Rotation.RotateVector(FVector(1, 0, 0));
		
		// Dock
		if (NavigateTo(DockingInfo.StartPoint))
		{
			// Align front to dock axis, ship top to station top, set speed
			PushCommandRotation((DockingInfo.EndPoint - DockingInfo.StartPoint), FVector(1, 0, 0));
			PushCommandRotation(FVector(0,0,1), FVector(0,0,1));
			LinearMaxVelocity = LinearMaxDockingVelocity;
			
			// Move there
			PushCommandLocation(DockingInfo.EndPoint);
			PushCommandDock(DockingInfo);
			FLOG("AFlareShip::DockAt : navigation sent");
			return true;
		}
	}

	// Failed
	FLOG("AFlareShip::DockAt failed");
	return false;
}

void AFlareShip::ConfirmDock(IFlareStationInterface* DockStation, int32 DockId)
{
	FLOG("AFlareShip::ConfirmDock");
	ClearCurrentCommand();

	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (PC && !ExternalCamera)
	{
		PC->SetExternalCamera(true);
	}

	// Set as docked
	DockStation->Dock(this, DockId);
	Status = EFlareShipStatus::SS_Docked;
	ShipData.DockedTo = *DockStation->_getUObject()->GetName();
	ShipData.DockedAt = DockId;
	
	// Disable physics, reset speed
	LinearMaxVelocity = ShipDescription->LinearMaxVelocity;
	Airframe->SetSimulatePhysics(false);

	// Cut engines
	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		Engine->SetAlpha(0.0f);
	}

}

bool AFlareShip::Undock()
{
	FLOG("AFlareShip::Undock");
	FFlareShipCommandData Head;

	// Try undocking
	if (IsDocked())
	{
		// Enable physics
		Airframe->SetSimulatePhysics(true);
	  
		// Evacuate
		ClearCurrentCommand();
		GetDockStation()->ReleaseDock(this, ShipData.DockedAt);
		PushCommandLocation(RootComponent->GetComponentTransform().TransformPositionNoScale(5000 * FVector(-1, 0, 0)));

		// Update data
		Status = EFlareShipStatus::SS_AutoPilot;
		ShipData.DockedTo = NAME_None;
		ShipData.DockedAt = -1;

		FLOG("AFlareShip::Undock successful");
		return true;
	}

	// Failed
	FLOG("AFlareShip::Undock failed");
	return false;
}

IFlareStationInterface* AFlareShip::GetDockStation()
{
	if (IsDocked())
	{
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareStation* Station = Cast<AFlareStation>(*ActorItr);
			if (Station && *Station->GetName() == ShipData.DockedTo)
			{
				return Station;
			}
		}
	}
	return NULL;
}


/*----------------------------------------------------
	Navigation commands and helpers
----------------------------------------------------*/

void AFlareShip::PushCommandLinearBrake()
{
	FFlareShipCommandData Data;
	Data.Type = EFlareCommandDataType::CDT_Location;
	PushCommand(Data);
}

void AFlareShip::PushCommandAngularBrake()
{
	FFlareShipCommandData Data;
	Data.Type = EFlareCommandDataType::CDT_BrakeRotation;
	PushCommand(Data);
}

void AFlareShip::PushCommandLocation(const FVector& Location)
{
	FFlareShipCommandData Data;
	Data.Type = EFlareCommandDataType::CDT_Location;
	Data.LocationTarget = Location;
	PushCommand(Data);
}

void AFlareShip::PushCommandRotation(const FVector& RotationTarget, const FVector& LocalShipAxis)
{
	FFlareShipCommandData Data;
	Data.Type = EFlareCommandDataType::CDT_Rotation;
	Data.RotationTarget = RotationTarget;
	Data.LocalShipAxis = LocalShipAxis;
	FLOGV("PushCommandRotation RotationTarget '%s'", *RotationTarget.ToString());
	FLOGV("PushCommandRotation LocalShipAxis '%s'", *LocalShipAxis.ToString());
	PushCommand(Data);
}

void AFlareShip::PushCommandDock(const FFlareDockingInfo& DockingInfo)
{
	FFlareShipCommandData Data;
	Data.Type = EFlareCommandDataType::CDT_Dock;
	Data.ActionTarget = Cast<AFlareStation>(DockingInfo.Station);
	Data.ActionTargetParam = DockingInfo.DockId;
	PushCommand(Data);
}

void AFlareShip::PushCommand(const FFlareShipCommandData& Command)
{
	Status = EFlareShipStatus::SS_AutoPilot;
	CommandData.Enqueue(Command);

	FLOGV("Pushed command '%s'", *EFlareCommandDataType::ToString(Command.Type));
}

void AFlareShip::ClearCurrentCommand()
{
	FFlareShipCommandData Command;
	CommandData.Dequeue(Command);

	FLOGV("Cleared command '%s'", *EFlareCommandDataType::ToString(Command.Type));
}

FVector AFlareShip::GetDockLocation()
{
	FVector WorldLocation = RootComponent->GetSocketLocation(FName("Dock"));
	return RootComponent->GetComponentTransform().InverseTransformPosition(WorldLocation);
}

bool AFlareShip::ComputePath(TArray<FVector>& Path, TArray<AActor*>& PossibleColliders, FVector OriginLocation, FVector TargetLocation, float ShipSize)
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
			DrawDebugSphere(GetWorld(), ColliderLocation, ColliderSize, 12, FColor::Blue, true);

			// Get an orthogonal plane
			FPlane TravelOrthoPlane = FPlane(ColliderLocation, TargetLocation - ColliderLocation);
			FVector IntersectedLocation = FMath::LinePlaneIntersection(OriginLocation, TargetLocation, TravelOrthoPlane);

			// Relocate intersection inside the sphere
			FVector Intersector = IntersectedLocation - ColliderLocation;
			Intersector.Normalize();
			IntersectedLocation = ColliderLocation + Intersector * ColliderSize;

			// Collisions
			bool IsColliding = IsPointColliding(IntersectedLocation, PossibleColliders[i]);
			DrawDebugPoint(GetWorld(), IntersectedLocation, 8, IsColliding ? FColor::Red : FColor::Green, true);

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

void AFlareShip::UpdateColliders()
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

bool AFlareShip::IsPointColliding(FVector Candidate, AActor* Ignore)
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


/*----------------------------------------------------
	Attitude control : linear version
----------------------------------------------------*/

void AFlareShip::UpdateLinearAttitudeManual(float DeltaSeconds)
{
	if (IsGliding())
	{
		// Add velocity command to current velocity
		LinearTargetVelocity = GetLinearVelocity() + Airframe->GetComponentToWorld().GetRotation().RotateVector(ManualLinearVelocity);
		LinearTargetVelocity = LinearTargetVelocity.GetClampedToMaxSize(LinearMaxVelocity);
	}
	else
	{
		LinearTargetVelocity = Airframe->GetComponentToWorld().GetRotation().RotateVector(ManualLinearVelocity);
	}
}

void AFlareShip::UpdateLinearAttitudeAuto(float DeltaSeconds)
{
	// Location data
	FFlareShipCommandData Data;
	CommandData.Peek(Data);

	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());

	FVector DeltaPosition = (Data.LocationTarget - GetActorLocation()) / 100; // Distance in meters
	FVector DeltaPositionDirection = DeltaPosition;
	DeltaPositionDirection.Normalize();
	float Distance = FMath::Max(0.0f, DeltaPosition.Size() - LinearDeadDistance);

	FVector DeltaVelocity = -GetLinearVelocity();
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;
	
	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else
	{
		
		FVector Acceleration = GetTotalMaxThrustInAxis(Engines, DeltaVelocityAxis, 0, false) / Airframe->GetMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, DeltaPositionDirection));
		
		TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float DistanceToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	FVector RelativeResultSpeed;

	if (DistanceToStop > Distance)
	{
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else {

		float MaxPreciseSpeed = FMath::Min((Distance - DistanceToStop) / DeltaSeconds, LinearMaxVelocity);

		RelativeResultSpeed = DeltaPositionDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	// Under this distance we consider the variation negligible, and ensure null delta + null speed
	if (Distance < LinearDeadDistance && DeltaVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Airframe->SetPhysicsLinearVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
		RelativeResultSpeed = FVector::ZeroVector;	

	}
	LinearTargetVelocity = RelativeResultSpeed;
}

void AFlareShip::UpdateLinearBraking(float DeltaSeconds)
{
	LinearTargetVelocity = FVector::ZeroVector;
	FVector LinearVelocity = WorldToLocal(Airframe->GetPhysicsLinearVelocity());

	// Null speed detection
	if (LinearVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Airframe->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		ClearCurrentCommand();
	}
}


/*----------------------------------------------------
	Attitude control : angular version
----------------------------------------------------*/

void AFlareShip::UpdateAngularAttitudeManual(float DeltaSeconds)
{
	AngularTargetVelocity = Airframe->GetComponentToWorld().GetRotation().RotateVector(ManualAngularVelocity);
}

void AFlareShip::UpdateAngularAttitudeAuto(float DeltaSeconds)
{
	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());
  
	// Rotation data
	FFlareShipCommandData Data;
	CommandData.Peek(Data);
	FVector TargetAxis = Data.RotationTarget;
	FVector LocalShipAxis = Data.LocalShipAxis;
	
	FVector AngularVelocity = Airframe->GetPhysicsAngularVelocity();
	FVector WorldShipAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalShipAxis);
	
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
	    float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, COM, 0, true, false) / GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, COM, 0, false, false);
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
		Airframe->SetPhysicsAngularVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
		RelativeResultSpeed = FVector::ZeroVector;
	}
	AngularTargetVelocity = RelativeResultSpeed;
}

void AFlareShip::UpdateAngularBraking(float DeltaSeconds)
{
	AngularTargetVelocity = FVector::ZeroVector;
	FVector AngularVelocity = Airframe->GetPhysicsAngularVelocity();
	// Null speed detection
	if (AngularVelocity.Size() < NegligibleSpeedRatio * AngularMaxVelocity)
	{
		AngularTargetVelocity = FVector::ZeroVector;
		Airframe->SetPhysicsAngularVelocity(FVector::ZeroVector, false); // TODO remove
		ClearCurrentCommand();
	}
}

/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlareShip::SetShipDescription(FFlareShipDescription* Description)
{
	ShipDescription = Description;

	// Load data from the ship info
	if (Description)
	{
		LinearMaxVelocity = Description->LinearMaxVelocity;
		AngularMaxVelocity = Description->AngularMaxVelocity;
	}
}

void AFlareShip::SetOrbitalEngineDescription(FFlareShipComponentDescription* Description)
{
	OrbitalEngineDescription = Description;
	//ReloadAllParts(UFlareOrbitalEngine::StaticClass(), Description);
}

void AFlareShip::SetRCSDescription(FFlareShipComponentDescription* Description)
{
	RCSDescription = Description;
	//ReloadAllParts(UFlareRCS::StaticClass(), Description);

	// Find the RCS turn and power rating, since RCSs themselves don't do anything
	if (Description)
	{
		for (int32 i = 0; i < Description->Characteristics.Num(); i++)
		{
			const FFlareShipComponentCharacteristic& Characteristic = Description->Characteristics[i];

			// Calculate the angular acceleration rate from the ton weight (data value in °/s per 100T)
			if (Airframe && Characteristic.CharacteristicType == EFlarePartCharacteristicType::RCSAccelerationRating)
			{
				float Mass = Airframe->GetMass() / 100000;
				AngularAccelerationRate = Characteristic.CharacteristicValue / (60 * Mass);
				break;
			}
		}
	}
}

void AFlareShip::UpdateCustomization()
{
	Super::UpdateCustomization();

	Airframe->UpdateCustomization();
}

void AFlareShip::StartPresentation()
{
	Super::StartPresentation();

	if (Airframe)
	{
		Airframe->SetSimulatePhysics(false);
	}
}


/*----------------------------------------------------
	Physics
----------------------------------------------------*/

void AFlareShip::PhysicSubTick(float DeltaSeconds)
{
	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());
	
	// Linear physics	
	FVector DeltaV = LinearTargetVelocity - GetLinearVelocity();
	FVector DeltaVAxis = DeltaV;
	DeltaVAxis.Normalize();
	
	if (!DeltaV.IsNearlyZero()) 
	{
		FVector Acceleration = DeltaVAxis * GetTotalMaxThrustInAxis(Engines, -DeltaVAxis, 0, ManualOrbitalBoost).Size() / Airframe->GetMass();
		FVector ClampedAcceleration = Acceleration.GetClampedToMaxSize(DeltaV.Size() / DeltaSeconds);

		Airframe->SetPhysicsLinearVelocity(ClampedAcceleration * DeltaSeconds * 100, true); // Multiply by 100 because UE4 works in cm
	}

	// Angular physics
	FVector DeltaAngularV = AngularTargetVelocity - Airframe->GetPhysicsAngularVelocity();
	FVector DeltaAngularVAxis = DeltaAngularV;
	DeltaAngularVAxis.Normalize();
	
	if (!DeltaAngularV.IsNearlyZero())
	{
		FVector SimpleAcceleration = DeltaAngularVAxis * AngularAccelerationRate;

		// Scale with damages
		float DamageRatio = GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, COM, 0, true, false) / GetTotalMaxTorqueInAxis(Engines, DeltaAngularVAxis, COM, 0, false, false);
		FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;
		FVector ClampedSimplifiedAcceleration = DamagedSimpleAcceleration.GetClampedToMaxSize(DeltaAngularV.Size() / DeltaSeconds);

		Airframe->SetPhysicsAngularVelocity(ClampedSimplifiedAcceleration  * DeltaSeconds, true);
	}
	
	// Update engine alpha
	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		FVector ThrustAxis = Engine->GetThrustAxis();
		float LinearAlpha = 0;
		float AngularAlpha = 0;
		
		if (IsPresentationMode()) {
			LinearAlpha = true;
		} else if (!DeltaV.IsNearlyZero()) { 
			if (!(!ManualOrbitalBoost && Engine->IsOrbitalEngine())) {	
				LinearAlpha = -FVector::DotProduct(ThrustAxis, DeltaVAxis);
			}
		}
		
		FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
		FVector TorqueDirection = FVector::CrossProduct(EngineOffset, ThrustAxis);
		TorqueDirection.Normalize();
		
		if (!DeltaAngularV.IsNearlyZero() && !Engine->IsOrbitalEngine()) { 
			AngularAlpha = -FVector::DotProduct(TorqueDirection, DeltaAngularVAxis);
		}
		
		Engine->SetAlpha(FMath::Clamp(LinearAlpha + AngularAlpha, 0.0f, 1.0f));
	}
}

void AFlareShip::UpdateCOM()
{
    COM = Airframe->GetBodyInstance()->GetCOMPosition();
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareShip::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("Thrust", this, &AFlareShip::ThrustInput);
	InputComponent->BindAxis("MoveVerticalInput", this, &AFlareShip::MoveVerticalInput);
	InputComponent->BindAxis("MoveHorizontalInput", this, &AFlareShip::MoveHorizontalInput);

	InputComponent->BindAxis("RollInput", this, &AFlareShip::RollInput);
	InputComponent->BindAxis("PitchInput", this, &AFlareShip::PitchInput);
	InputComponent->BindAxis("YawInput", this, &AFlareShip::YawInput);
	InputComponent->BindAxis("MouseInputY", this, &AFlareShip::PitchInput);
	InputComponent->BindAxis("MouseInputX", this, &AFlareShip::YawInput);

	InputComponent->BindAction("ZoomIn", EInputEvent::IE_Released, this, &AFlareShip::ZoomIn);
	InputComponent->BindAction("ZoomOut", EInputEvent::IE_Released, this, &AFlareShip::ZoomOut);

	InputComponent->BindAction("FaceForward", EInputEvent::IE_Released, this, &AFlareShip::FaceForward);
	InputComponent->BindAction("FaceBackward", EInputEvent::IE_Released, this, &AFlareShip::FaceBackward);
	InputComponent->BindAction("Boost", EInputEvent::IE_Pressed, this, &AFlareShip::BoostOn);
	InputComponent->BindAction("Boost", EInputEvent::IE_Released, this, &AFlareShip::BoostOff);
	InputComponent->BindAction("Glide", EInputEvent::IE_Released, this, &AFlareShip::ToggleGliding);

	InputComponent->BindAction("MouseLeft", EInputEvent::IE_Pressed, this, &AFlareShip::StartFire);
	InputComponent->BindAction("MouseLeft", EInputEvent::IE_Released, this, &AFlareShip::StopFire);
}

void AFlareShip::MousePositionInput(FVector2D Val)
{
	if (!ExternalCamera)
	{
		// Compensation curve = 1 + (input-1)/(1-AngularInputDeadRatio)
		Val.X = FMath::Clamp(1. + (FMath::Abs(Val.X) - 1. ) / (1. - AngularInputDeadRatio) , 0., 1.) * FMath::Sign(Val.X);
		Val.Y = FMath::Clamp(1. + (FMath::Abs(Val.Y) - 1. ) / (1. - AngularInputDeadRatio) , 0., 1.) * FMath::Sign(Val.Y);
		

		ManualAngularVelocity.Z = Val.X * AngularMaxVelocity;
		ManualAngularVelocity.Y = Val.Y * AngularMaxVelocity;
	}
}

void AFlareShip::ThrustInput(float Val)
{
	if (!ExternalCamera)
	{
		ManualLinearVelocity.X = Val * LinearMaxVelocity;
	}
}

void AFlareShip::MoveVerticalInput(float Val)
{
	if (!ExternalCamera)
	{
		ManualLinearVelocity.Z = LinearMaxVelocity * Val;
	}
}

void AFlareShip::MoveHorizontalInput(float Val)
{
	if (!ExternalCamera)
	{
		ManualLinearVelocity.Y = LinearMaxVelocity * Val;
	}
}

void AFlareShip::RollInput(float Val)
{
	if (!ExternalCamera)
	{
		ManualAngularVelocity.X = - Val * AngularMaxVelocity;
	}
}

void AFlareShip::PitchInput(float Val)
{
	if (ExternalCamera)
	{
		FRotator CurrentRot = WorldToLocal(CameraContainerPitch->GetComponentRotation().Quaternion()).Rotator();
		SetCameraPitch(CurrentRot.Pitch + Val * CameraPanSpeed);
	}
}

void AFlareShip::YawInput(float Val)
{
	if (ExternalCamera)
	{
		FRotator CurrentRot = WorldToLocal(CameraContainerPitch->GetComponentRotation().Quaternion()).Rotator();
		SetCameraYaw(CurrentRot.Yaw + Val * CameraPanSpeed);
	}
}

void AFlareShip::ZoomIn()
{
	if (ExternalCamera)
	{
		StepCameraDistance(true);
	}
}

void AFlareShip::ZoomOut()
{
	if (ExternalCamera)
	{
		StepCameraDistance(false);
	}
}

void AFlareShip::FaceForward()
{
	if (IsManualPilot())
	{
		PushCommandRotation(Airframe->GetPhysicsLinearVelocity(), FVector(1,0,0));
	}
}

void AFlareShip::FaceBackward()
{
	if (IsManualPilot())
	{
		PushCommandRotation((-Airframe->GetPhysicsLinearVelocity()), FVector(1,0,0));
	}
}

void AFlareShip::BoostOn()
{
	if (IsManualPilot())
	{
		ManualOrbitalBoost = true;
	}
}

void AFlareShip::BoostOff()
{
	ManualOrbitalBoost = false;
}


void AFlareShip::ToggleGliding()
{
	if (IsGliding())
	{
		Status = EFlareShipStatus::SS_Manual;
	}
	else
	{
		Status = EFlareShipStatus::SS_Gliding;
	}
}

void AFlareShip::StartFire()
{
	if (!ExternalCamera)
	{
		for (int32 i = 0; i < WeaponList.Num(); i++)
		{
			WeaponList[i]->StartFire();
		}
	}
}

void AFlareShip::StopFire()
{
	if (!ExternalCamera)
	{
		for (int32 i = 0; i < WeaponList.Num(); i++)
		{
			WeaponList[i]->StopFire();
		}
	}
}

/*----------------------------------------------------
		Getters (Attitude)
----------------------------------------------------*/

FVector AFlareShip::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}

FVector AFlareShip::GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, float ThrustAngleLimit, bool WithOrbitalEngines) const
{
	Axis.Normalize();
	FVector TotalMaxThrust = FVector::ZeroVector;
	for (int32 i = 0; i < Engines.Num(); i++)
	{
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if (Engine->IsOrbitalEngine() && !WithOrbitalEngines)
		{
		  continue;
		}

		FVector WorldThrustAxis = Engine->GetThrustAxis();

		float Dot = FVector::DotProduct(WorldThrustAxis, Axis);
		if (Dot > ThrustAngleLimit)
		{
			float Ratio = (Dot - ThrustAngleLimit) / (1 - ThrustAngleLimit);

			TotalMaxThrust += WorldThrustAxis * Engine->GetMaxThrust() * Ratio;
		}
	}

	return TotalMaxThrust;
}

float AFlareShip::GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueAxis, FVector COM, float ThrustAngleLimit, bool WithDamages, bool WithOrbitalEngines) const
{
	TorqueAxis.Normalize();
	float TotalMaxTorque = 0;

	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if (!WithOrbitalEngines && Engine->IsOrbitalEngine()) {
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

		float dot = FVector::DotProduct(TorqueAxis, TorqueDirection);
		
		
		if (dot > ThrustAngleLimit) {
			float ratio = (dot - ThrustAngleLimit) / (1 - ThrustAngleLimit);

			TotalMaxTorque += FVector::CrossProduct(EngineOffset, WorldThrustAxis).Size() * MaxThrust * ratio;
		}

	}

	return TotalMaxTorque;
}
