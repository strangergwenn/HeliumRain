
#include "../Flare.h"

#include "FlareSpacecraft.h"
#include "FlareAirframe.h"
#include "FlareOrbitalEngine.h"
#include "FlareRCS.h"
#include "FlareWeapon.h"
#include "FlareInternalComponent.h"

#include "Particles/ParticleSystemComponent.h"

#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"


#define LOCTEXT_NAMESPACE "FlareSpacecraft"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareSpacecraft::AFlareSpacecraft(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, AngularInputDeadRatio(0.0025)
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

	// Pilot
	IsPiloted = true;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareSpacecraft::BeginPlay()
{
	Super::BeginPlay();
}

void AFlareSpacecraft::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());

	// Update Camera
	if (!ExternalCamera && CombatMode)
	{
		//TODO Only for played ship, in Ship class
		if (CombatMode)
		{
			TArray<UFlareWeapon*> Weapons = GetWeaponList();
			if (Weapons.Num() > 0)
			{
				float AmmoVelocity = Weapons[0]->GetAmmoVelocity();
				FRotator ShipAttitude = GetActorRotation();
				FVector ShipVelocity = 100.f * GetLinearVelocity();

				// Bullet velocity
				FVector BulletVelocity = ShipAttitude.Vector();
				BulletVelocity.Normalize();
				BulletVelocity *= 100.f * AmmoVelocity; // TODO get from projectile


				FVector BulletDirection = (ShipVelocity + BulletVelocity).GetUnsafeNormal();

				FVector LocalBulletDirection = Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(BulletDirection);

				float Pitch = FMath::RadiansToDegrees(FMath::Asin(LocalBulletDirection.Z));
				float Yaw = FMath::RadiansToDegrees(FMath::Asin(LocalBulletDirection.Y));

				SetCameraPitch(Pitch);
				SetCameraYaw(Yaw);
			}
		}
		else
		{
			SetCameraPitch(0);
			SetCameraYaw(0);
		}
	}


	if (!IsPresentationMode())
	{

		if (GetDamageSystem()->IsAlive() && IsPiloted) // Do not tick the pilot if a player has disable the pilot
		{
			Pilot->TickPilot(DeltaSeconds);
		}

		DockingSystem->TickSystem(DeltaSeconds);
		NavigationSystem->TickSystem(DeltaSeconds);
		WeaponsSystem->TickSystem(DeltaSeconds);
		DamageSystem->TickSystem(DeltaSeconds);
	}
}

void AFlareSpacecraft::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	DamageSystem->OnCollision(Other, HitLocation, HitNormal);
}

void AFlareSpacecraft::Destroyed()
{
	if (Company)
	{
		Company->Unregister(this);
	}
}


/*----------------------------------------------------
	Player interface
----------------------------------------------------*/

void AFlareSpacecraft::SetExternalCamera(bool NewState)
{
	// Stop firing
	if (NewState)
	{
		StopFire();
		BoostOff();
	}

	// Reset rotations
	ExternalCamera = NewState;
	SetCameraPitch(0);
	SetCameraYaw(0);

	// Reset controls
	PlayerManualLinearVelocity = FVector::ZeroVector;
	PlayerManualAngularVelocity = FVector::ZeroVector;

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

void AFlareSpacecraft::SetCombatMode(bool NewState)
{
	CombatMode = NewState;
	PlayerMouseOffset = FVector2D(0,0);
	MousePositionInput(PlayerMouseOffset);

	if (!NewState && FiringPressed)
	{
		StopFire();
	}
}

// TODO move in helper class


FVector AFlareSpacecraft::GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay) const
{
	return GetAimPosition(TargettingShip->GetActorLocation(), TargettingShip->GetLinearVelocity() * 100, BulletSpeed, PredictionDelay);
}


