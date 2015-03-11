
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
	, LinearDeadDistance(10)
	, NegligibleSpeedRatio(0.05)
	, Status(EFlareShipStatus::SS_Manual)
	, FakeThrust(false)
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

	// Check which moves are allowed
	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		// RCS
		UFlareRCS* RCS = Cast<UFlareRCS>(*ComponentIt);
		if (RCS)
		{
// 			if (RCS->CanMoveVertical())
			{
				CanMoveVertical = true;
			}
		}

		// If this is a weapon, reinitialize it directly so that it updates its properties
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(*ComponentIt);
		if (Weapon && WeaponList.Num() < WeaponDescriptionList.Num())
		{
			ReloadPart(Weapon, WeaponDescriptionList[WeaponList.Num()]);
			WeaponList.Add(Weapon);
		}
	}
	
	// Compute Inertia tensor for homogeneous rotation
	UpdateCOM();
	LocalInertiaTensor = FVector::ZeroVector;
	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());
	FVector WorldXAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));
	FVector WorldYAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(0,1,0));
	FVector WorldZAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(0,0,1));
	//float angularStantardAcceleration = 90; // °/s² // TODO In ship spec
	LocalInertiaTensor.X = GetTotalMaxTorqueInAxis(Engines, WorldXAxis, COM, 0, false, false).Size() / AngularAccelerationRate;
	LocalInertiaTensor.Y = GetTotalMaxTorqueInAxis(Engines, WorldYAxis, COM, 0, false, false).Size() / AngularAccelerationRate;
	LocalInertiaTensor.Z = GetTotalMaxTorqueInAxis(Engines, WorldZAxis, COM, 0, false, false).Size() / AngularAccelerationRate;
	FLOGV("AngularAccelerationRate = %f", AngularAccelerationRate);
}

void AFlareShip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Attitude control
	if (Airframe && !FakeThrust)
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
		  
			

			LowLevelAutoPilotSubTick(DeltaSeconds);

			// Tick Modules
			TArray<UActorComponent*> Modules = GetComponentsByClass(UFlareShipModule::StaticClass());
			for (int32 i = 0; i < Modules.Num(); i++) {
				UFlareShipModule* Module = Cast<UFlareShipModule>(Modules[i]);
				Module->TickModule(DeltaSeconds);
			}

			//UpdateLinearPhysics(DeltaSeconds);

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
		SetCameraDistance(CameraMaxDistance * GetMeshScale());
	}
	else
	{
		FVector CameraDistance = WorldToLocal(Airframe->GetSocketLocation(FName("Camera")) - GetActorLocation());
		SetCameraDistance(-CameraDistance.Size());
	}
}


/*----------------------------------------------------
	Ship interface
----------------------------------------------------*/

