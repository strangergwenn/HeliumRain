
#include "../Flare.h"

#include "FlareSpacecraft.h"
#include "FlareOrbitalEngine.h"
#include "FlareRCS.h"
#include "FlareWeapon.h"
#include "FlareInternalComponent.h"

#include "Particles/ParticleSystemComponent.h"

#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareAsteroid.h"


#define LOCTEXT_NAMESPACE "FlareSpacecraft"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareSpacecraft::AFlareSpacecraft(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Create static mesh component
	Airframe = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("Airframe"));
	Airframe->SetSimulatePhysics(true);
	RootComponent = Airframe;

	// Camera settings
	CameraContainerYaw->AttachTo(Airframe);
	CameraMaxPitch = 80;
	CameraPanSpeed = 2;

	// Dock info
	ShipData.DockedTo = NAME_None;
	ShipData.DockedAt = -1;

	// Asteroid info
	ShipData.AsteroidData.Identifier = NAME_None;
	ShipData.AsteroidData.AsteroidMeshID = 0;
	ShipData.AsteroidData.Scale = FVector(1, 1, 1);

	Paused = false;
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
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());

	if (!IsPresentationMode() && StateManager)
	{
		StateManager->Tick(DeltaSeconds);

		DockingSystem->TickSystem(DeltaSeconds);
		NavigationSystem->TickSystem(DeltaSeconds);
		WeaponsSystem->TickSystem(DeltaSeconds);
		DamageSystem->TickSystem(DeltaSeconds);

		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
			if (PlayerShip && !GetDamageSystem()->IsAlive())
			{
				float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
				if (Distance > 500000)
				{
					// 5 km
					if (Company)
					{
						GetGame()->GetActiveSector()->DestroySpacecraft(this);
					}
				}
			}
		}

		float SmoothedVelocityChangeSpeed = FMath::Clamp(DeltaSeconds * 8, 0.f, 1.f);
		SmoothedVelocity = SmoothedVelocity * (1 - SmoothedVelocityChangeSpeed) + GetLinearVelocity() * SmoothedVelocityChangeSpeed;
	}

	// The FlareSpacecraftPawn do the camera effective update in its Tick so call it after camera order update
	Super::Tick(DeltaSeconds);
}

void AFlareSpacecraft::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// ghoul 10m/s -> asteroid : 5919376.500000
	// ghoul 10m/s -> outpost  : 8190371.000000
	// ghoul 10m/s -> hub : 4157000.750000 + 4161034.500000 =                                                        8318035.25
	// ghoul 10m/s -> asteroid 0 : 5320989.500000 + 1186121.500000 + 920986.437500 =                                 7428097.4375

	// ghoul 20m/s -> asteroid 0 : 13991330.000000 + 1485886.250000 =                                               15477216.25
	// ghoul 20m/s -> outpost : 59829.214844 + 159081.968750  + 513187.187500  + 1729557.875000 + 16334147.000000 = 18795803.246094
	// ghoul 20m/s -> hub : 9352049.000000 8732764.000000 =                                                         18084813.0

	// orca 10m/s -> hub : 8604104.000000 + 12141461.000000 =                                                       20745565.0
	// dragon 10m/s -> hub :7552520.500000 + 148669488.000000 =                                                    156222008.5
	// invader 10m/s -> hub :                                                                                      425907776.000000

	// ghoul 6301.873047            13 19.930628237551
	// orca 19790.914062            10 48.2368290322174
	// dragon 136862.984375         11 41.4482092543008
	// invader 530312.250000        8 03.1264146736191


   // ghoul 20m/s	                                2455.970812449119
   // ghoul 50m/s	20529212.000000 + 20547770.000000    =  41076982.0 / 6301.873047 65.18 m/s

   // ghoul 100 m/s 40483100.000000 + 40519540.000000                    =  81002640.0 / 6301.873047 128 53 m/s

   // 2190338.000000  + 394164960.000000 + 142830160.000000 + 482529472.000000 = 1021714930.0 / 530312.250000 =


	// DrawDebugSphere(GetWorld(), Hit.Location, 10, 12, FColor::Blue, true);

	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (!Other)
	{
		// Colliding a kill pending actor
		return;
	}

	//FLOGV("AFlareSpacecraft Hit  Mass %f NormalImpulse %s NormalImpulse.Size() %f", Airframe->GetMass(), *NormalImpulse.ToString(), NormalImpulse.Size());
	DamageSystem->OnCollision(Other, HitLocation, NormalImpulse);

	// If hit, check if the is a docking in progress. If yes, check if the ship is correctly aligned
	AFlareSpacecraft* OtherSpacecraft = Cast<AFlareSpacecraft>(Other);
	if (OtherSpacecraft)
	{
		// The other actor is a spacecraft, check if it's not the station we want to dock to.
		GetNavigationSystem()->CheckCollisionDocking(OtherSpacecraft);
	}
}

