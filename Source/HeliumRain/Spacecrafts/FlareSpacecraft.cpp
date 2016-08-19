
#include "../Flare.h"

#include "FlareSpacecraft.h"
#include "FlareOrbitalEngine.h"
#include "FlareRCS.h"
#include "FlareWeapon.h"
#include "FlareShipPilot.h"
#include "FlareInternalComponent.h"

#include "Particles/ParticleSystemComponent.h"

#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareAsteroid.h"
#include "../Game/AI/FlareCompanyAI.h"

#include "../UI/Menus/FlareShipMenu.h"


#define LOCTEXT_NAMESPACE "FlareSpacecraft"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareSpacecraft::AFlareSpacecraft(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Ship name font
	static ConstructorHelpers::FObjectFinder<UFont> ShipNameFontObj(TEXT("/Game/Slate/Fonts/ShipNameFont.ShipNameFont"));
	ShipNameFont = ShipNameFontObj.Object;

	// Create static mesh component
	Airframe = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("Airframe"));
	Airframe->SetSimulatePhysics(false);
	RootComponent = Airframe;

	// Camera settings
	CameraContainerYaw->AttachToComponent(Airframe, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	CameraMaxPitch = 80;
	CameraPanSpeed = 2;

	// Dock info
	/*ShipData.DockedTo = NAME_None;
	ShipData.DockedAt = -1;

	// Asteroid info
	ShipData.AsteroidData.Identifier = NAME_None;
	ShipData.AsteroidData.AsteroidMeshID = 0;
	ShipData.AsteroidData.Scale = FVector(1, 1, 1);
*/
	// Gameplay
	Paused = false;
	LastMass = 0;
	TargetIndex = 0;
	TimeSinceSelection = 0;
	MaxTimeBeforeSelectionReset = 3.0;
	StateManager = NULL;
	CurrentTarget = NULL;
	HarpoonCompany = NULL;
	NavigationSystem = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareSpacecraft::BeginPlay()
{
	Super::BeginPlay();

	// Setup asteroid components, if any
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareAsteroidComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareAsteroidComponent* AsteroidComponent = Cast<UFlareAsteroidComponent>(Components[ComponentIndex]);
		if (AsteroidComponent)
		{
			bool IsIcy = false;

			AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
			if (Game)
			{
				if (Game->GetActiveSector())
				{
					IsIcy = Game->GetActiveSector()->GetDescription()->IsIcy;
				}
				else
				{
					UFlareSimulatedSpacecraft* Spacecraft = Game->GetPC()->GetMenuManager()->GetShipMenu()->GetTargetSpacecraft();
					if (Spacecraft)
					{
						IsIcy = Spacecraft->GetCurrentSector()->GetDescription()->IsIcy;
					}
				}
			}

			AsteroidComponent->SetIcy(IsIcy);
		}
	}

	CurrentTarget = NULL;
}