void AFlareShip::Load(const FFlareShipSave& Data)
{
	// Update local data
	ShipData = Data;
	ShipData.Name = FName(*GetName());

	// Look for parent company
	SetOwnerCompany(GetGame()->FindCompany(Data.CompanyIdentifier));

	// Load ship description
	UFlareShipPartsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	FFlareShipDescription* Desc = GetGame()->GetShipCatalog()->Get(Data.Identifier);
	SetShipDescription(Desc);

	// Load component descriptions 
	SetOrbitalEngineDescription(Catalog->Get(Data.OrbitalEngineIdentifier));
	SetRCSDescription(Catalog->Get(Data.RCSIdentifier));

	// Load weapon descriptions
	WeaponDescriptionList.Empty();
	for (int32 i = 0; i < Data.WeaponIdentifiers.Num(); i++)
	{
		WeaponDescriptionList.Add(Catalog->Get(Data.WeaponIdentifiers[i]));
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

	// Engines
	ShipData.OrbitalEngineIdentifier = OrbitalEngineDescription->Identifier;
	ShipData.RCSIdentifier = RCSDescription->Identifier;

	// Weapons
	ShipData.WeaponIdentifiers.Empty();
	for (int32 i = 0; i < WeaponDescriptionList.Num(); i++)
	{
		ShipData.WeaponIdentifiers.Add(WeaponDescriptionList[i]->Identifier);
	}

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
		FVector ShipDockOffset = GetDockLocation();
		DockingInfo.EndPoint += DockingInfo.Rotation.RotateVector(ShipDockOffset);
		DockingInfo.StartPoint += 5000 * DockingInfo.Rotation.RotateVector(FVector(1, 0, 0));

		// Dock
		if (NavigateTo(DockingInfo.StartPoint))
		{
			PushCommandRotation((DockingInfo.EndPoint - DockingInfo.StartPoint), FVector(1,0,0));
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
}

bool AFlareShip::Undock()
{
	FLOG("AFlareShip::Undock");
	FFlareShipCommandData Head;

	// Try undocking
	if (IsDocked())
	{
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
				FLOGV("AFlareShip::GetDockStation : Found dock station '%s'", *Station->GetName());
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
  
	// RCS impulse
	LinearTargetVelocity = ManualLinearVelocity;

	//FLOGV("UpdateLinearAttitudeManual LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
	// Orbital impulse
	if (ManualOrbitalBoost)
	{
		FVector OrbitalPushDirection = GetActorQuat().RotateVector(FVector(1, 0, 0));
		// TODO remove and add in global engine system
		//Airframe->AddForce(LinearOrbitalThrust * OrbitalPushDirection);
	}
}

void AFlareShip::UpdateLinearAttitudeAuto(float DeltaSeconds)
{
	// Location data
	FFlareShipCommandData Data;
	CommandData.Peek(Data);
	FVector LocalCommand = WorldToLocal(Data.LocationTarget - GetActorLocation());

	// Check legit movements
	if (!CanMoveVertical)
	{
		LocalCommand.Z = 0;
	}
	float RemainingTravelDistance = LocalCommand.Size();
		
	// Under this angle we consider the variation negligible, and ensure null delta + null speed
	if (RemainingTravelDistance < LinearDeadDistance && LinearVelocity.Size() < NegligibleSpeedRatio * LinearMaxVelocity)
	{
		Airframe->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		SetActorLocation(Data.LocationTarget, true);
		LinearTargetVelocity = FVector::ZeroVector;
		ClearCurrentCommand();
	}

	// Brake if "close" to target and not going away from it
	else if (RemainingTravelDistance <= LinearStopDistance)
	{
		LinearTargetVelocity = FVector::ZeroVector;
	}

	// The rest of the time we try to follow the command
	else
	{
		LinearTargetVelocity = LocalCommand;
	}
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
	float dot = FVector::DotProduct(WorldShipAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(dot));

	FVector DeltaVelocity = -AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	// Time to reach Target velocity

	// To reach target velocity 

	// The profil to reach the target velocity is the following :
	// - a minimum DeltaV, induce by the time to the engine to reach their final trust.
	// - 2 thrust variation profil to accelerate more then less
	// - a constant thrust profil : at max thrust

	// Each engine as it's own final thust ratio and intertia so the minimun deltaV 
	// The sum of all engine give the minium delta V

	// For all engine that finish to reach there delta V the deltaV generated during all this time must be added.

	// Then compute the distance done during this time. If the distance is higther 


	// Find Usefull thrust
	
	//float UsefullTorque = FVector::DotProduct(DeltaVelocityAxis, MaxTorqueInAxis);

	float TimeToFinalVelocity;
	
	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
	  
	    
		FVector LocalDeltaVAxis = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(DeltaVelocityAxis);
		FVector LocalThustAxis = LocalDeltaVAxis * LocalInertiaTensor;
		LocalThustAxis.Normalize();
		FVector WorldThustAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalThustAxis);
		FVector MaxTorqueInAxis = GetTotalMaxTorqueInAxis(Engines, WorldThustAxis, COM, 0, true, false);
	  
		FVector LocalMaxTorqueInAxis = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(MaxTorqueInAxis);
	  
		FVector LocalAcceleration = LocalMaxTorqueInAxis / LocalInertiaTensor;
		
		FVector Acceleration = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalAcceleration);
		
		TimeToFinalVelocity = (DeltaVelocity / Acceleration).Size();
		FVector TimeToFinalVelocity2 = DeltaVelocity / Acceleration;
		float TimeToFinalVelocity3 = TimeToFinalVelocity2.Size();
		
		UE_LOG(LogTemp, Warning, TEXT("===="));
	UE_LOG(LogTemp, Warning, TEXT("3 - LocalInertiaTensor: %s"), *LocalInertiaTensor.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - MaxTorqueInAxis: %s"), *MaxTorqueInAxis.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - LocalMaxTorqueInAxis: %s"), *LocalMaxTorqueInAxis.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - LocalAcceleration: %s"), *LocalAcceleration.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - Acceleration: %s"), *Acceleration.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - TimeToFinalVelocity2: %s"), *TimeToFinalVelocity2.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - TimeToFinalVelocity3: %f"), TimeToFinalVelocity3);

		
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (TimeToFinalVelocity + DeltaSeconds);

	UE_LOG(LogTemp, Warning, TEXT("===="));
	UE_LOG(LogTemp, Warning, TEXT("3 - TargetAxis: %s"), *TargetAxis.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - RotationDirection: %s"), *RotationDirection.ToString());
	
	UE_LOG(LogTemp, Warning, TEXT("3 - LocalShipAxis: %s"), *LocalShipAxis.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - WorldShipAxis: %s"), *WorldShipAxis.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - AngularVelocity: %s"), *AngularVelocity.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - DeltaVelocity: %s"), *DeltaVelocity.ToString());
	UE_LOG(LogTemp, Warning, TEXT("3 - dot: %f"), dot);
	UE_LOG(LogTemp, Warning, TEXT("3 - angle: %f"), angle);
	UE_LOG(LogTemp, Warning, TEXT("3 - TimeToFinalVelocity: %f"), TimeToFinalVelocity);
	UE_LOG(LogTemp, Warning, TEXT("3 - DeltaSeconds: %f"), DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("3 - AngleToStop: %f"), AngleToStop);

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else {

		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / DeltaSeconds, AngularMaxVelocity);

		UE_LOG(LogTemp, Warning, TEXT("3 - MaxPreciseSpeed: %f"), MaxPreciseSpeed);

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}	
	
	UE_LOG(LogTemp, Warning, TEXT("3 - RelativeResultSpeed: %s"), *RelativeResultSpeed.ToString());
	
	
	
	
	
	
	
	//////////////////////
	
	/*
	float DeltaVelocityToTarget = (LocalTargetVelocity - WorldVelocityToEnginesTarget);
		float AccelerationToTarget = DeltaVelocityToTarget / DeltaSeconds;
		
		FVector LocalAccelerationToTarget = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(AccelerationToTarget * TorqueDirection);
		FVector LocalTorqueToTarget = LocalAccelerationToTarget * LocalInertiaTensor;
		
		FVector TorqueToTarget = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalTorqueToTarget);
		
		float TorqueToTargetInAxis = FVector::DotProduct(TorqueDirection, TorqueToTarget);
	
	
	
	
	*/
	
	//////////////////
	
	
	
	
	
	
	
	
	
	
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

