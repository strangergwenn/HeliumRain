
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
#include "../Game/AI/FlareCompanyAI.h"


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

	// Gameplay
	CurrentTarget = NULL;
	TargetIndex = 0;
	TimeSinceSelection = 0;
	MaxTimeBeforeSelectionReset = 3.0;
	Paused = false;
	LastMass = 0;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareSpacecraft::BeginPlay()
{
	Super::BeginPlay();
	CurrentTarget = NULL;
}

void AFlareSpacecraft::Tick(float DeltaSeconds)
{
	check(IsValidLowLevelFast());
	//check(Airframe && Airframe->GetBodyInstance()->IsValidBodyInstance());

	// Show mass in logs
	if (LastMass <= KINDA_SMALL_NUMBER && Airframe && Airframe->IsSimulatingPhysics())
	{
		LastMass = Airframe->GetMass();
		FLOGV("AFlareSpacecraft::Tick : Mass is %f for spacecraft '%s'", LastMass, *GetName());
	}

	if (!IsPresentationMode() && StateManager && !Paused)
	{
		// Tick systems
		StateManager->Tick(DeltaSeconds);
		DockingSystem->TickSystem(DeltaSeconds);
		NavigationSystem->TickSystem(DeltaSeconds);
		WeaponsSystem->TickSystem(DeltaSeconds);
		DamageSystem->TickSystem(DeltaSeconds);

		// Player ship updates
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

			// 5km limit
			if (PlayerShip && !GetDamageSystem()->IsAlive())
			{
				float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
				if (Company && Distance > 500000)
				{
					GetGame()->GetActiveSector()->DestroySpacecraft(this);
					return;
				}
			}

			// Set a default target if there is no manual choice
			if (this == PlayerShip && TargetIndex == 0 && PC->GetNavHUD()->GetCurrentTargetsOwner() == GetImmatriculation())
			{
				TArray<FFlareScreenTarget>& ScreenTargets = PC->GetNavHUD()->GetCurrentTargets();
				if (ScreenTargets.Num())
				{
					int32 ActualIndex = TargetIndex % ScreenTargets.Num();
					CurrentTarget = ScreenTargets[ActualIndex].Spacecraft;
				}
			}

			TimeSinceSelection += DeltaSeconds;
		}

		float SmoothedVelocityChangeSpeed = FMath::Clamp(DeltaSeconds * 8, 0.f, 1.f);
		SmoothedVelocity = SmoothedVelocity * (1 - SmoothedVelocityChangeSpeed) + GetLinearVelocity() * SmoothedVelocityChangeSpeed;
	}

	// The FlareSpacecraftPawn do the camera effective update in its Tick so call it after camera order update
	Super::Tick(DeltaSeconds);
}

void AFlareSpacecraft::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// TODO #158 aka the self-collision event of death
	if (Other == this)
	{
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		PC->Notify(
			FText::FromString("KNOWN BUG #158"),
			FText::FromString("You just encountered the known bug #158. You can re-fly your ship by clicking \"fly previous\". Sorry for the inconvenience."),
			"known-bug-155",
			EFlareNotification::NT_Military,
			10.0f);
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Orbit, PC->GetShipPawn());

		return;
	}

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

	// Colliding a kill pending actor, or itself
	if (!Other)
	{
		return;
	}

	//FLOGV("AFlareSpacecraft Hit  Mass %f NormalImpulse %s NormalImpulse.Size() %f", GetSpacecraftMass(), *NormalImpulse.ToString(), NormalImpulse.Size());
	DamageSystem->OnCollision(Other, HitLocation, NormalImpulse);
	
	// If hit, check if the is a docking in progress. If yes, check if the ship is correctly aligned
	AFlareSpacecraft* OtherSpacecraft = Cast<AFlareSpacecraft>(Other);
	if (OtherSpacecraft)
	{
		//FLOGV(">>>>>>>>>> COLL %s %s", *GetImmatriculation().ToString(), *OtherSpacecraft->GetImmatriculation().ToString());
		//FLOGV(">>>>>>>>>> DIST %s", *AFlareHUD::FormatDistance( (LOC1 - LOC2).Size() / 100) );

		// The other actor is a spacecraft, check if it's not the station we want to dock to.
		GetNavigationSystem()->CheckCollisionDocking(OtherSpacecraft);
	}
}