void AFlareSpacecraft::Tick(float DeltaSeconds)
{
	check(IsValidLowLevel());
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
			
			if (this == PlayerShip)
			{
				// Reload the sector if player leave the limits
				float Distance = GetActorLocation().Size();
				float Limits = GetGame()->GetActiveSector()->GetSectorLimits();
				if (Distance > Limits)
				{
					FLOGV("%s exit sector distance to center=%f and limits=%f", *GetImmatriculation().ToString(), Distance, Limits)

					// Notify
					PC->Notify(
						LOCTEXT("ExitSector", "Exited sector"),
						LOCTEXT("ExitSectorDescription", "Your ship went too far from the orbit reference."),
						"exit-sector",
						EFlareNotification::NT_Info);
					GetData().SpawnMode = EFlareSpawnMode::Exit;

					// Reload
					FFlareMenuParameterData MenuParameters;
					MenuParameters.Spacecraft = GetParent();
					MenuParameters.Sector = GetParent()->GetCurrentSector();
					PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_ReloadSector, MenuParameters);

					return;
				}
				else
				{
					float MinDistance = Limits - Distance;
					bool ExitImminent = false;

					if (MinDistance < 0.1 * Limits)
					{
						ExitImminent = true;
					}
					else if (! GetLinearVelocity().IsNearlyZero())
					{
						// https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
						FVector ShipDirection = GetLinearVelocity().GetUnsafeNormal();
						FVector ShipLocation = GetActorLocation();
						float Dot = FVector::DotProduct(ShipDirection, ShipLocation);

						float DistanceBeforeExit = - Dot + FMath::Sqrt(Dot * Dot - Distance * Distance + Limits * Limits);
						float Velocity = GetLinearVelocity().Size() * 100;
						float DurationBeforeExit = DistanceBeforeExit / Velocity;

						if (DurationBeforeExit < 15)
						{
							ExitImminent = true;
						}
					}

					if (ExitImminent)
					{
						if (!InWarningZone)
						{
							PC->Notify(
								LOCTEXT("ExitSectorImminent", "Exiting sector"),
								LOCTEXT("ExitSectorImminentDescription", "Your ship is going to exit the sector. Go back near the sector center if you want to stay."),
								"exit-sector-imminent",
								EFlareNotification::NT_Info);
						}

						InWarningZone = true;
					}
					else
					{
						InWarningZone = false;
					}

				}
			}

			// 5km limit
			if (PlayerShip && !Parent->GetDamageSystem()->IsAlive())
			{
				float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
				if (Company && Distance > 500000)
				{
					GetGame()->GetActiveSector()->DestroySpacecraft(this);
					return;
				}
			}

			// Destroy lost ship
			float Distance = GetActorLocation().Size();
			float Limits = GetGame()->GetActiveSector()->GetSectorLimits();
			if(Distance > Limits * 3)
			{
				// Ship is lost, destroy it
				if(GetCompany() ==	PC->GetCompany())
				{
					PC->Notify(LOCTEXT("MyShipLost", "Ship lost"),
						FText::Format(LOCTEXT("MyShipLostFormat", "One of your ships was lost in space ({0})"),
							 FText::FromString(GetImmatriculation().ToString())),
						FName("my-ship-lost"),
						EFlareNotification::NT_Info);
				}
				else
				{
					PC->Notify(LOCTEXT("ShipLostCompany", "Ship lost"),
						FText::Format(LOCTEXT("ShipLostCompanyFormat", "{0} lost a ship in space"),
							GetCompany()->GetCompanyName()),
						FName("company-ship-lost"),
						EFlareNotification::NT_Info);
				}
				GetGame()->GetActiveSector()->DestroySpacecraft(this);
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
	// Strictly disallow self-collision : this should never happen
	if (Other == this)
	{
		// TODO fixme
		FLOGV("%s (%d) self-collision: %s collide with %s", *GetImmatriculation().ToString(), IsPresentationMode(), (MyComp ? *MyComp->GetName() : TEXT("null")), (OtherComp ? *OtherComp->GetName() : TEXT("null")));
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		PC->ConsoleCommand("quit");
		return;

		PC->Notify(
			FText::FromString("KNOWN BUG #158"),
			FText::FromString("You just encountered the known bug #158. You can re-fly your ship by clicking \"fly previous\". Sorry for the inconvenience."),
			"known-bug-155",
			EFlareNotification::NT_Military);

		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Orbit);

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
	// Notify PC
	if(!IsPresentationMode())
	{
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			if (PC->GetShipPawn())
			{
				AFlareSpacecraft* PlayerTarget = PC->GetShipPawn()->GetCurrentTarget();
				if (PlayerTarget == this)
				{
					PC->GetShipPawn()->ResetCurrentTarget();
				}
			}

			if (PC->GetNavHUD())
			{
				PC->GetNavHUD()->RemoveTarget(this);
			}
		}

		if(Parent)
		{
			Parent->SetActiveSpacecraft(NULL);
		}
	}


	Super::Destroyed();

	// Clear bombs
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
		//FLOGV("%s save linear velocity : %s", *GetImmatriculation().ToString(), *GetData().LinearVelocity.ToString());
	}

	Airframe->SetSimulatePhysics(!Pause);

	Paused = Pause;

	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		//FLOGV("%s restore linear velocity : %s", *GetImmatriculation().ToString(), *GetData().LinearVelocity.ToString());
		Airframe->SetPhysicsLinearVelocity(GetData().LinearVelocity);
		Airframe->SetPhysicsAngularVelocity(GetData().AngularVelocity);
		SmoothedVelocity = GetLinearVelocity();
	}
}