void AFlareShip::SetOrbitalEngineDescription(FFlareShipModuleDescription* Description)
{
	OrbitalEngineDescription = Description;
	ReloadAllParts(UFlareOrbitalEngine::StaticClass(), Description);

	// Find the orbital thrust rating
	if (Description)
	{
		for (int32 i = 0; i < Description->Characteristics.Num(); i++)
		{
			const FFlarePartCharacteristic& Characteristic = Description->Characteristics[i];

			// Calculate the orbital engine linear thrust force in N (data value in kN)
			if (Characteristic.CharacteristicType == EFlarePartAttributeType::EnginePower)
			{
				//LinearOrbitalThrust = 100 * 1000 * Characteristic.CharacteristicValue;
			}
		}
	}
}

void AFlareShip::SetRCSDescription(FFlareShipModuleDescription* Description)
{
	RCSDescription = Description;
	ReloadAllParts(UFlareRCS::StaticClass(), Description);

	// Find the RCS turn and power rating, since RCSs themselves don't do anything
	if (Description)
	{
		for (int32 i = 0; i < Description->Characteristics.Num(); i++)
		{
			const FFlarePartCharacteristic& Characteristic = Description->Characteristics[i];

			// Calculate the angular acceleration rate from the ton weight (data value in °/s per 100T)
			if (Airframe && Characteristic.CharacteristicType == EFlarePartAttributeType::RCSAccelerationRating)
			{
				float Mass = Airframe->GetMass() / 100000;
				AngularAccelerationRate = Characteristic.CharacteristicValue / (60 * Mass);
			}

			// Calculate the RCS linear thrust force in N (data value in kN)
			else if (Characteristic.CharacteristicType == EFlarePartAttributeType::EnginePower)
			{
				LinearThrust = 100 * 1000 * Characteristic.CharacteristicValue;
			}
		}
	}
}