FVector AFlareSpacecraft::GetAimPosition(FVector GunLocation, FVector GunVelocity, float BulletSpeed, float PredictionDelay) const
{
	//Relative Target Speed
	FVector TargetVelocity = Airframe->GetPhysicsLinearVelocity();
	FVector TargetLocation = GetActorLocation() + TargetVelocity * PredictionDelay;
	FVector BulletLocation = GunLocation + GunVelocity * PredictionDelay;

	// Find the relative speed in the axis of target
	FVector TargetDirection = (TargetLocation - BulletLocation).GetUnsafeNormal();
	FVector BonusVelocity = GunVelocity;
	float BonusVelocityInTargetAxis = FVector::DotProduct(TargetDirection, BonusVelocity);
	float EffectiveBulletSpeed = BulletSpeed * 100.f + BonusVelocityInTargetAxis;

	float Divisor = FMath::Square(EffectiveBulletSpeed) - TargetVelocity.SizeSquared();

	float A = -1;
	float B = 2 * (TargetVelocity.X * (TargetLocation.X - BulletLocation.X) + TargetVelocity.Y * (TargetLocation.Y - BulletLocation.Y) + TargetVelocity.Z * (TargetLocation.Z - BulletLocation.Z)) / Divisor;
	float C = (TargetLocation - BulletLocation).SizeSquared() / Divisor;

	float Delta = FMath::Square(B) - 4 * A * C;

	float InterceptTime1 = (- B - FMath::Sqrt(Delta)) / (2 * A);
	float InterceptTime2 = (- B + FMath::Sqrt(Delta)) / (2 * A);

	float InterceptTime = FMath::Max(InterceptTime1, InterceptTime2);

	FVector InterceptLocation = TargetLocation + TargetVelocity * InterceptTime;

	return InterceptLocation;
}

/*----------------------------------------------------
	Ship interface
----------------------------------------------------*/

void AFlareSpacecraft::Load(const FFlareSpacecraftSave& Data)
{
	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();

	// Update local data
	ShipData = Data;
	ShipData.Name = FName(*GetName());

		// Load ship description
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(Data.Identifier);
	SetShipDescription(Desc);

	// Initialize damage system
	DamageSystem = NewObject<UFlareSpacecraftDamageSystem>(this, UFlareSpacecraftDamageSystem::StaticClass());
	DamageSystem->Initialize(this, &ShipData);

	// Initialize navigation system
	NavigationSystem = NewObject<UFlareSpacecraftNavigationSystem>(this, UFlareSpacecraftNavigationSystem::StaticClass());
	NavigationSystem->Initialize(this, &ShipData);

	// Initialize docking system
	DockingSystem = NewObject<UFlareSpacecraftDockingSystem>(this, UFlareSpacecraftDockingSystem::StaticClass());
	DockingSystem->Initialize(this, &ShipData);

	// Initialize weapons system
	WeaponsSystem = NewObject<UFlareSpacecraftWeaponsSystem>(this, UFlareSpacecraftWeaponsSystem::StaticClass());
	WeaponsSystem->Initialize(this, &ShipData);

	// Look for parent company
	SetOwnerCompany(GetGame()->FindCompany(Data.CompanyIdentifier));

	// Initialize components
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		FFlareSpacecraftComponentSave ComponentData;

		// Find component the corresponding component data comparing the slot id
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

		// If no data, this is a cosmetic component and it don't need to be initialized
		if (!found)
		{
			continue;
		}

		// Reload the component
		ReloadPart(Component, &ComponentData);

		// Set RCS description
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData.ComponentIdentifier);
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

		// Find the cockpit
		if(ComponentDescription->GeneralCharacteristics.LifeSupport)
		{
			ShipCockit = Component;
		}
	}

	// Load weapon descriptions
	for (int32 i = 0; i < Data.Components.Num(); i++)
	{
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(Data.Components[i].ComponentIdentifier);
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
		FLOGV("AFlareSpacecraft::Load : Looking for station '%s'", *ShipData.DockedTo.ToString());
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* Station = Cast<AFlareSpacecraft>(*ActorItr);
			if (Station && *Station->GetName() == ShipData.DockedTo)
			{
				FLOGV("AFlareSpacecraft::Load : Found dock station '%s'", *Station->GetName());
				NavigationSystem->ConfirmDock(Station, ShipData.DockedAt);
				break;
			}
		}
	}

	// Initialize pilot
	Pilot = NewObject<UFlareShipPilot>(this, UFlareShipPilot::StaticClass());
	Pilot->Initialize(&ShipData.Pilot, GetCompany(), this);

	DamageSystem->Start();
	NavigationSystem->Start();
	DockingSystem->Start();
	WeaponsSystem->Start();
}