void AFlareSpacecraft::Destroyed()
{
	Super::Destroyed();

	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Components[ComponentIndex]);
		if (Weapon)
		{
			Weapon->ClearBombs();
		}
	}

	CurrentTarget = NULL;
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
		CurrentTarget = NULL;
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

float AFlareSpacecraft::GetSpacecraftMass()
{
	float Mass = GetDescription()->Mass;
	if (Mass)
	{
		return Mass;
	}
	else if (Airframe)
	{
		return Airframe->GetMass();
	}
	else
	{
		return 0;
	}
}


/*----------------------------------------------------
	Player interface
----------------------------------------------------*/

float AFlareSpacecraft::GetAimPosition(AFlareSpacecraft* TargettingShip, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const
{
	return GetAimPosition(TargettingShip->GetActorLocation(), TargettingShip->GetLinearVelocity() * 100, BulletSpeed, PredictionDelay, ResultPosition);
}

float AFlareSpacecraft::GetAimPosition(FVector GunLocation, FVector GunVelocity, float BulletSpeed, float PredictionDelay, FVector* ResultPosition) const
{
	// TODO : use helper

	// Target Speed
	FVector TargetVelocity = Airframe->GetPhysicsLinearVelocity() - GunVelocity;
	FVector TargetLocation = GetActorLocation() + TargetVelocity * PredictionDelay;
	FVector BulletLocation = GunLocation + GunVelocity * PredictionDelay;

	// Find the relative speed in the axis of target
	FVector TargetDirection = (TargetLocation - BulletLocation).GetUnsafeNormal();
	float EffectiveBulletSpeed = BulletSpeed * 100.f;
	float Divisor = FMath::Square(EffectiveBulletSpeed) - TargetVelocity.SizeSquared();

	// Intersect at an infinite time ?
	if (EffectiveBulletSpeed < 0 || FMath::IsNearlyZero(Divisor))
	{
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

void AFlareSpacecraft::ResetCurrentTarget()
{
	CurrentTarget = NULL;
}

AFlareSpacecraft* AFlareSpacecraft::GetCurrentTarget() const
{
	// Don't try in menus
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC->GetMenuManager()->IsMenuOpen())
	{
		return NULL;
	}

	// Crash "preventer" - ensure we've got a really valid target, this isn't a solution, but it seems to only happen when using CreateShip commands
	// TODO : remove all IsValidLowLevelFast checks, play a little, check during real gameplay if stuff happens
	if (CurrentTarget                    && CurrentTarget->IsValidLowLevelFast()
	 && CurrentTarget->Airframe          && CurrentTarget->Airframe->IsValidLowLevelFast()
	 && CurrentTarget->GetDamageSystem() && CurrentTarget->GetDamageSystem()->IsValidLowLevelFast()
	 && CurrentTarget->GetDamageSystem()->IsAlive())
	{
		return CurrentTarget;
	}
	else
	{
		return NULL;
	}
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

	// Load dynamic components
	UpdateDynamicComponents();

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

	CargoBay = NewObject<UFlareCargoBay>(this, UFlareCargoBay::StaticClass());
	CargoBay->Load(this, ShipData.Cargo);
	CurrentTarget = NULL;
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

	ShipData.Cargo = *CargoBay->Save();

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

bool AFlareSpacecraft::CanFight() const
{
	return GetDamageSystem()->IsAlive() && IsMilitary() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
}

bool AFlareSpacecraft::CanTravel() const
{
	return GetDamageSystem()->IsAlive() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) > 0;
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

				// Get a valid sector
				UFlareSectorInterface* Sector = GetGame()->GetActiveSector();
				if (!Sector)
				{
					Sector = GetOwnerSector();
				}

				// Setup the asteroid
				bool IsIcy = Sector->GetDescription()->IsIcy;
				UStaticMeshComponent* AsteroidComponentCandidate = Cast<UStaticMeshComponent>(Component);
				AFlareAsteroid::SetupAsteroidMesh(GetGame(), AsteroidComponentCandidate, ShipData.AsteroidData, IsIcy);
				break;
			}
		}
	}
}