void AFlareShip::SetWeaponDescription(int32 Index, FFlareShipModuleDescription* Description)
{
	if (Index < WeaponList.Num())
	{
		WeaponDescriptionList[Index] = Description;
		ReloadPart(WeaponList[Index], Description);
	}
	else
	{
		FLOGV("AFlareShip::SetWeaponDescription : failed (no such index %d)", Index);
	}
}

void AFlareShip::StartPresentation()
{
	Airframe->SetSimulatePhysics(false);
	FakeThrust = true;
}

void AFlareShip::UpdateCustomization()
{
	Super::UpdateCustomization();

	Airframe->UpdateCustomization();
}

/*----------------------------------------------------
	Physics
----------------------------------------------------*/

void AFlareShip::LowLevelAutoPilotSubTick(float DeltaSeconds)
{

  
	TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());

	//UE_LOG(LogTemp, Warning, TEXT("LowLevelAutoPilotSubTick num engine=%d"), Engines.Num());

	TArray<float*> EngineCommands;

	FVector LinearTarget = Airframe->GetComponentToWorld().GetRotation().RotateVector(LinearTargetVelocity);
	FVector AngularTarget = AngularTargetVelocity;
	
	EngineCommands.Add(ComputeLinearVelocityStabilisation(DeltaSeconds, Engines, LinearTarget, 0.0));
	EngineCommands.Add(ComputeAngularVelocityStabilisation(DeltaSeconds, Engines, AngularTarget));


	for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++) {
		float ThrustRatio = 0;
		for (int32 CommandIndex = 0; CommandIndex < EngineCommands.Num(); CommandIndex++) {
			float newThustRatio =EngineCommands[CommandIndex][EngineIndex];
			//FLOGV("Merge command engine %d Commande %d newThustRatio=%f",EngineIndex, CommandIndex, newThustRatio);
			/*if (newThustRatio * ThrustRatio < 0) {
				// Opposite order
				ThrustRatio = 0;
			}*/
			ThrustRatio = ThrustRatio + newThustRatio;
		}
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
		//FLOGV("Merge command engine %d ThrustRatio=%f",EngineIndex, ThrustRatio);
		//Engine->SetTargetThrustRatio(ThrustRatio);
	}
}