FFlareSpacecraftSave* AFlareSpacecraft::Save()
{
	// Physical data
	ShipData.Location = GetActorLocation();
	ShipData.Rotation = GetActorRotation();
	ShipData.LinearVelocity = Airframe->GetPhysicsLinearVelocity();
	ShipData.AngularVelocity = Airframe->GetPhysicsAngularVelocity();

	// Save all components datas
	ShipData.Components.Empty();
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		FFlareSpacecraftComponentSave* ComponentSave = Component->Save();

		if (ComponentSave) {
			ShipData.Components.Add(*ComponentSave);
		}
	}

	return &ShipData;
}

void AFlareSpacecraft::SetOwnerCompany(UFlareCompany* NewCompany)
{
	SetCompany(NewCompany);
	ShipData.CompanyIdentifier = NewCompany->GetIdentifier();
	Airframe->Initialize(NULL, Company, this);
	NewCompany->Register(this);
}

UFlareCompany* AFlareSpacecraft::GetCompany()
{
	return Company;
}

bool AFlareSpacecraft::IsMilitary()
{
	return IFlareSpacecraftInterface::IsMilitary(ShipDescription);
}

bool AFlareSpacecraft::IsStation()
{
	return IFlareSpacecraftInterface::IsStation(ShipDescription);
}

UFlareSpacecraftDamageSystem* AFlareSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}


UFlareSpacecraftNavigationSystem* AFlareSpacecraft::GetNavigationSystem() const
{
	return NavigationSystem;
}

UFlareSpacecraftWeaponsSystem* AFlareSpacecraft::GetWeaponsSystem() const
{
	return WeaponsSystem;
}

UFlareSpacecraftDockingSystem* AFlareSpacecraft::GetDockingSystem() const
{
	return DockingSystem;
}

UFlareInternalComponent* AFlareSpacecraft::GetInternalComponentAtLocation(FVector Location) const
{
	float MinDistance = 100000; // 1km
	UFlareInternalComponent* ClosestComponent = NULL;

	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareInternalComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareInternalComponent* InternalComponent = Cast<UFlareInternalComponent>(Components[ComponentIndex]);

		FVector ComponentLocation;
		float ComponentSize;
		InternalComponent->GetBoundingSphere(ComponentLocation, ComponentSize);

		float Distance = (ComponentLocation - Location).Size() - ComponentSize;
		if(Distance < MinDistance)
		{
			ClosestComponent = InternalComponent;
			MinDistance = Distance;
		}
	}
	return ClosestComponent;
}

/*----------------------------------------------------
		Pilot
----------------------------------------------------*/

void AFlareSpacecraft::EnablePilot(bool PilotEnabled)
{
	FLOGV("EnablePilot %d", PilotEnabled);
	IsPiloted = PilotEnabled;
	BoostOff();
}



/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlareSpacecraft::SetShipDescription(FFlareSpacecraftDescription* Description)
{
	ShipDescription = Description;
}

void AFlareSpacecraft::SetOrbitalEngineDescription(FFlareSpacecraftComponentDescription* Description)
{
	OrbitalEngineDescription = Description;
}

void AFlareSpacecraft::SetRCSDescription(FFlareSpacecraftComponentDescription* Description)
{
	RCSDescription = Description;

	// Find the RCS turn and power rating, since RCSs themselves don't do anything
	if (Description)
	{
		if (Airframe && Description->EngineCharacteristics.AngularAccelerationRate > 0)
		{
			float Mass = Airframe->GetMass() / 100000;
			NavigationSystem->SetAngularAccelerationRate(Description->EngineCharacteristics.AngularAccelerationRate / (60 * Mass));
		}
	}
}

void AFlareSpacecraft::UpdateCustomization()
{
	Super::UpdateCustomization();

	Airframe->UpdateCustomization();
}