UFlareSimulatedSector* AFlareSpacecraft::GetOwnerSector()
{
	TArray<UFlareSimulatedSector*> Sectors = GetGame()->GetGameWorld()->GetSectors();

	for (int32 Index = 0; Index < Sectors.Num(); Index++)
	{
		UFlareSimulatedSector* CandidateSector = Sectors[Index];
		for (int32 ShipIndex = 0; ShipIndex < CandidateSector->GetSectorSpacecrafts().Num(); ShipIndex++)
		{
			if (CandidateSector->GetSectorSpacecrafts()[ShipIndex]->Save()->Identifier == ShipData.Identifier)
			{
				return CandidateSector;
			}
		}
	}

	return NULL;
}

void AFlareSpacecraft::UpdateDynamicComponents()
{
	TArray<UActorComponent*> DynamicComponents = GetComponentsByClass(UChildActorComponent::StaticClass());

	if(DynamicComponents.Num() == 0)
	{
		return;
	}

	FFlareSpacecraftDynamicComponentStateDescription* CurrentState = NULL;
	int32 TemplateIndex = 0;

	for (int32 StateIndex = 0; StateIndex < ShipDescription->DynamicComponentStates.Num(); StateIndex++)
	{
		FFlareSpacecraftDynamicComponentStateDescription* State = &ShipDescription->DynamicComponentStates[StateIndex];



		if (State->StateIdentifier == ShipData.DynamicComponentStateIdentifier)
		{
			if(State->StateTemplates.Num() == 0)
			{
				FLOGV("Dynamic component state '%s' has no template", *ShipData.DynamicComponentStateIdentifier.ToString())
				break;
			}

			CurrentState = State;
			TemplateIndex = FMath::Clamp((int32) (ShipData.DynamicComponentStateProgress * CurrentState->StateTemplates.Num()), 0, CurrentState->StateTemplates.Num()-1);
			break;
		}
	}


	for (int32 ComponentIndex = 0; ComponentIndex < DynamicComponents.Num(); ComponentIndex++)
	{
		UChildActorComponent* Component = Cast<UChildActorComponent>(DynamicComponents[ComponentIndex]);

		if(!Component)
		{
			continue;
		}

		if (CurrentState == NULL)
		{
			FLOGV("Fail to find State '%s'", *ShipData.DynamicComponentStateIdentifier.ToString());
			Component->SetChildActorClass(NULL);
			return;
		}
		else
		{
			Component->SetChildActorClass(*(CurrentState->StateTemplates[TemplateIndex]->GeneratedClass));

			if (Component->ChildActor)
			{
				TArray<UActorComponent*> SubDynamicComponents = Component->ChildActor->GetComponentsByClass(UChildActorComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UChildActorComponent* SubDynamicComponent = Cast<UChildActorComponent>(SubDynamicComponents[SubComponentIndex]);

					if(SubDynamicComponent->ChildActor)
					{
						SubDynamicComponent->ChildActor->AttachRootComponentToActor(this,"", EAttachLocation::KeepWorldPosition, true);
						SubDynamicComponent->ChildActor->SetOwner(this);

						UFlareSpacecraftComponent* ChildRootComponent = Cast<UFlareSpacecraftComponent>(SubDynamicComponent->ChildActor->GetRootComponent());
						if (ChildRootComponent)
						{
							FFlareSpacecraftComponentSave Data;
							Data.Damage = 0;
							Data.ComponentIdentifier = NAME_None;
							ChildRootComponent->Initialize(&Data, Company, this, false);
						}
					}
				}

				SubDynamicComponents = Component->ChildActor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UStaticMeshComponent* SubDynamicComponent = Cast<UStaticMeshComponent>(SubDynamicComponents[SubComponentIndex]);

					if (SubDynamicComponent)
					{
						USceneComponent* ParentDefaultAttachComponent = GetDefaultAttachComponent();
						if (ParentDefaultAttachComponent)
						{
							SubDynamicComponent->AttachTo(ParentDefaultAttachComponent, "", EAttachLocation::KeepWorldPosition, true);
						}
					}

				}

			}
		}
	}
}

UFlareSectorInterface* AFlareSpacecraft::GetCurrentSectorInterface()
{
	return GetGame()->GetActiveSector();
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
			float Mass = GetSpacecraftMass() / 100000;
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

	CurrentTarget = NULL;
}


/*----------------------------------------------------
		Damage system
----------------------------------------------------*/