void AFlareShip::PhysicSubTick(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("PhysicSubTick"));

	if(!TickSumForce.IsNearlyZero()) {

	FVector Acceleration = FVector(0);
	Acceleration += TickSumForce / Airframe->GetMass();

	
	
	Airframe->SetPhysicsLinearVelocity(Acceleration * DeltaTime * 100, true); // Multiply by 100 because UE4 works in cm
	UE_LOG(LogTemp, Warning, TEXT("====================="));
	UE_LOG(LogTemp, Warning, TEXT("0 - TickSumForce: %s"), *TickSumForce.ToString());
	UE_LOG(LogTemp, Warning, TEXT("1 - Mass: %f"), Airframe->GetMass());
	UE_LOG(LogTemp, Warning, TEXT("2 - deltaTime: %f"), DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("3 - Acceleration: %s"), *Acceleration.ToString());
	UE_LOG(LogTemp, Warning, TEXT("4 - GetPhysicsLinearVelocity: %s"), *Airframe->GetPhysicsLinearVelocity().ToString());

	
	}
	// TODO find a better place to set
	//float WorldInertiaTensor = 1000;

	//TODO inertia in axis
	if(!TickSumTorque.IsNearlyZero()) {
	  /*FVector LocalAccelerationAxis = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(TickSumTorque);
	  LocalAccelerationAxis.Normalize();
	  
	  
	  
	  
	  */
	  
	  FVector AngularAcceleration = FVector(0);
	  
	  FVector LocalTickSumTorque = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(TickSumTorque);
	  FVector LocalAngularAcceleration = LocalTickSumTorque / LocalInertiaTensor;
	  
	  
	  
	  
	  
	  
	  //FVector InertiaTensor = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalInertiaTensor);
	  
	  
	  AngularAcceleration += Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalAngularAcceleration);
	  
	  
	  // Align with target speed delta to be more MaxPreciseSpeed
	  
	  FVector DeltaV = AngularTargetVelocity - Airframe->GetPhysicsAngularVelocity();
	  FVector DeltaVAxis = DeltaV;
	  DeltaVAxis.Normalize();
	  float AccelerationInAxis = FVector::DotProduct(DeltaVAxis, AngularAcceleration);
	  
	  FVector CorrectedAngularAcceleration = DeltaVAxis * AccelerationInAxis;
	  
	  TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());
	  
	  
	  FVector LocalDeltaVAxis = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(DeltaVAxis);
	  FVector LocalThustAxis = LocalDeltaVAxis * LocalInertiaTensor;
	  LocalThustAxis.Normalize();
	  FVector WorldThustAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalThustAxis);
	  
	  FVector MaxTorqueInAxis = GetTotalMaxTorqueInAxis(Engines, WorldThustAxis, COM, 0, true, false);
	  FVector LocalMaxTorqueInAxis = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(MaxTorqueInAxis);
	  
	  FVector LocalAcceleration = LocalMaxTorqueInAxis / LocalInertiaTensor;
		
	  
	  FVector UnifiedAcceleration = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalAcceleration);
	  FVector ClampedUnifiedAcceleration = UnifiedAcceleration.ClampMaxSize(DeltaV.Size() / DeltaTime);
		
	  // TODO Clamp
	  
	  UE_LOG(LogTemp, Warning, TEXT("====================="));
	  UE_LOG(LogTemp, Warning, TEXT("0 - TickSumTorque: %s"), *TickSumTorque.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("0 - LocalInertiaTensor: %s"), *LocalInertiaTensor.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalTickSumTorque: %s"), *LocalTickSumTorque.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalAngularAcceleration: %s"), *LocalAngularAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - AngularAcceleration: %s"), *AngularAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - AngularTargetVelocity: %s"), *AngularTargetVelocity.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - Airframe->GetPhysicsAngularVelocity(): %s"), *Airframe->GetPhysicsAngularVelocity().ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - DeltaV: %s"), *DeltaV.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - DeltaVAxis: %s"), *DeltaVAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - AccelerationInAxis: %f"), AccelerationInAxis);
	  UE_LOG(LogTemp, Warning, TEXT("1 - CorrectedAngularAcceleration: %s"), *CorrectedAngularAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalDeltaVAxis: %s"), *LocalDeltaVAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalThustAxis: %s"), *LocalThustAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - WorldThustAxis: %s"), *WorldThustAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - MaxTorqueInAxis: %s"), *MaxTorqueInAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalMaxTorqueInAxis: %s"), *LocalMaxTorqueInAxis.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - LocalAcceleration: %s"), *LocalAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - UnifiedAcceleration: %s"), *UnifiedAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT("1 - ClampedUnifiedAcceleration: %s"), *ClampedUnifiedAcceleration.ToString());
	  UE_LOG(LogTemp, Warning, TEXT(""));
	
	/*FVector LocalInertiaTensor2;
	FVector WorldXAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));
	FVector WorldYAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(0,1,0));
	FVector WorldZAxis = Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(0,0,1));
	float angularStantardAcceleration = 90; // °/s² // TODO In ship spec
	LocalInertiaTensor2.X = GetTotalMaxTorqueInAxis(Engines, WorldXAxis, COM, 0, false).Size();
	LocalInertiaTensor2.Y = GetTotalMaxTorqueInAxis(Engines, WorldYAxis, COM, 0, false).Size();
	LocalInertiaTensor2.Z = GetTotalMaxTorqueInAxis(Engines, WorldZAxis, COM, 0, false).Size();
	
	UE_LOG(LogTemp, Warning, TEXT("2 - X: %s"), *GetTotalMaxTorqueInAxis(Engines, WorldXAxis, COM, 0, false).ToString());
	UE_LOG(LogTemp, Warning, TEXT("2 - Y: %s"), *GetTotalMaxTorqueInAxis(Engines, WorldYAxis, COM, 0, false).ToString());
	UE_LOG(LogTemp, Warning, TEXT("2 - Z: %s"), *GetTotalMaxTorqueInAxis(Engines, WorldZAxis, COM, 0, false).ToString());
	UE_LOG(LogTemp, Warning, TEXT("2 - LocalInertiaTensor2: %s"), *LocalInertiaTensor2.ToString());*/
	
	  Airframe->SetPhysicsAngularVelocity(ClampedUnifiedAcceleration  * DeltaTime, true);
	}
	

	

	// Reset force and torque for next tick
	TickSumForce = FVector::ZeroVector;
	TickSumTorque = FVector::ZeroVector;
}