void AFlareSpacecraft::Redock()
{
	// Re-dock if we were docked
	if (GetData().DockedTo != NAME_None && !IsPresentationMode())
	{
		// TODO use sector iterator
		FLOGV("AFlareSpacecraft::Redock : Looking for station '%s'", *GetData().DockedTo.ToString());

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Station = GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

			if (Station->GetImmatriculation() == GetData().DockedTo)
			{
				FLOGV("AFlareSpacecraft::Redock : Found dock station '%s'", *Station->GetImmatriculation().ToString());
				NavigationSystem->ConfirmDock(Station, GetData().DockedAt);
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
	// Crash "preventer" - ensure we've got a really valid target, this isn't a solution, but it seems to only happen when using CreateShip commands
	if (IsValidLowLevel() && CurrentTarget  && CurrentTarget->IsValidLowLevel()
	 && CurrentTarget->GetDamageSystem() && CurrentTarget->GetDamageSystem()->IsValidLowLevel() && CurrentTarget->GetParent()->GetDamageSystem()->IsAlive())
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

void AFlareSpacecraft::Load(UFlareSimulatedSpacecraft* ParentSpacecraft)
{
	// Update local data
	Parent = ParentSpacecraft;

	if (!IsPresentationMode())
	{
		Airframe->SetSimulatePhysics(true);
		Parent->SetActiveSpacecraft(this);
	}

	FLOGV("AFlareSpacecraft::Load %s", *ParentSpacecraft->GetImmatriculation().ToString());

	// Load ship description
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();

	// Unload
	if(NavigationSystem)
	{
		NavigationSystem->BreakDock();
	}

	// Initialize damage system
	DamageSystem = NewObject<UFlareSpacecraftDamageSystem>(this, UFlareSpacecraftDamageSystem::StaticClass());
	DamageSystem->Initialize(this, &GetData());

	// Initialize navigation system
	NavigationSystem = NewObject<UFlareSpacecraftNavigationSystem>(this, UFlareSpacecraftNavigationSystem::StaticClass());
	NavigationSystem->Initialize(this, &GetData());

	// Initialize docking system
	DockingSystem = NewObject<UFlareSpacecraftDockingSystem>(this, UFlareSpacecraftDockingSystem::StaticClass());
	DockingSystem->Initialize(this, &GetData());

	// Initialize weapons system
	WeaponsSystem = NewObject<UFlareSpacecraftWeaponsSystem>(this, UFlareSpacecraftWeaponsSystem::StaticClass());
	WeaponsSystem->Initialize(this, &GetData());

	// Look for parent company
	SetOwnerCompany(ParentSpacecraft->GetCompany());

	// Load dynamic components
	UpdateDynamicComponents();

	// Initialize components
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		FFlareSpacecraftComponentSave* ComponentData = NULL;

		// Find component the corresponding component data comparing the slot id
		bool Found = false;
		for (int32 i = 0; i < GetData().Components.Num(); i++)
		{
			if (Component->SlotIdentifier == GetData().Components[i].ShipSlotIdentifier)
			{
				ComponentData = &GetData().Components[i];
				Found = true;
				break;
			}
		}

		// If no data, this is a cosmetic component and it don't need to be initialized
		if (!Found)
		{
			continue;
		}

		// Reload the component
		ReloadPart(Component, ComponentData);

		// Set RCS description
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);
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

	// Setup ship name texture target
	if (GetDescription()->Size == EFlarePartSize::L)
	{
		ShipNameTexture = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 256, 256);
		check(ShipNameTexture);
		ShipNameTexture->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlareSpacecraft::DrawShipName);
		ShipNameTexture->ClearColor = FLinearColor::Black;
	}

	// Customization
	UpdateCustomization();
	Redock();

	// If not rcs, add passive stabilization
	if (GetDescription()->RCSCount == 0)
	{
		Airframe->SetLinearDamping(0.1);
		Airframe->SetAngularDamping(0.1);
	}

	// Initialize pilot
	Pilot = NewObject<UFlareShipPilot>(this, UFlareShipPilot::StaticClass());
	Pilot->Initialize(&GetData().Pilot, GetCompany(), this);
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

	CurrentTarget = NULL;
	InWarningZone = false;
}