void AFlareSpacecraft::Destroyed()
{
}

void AFlareSpacecraft::SetPause(bool Pause)
{
	if (Paused == Pause)
	{
		return;
	}

	CustomTimeDilation = (Pause ? 0.f : 1.0);
	if (Pause)
	{
		Save(); // Save must be performed with the old pause state
		FLOGV("%s save linear velocity : %s", *GetImmatriculation().ToString(), *ShipData.LinearVelocity.ToString());

	}

	Airframe->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		FLOGV("%s restore linear velocity : %s", *GetImmatriculation().ToString(), *ShipData.LinearVelocity.ToString());
		Airframe->SetPhysicsLinearVelocity(ShipData.LinearVelocity);
		Airframe->SetPhysicsAngularVelocity(ShipData.AngularVelocity);
		SmoothedVelocity = GetLinearVelocity();
	}
}


void AFlareSpacecraft::Redock()
{
	// Re-dock if we were docked
	if (ShipData.DockedTo != NAME_None && !IsPresentationMode())
	{
		// TODO use sector iterator
		FLOGV("AFlareSpacecraft::Redock : Looking for station '%s'", *ShipData.DockedTo.ToString());

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Station = GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

			if (Station->GetImmatriculation() == ShipData.DockedTo)
			{
				FLOGV("AFlareSpacecraft::Redock : Found dock station '%s'", *Station->GetImmatriculation().ToString());
				NavigationSystem->ConfirmDock(Station, ShipData.DockedAt);
				break;
			}
		}
	}
}

/*----------------------------------------------------
	Player interface
----------------------------------------------------*/


// TODO move in helper class


float AFlareSpacecraft::GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const
{
	return GetAimPosition(TargettingShip->GetActorLocation(), TargettingShip->GetLinearVelocity() * 100, BulletSpeed, PredictionDelay, ResultPosition);
}