void AFlareShip::AddForceAtLocation(FVector LinearForce, FVector AngularForce, FVector applicationPoint)
{
	TickSumForce += LinearForce;
	FVector ApplicationOffset = (applicationPoint - COM) / 100; // TODO divise by 100 in parameter
	
	//ApplicationOffset.Normalize();
	/*UE_LOG(LogTemp, Warning, TEXT("AddForceAtLocation before: %s"), *TickSumTorque.ToString());
	UE_LOG(LogTemp, Warning, TEXT("ApplicationOffset: %s"), *ApplicationOffset.ToString());
	UE_LOG(LogTemp, Warning, TEXT("AngularForce: %s"), *AngularForce.ToString());*/
	

	TickSumTorque += FVector::CrossProduct(ApplicationOffset, AngularForce);
	/*UE_LOG(LogTemp, Warning, TEXT("AddForceAtLocation after: %s"), *TickSumTorque.ToString());*/
}

FVector AFlareShip::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}

FVector AFlareShip::GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, float ThurstAngleLimit, bool WithOrbitalEngines) const
{
	Axis.Normalize();
	FVector TotalMaxThrust = FVector::ZeroVector;
	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if(!WithOrbitalEngines && Engine->IsOrbitalEngine()) {
		  continue;
		}

		FVector WorldThurstAxis = Engine->GetThurstAxis();

		float dot = FVector::DotProduct(WorldThurstAxis, Axis);
		if (dot > ThurstAngleLimit) {
			float ratio = (dot - ThurstAngleLimit) / (1 - ThurstAngleLimit);

			TotalMaxThrust += WorldThurstAxis * Engine->GetMaxThrust() * ratio;
		}
		
	}

	return TotalMaxThrust;
}

FVector AFlareShip::GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueAxis, FVector COM, float ThurstAngleLimit, bool WithDamages, bool WithOrbitalEngines) const
{
	/*UE_LOG(LogTemp, Warning, TEXT("----"));*/
	TorqueAxis.Normalize();
	FVector TotalMaxTorque = FVector::ZeroVector;
	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if(!WithOrbitalEngines && Engine->IsOrbitalEngine()) {
		  continue;
		}

		float MaxThrust = (WithDamages ? Engine->GetMaxThrust() : Engine->GetInitialMaxThrust());
		
		if (MaxThrust == 0) {
			// Not controlable engine
			continue;
		}

		FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
		
		FVector WorldThurstAxis = Engine->GetThurstAxis();
		FVector TorqueDirection = FVector::CrossProduct(EngineOffset, WorldThurstAxis);
		TorqueDirection.Normalize();

		float dot = FVector::DotProduct(TorqueAxis, TorqueDirection);
		
		/*UE_LOG(LogTemp, Warning, TEXT("TotalMaxTorque before: %s"), *TotalMaxTorque.ToString());*/
		
		if (dot > ThurstAngleLimit) {
			float ratio = (dot - ThurstAngleLimit) / (1 - ThurstAngleLimit);

			/*UE_LOG(LogTemp, Warning, TEXT("EngineOffset : %s"), *EngineOffset.ToString());
			UE_LOG(LogTemp, Warning, TEXT("TorqueDirection : %s"), *TorqueDirection.ToString());
			UE_LOG(LogTemp, Warning, TEXT("TorqueAxis : %s"), *TorqueAxis.ToString());
			UE_LOG(LogTemp, Warning, TEXT("WorldThurstAxis : %s"), *WorldThurstAxis.ToString());
			UE_LOG(LogTemp, Warning, TEXT("MaxThrust : %f"), MaxThrust);
			UE_LOG(LogTemp, Warning, TEXT("ratio : %f"), ratio);
			UE_LOG(LogTemp, Warning, TEXT("dot : %f"), dot);*/
			TotalMaxTorque += TorqueAxis * FVector::CrossProduct(EngineOffset, WorldThurstAxis).Size() * MaxThrust * ratio;
		}
		/*UE_LOG(LogTemp, Warning, TEXT("TotalMaxTorque after: %s"), *TotalMaxTorque.ToString());*/
	}

	return TotalMaxTorque;


}

void AFlareShip::UpdateCOM() {
    COM = Airframe->GetBodyInstance()->GetCOMPosition();
}

/*----------------------------------------------------
	Autopilot
----------------------------------------------------*/