void AFlareSpacecraft::Save()
{
	// Physical data
	GetData().Location = GetActorLocation();
	GetData().Rotation = GetActorRotation();
	if (!IsPaused())
	{
		GetData().LinearVelocity = Airframe->GetPhysicsLinearVelocity();
		GetData().AngularVelocity = Airframe->GetPhysicsAngularVelocity();
	}

	// Save all components datas
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->Save();
	}
}

void AFlareSpacecraft::SetOwnerCompany(UFlareCompany* NewCompany)
{
	SetCompany(NewCompany);
	Airframe->Initialize(NULL, Company, this);
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
	GetData().AsteroidData.Identifier = Data->Identifier;
	GetData().AsteroidData.AsteroidMeshID = Data->AsteroidMeshID;
	GetData().AsteroidData.Scale = Data->Scale;

	// Apply
	ApplyAsteroidData();
	SetActorLocation(Data->Location);
	SetActorRotation(Data->Rotation);
}

void AFlareSpacecraft::ApplyAsteroidData()
{
	if (GetData().AsteroidData.Identifier != NAME_None)
	{
		TArray<UActorComponent*> Components = GetComponentsByClass(UActorComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UActorComponent* Component = Components[ComponentIndex];
			if (Component->GetName().Contains("Asteroid"))
			{
				FLOGV("AFlareSpacecraft::ApplyAsteroidData : Found asteroid component '%s', previously set from '%s'",
					*Component->GetName(), *GetData().AsteroidData.Identifier.ToString());

				// Get a valid sector
				UFlareSimulatedSector* Sector = GetParent()->GetCurrentSector();
				if (!Sector)
				{
					Sector = GetOwnerSector();
				}

				// Setup the asteroid
				bool IsIcy = Sector->GetDescription()->IsIcy;
				UStaticMeshComponent* AsteroidComponentCandidate = Cast<UStaticMeshComponent>(Component);
				AFlareAsteroid::SetupAsteroidMesh(GetGame(), AsteroidComponentCandidate, GetData().AsteroidData, IsIcy);
				break;
			}
		}
	}
}

void AFlareSpacecraft::SetHarpooned(UFlareCompany* OwnerCompany)
{
	FLOGV("AFlareSpacecraft::SetHarpooned : %s harpooned by %s", *GetImmatriculation().ToString(), *OwnerCompany->GetCompanyName().ToString());
	Harpooned = true;
	HarpoonCompany = OwnerCompany;
}