float AFlareSpacecraft::GetAimPosition(FVector GunLocation, FVector GunVelocity, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const
{
	// TODO : use helper

	// Relative Target Speed
	FVector TargetVelocity = Airframe->GetPhysicsLinearVelocity();
	FVector TargetLocation = GetActorLocation() + TargetVelocity * PredictionDelay;
	FVector BulletLocation = GunLocation + GunVelocity * PredictionDelay;

	// Find the relative speed in the axis of target
	FVector TargetDirection = (TargetLocation - BulletLocation).GetUnsafeNormal();
	FVector BonusVelocity = GunVelocity;
	float BonusVelocityInTargetAxis = FVector::DotProduct(TargetDirection, BonusVelocity);
	float EffectiveBulletSpeed = BulletSpeed * 100.f + BonusVelocityInTargetAxis;

	float Divisor = FMath::Square(EffectiveBulletSpeed) - TargetVelocity.SizeSquared();

	if (EffectiveBulletSpeed < 0 || FMath::IsNearlyZero(Divisor))
	{
		// Intersect at an infinite time
		return -1;
	}

	float A = -1;
	float B = 2 * (TargetVelocity.X * (TargetLocation.X - BulletLocation.X) + TargetVelocity.Y * (TargetLocation.Y - BulletLocation.Y) + TargetVelocity.Z * (TargetLocation.Z - BulletLocation.Z)) / Divisor;
	float C = (TargetLocation - BulletLocation).SizeSquared() / Divisor;

	float Delta = FMath::Square(B) - 4 * A * C;

	float InterceptTime1 = (- B - FMath::Sqrt(Delta)) / (2 * A);
	float InterceptTime2 = (- B + FMath::Sqrt(Delta)) / (2 * A);

	float InterceptTime;
	if (InterceptTime1 > 0 && InterceptTime2 > 0)
	{
		InterceptTime = FMath::Min(InterceptTime1, InterceptTime2);
	}
	else
	{
		InterceptTime = FMath::Max(InterceptTime1, InterceptTime2);
	}

	if (InterceptTime > 0)
	{
		FVector InterceptLocation = TargetLocation + TargetVelocity * InterceptTime;
		*ResultPosition = InterceptLocation;
	}
	return InterceptTime;
}

/*----------------------------------------------------
	Ship interface
----------------------------------------------------*/

void AFlareSpacecraft::Load(const FFlareSpacecraftSave& Data)
{
	if (!IsPresentationMode())
	{
		Airframe->SetSimulatePhysics(true);
	}

	// Update local data
	ShipData = Data;
	ShipData.SpawnMode = EFlareSpawnMode::Safe;

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
	SetOwnerCompany(GetGame()->GetGameWorld()->FindCompany(Data.CompanyIdentifier));

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


		// Find the cockpit
		if (ComponentDescription->GeneralCharacteristics.LifeSupport)
		{
			ShipCockit = Component;
		}
	}

	// Look for an asteroid component
	ApplyAsteroidData();

	// Customization
	UpdateCustomization();
	Redock();

	// If not rcs, add passive stabilization
	if (ShipDescription->RCSCount == 0)
	{
		Airframe->SetLinearDamping(0.1);
		Airframe->SetAngularDamping(0.1);
	}

	// Initialize pilot
	Pilot = NewObject<UFlareShipPilot>(this, UFlareShipPilot::StaticClass());
	Pilot->Initialize(&ShipData.Pilot, GetCompany(), this);
	if (!StateManager)
	{
		StateManager = NewObject<UFlareSpacecraftStateManager>(this, UFlareSpacecraftStateManager::StaticClass());
	}
	StateManager->Initialize(this);

	// Subsystems
	DamageSystem->Start();
	NavigationSystem->Start();
	DockingSystem->Start();
	WeaponsSystem->Start();
	SmoothedVelocity = GetLinearVelocity();

	if (IsPaused())
	{
		Airframe->SetSimulatePhysics(false);
	}
}