void AFlareSpacecraft::OnDocked(IFlareSpacecraftInterface* DockStation)
{
	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (PC)
	{
		if (!StateManager->IsExternalCamera())
		{
			PC->SetExternalCamera(true);
		}

		PC->Notify(
			LOCTEXT("DockingSuccess", "Docking successful"),
			FText::Format(LOCTEXT("DockingSuccessInfoFormat", "Your ship is now docked at {0}"), FText::FromName(DockStation->GetImmatriculation())),
			"docking-success",
			EFlareNotification::NT_Info);
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

void AFlareSpacecraft::OnUndocked(IFlareSpacecraftInterface* DockStation)
{
	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (PC)
	{
		if (StateManager->IsExternalCamera())
		{
			PC->SetExternalCamera(false);
		}

		PC->Notify(
			LOCTEXT("Undocked", "Undocked"),
			FText::Format(LOCTEXT("UndockedInfoFormat", "Undocked from {0}"), FText::FromName(DockStation->GetImmatriculation())),
			"docking-success",
			EFlareNotification::NT_Info);
	}
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareSpacecraft::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("Thrust", this, &AFlareSpacecraft::ThrustInput);
	InputComponent->BindAxis("RollInput", this, &AFlareSpacecraft::RollInput);
	InputComponent->BindAxis("MoveVerticalInput", this, &AFlareSpacecraft::MoveVerticalInput);
	InputComponent->BindAxis("MoveHorizontalInput", this, &AFlareSpacecraft::MoveHorizontalInput);

	InputComponent->BindAction("ZoomIn", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomIn);
	InputComponent->BindAction("ZoomOut", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomOut);
	InputComponent->BindAction("ZoomIn", EInputEvent::IE_Repeat, this, &AFlareSpacecraft::ZoomIn);
	InputComponent->BindAction("ZoomOut", EInputEvent::IE_Repeat, this, &AFlareSpacecraft::ZoomOut);

	InputComponent->BindAction("FaceForward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceForward);
	InputComponent->BindAction("FaceBackward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceBackward);
	InputComponent->BindAction("Brake", EInputEvent::IE_Released, this, &AFlareSpacecraft::Brake);
	InputComponent->BindAction("LockDirection", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::LockDirectionOn);
	InputComponent->BindAction("LockDirection", EInputEvent::IE_Released, this, &AFlareSpacecraft::LockDirectionOff);
	InputComponent->BindAction("Manual", EInputEvent::IE_Released, this, &AFlareSpacecraft::ForceManual);

	InputComponent->BindAction("LeftMouse", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::LeftMousePress);
	InputComponent->BindAction("LeftMouse", EInputEvent::IE_Released, this, &AFlareSpacecraft::LeftMouseRelease);

	InputComponent->BindAction("DeactivateWeapon", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::DeactivateWeapon);
	InputComponent->BindAction("WeaponGroup1", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup1);
	InputComponent->BindAction("WeaponGroup2", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup2);
	InputComponent->BindAction("WeaponGroup3", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::ActivateWeaponGroup3);

	InputComponent->BindAction("NextWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::NextWeapon);
	InputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::PreviousWeapon);

	InputComponent->BindAction("NextTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::NextTarget);
	InputComponent->BindAction("PreviousTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::PreviousTarget);
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
	// Capital ship
	if (GetDescription()->Size == EFlarePartSize::L)
	{
		GetCompany()->GetAI()->SetCurrentShipGroup(static_cast<EFlareCombatGroup::Type>(0));
	}

	// Fighter
	else if(!StateManager->IsPilotMode())
	{
		FLOG("AFlareSpacecraft::DeactivateWeapon");
		GetPC()->SetSelectingWeapon();
		GetWeaponsSystem()->DeactivateWeapons();
	}
}

void AFlareSpacecraft::ActivateWeaponGroup1()
{
	ActivateWeaponGroupByIndex(0);
}

void AFlareSpacecraft::ActivateWeaponGroup2()
{
	ActivateWeaponGroupByIndex(1);
}

void AFlareSpacecraft::ActivateWeaponGroup3()
{
	ActivateWeaponGroupByIndex(2);
}

void AFlareSpacecraft::ActivateWeaponGroupByIndex(int32 Index)
{
	FLOGV("AFlareSpacecraft::ActivateWeaponGroup : %d", Index);

	// Capital ship
	if (GetDescription()->Size == EFlarePartSize::L)
	{
		GetCompany()->GetAI()->SetCurrentShipGroup(static_cast<EFlareCombatGroup::Type>(Index + 1));
	}

	// Fighter
	else if(!StateManager->IsPilotMode())
	{
		GetWeaponsSystem()->ActivateWeaponGroup(Index);
		if (GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			StateManager->SetExternalCamera(false);
		}
	}

	GetPC()->SetSelectingWeapon();
}

void AFlareSpacecraft::NextWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	
	// Capital ship
	if (GetDescription()->Size == EFlarePartSize::L)
	{
		int32 CurrentIndex = GetCompany()->GetAI()->GetCurrentShipGroup() + 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, static_cast<int32>(EFlareCombatGroup::Civilan));
		FLOGV("AFlareSpacecraft::NextWeapon : group %d", CurrentIndex);

		GetCompany()->GetAI()->SetCurrentShipGroup(static_cast<EFlareCombatGroup::Type>(CurrentIndex));
	}

	// Fighter
	else if (WeaponSystems && !StateManager->IsPilotMode())
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() + 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		WeaponSystems->ActivateWeaponGroup(CurrentIndex);
	}

	GetPC()->SetSelectingWeapon();
}

void AFlareSpacecraft::PreviousWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	
	// Capital ship
	if (GetDescription()->Size == EFlarePartSize::L)
	{
		int32 CurrentIndex = GetCompany()->GetAI()->GetCurrentShipGroup() - 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, static_cast<int32>(EFlareCombatGroup::Civilan));
		FLOGV("AFlareSpacecraft::NextWeapon : group %d", CurrentIndex);

		GetCompany()->GetAI()->SetCurrentShipGroup(static_cast<EFlareCombatGroup::Type>(CurrentIndex));
	}

	// Fighter
	else if (WeaponSystems && !StateManager->IsPilotMode())
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() - 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, -1, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		if (CurrentIndex >= 0)
		{
			WeaponSystems->ActivateWeaponGroup(CurrentIndex);
		}
		else
		{
			WeaponSystems->DeactivateWeapons();
		}
	}

	GetPC()->SetSelectingWeapon();
}