void AFlareSpacecraft::StartPresentation()
{
	Super::StartPresentation();

	if (Airframe)
	{
		Airframe->SetSimulatePhysics(false);
	}
}



/*----------------------------------------------------
		Damage system
----------------------------------------------------*/

void AFlareSpacecraft::OnEnemyKilled(IFlareSpacecraftInterface* Enemy)
{
	AFlarePlayerController* PC = GetPC();
	if (PC)
	{
		PC->Notify(LOCTEXT("ShipKilled", "Target destroyed"), EFlareNotification::NT_Military);
	}
}

void AFlareSpacecraft::OnDocked()
{

	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (PC && !ExternalCamera)
	{
		PC->SetExternalCamera(true);
	}

	// Reload and repair
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->Repair();

		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Components[ComponentIndex]);
		if (Weapon)
		{
			Weapon->RefillAmmo();
		}
	}

	DamageSystem->UpdatePower();
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareSpacecraft::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("Thrust", this, &AFlareSpacecraft::ThrustInput);
	InputComponent->BindAxis("MoveVerticalInput", this, &AFlareSpacecraft::MoveVerticalInput);
	InputComponent->BindAxis("MoveHorizontalInput", this, &AFlareSpacecraft::MoveHorizontalInput);

	InputComponent->BindAxis("RollInput", this, &AFlareSpacecraft::RollInput);
	InputComponent->BindAxis("PitchInput", this, &AFlareSpacecraft::PitchInput);
	InputComponent->BindAxis("YawInput", this, &AFlareSpacecraft::YawInput);
	InputComponent->BindAxis("MouseInputY", this, &AFlareSpacecraft::PitchInput);
	InputComponent->BindAxis("MouseInputX", this, &AFlareSpacecraft::YawInput);

	InputComponent->BindAction("ZoomIn", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomIn);
	InputComponent->BindAction("ZoomOut", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomOut);

	InputComponent->BindAction("FaceForward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceForward);
	InputComponent->BindAction("FaceBackward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceBackward);
	InputComponent->BindAction("Brake", EInputEvent::IE_Released, this, &AFlareSpacecraft::Brake);
	InputComponent->BindAction("Boost", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::BoostOn);
	InputComponent->BindAction("Boost", EInputEvent::IE_Released, this, &AFlareSpacecraft::BoostOff);
	InputComponent->BindAction("Manual", EInputEvent::IE_Released, this, &AFlareSpacecraft::ForceManual);

	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::FirePress);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AFlareSpacecraft::FireRelease);
}

void AFlareSpacecraft::FirePress()
{
	FiringPressed = true;

	if (CombatMode)
	{
		StartFire();
	}
}

void AFlareSpacecraft::FireRelease()
{
	FiringPressed = false;

	if (CombatMode)
	{
		StopFire();
	}
}

void AFlareSpacecraft::MousePositionInput(FVector2D Val)
{
	if (!ExternalCamera)
	{
		if (FiringPressed  || CombatMode)
		{
			float DistanceToCenter = FMath::Sqrt(FMath::Square(Val.X) + FMath::Square(Val.Y));

			// Compensation curve = 1 + (input-1)/(1-AngularInputDeadRatio)
			float CompensatedDistance = FMath::Clamp(1. + (DistanceToCenter - 1. ) / (1. - AngularInputDeadRatio) , 0., 1.);
			float Angle = FMath::Atan2(Val.Y, Val.X);

			PlayerManualAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * NavigationSystem->GetAngularMaxVelocity();
			PlayerManualAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * NavigationSystem->GetAngularMaxVelocity();
		}
		else
		{
			PlayerManualAngularVelocity.Z = 0;
			PlayerManualAngularVelocity.Y = 0;
		}
	}
}

void AFlareSpacecraft::ThrustInput(float Val)
{
	if (!ExternalCamera)
	{
		PlayerManualLinearVelocity.X = Val * NavigationSystem->GetLinearMaxVelocity();
	}
}

void AFlareSpacecraft::MoveVerticalInput(float Val)
{
	if (!ExternalCamera)
	{
		PlayerManualLinearVelocity.Z = NavigationSystem->GetLinearMaxVelocity() * Val;
	}
}