FFlareSpacecraftSave* AFlareSpacecraft::Save()
{
	// Physical data
	ShipData.Location = GetActorLocation();
	ShipData.Rotation = GetActorRotation();
	if (!IsPaused())
	{
		ShipData.LinearVelocity = Airframe->GetPhysicsLinearVelocity();
		ShipData.AngularVelocity = Airframe->GetPhysicsAngularVelocity();
	}

	// Save all components datas
	ShipData.Components.Empty();
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		FFlareSpacecraftComponentSave* ComponentSave = Component->Save();

		if (ComponentSave)
		{
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
}

UFlareCompany* AFlareSpacecraft::GetCompany()
{
	return Company;
}

EFlarePartSize::Type AFlareSpacecraft::GetSize()
{
	return ShipDescription->Size;
}


bool AFlareSpacecraft::IsMilitary() const
{
	return IFlareSpacecraftInterface::IsMilitary(ShipDescription);
}

bool AFlareSpacecraft::IsStation() const
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

void AFlareSpacecraft::SetAsteroidData(FFlareAsteroidSave* Data)
{
	// Copy data
	ShipData.AsteroidData.Identifier = Data->Identifier;
	ShipData.AsteroidData.AsteroidMeshID = Data->AsteroidMeshID;
	ShipData.AsteroidData.Scale = Data->Scale;

	// Apply
	ApplyAsteroidData();
	SetActorLocation(Data->Location);
	SetActorRotation(Data->Rotation);
}

void AFlareSpacecraft::ApplyAsteroidData()
{
	if (ShipData.AsteroidData.Identifier != NAME_None)
	{
		TArray<UActorComponent*> Components = GetComponentsByClass(UActorComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UActorComponent* Component = Components[ComponentIndex];
			if (Component->GetName().Contains("Asteroid"))
			{
				FLOGV("AFlareSpacecraft::ApplyAsteroidData : Found asteroid component '%s', previously set from '%s'",
					*Component->GetName(), *ShipData.AsteroidData.Identifier.ToString());

				UStaticMeshComponent* AsteroidComponentCandidate = Cast<UStaticMeshComponent>(Component);
				AFlareAsteroid::SetupAsteroidMesh(GetGame(), AsteroidComponentCandidate, ShipData.AsteroidData);
				break;
			}
		}
	}
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
		if (Distance < MinDistance)
		{
			ClosestComponent = InternalComponent;
			MinDistance = Distance;
		}
	}
	return ClosestComponent;
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
		if (Airframe && Description->EngineCharacteristics.AngularAccelerationRate > 0 && !IsPresentationMode())
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

	// Customize decal materials
	TArray<UActorComponent*> Components = GetComponentsByClass(UDecalComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UDecalComponent* Component = Cast<UDecalComponent>(Components[ComponentIndex]);
		if (Component)
		{
			if (!DecalMaterial)
			{
				DecalMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), GetWorld());
			}
			if (Company && DecalMaterial)
			{
				Company->CustomizeComponentMaterial(DecalMaterial);
				Component->SetMaterial(0, DecalMaterial);
			}
		}
	}
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
		PC->Notify(LOCTEXT("ShipKilled", "Target destroyed"), LOCTEXT("SkillKilledInfo", "You destroyed an enemy ship !"), FName("ship-killed"), EFlareNotification::NT_Military);
	}
}

void AFlareSpacecraft::OnDocked()
{

	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (PC && !StateManager->IsExternalCamera())
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

	InputComponent->BindAction("ZoomIn", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomIn);
	InputComponent->BindAction("ZoomOut", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomOut);

	InputComponent->BindAction("FaceForward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceForward);
	InputComponent->BindAction("FaceBackward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceBackward);
	InputComponent->BindAction("Brake", EInputEvent::IE_Released, this, &AFlareSpacecraft::Brake);
	InputComponent->BindAction("Boost", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::BoostOn);
	InputComponent->BindAction("Boost", EInputEvent::IE_Released, this, &AFlareSpacecraft::BoostOff);
	InputComponent->BindAction("Manual", EInputEvent::IE_Released, this, &AFlareSpacecraft::ForceManual);

	InputComponent->BindAction("LeftMouse", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::LeftMousePress);
	InputComponent->BindAction("LeftMouse", EInputEvent::IE_Released, this, &AFlareSpacecraft::LeftMouseRelease);

	InputComponent->BindAction("DeactivateWeapon", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::DeactivateWeapon);
	InputComponent->BindAction("WeaponGroup1", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup1);
	InputComponent->BindAction("WeaponGroup2", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup2);
	InputComponent->BindAction("WeaponGroup3", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup3);

	InputComponent->BindAction("NextWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::NextWeapon);
	InputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::PreviousWeapon);
}

void AFlareSpacecraft::LeftMousePress()
{
	StateManager->SetPlayerLeftMouse(true);
}

void AFlareSpacecraft::LeftMouseRelease()
{
	StateManager->SetPlayerLeftMouse(false);
}

void AFlareSpacecraft::DeactivateWeapon()
{
	if (!StateManager->IsPilotMode())
	{
		//StateManager->SetExternalCamera(false);
		FLOG("AFlareSpacecraft::DeactivateWeapon");
		GetPC()->SetSelectingWeapon();
		GetWeaponsSystem()->DeactivateWeapons();
	}
}

void AFlareSpacecraft::ActivateWeaponGroup1()
{
	if (!StateManager->IsPilotMode())
	{
		FLOG("AFlareSpacecraft::ActivateWeaponGroup1");
		GetPC()->SetSelectingWeapon();
		GetWeaponsSystem()->ActivateWeaponGroup(0);

		if(GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			StateManager->SetExternalCamera(false);
		}
	}
}

void AFlareSpacecraft::ActivateWeaponGroup2()
{
	if (!StateManager->IsPilotMode())
	{
		FLOG("AFlareSpacecraft::ActivateWeaponGroup2");
		GetPC()->SetSelectingWeapon();
		GetWeaponsSystem()->ActivateWeaponGroup(1);
		if(GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			StateManager->SetExternalCamera(false);
		}
	}
}

void AFlareSpacecraft::ActivateWeaponGroup3()
{
	if (!StateManager->IsPilotMode())
	{
		FLOG("AFlareSpacecraft::ActivateWeaponGroup3");
		GetPC()->SetSelectingWeapon();
		GetWeaponsSystem()->ActivateWeaponGroup(2);
		if(GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			StateManager->SetExternalCamera(false);
		}
	}
}

void AFlareSpacecraft::NextWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	if (WeaponSystems && !StateManager->IsPilotMode() && !StateManager->IsExternalCamera())
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() + 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		GetPC()->SetSelectingWeapon();
		WeaponSystems->ActivateWeaponGroup(CurrentIndex);
	}
}