void AFlareSpacecraft::NextTarget()
{
	if (!StateManager->IsPilotMode())
	{
		// Data
		TArray<FFlareScreenTarget>& ScreenTargets = GetPC()->GetNavHUD()->GetCurrentTargets();
		auto FindCurrentTarget = [=](const FFlareScreenTarget& Candidate)
		{
			return Candidate.Spacecraft == CurrentTarget;
		};

		// Is visible on screen
		if (TimeSinceSelection < MaxTimeBeforeSelectionReset && ScreenTargets.FindByPredicate(FindCurrentTarget))
		{
			TargetIndex++;
			TargetIndex = FMath::Min(TargetIndex, ScreenTargets.Num() - 1);
			CurrentTarget = ScreenTargets[TargetIndex].Spacecraft;
			FLOGV("AFlareSpacecraft::NextTarget : %d", TargetIndex);
		}

		// Else reset
		else
		{
			TargetIndex = 0;
			FLOG("AFlareSpacecraft::NextTarget : reset to center");
		}

		TimeSinceSelection = 0;
	}
}

void AFlareSpacecraft::PreviousTarget()
{
	if (!StateManager->IsPilotMode())
	{
		// Data
		TArray<FFlareScreenTarget>& ScreenTargets = GetPC()->GetNavHUD()->GetCurrentTargets();
		auto FindCurrentTarget = [=](const FFlareScreenTarget& Candidate)
		{
			return Candidate.Spacecraft == CurrentTarget;
		};

		// Is visible on screen
		if (TimeSinceSelection < MaxTimeBeforeSelectionReset && ScreenTargets.FindByPredicate(FindCurrentTarget))
		{
			TargetIndex--;
			TargetIndex = FMath::Max(TargetIndex, 0);
			CurrentTarget = ScreenTargets[TargetIndex].Spacecraft;
			FLOGV("AFlareSpacecraft::PreviousTarget : %d", TargetIndex);
		}

		// Else reset
		else
		{
			TargetIndex = 0;
			FLOG("AFlareSpacecraft::PreviousTarget : reset to center");
		}

		TimeSinceSelection = 0;
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

void AFlareSpacecraft::LockDirectionOn()
{
	StateManager->SetPlayerLockDirection(true);
}

void AFlareSpacecraft::LockDirectionOff()
{
	StateManager->SetPlayerLockDirection(false);
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