float* AFlareShip::ComputeLinearVelocityStabilisation(float DeltaSeconds, TArray<UActorComponent*>& Engines, FVector WorldTargetSpeed, float ThrustAngleLimit) const
{
	//FLOGV("ComputeLinearVelocityStabilisation WorldTargetSpeed=%s",*WorldTargetSpeed.ToString());
  
  	FVector WorldVelocity = GetLinearVelocity();
	
	//FLOGV("ComputeLinearVelocityStabilisation WorldVelocity=%s",*WorldVelocity.ToString());
	
	float* command = new float[Engines.Num()];
	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if(!ManualOrbitalBoost && Engine->IsOrbitalEngine()) {
		  Engine->SetTargetLinearThrustRatio(0.0);
		  continue;
		}

		FVector WorldThurstAxis = Engine->GetThurstAxis();
		
		float LocalTargetVelocity = FVector::DotProduct(WorldThurstAxis, WorldTargetSpeed);

		float TotalMaxThrustInAxis = FVector::DotProduct(WorldThurstAxis, GetTotalMaxThrustInAxis(Engines, WorldThurstAxis, ThrustAngleLimit, ManualOrbitalBoost));
				
		// Compute delta to stop
		float WorldVelocityToEnginesStop = FVector::DotProduct(WorldThurstAxis, WorldVelocity);
		//WorldVelocityToEnginesStop += FVector::DotProduct(WorldThurstAxis, getDeltaVToEnginesRatio(Engines, Mass, FinalThurstRatio, WorldThurstAxis, ThrustAngleLimit));

		// Check if air resistant won't make the estimation optimist.
		//WorldVelocityToEnginesStop += FVector::DotProduct(WorldThurstAxis, (getEngineToRatioDuration(Engine, FinalThurstRatio) * (-FinalThurst) / Mass)); // Assusme the air resistance will be almost constant during all the process. It's wrong, but it's better than noting

		float DeltaVelocityToStop = (LocalTargetVelocity - WorldVelocityToEnginesStop);
		
		/*FLOGV("ComputeLinearVelocityStabilisation for engine %d WorldThurstAxis=%s",i, *WorldThurstAxis.ToString());
		FLOGV("ComputeLinearVelocityStabilisation for engine %d LocalTargetVelocity=%f",i, LocalTargetVelocity);
		FLOGV("ComputeLinearVelocityStabilisation for engine %d TotalMaxThrustInAxis=%f",i, TotalMaxThrustInAxis);
		FLOGV("ComputeLinearVelocityStabilisation for engine %d WorldVelocityToEnginesStop=%f",i, WorldVelocityToEnginesStop);
		FLOGV("ComputeLinearVelocityStabilisation for engine %d DeltaVelocityToStop=%f",i, DeltaVelocityToStop);*/
		
		float ThrustAjustement = DeltaVelocityToStop * Airframe->GetMass() / (1.5*DeltaSeconds);
		float ThrustCommand = FMath::Clamp((ThrustAjustement / TotalMaxThrustInAxis), -1.0f, 1.0f);
	
		

		if (FMath::IsNearlyZero(ThrustCommand)) {
			ThrustCommand = 0;
		}
		
		/*FLOGV("ComputeLinearVelocityStabilisation for engine %d DeltaSeconds=%f",i, DeltaSeconds);
		FLOGV("ComputeLinearVelocityStabilisation for engine %d Mass=%f",i, Airframe->GetMass());
		FLOGV("ComputeLinearVelocityStabilisation for engine %d ThrustAjustement=%f",i, ThrustAjustement);
		
		FLOGV("ComputeLinearVelocityStabilisation for engine %d ThrustCommand=%f",i, ThrustCommand);*/
		Engine->SetTargetLinearThrustRatio(ThrustCommand);
		command[i] = ThrustCommand;
	}
	return command;
}