void AFlareSpacecraft::PreviousWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	if (WeaponSystems && !StateManager->IsPilotMode() && !StateManager->IsExternalCamera())
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() - 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, -1, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		GetPC()->SetSelectingWeapon();
		if (CurrentIndex >= 0)
		{
			WeaponSystems->ActivateWeaponGroup(CurrentIndex);
		}
		else
		{
			WeaponSystems->DeactivateWeapons();
		}
	}
}

void AFlareSpacecraft::ThrustInput(float Val)
{
	StateManager->SetPlayerXLinearVelocity(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::MoveVerticalInput(float Val)
{
	StateManager->SetPlayerZLinearVelocity(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::MoveHorizontalInput(float Val)
{
	StateManager->SetPlayerYLinearVelocity(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::RollInput(float Val)
{
	StateManager->SetPlayerRollAngularVelocity(- Val * NavigationSystem->GetAngularMaxVelocity());
}

void AFlareSpacecraft::PitchInput(float Val)
{
	StateManager->SetPlayerMouseOffset(FVector2D(0, Val), true);
}

void AFlareSpacecraft::YawInput(float Val)
{
	StateManager->SetPlayerMouseOffset(FVector2D(Val, 0), true);
}

void AFlareSpacecraft::ZoomIn()
{
	StateManager->ExternalCameraZoom(true);
}

void AFlareSpacecraft::ZoomOut()
{
	StateManager->ExternalCameraZoom(false);
}

void AFlareSpacecraft::FaceForward()
{
	// TODO do better
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation(Airframe->GetPhysicsLinearVelocity(), FVector(1,0,0));
	}
}

void AFlareSpacecraft::FaceBackward()
{
	// TODO do better
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation((-Airframe->GetPhysicsLinearVelocity()), FVector(1, 0, 0));
	}
}

void AFlareSpacecraft::Brake()
{
	BrakeToVelocity();
}

void AFlareSpacecraft::BrakeToVelocity(const FVector& VelocityTarget)
{
	// TODO do better
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandLinearBrake(VelocityTarget);
	}
}

void AFlareSpacecraft::BoostOn()
{
	StateManager->SetPlayerOrbitalBoost(true);
}

void AFlareSpacecraft::BoostOff()
{
	StateManager->SetPlayerOrbitalBoost(false);
}

void AFlareSpacecraft::ForceManual()
{
	// TODO do better
	if (!StateManager->IsPilotMode() && NavigationSystem->GetStatus() != EFlareShipStatus::SS_Docked)
	{
		NavigationSystem->AbortAllCommands();
	}
}

/*----------------------------------------------------
		Getters
----------------------------------------------------*/

/** Linear velocity, in m/s in world reference*/
FVector AFlareSpacecraft::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}

FName AFlareSpacecraft::GetImmatriculation() const
{
	return ShipData.Immatriculation;
}


#undef LOCTEXT_NAMESPACE