void AFlareSpacecraft::MoveHorizontalInput(float Val)
{
	if (!ExternalCamera)
	{
		PlayerManualLinearVelocity.Y = NavigationSystem->GetLinearMaxVelocity() * Val;
	}
}

void AFlareSpacecraft::RollInput(float Val)
{
	if (!ExternalCamera)
	{
		PlayerManualAngularVelocity.X = - Val * NavigationSystem->GetAngularMaxVelocity();
	}
}

void AFlareSpacecraft::PitchInput(float Val)
{
	if (ExternalCamera)
	{
		FRotator CurrentRot = WorldToLocal(CameraContainerPitch->GetComponentRotation().Quaternion()).Rotator();
		SetCameraPitch(CurrentRot.Pitch + Val * CameraPanSpeed);
	}
	else if (CombatMode)
	{
		PlayerMouseOffset.Y -= FMath::Sign(Val) * FMath::Pow(FMath::Abs(Val),1.3) * 0.05; // TODO Config sensibility
		if(PlayerMouseOffset.Size() > 1)
		{
			PlayerMouseOffset /= PlayerMouseOffset.Size();
		}
		MousePositionInput(PlayerMouseOffset);

	}
}

void AFlareSpacecraft::YawInput(float Val)
{
	if (ExternalCamera)
	{
		FRotator CurrentRot = WorldToLocal(CameraContainerPitch->GetComponentRotation().Quaternion()).Rotator();
		SetCameraYaw(CurrentRot.Yaw + Val * CameraPanSpeed);
	}
	else if (CombatMode)
	{
		PlayerMouseOffset.X += FMath::Sign(Val) * FMath::Pow(FMath::Abs(Val),1.3) * 0.05; // TODO Config sensibility
		if(PlayerMouseOffset.Size() > 1)
		{
			PlayerMouseOffset /= PlayerMouseOffset.Size();
		}
		MousePositionInput(PlayerMouseOffset);
	}
}

void AFlareSpacecraft::ZoomIn()
{
	if (ExternalCamera)
	{
		StepCameraDistance(true);
	}
}

void AFlareSpacecraft::ZoomOut()
{
	if (ExternalCamera)
	{
		StepCameraDistance(false);
	}
}


void AFlareSpacecraft::StartFire()
{
	if (GetDamageSystem()->IsAlive() && (IsPiloted || !ExternalCamera))
	{
		for (int32 i = 0; i < WeaponList.Num(); i++)
		{
			if (!WeaponList[i]->IsTurret())
			{
				WeaponList[i]->StartFire();
			}
		}
	}
}

void AFlareSpacecraft::StopFire()
{
	if (GetDamageSystem()->IsAlive() && (IsPiloted || !ExternalCamera))
	{
		for (int32 i = 0; i < WeaponList.Num(); i++)
		{
			if (!WeaponList[i]->IsTurret())
			{
				WeaponList[i]->StopFire();
			}
		}
	}
}

void AFlareSpacecraft::FaceForward()
{
	if (NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation(Airframe->GetPhysicsLinearVelocity(), FVector(1,0,0));
	}
}

void AFlareSpacecraft::FaceBackward()
{
	if (NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation((-Airframe->GetPhysicsLinearVelocity()), FVector(1, 0, 0));
	}
}

void AFlareSpacecraft::Brake()
{
	if (NavigationSystem->IsManualPilot())
	{
		// TODO
	}
}

void AFlareSpacecraft::BoostOn()
{
	if (NavigationSystem->IsManualPilot() && !ExternalCamera)
	{
		PlayerManualOrbitalBoost = true;
	}
}

void AFlareSpacecraft::BoostOff()
{
	PlayerManualOrbitalBoost = false;
}

void AFlareSpacecraft::ForceManual()
{
	if (NavigationSystem->GetStatus() != EFlareShipStatus::SS_Docked)
	{
		NavigationSystem->AbortAllCommands();
	}
}

/*----------------------------------------------------
		Getters
----------------------------------------------------*/

FVector AFlareSpacecraft::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}


#undef LOCTEXT_NAMESPACE