float* AFlareShip::ComputeAngularVelocityStabilisation(float DeltaSeconds, TArray<UActorComponent*>& Engines, FVector WorldTargetSpeed) const
{
	FVector AngularVelocity = Airframe->GetPhysicsAngularVelocity();
	/*FLOG("=======================================================");
	FLOGV("ComputeAngularVelocityStabilisation WorldTargetSpeed=%s", *WorldTargetSpeed.ToString());
	FLOGV("ComputeAngularVelocityStabilisation AngularVelocity=%s", *AngularVelocity.ToString());*/
	
	float* command = new float[Engines.Num()];
	int index = 0;
	for (int32 i = 0; i < Engines.Num(); i++) {
		UFlareEngine* Engine = Cast<UFlareEngine>(Engines[i]);

		if(Engine->IsOrbitalEngine()) {
		  Engine->SetTargetAngularThrustRatio(0.0);
		  continue;
		}

		FVector EngineOffset = (Engine->GetComponentLocation() - COM) / 100;
		
		FVector WorldThurstAxis = Engine->GetThurstAxis();
		FVector TorqueDirection = FVector::CrossProduct(EngineOffset, WorldThurstAxis); 
		if (TorqueDirection.Size() < 0.001) {
			command[index++] = 0;
			continue;
		}
		TorqueDirection.Normalize();

		float LocalTargetVelocity = FVector::DotProduct(TorqueDirection, WorldTargetSpeed);

		float TotalMaxTorqueInAxis = FVector::DotProduct(TorqueDirection, GetTotalMaxTorqueInAxis(Engines, TorqueDirection, COM, 0, true, false));
		if (FMath::IsNearlyZero(TotalMaxTorqueInAxis)) {
			// Just wait better days
			command[index++] = 0;
			continue;
		}
		
		// Compute delta to stop
		float WorldVelocityToEnginesTarget = FVector::DotProduct(TorqueDirection, AngularVelocity);
		//WorldVelocityToEnginesStop += FVector::DotProduct(TorqueDirection, getDeltaAngularVelocityToEnginesRatio(Engines, COM, InertiaTensor, FinalThurstRatio)); // TODO inertia

		// Check if air resistant won't make the estimation optimist.
		//WorldVelocityToEnginesStop += FVector::DotProduct(TorqueDirection, (getEngineToRatioDuration(Engine, FinalThurstRatio) * (-FinalTorque) / InertiaTensor)); // Assusme the air resistance will be almost constant during all the process. It's wrong, but it's better than noting

		float DeltaVelocityToTarget = (LocalTargetVelocity - WorldVelocityToEnginesTarget);
		float AccelerationToTarget = DeltaVelocityToTarget / DeltaSeconds;
		
		FVector LocalAccelerationToTarget = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(AccelerationToTarget * TorqueDirection);
		FVector LocalTorqueToTarget = LocalAccelerationToTarget * LocalInertiaTensor;
		
		FVector TorqueToTarget = Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalTorqueToTarget);
		
		float TorqueToTargetInAxis = FVector::DotProduct(TorqueDirection, TorqueToTarget);
		float ThrustCommand = FMath::Clamp((TorqueToTargetInAxis / TotalMaxTorqueInAxis), -1.0f, 1.0f);
		
		/*FLOG("================");
		FLOGV("ComputeAngularVelocityStabilisation for engine %d DeltaSeconds=%f",i, DeltaSeconds);
		FLOGV("ComputeAngularVelocityStabilisation for engine %d TotalMaxTorqueInAxis=%f",i, TotalMaxTorqueInAxis);
		FLOGV("ComputeAngularVelocityStabilisation for engine %d WorldThurstAxis=%s",i, *WorldThurstAxis.ToString());
		FLOGV("ComputeAngularVelocityStabilisation for engine %d DeltaVelocityToTarget=%f",i, DeltaVelocityToTarget);
		FLOGV("ComputeAngularVelocityStabilisation for engine %d AccelerationToTarget=%f",i, AccelerationToTarget);
		FLOGV("ComputeAngularVelocityStabilisation for engine %d LocalAccelerationToTarget=%s",i, *LocalAccelerationToTarget.ToString());
		FLOGV("ComputeAngularVelocityStabilisation for engine %d LocalTorqueToTarget=%s",i, *LocalTorqueToTarget.ToString());
		FLOGV("ComputeAngularVelocityStabilisation for engine %d TorqueToTarget=%s",i, *TorqueToTarget.ToString());
		FLOGV("ComputeAngularVelocityStabilisation for engine %d TorqueToTargetInAxis=%f",i, TorqueToTargetInAxis);
		FLOGV("ComputeAngularVelocityStabilisation for engine %d ThrustCommand=%f",i, ThrustCommand);*/
		
		
		
		if (FMath::IsNearlyZero(ThrustCommand)) {
			ThrustCommand = 0;
		}
		Engine->SetTargetAngularThrustRatio(ThrustCommand);
		command[index++] = ThrustCommand;
	}

	return command;
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
	if (!ExternalCamera && CanMoveVertical)
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