UFlareSimulatedSector* AFlareSpacecraft::GetOwnerSector()
{
	TArray<UFlareSimulatedSector*> Sectors = GetGame()->GetGameWorld()->GetSectors();

	for (int32 Index = 0; Index < Sectors.Num(); Index++)
	{
		UFlareSimulatedSector* CandidateSector = Sectors[Index];
		for (int32 ShipIndex = 0; ShipIndex < CandidateSector->GetSectorSpacecrafts().Num(); ShipIndex++)
		{
			if (CandidateSector->GetSectorSpacecrafts()[ShipIndex]->Save()->Identifier == GetData().Identifier)
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

	for (int32 StateIndex = 0; StateIndex < GetDescription()->DynamicComponentStates.Num(); StateIndex++)
	{
		FFlareSpacecraftDynamicComponentStateDescription* State = &GetDescription()->DynamicComponentStates[StateIndex];



		if (State->StateIdentifier == GetData().DynamicComponentStateIdentifier)
		{
			if(State->StateTemplates.Num() == 0)
			{
				FLOGV("Dynamic component state '%s' has no template", *GetData().DynamicComponentStateIdentifier.ToString())
				break;
			}

			CurrentState = State;
			TemplateIndex = FMath::Clamp((int32) (GetData().DynamicComponentStateProgress * CurrentState->StateTemplates.Num()), 0, CurrentState->StateTemplates.Num()-1);
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
			FLOGV("Fail to find State '%s'", *GetData().DynamicComponentStateIdentifier.ToString());
			Component->SetChildActorClass(NULL);
			return;
		}
		else
		{
			Component->SetChildActorClass(*(CurrentState->StateTemplates[TemplateIndex]->GeneratedClass));

			if (Component->GetChildActor())
			{
				TArray<UActorComponent*> SubDynamicComponents = Component->GetChildActor()->GetComponentsByClass(UChildActorComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UChildActorComponent* SubDynamicComponent = Cast<UChildActorComponent>(SubDynamicComponents[SubComponentIndex]);

					if(SubDynamicComponent->GetChildActor())
					{
						SubDynamicComponent->GetChildActor()->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
						SubDynamicComponent->GetChildActor()->SetOwner(this);

						UFlareSpacecraftComponent* ChildRootComponent = Cast<UFlareSpacecraftComponent>(SubDynamicComponent->GetChildActor()->GetRootComponent());
						if (ChildRootComponent)
						{
							FFlareSpacecraftComponentSave Data;
							Data.Damage = 0;
							Data.ComponentIdentifier = NAME_None;
							ChildRootComponent->Initialize(&Data, Company, this, false);
						}
					}
				}

				SubDynamicComponents = Component->GetChildActor()->GetComponentsByClass(UStaticMeshComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UStaticMeshComponent* SubDynamicComponent = Cast<UStaticMeshComponent>(SubDynamicComponents[SubComponentIndex]);

					if (SubDynamicComponent)
					{
						USceneComponent* ParentDefaultAttachComponent = GetDefaultAttachComponent();
						if (ParentDefaultAttachComponent)
						{
							SubDynamicComponent->AttachToComponent(ParentDefaultAttachComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
						}
					}

				}

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
	// Send the update event to subsystems
	Super::UpdateCustomization();
	Airframe->UpdateCustomization();
	if (ShipNameTexture)
	{
		ShipNameTexture->UpdateResource();
	}

	// Customize decal materials
	TArray<UActorComponent*> Components = GetComponentsByClass(UDecalComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UDecalComponent* Component = Cast<UDecalComponent>(Components[ComponentIndex]);
		if (Component)
		{
			// Ship name decal
			if (Component->GetName().Contains("ShipNameDecal"))
			{
				if (ShipNameTexture && !ShipNameDecalMaterial)
				{
					ShipNameDecalMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), GetWorld());
				}
				if (Company && ShipNameDecalMaterial)
				{
					FLinearColor BasePaintColor = GetGame()->GetCustomizationCatalog()->GetColor(Company->GetPaintColorIndex());
					FLinearColor ShipNameColor = (BasePaintColor.GetLuminance() > 0.5) ? FLinearColor::Black : FLinearColor::White;
					ShipNameDecalMaterial->SetVectorParameterValue("NameColor", ShipNameColor);
					ShipNameDecalMaterial->SetTextureParameterValue("NameTexture", ShipNameTexture);
					Component->SetMaterial(0, ShipNameDecalMaterial);
				}
			}

			// Company decal
			else
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
}

void AFlareSpacecraft::StartPresentation()
{
	Super::StartPresentation();

	if (Airframe)
	{
		Airframe->SetSimulatePhysics(false);
		Airframe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	CurrentTarget = NULL;
}

void AFlareSpacecraft::DrawShipName(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	if (TargetCanvas)
	{
		// Cleanup immatriculation
		FString Text = GetImmatriculation().ToString().ToUpper();
		int32 Index = Text.Find(*FString("-")) + 1;
		Text = Text.RightChop(Index);
		Text = Text.Replace(*FString("-"), *FString(" "));

		// Centering
		float XL, YL;
		TargetCanvas->TextSize(ShipNameFont, Text, XL, YL);
		float X = TargetCanvas->ClipX / 2.0f - XL / 2.0f;
		float Y = TargetCanvas->ClipY / 2.0f - YL / 2.0f;

		// Drawing
		FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), ShipNameFont, FLinearColor::White);
		TextItem.Scale = FVector2D(1, 1);
		TargetCanvas->DrawItem(TextItem);
	}
}


/*----------------------------------------------------
		Damage system
----------------------------------------------------*/

void AFlareSpacecraft::OnDocked(AFlareSpacecraft* DockStation)
{
	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	if (IsFlownByPlayer() && PC)
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

void AFlareSpacecraft::OnUndocked(AFlareSpacecraft* DockStation)
{
	// Signal the PC
	AFlarePlayerController* PC = GetGame()->GetPC();
	if (IsFlownByPlayer() && PC)
	{
		if (StateManager->IsExternalCamera())
		{
			PC->SetExternalCamera(false);
		}

		PC->Notify(
			LOCTEXT("Undocked", "Undocked"),
			FText::Format(LOCTEXT("UndockedInfoFormat", "Undocked from {0}"), FText::FromName(DockStation->GetImmatriculation())),
			"undocking-success",
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

FText AFlareSpacecraft::GetShipStatus() const
{
	FText PauseText;
	FText ModeText;
	FText AutopilotText;
	UFlareSpacecraftNavigationSystem* Nav = GetNavigationSystem();
	FFlareShipCommandData Command = Nav->GetCurrentCommand();
	UFlareSector* CurrentSector = GetGame()->GetActiveSector();

	// Modifiers
	if (Paused)
	{
		PauseText = LOCTEXT("GamePausedModInfo", "Game paused - ");
	}
	if (Nav->IsAutoPilot())
	{
		AutopilotText = LOCTEXT("AutopilotMod", "(Autopilot)");
	}

	// Build mode text
	if (Nav->IsDocked())
	{
		ModeText = LOCTEXT("Docked", "Docked");
	}
	else if (Parent->GetCurrentFleet()->IsTraveling())
	{
		ModeText = FText::Format(LOCTEXT("TravellingAtFormat", "Travelling to {0}"),
			Parent->GetCurrentFleet()->GetCurrentTravel()->GetDestinationSector()->GetSectorName());
	}
	else if (Command.Type == EFlareCommandDataType::CDT_Dock)
	{
		AFlareSpacecraft* Target = Command.ActionTarget;

		ModeText = FText::Format(LOCTEXT("DockingAtFormat", "Docking at {0}"),
			FText::FromName(Target->GetImmatriculation()));
	}
	else if (!Paused)
	{
		ModeText = FText::Format(LOCTEXT("SpeedNotPausedFormat", "{0}m/s - {1} in {2}"),
			FText::AsNumber(FMath::RoundToInt(GetLinearVelocity().Size())),
			GetWeaponsSystem()->GetWeaponModeInfo(),
			CurrentSector->GetSimulatedSector()->GetSectorName());
	}
	else
	{
		ModeText = FText::Format(LOCTEXT("SpeedPausedFormat-", "{1} in {2}"),
			GetWeaponsSystem()->GetWeaponModeInfo(),
			CurrentSector->GetSimulatedSector()->GetSectorName());
	}

	return FText::Format(LOCTEXT("ShipInfoTextFormat", "{0}{1} {2}"),
		PauseText, ModeText, AutopilotText);
}

/** Linear velocity, in m/s in world reference*/
FVector AFlareSpacecraft::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}

#undef LOCTEXT_NAMESPACE
