
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Game/FlareGameTools.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
#include "../Game/FlareGameUserSettings.h"
#include "../Game/AI/FlareCompanyAI.h"
#include "FlareMenuManager.h"
#include "EngineUtils.h"


#define LOCTEXT_NAMESPACE "AFlarePlayerController"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlayerController::AFlarePlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, DustEffect(NULL)
	, Company(NULL)
	, WeaponSwitchTime(10.0f)
	, TimeSinceWeaponSwitch(0)
{
	CheatClass = UFlareGameTools::StaticClass();

	// Sounds
	static ConstructorHelpers::FObjectFinder<USoundCue> OnSoundObj(TEXT("/Game/Master/Sound/A_Beep_On"));
	static ConstructorHelpers::FObjectFinder<USoundCue> OffSoundObj(TEXT("/Game/Master/Sound/A_Beep_Off"));
	OnSound = OnSoundObj.Object;
	OffSound = OffSoundObj.Object;
	
	// Mouse
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_Dust"));
	DustEffectTemplate = DustEffectTemplateObj.Object;
	DefaultMouseCursor = EMouseCursor::Default;
	
	// Gameplay
	QuickSwitchNextOffset = 0;
	CurrentObjective.Set = false;
	CurrentObjective.Version = 0;
	IsTest1 = false;
	IsTest2 = false;
	LastBattleState = EFlareSectorBattleState::NoBattle;

	// Setup
	ShipPawn = NULL;
	PlayerShip = NULL;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlarePlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnableCheats();

	// Settings
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	check(MyGameSettings);
	UseCockpit = MyGameSettings->UseCockpit;
	PauseGameInMenus = MyGameSettings->PauseGameInMenus;

	// Cockpit
	SetupCockpit();

	// Menu manager
	SetupMenu();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);

	// Sound manager
	SoundManager = NewObject<UFlareSoundManager>(this, UFlareSoundManager::StaticClass());
	if (SoundManager)
	{
		SoundManager->Setup(this);
		SoundManager->SetMusicVolume(MyGameSettings->MusicVolume);
		SoundManager->SetMasterVolume(MyGameSettings->MasterVolume);
	}
}

void AFlarePlayerController::PlayerTick(float DeltaSeconds)
{
	Super::PlayerTick(DeltaSeconds);
	AFlareHUD* HUD = GetNavHUD();
	TimeSinceWeaponSwitch += DeltaSeconds;

	// We are flying
	if (ShipPawn)
	{
		HUD->SetInteractive(ShipPawn->GetStateManager()->IsWantContextMenu());
		
		UFlareSimulatedSector* Sector = ShipPawn->GetParent()->GetCurrentSector();
		GetCompany()->GetAI()->ResetControlGroups(Sector);


		// Battle state
		if (GetGame()->GetActiveSector())
		{
			EFlareSectorBattleState::Type BattleState = GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorBattleState(GetCompany());
			if (BattleState != LastBattleState)
			{
				LastBattleState = BattleState;
				OnBattleStateChanged(BattleState);
			}
		}

		// FLIR Debug Code. Keep it for future ship setup
		/*TArray<FName> SocketNames  = ShipPawn->Airframe->GetAllSocketNames();
		for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); SocketIndex++)
		{
			FLOGV("Check socket = %s", *SocketNames[SocketIndex].ToString());
			if (SocketNames[SocketIndex] == "Dock" || SocketNames[SocketIndex].ToString().StartsWith("FLIR"))
			{
				FTransform CameraWorldTransform = ShipPawn->Airframe->GetSocketTransform(SocketNames[SocketIndex]);

				FVector CameraLocation = CameraWorldTransform.GetTranslation();
				FVector CandidateCameraMainDirection = CameraWorldTransform.GetRotation().RotateVector(FVector(1,0,0));

				DrawDebugLine(GetWorld(), CameraLocation, CameraLocation + 500 * CandidateCameraMainDirection, FColor::Red, false);
				DrawDebugSphere(GetWorld(), CameraLocation, 50, 32, FColor::Green, false);
			}
		}*/
	}

	// Mouse cursor
	bool NewShowMouseCursor = true;
	if (!MenuManager->IsUIOpen() && ShipPawn && !ShipPawn->GetStateManager()->IsWantCursor())
	{
		NewShowMouseCursor = false;
	}

	if (NewShowMouseCursor != bShowMouseCursor)
	{
		// Set the mouse status
		FLOGV("AFlarePlayerController::PlayerTick : New mouse cursor state is %d", NewShowMouseCursor);
		bShowMouseCursor = NewShowMouseCursor;

		ResetMousePosition();

		// Force focus to UI
		if (NewShowMouseCursor)
		{
			FInputModeGameAndUI InputMode;
			SetInputMode(InputMode);

			if (!NewShowMouseCursor)
			{
				ULocalPlayer* LocalPlayer = Cast< ULocalPlayer >( Player );

				UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport();
				TSharedPtr<SViewport> ViewportWidget = GameViewportClient->GetGameViewportWidget();
				if (ViewportWidget.IsValid())
				{
					TSharedRef<SViewport> ViewportWidgetRef = ViewportWidget.ToSharedRef();
					LocalPlayer->GetSlateOperations().UseHighPrecisionMouseMovement(ViewportWidgetRef);
				}
			}
		}

		// Force focus to game
		else
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}

	// Spawn dust effects if they are not already here
	if (!DustEffect && ShipPawn)
	{
		DustEffect = UGameplayStatics::SpawnEmitterAtLocation(this, DustEffectTemplate, FVector::ZeroVector);
	}

	// Update dust effects
	if (DustEffect && ShipPawn && !IsInMenu())
	{
		// Ship velocity
		FVector Velocity = ShipPawn->GetLinearVelocity();
		FVector Direction = Velocity;
		Direction.Normalize();

		// Particle position
		FVector ViewLocation;
		FRotator ViewRotation;
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewLocation += Direction.Rotation().RotateVector(5000 * FVector(1, 0, 0));
		DustEffect->SetWorldLocation(ViewLocation);

		// Particle params
		float VelocityFactor = FMath::Clamp(Velocity.Size() / 100.0f, 0.0f, 1.0f);
		FLinearColor Color = FLinearColor::White * VelocityFactor;
		DustEffect->SetFloatParameter("SpawnCount", VelocityFactor);
		DustEffect->SetColorParameter("Intensity", Color);
		DustEffect->SetVectorParameter("Direction", -Direction);
		DustEffect->SetVectorParameter("Size", FVector(1, VelocityFactor, 1));
	}
	else if (DustEffect)
	{
		DustEffect->SetColorParameter("Intensity", FVector::ZeroVector);
		DustEffect->SetColorParameter("Direction", FVector::ZeroVector);
	}

	// Sound
	if (SoundManager)
	{
		SoundManager->Update(DeltaSeconds);
	}
}

void AFlarePlayerController::SetExternalCamera(bool NewState)
{
	if (ShipPawn)
	{
		// No internal camera when docked
		if (ShipPawn && ShipPawn->GetNavigationSystem()->IsDocked())
		{
			NewState = true;
		}

		// Send the camera order to the ship
		ShipPawn->GetStateManager()->SetExternalCamera(NewState);
	}
}

void AFlarePlayerController::FlyShip(AFlareSpacecraft* Ship, bool PossessNow)
{
	check(Ship);

	if (ShipPawn == Ship)
	{
		// Already flying this ship
		// Fly the new ship
		if (PossessNow)
		{
			Possess(Ship);
		}

		return;
	}

	// Reset the current ship to auto
	if (ShipPawn)
	{
		ShipPawn->ResetCurrentTarget();
		ShipPawn->GetStateManager()->EnablePilot(true);
	}

	// Fly the new ship
	if (PossessNow)
	{
		Ship->ResetCurrentTarget();
		Possess(Ship);
	}

	// Setup everything
	ShipPawn = Ship;
	SetExternalCamera(false);
	ShipPawn->GetStateManager()->EnablePilot(false);
	ShipPawn->GetWeaponsSystem()->DeactivateWeapons();
	CockpitManager->OnFlyShip(ShipPawn);

	// Setup FOV
	if (ShipPawn->GetParent()->IsMilitary())
	{
		PlayerCameraManager->SetFOV(92);
	}
	else
	{
		PlayerCameraManager->SetFOV(93);
	}

	// Combat groups
	Company->GetAI()->SetCurrentShipGroup(EFlareCombatGroup::AllMilitary);
	Company->GetAI()->ResetShipGroup(EFlareCombatTactic::ProtectMe);

	// Set player ship
	SetPlayerShip(ShipPawn->GetParent());
	GetGame()->GetQuestManager()->OnFlyShip(Ship);

	// Update HUD
	GetNavHUD()->OnTargetShipChanged();
	SetSelectingWeapon();
}

void AFlarePlayerController::ExitShip()
{
	ShipPawn = NULL;
}

void AFlarePlayerController::PrepareForExit()
{
	PlayerShip = NULL;

	if (IsInMenu())
	{
		MenuManager->CloseMenu(true);
	}
}


/*----------------------------------------------------
	Data management
----------------------------------------------------*/

void AFlarePlayerController::SetCompanyDescription(const FFlareCompanyDescription& SaveCompanyData)
{
	CompanyData = SaveCompanyData;
}

void AFlarePlayerController::Load(const FFlarePlayerSave& SavePlayerData)
{
	PlayerData = SavePlayerData;
	Company = GetGame()->GetGameWorld()->FindCompany(PlayerData.CompanyIdentifier);
}

void AFlarePlayerController::OnLoadComplete()
{
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::OnSectorActivated(UFlareSector* ActiveSector)
{
	FLOGV("AFlarePlayerController::OnSectorActivated %s", *ActiveSector->GetSimulatedSector()->GetData()->Identifier.ToString());
	bool CandidateFound = false;

	// Last flown ship

	if(PlayerShip && PlayerShip->IsActive())
	{
		// Disable pilot during the switch
		PlayerShip->GetActive()->GetStateManager()->EnablePilot(false);
		FlyShip(PlayerShip->GetActive(), false);
	}
	else
	{
		FLOG("AFlarePlayerController::OnSectorActivated no candidate");
		SwitchToNextShip(true);
	}

	// Level music
	EFlareMusicTrack::Type LevelMusic = ActiveSector->GetSimulatedSector()->GetDescription()->LevelTrack;
	if (LevelMusic != EFlareMusicTrack::None)
	{
		SoundManager->RequestMusicTrack(LevelMusic);
	}
	else
	{
		SoundManager->RequestMusicTrack(EFlareMusicTrack::Exploration);
	}
}

void AFlarePlayerController::OnSectorDeactivated()
{
	FLOG("AFlarePlayerController::OnSectorDeactivated");

	// Reset the ship
	CockpitManager->OnStopFlying();
	if (ShipPawn)
	{
		ShipPawn->ResetCurrentTarget();
		ShipPawn = NULL;
	}

	// Reset the HUD
	if (GetNavHUD())
	{
		GetNavHUD()->RemoveAllTargets();
	}

	// Reset states
	LastBattleState = EFlareSectorBattleState::NoBattle;
}

void AFlarePlayerController::OnBattleStateChanged(EFlareSectorBattleState::Type NewBattleState)
{
	FLOG("AFlarePlayerController::OnBattleStateChanged");

	if (NewBattleState == EFlareSectorBattleState::NoBattle)
	{
		FLOG("AFlarePlayerController::OnBattleStateChanged : peace");
		if (GetGame()->GetActiveSector())
		{
			EFlareMusicTrack::Type LevelMusic = GetGame()->GetActiveSector()->GetSimulatedSector()->GetDescription()->LevelTrack;
			if (LevelMusic != EFlareMusicTrack::None)
			{
				SoundManager->RequestMusicTrack(LevelMusic);
			}
			else
			{
				SoundManager->RequestMusicTrack(EFlareMusicTrack::Exploration);
			}
		}
		else
		{
			SoundManager->RequestMusicTrack(EFlareMusicTrack::Exploration);
		}
	}
	else
	{
		FLOG("AFlarePlayerController::OnBattleStateChanged : battle");
		SoundManager->RequestMusicTrack(EFlareMusicTrack::Combat);
	}
}

void AFlarePlayerController::Save(FFlarePlayerSave& SavePlayerData, FFlareCompanyDescription& SaveCompanyData)
{
	SavePlayerData = PlayerData;
	SaveCompanyData = CompanyData;
}

void AFlarePlayerController::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}

void AFlarePlayerController::SetPlayerShip(UFlareSimulatedSpacecraft* NewPlayerShip)
{
	PlayerData.LastFlownShipIdentifier = NewPlayerShip->GetImmatriculation();
	PlayerShip = NewPlayerShip;
}

void AFlarePlayerController::Clean()
{
	PlayerData.ScenarioId = 0;
	PlayerData.CompanyIdentifier = NAME_None;
	PlayerData.LastFlownShipIdentifier = NAME_None;

	ShipPawn = NULL;
	PlayerShip = NULL;
	Company = NULL;

	CurrentObjective.Set = false;
	CurrentObjective.Version = 0;

	QuickSwitchNextOffset = 0;
	TimeSinceWeaponSwitch = 0;

	LastBattleState = EFlareSectorBattleState::NoBattle;

	MenuManager->FlushNotifications();
}


/*----------------------------------------------------
	Menus
----------------------------------------------------*/

void AFlarePlayerController::Notify(FText Title, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	FLOGV("AFlarePlayerController::Notify : '%s'", *Title.ToString());
	MenuManager->Notify(Title, Info, Tag, Type, Pinned, TargetMenu, TargetInfo);
}

void AFlarePlayerController::SetupCockpit()
{
	if (!CockpitManager)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = Instigator;
		SpawnInfo.ObjectFlags |= RF_Transient;
		CockpitManager = GetWorld()->SpawnActor<AFlareCockpitManager>(AFlareCockpitManager::StaticClass(), SpawnInfo);
	}
	CockpitManager->SetupCockpit(this);
}

void AFlarePlayerController::SetupMenu()
{
	// Save the default pawn
	APawn* DefaultPawn = GetPawn();

	// Spawn the menu pawn at an arbitrarily large location
	FVector SpawnLocation(1000000 * FVector(-1, -1, -1));
	MenuPawn = GetWorld()->SpawnActor<AFlareMenuPawn>(GetGame()->GetMenuPawnClass(), SpawnLocation, FRotator::ZeroRotator);
	Possess(MenuPawn);
	
	// Spawn the menu manager
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;
	MenuManager = GetWorld()->SpawnActor<AFlareMenuManager>(AFlareMenuManager::StaticClass(), SpawnInfo);

	// Setup menus and HUD
	MenuManager->SetupMenu();
	GetNavHUD()->Setup(MenuManager);

	// Destroy the old pawn
	if (DefaultPawn)
	{
		DefaultPawn->Destroy();
	}
}

void AFlarePlayerController::OnEnterMenu()
{
	if (!IsInMenu())
	{
		ClientPlaySound(OnSound);
		Possess(MenuPawn);

		// Pause all gameplay actors
		SetWorldPause(true);
		MenuPawn->SetActorHiddenInGame(false);
	}

	GetGame()->GetPlanetarium()->SetActorHiddenInGame(true);
	GetNavHUD()->UpdateHUDVisibility();
}

void AFlarePlayerController::OnExitMenu()
{
	if (IsInMenu())
	{
		// Quit menu
		MenuPawn->SetActorHiddenInGame(true);
		ClientPlaySound(OffSound);

		// Unpause all gameplay actors
		SetWorldPause(false);

		// Fly the ship
		Possess(ShipPawn);
		GetNavHUD()->OnTargetShipChanged();
		CockpitManager->OnFlyShip(ShipPawn);
		SetSelectingWeapon();
	}

	if (GetNavHUD())
	{
		GetNavHUD()->UpdateHUDVisibility();
	}
	GetGame()->GetPlanetarium()->SetActorHiddenInGame(false);
}

void AFlarePlayerController::SetWorldPause(bool Pause)
{
	FLOGV("AFlarePlayerController::SetWorldPause world %d", Pause);

	if (PauseGameInMenus && GetGame()->GetActiveSector())
	{
		GetGame()->SetWorldPause(Pause);
		GetGame()->GetActiveSector()->SetPause(Pause);
	}
}

UFlareFleet* AFlarePlayerController::GetPlayerFleet()
{
	return GetPlayerShip()->GetCurrentFleet();
}

bool AFlarePlayerController::SwitchToNextShip(bool Instant)
{
	FLOG("AFlarePlayerController::SwitchToNextShip");

	if (GetGame()->GetActiveSector() && Company)
	{
		TArray<AFlareSpacecraft*> CompanyShips = GetGame()->GetActiveSector()->GetCompanyShips(Company);

		if (CompanyShips.Num())
		{
			int32 QuickSwitchOffset = QuickSwitchNextOffset;
			int32 OffsetIndex = 0;
			AFlareSpacecraft* SeletedCandidate = NULL;

			// First loop in military armed alive ships
			for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
			{
				OffsetIndex = (ShipIndex + QuickSwitchOffset) % CompanyShips.Num();
				AFlareSpacecraft* Candidate = CompanyShips[OffsetIndex];

				if (Candidate && Candidate != ShipPawn && Candidate->GetParent()->CanFight())
				{
					SeletedCandidate = Candidate;
					break;
				}
			}

			// If not, loop in all alive ships
			if (!SeletedCandidate)
			{
				for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
				{
					OffsetIndex = (ShipIndex + QuickSwitchOffset) % CompanyShips.Num();
					AFlareSpacecraft* Candidate = CompanyShips[OffsetIndex];
					if (Candidate && Candidate != ShipPawn && Candidate->GetParent()->GetDamageSystem()->IsAlive())
					{
						SeletedCandidate = Candidate;
						break;
					}
				}
			}

			// Switch to the found ship
			if (SeletedCandidate)
			{
				FLOGV("AFlarePlayerController::SwitchToNextShip : found new ship %s", *SeletedCandidate->GetImmatriculation().ToString());
				QuickSwitchNextOffset = OffsetIndex + 1;
				// Disable pilot during the switch
				SeletedCandidate->GetStateManager()->EnablePilot(false);

				if (Instant)
				{
					FlyShip(SeletedCandidate, false);
				}
				else
				{
					FFlareMenuParameterData Data;
					Data.Spacecraft = SeletedCandidate->GetParent();
					MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
				}

				return true;
			}
			else
			{
				FLOG("AFlarePlayerController::SwitchToNextShip : no ship found");
			}
		}
		else
		{
			FLOG("AFlarePlayerController::SwitchToNextShip : no ships in company !");
		}
	}
	else
	{
		FLOG("AFlarePlayerController::SwitchToNextShip : no sector or company !");
	}

	return false;
}

bool AFlarePlayerController::IsInMenu()
{
	return (GetPawn() == MenuPawn);
}

FVector2D AFlarePlayerController::GetMousePosition()
{
	FVector2D Result;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D ScreenPosition;
		LocalPlayer->ViewportClient->GetMousePosition(Result);
	}

	return Result;
}

void AFlarePlayerController::ResetMousePosition()
{
	auto& App = FSlateApplication::Get();
	FVector2D CursorPos = App.GetCursorPos();
	App.SetCursorPos(CursorPos + FVector2D(0, 1));
	App.OnMouseMove();
	App.SetCursorPos(CursorPos);
	App.OnMouseMove();
	App.SetAllUserFocusToGameViewport();
}

void AFlarePlayerController::SetSelectingWeapon()
{
	TimeSinceWeaponSwitch = 0;
}

bool AFlarePlayerController::IsSelectingWeapon() const
{
	return (TimeSinceWeaponSwitch < WeaponSwitchTime);
}

void AFlarePlayerController::NotifyDockingResult(bool Success, UFlareSimulatedSpacecraft* Target)
{
	if (Success)
	{
		Notify(
			LOCTEXT("DockingGranted", "Docking granted"),
			FText::Format(
				LOCTEXT("DockingGrantedInfoFormat", "Your ship is now automatically docking at {0}. Using manual controls will abort docking."),
				FText::FromName(Target->GetImmatriculation())),
			"docking-granted",
			EFlareNotification::NT_Info,
			false);
	}
	else
	{
		Notify(
			LOCTEXT("DockingDenied", "Docking denied"),
			FText::Format(LOCTEXT("DockingDeniedInfoFormat", "{0} denied your docking request"), FText::FromName(Target->GetImmatriculation())),
			"docking-denied",
			EFlareNotification::NT_Info,
			false);
	}
}


/*----------------------------------------------------
	Objectives
----------------------------------------------------*/

void AFlarePlayerController::StartObjective(FText Name, FFlarePlayerObjectiveData Data)
{
	if (!CurrentObjective.Set)
	{
		CurrentObjective.Set = true;
		CurrentObjective.Version++;
	}

	CurrentObjective.Data = Data;
}

void AFlarePlayerController::CompleteObjective()
{
	CurrentObjective.Set = false;
}

bool AFlarePlayerController::HasObjective() const
{
	return CurrentObjective.Set;
}

const FFlarePlayerObjective* AFlarePlayerController::GetCurrentObjective() const
{
	return (CurrentObjective.Set? &CurrentObjective : NULL);
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlarePlayerController::SetBasePaintColorIndex(int32 Index)
{
	CompanyData.CustomizationBasePaintColorIndex = Index;
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::SetPaintColorIndex(int32 Index)
{
	CompanyData.CustomizationPaintColorIndex = Index;
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::SetOverlayColorIndex(int32 Index)
{
	CompanyData.CustomizationOverlayColorIndex = Index;
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::SetLightColorIndex(int32 Index)
{
	CompanyData.CustomizationLightColorIndex = Index;
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::SetPatternIndex(int32 Index)
{
	CompanyData.CustomizationPatternIndex = Index;
	Company->UpdateCompanyCustomization();
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlarePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ToggleCamera", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCamera);
	InputComponent->BindAction("ToggleMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleMenu);
	InputComponent->BindAction("BackMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::BackMenu);
	InputComponent->BindAction("Simulate", EInputEvent::IE_Released, this, &AFlarePlayerController::Simulate);
	InputComponent->BindAction("SettingsMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::SettingsMenu);
	InputComponent->BindAction("ToggleCombat", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCombat);
	InputComponent->BindAction("TooglePilot", EInputEvent::IE_Released, this, &AFlarePlayerController::TogglePilot);
	InputComponent->BindAction("ToggleHUD", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleHUD);
	InputComponent->BindAction("QuickSwitch", EInputEvent::IE_Released, this, &AFlarePlayerController::QuickSwitch);

	InputComponent->BindAction("Wheel", EInputEvent::IE_Pressed, this, &AFlarePlayerController::WheelPressed);
	InputComponent->BindAction("Wheel", EInputEvent::IE_Released, this, &AFlarePlayerController::WheelReleased);

	InputComponent->BindAxis("MouseInputX", this, &AFlarePlayerController::MouseInputX);
	InputComponent->BindAxis("MouseInputY", this, &AFlarePlayerController::MouseInputY);

	InputComponent->BindAction("Test1", EInputEvent::IE_Released, this, &AFlarePlayerController::Test1);
	InputComponent->BindAction("Test2", EInputEvent::IE_Released, this, &AFlarePlayerController::Test2);
}

void AFlarePlayerController::MousePositionInput(FVector2D Val)
{
	if (ShipPawn)
	{
		ShipPawn->GetStateManager()->SetPlayerMousePosition(Val);
	}
}

void AFlarePlayerController::ToggleCamera()
{
	if (ShipPawn && ShipPawn->GetParent()->GetDamageSystem()->IsAlive() && !MenuManager->IsUIOpen())
	{
		SetExternalCamera(!ShipPawn->GetStateManager()->IsExternalCamera());
	}
}

void AFlarePlayerController::ToggleMenu()
{
	if (GetGame()->IsLoadedOrCreated() && ShipPawn && GetGame()->GetActiveSector())
	{
		if (MenuManager->IsMenuOpen())
		{
			MenuManager->CloseMenu();
			MenuManager->CloseMainOverlay();
		}
		else if (MenuManager->IsOverlayOpen())
		{
			MenuManager->CloseMainOverlay();
		}
		else
		{
			MenuManager->OpenMainOverlay();
		}
	}
}

void AFlarePlayerController::BackMenu()
{
	FLOG("AFlarePlayerController::BackMenu");
	if (IsInMenu())
	{
		FLOG("AFlarePlayerController::BackMenu IsInMenu");
		MenuManager->Back();
	}
}

void AFlarePlayerController::Simulate()
{
	if (!GetGame()->IsLoadedOrCreated())
	{
		return;
	}
	

	if(GetGame()->GetActiveSector())
	{
		GetGame()->DeactivateSector();
	}

	GetGame()->GetGameWorld()->Simulate();

	GetGame()->ActivateCurrentSector();
}

void AFlarePlayerController::SettingsMenu()
{
	if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Settings)
	{
		FLOG("AFlarePlayerController::SettingsMenu");
		MenuManager->OpenMenu(EFlareMenu::MENU_Settings);
	}
}

void AFlarePlayerController::ToggleCombat()
{
	if (ShipPawn && ShipPawn->GetParent()->IsMilitary() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
	{
		FLOG("AFlarePlayerController::ToggleCombat");
		ShipPawn->GetWeaponsSystem()->ToogleWeaponActivation();
		if(ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			SetExternalCamera(false);
		}
	}
}

void AFlarePlayerController::TogglePilot()
{
	bool NewState = !ShipPawn->GetStateManager()->IsPilotMode();
	FLOGV("AFlarePlayerController::TooglePilot : new state is %d", NewState);
	ShipPawn->GetStateManager()->EnablePilot(NewState);
}

void AFlarePlayerController::ToggleHUD()
{
	if (!IsInMenu())
	{
		FLOG("AFlarePlayerController::ToggleHUD");
		GetNavHUD()->ToggleHUD();
	}
	else
	{
		FLOG("AFlarePlayerController::ToggleHUD : don't do it in menus");
	}
}

void AFlarePlayerController::QuickSwitch()
{
	SwitchToNextShip(false);
}

void AFlarePlayerController::MouseInputX(float Val)
{
	if (MenuManager->IsUIOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->YawInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		ShipPawn->YawInput(Val);
	}
}

void AFlarePlayerController::MouseInputY(float Val)
{
	if (MenuManager->IsUIOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->PitchInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, -Val));
	}
	else if (ShipPawn)
	{
		ShipPawn->PitchInput(Val);
	}
}

void AFlarePlayerController::Test1()
{
	IsTest1 = !IsTest1;
	FLOGV("AFlarePlayerController::Test1 %d", IsTest1);
}

void AFlarePlayerController::Test2()
{
	IsTest2 = !IsTest2;
	FLOGV("AFlarePlayerController::Test2 %d", IsTest2);
}


/*----------------------------------------------------
	Wheel menu
----------------------------------------------------*/

void AFlarePlayerController::WheelPressed()
{
	if (GetGame()->IsLoadedOrCreated() && MenuManager && !MenuManager->IsUIOpen() && !GetNavHUD()->IsWheelMenuOpen())
	{
		TSharedPtr<SFlareMouseMenu> MouseMenu = GetNavHUD()->GetMouseMenu();

		// Setup mouse menu
		MouseMenu->ClearWidgets();
		MouseMenu->AddDefaultWidget("Mouse_Nothing", LOCTEXT("Cancel", "Cancel"));

		// Docked controls
		if (ShipPawn->GetNavigationSystem()->IsDocked())
		{
			if (ShipPawn->GetParent()->GetCurrentSector()->CanUpgrade(ShipPawn->GetParent()->GetCompany()))
			{
				MouseMenu->AddWidget("ShipUpgrade_Button", LOCTEXT("Upgrade", "Upgrade"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UpgradeShip));
			}
			MouseMenu->AddWidget("Undock_Button", LOCTEXT("Undock", "Undock"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UndockShip));

			// Trade if possible
			if (ShipPawn->GetParent()->GetDescription()->CargoBayCount > 0)
			{
				MouseMenu->AddWidget("Trade_Button", LOCTEXT("Trade", "Trade"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::StartTrading));
			}
		}

		// Flying controls
		else
		{
			// Targetting
			AFlareSpacecraft* Target = ShipPawn->GetCurrentTarget();
			if (Target)
			{
				// Inspect
				FText Text = FText::Format(LOCTEXT("InspectTargetFormat", "Inspect {0}"), FText::FromName(Target->GetParent()->GetImmatriculation()));
				MouseMenu->AddWidget(Target->GetParent()->IsStation() ? "Mouse_Inspect_Station" : "Mouse_Inspect_Ship", Text,
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::InspectTargetSpacecraft));

				// Fly
				if (Target->GetParent()->GetCompany() == GetCompany() && !Target->GetParent()->IsStation())
				{
					Text = FText::Format(LOCTEXT("FlyTargetFormat", "Fly {0}"), FText::FromName(Target->GetParent()->GetImmatriculation()));
					MouseMenu->AddWidget("Mouse_Fly", Text,	FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::FlyTargetSpacecraft));
				}

				// Dock
				if (Target->GetDockingSystem()->HasCompatibleDock(GetShipPawn()) && Target->GetParent()->GetCompany()->GetPlayerWarState() >= EFlareHostility::Neutral)
				{
					Text = FText::Format(LOCTEXT("DockAtTargetFormat", "Dock at {0}"), FText::FromName(Target->GetParent()->GetImmatriculation()));
					MouseMenu->AddWidget("Mouse_DockAt", Text, FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::DockAtTargetSpacecraft));
				}
			}

			// Capital ship controls
			if (ShipPawn->GetParent()->GetDescription()->Size == EFlarePartSize::L && ShipPawn->IsMilitary())
			{
				MouseMenu->AddWidget("Mouse_ProtectMe", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::ProtectMe),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::ProtectMe));
				MouseMenu->AddWidget("Mouse_AttackAll", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackMilitary),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackMilitary));
				MouseMenu->AddWidget("Mouse_AttackStations", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackStations),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackStations));
				MouseMenu->AddWidget("Mouse_AttackCivilians", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackCivilians),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackCivilians));
				MouseMenu->AddWidget("Mouse_Nothing", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::StandDown),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::StandDown));
			}

			// Fighter controls
			else
			{
			}
		}

		GetNavHUD()->SetWheelMenu(true);
	}
}

void AFlarePlayerController::WheelReleased()
{
	if (GetGame()->IsLoadedOrCreated() && MenuManager && !MenuManager->IsUIOpen() && GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelMenu(false);
	}
}

void AFlarePlayerController::AlignToSpeed()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->FaceForward();
	}
}

void AFlarePlayerController::AlignToReverse()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->FaceBackward();
	}
}

void AFlarePlayerController::Brake()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->Brake();
	}
}

void AFlarePlayerController::InspectTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget();
		if (TargetSpacecraft)
		{
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft->GetParent();
			MenuManager->OpenMenu(EFlareMenu::MENU_Ship, Data);
		}
	}
}

void AFlarePlayerController::FlyTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget();
		if (TargetSpacecraft)
		{
			// Disable pilot during the switch
			TargetSpacecraft->GetStateManager()->EnablePilot(false);
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft->GetParent();
			MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
		}
	}
}

void AFlarePlayerController::DockAtTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget();
		if (TargetSpacecraft)
		{
			bool DockingConfirmed = ShipPawn->GetNavigationSystem()->DockAt(TargetSpacecraft);
			NotifyDockingResult(DockingConfirmed, TargetSpacecraft->GetParent());
		}
	}
}

void AFlarePlayerController::SetTacticForCurrentGroup(EFlareCombatTactic::Type Tactic)
{
	GetCompany()->GetAI()->SetTacticForCurrentShipGroup(Tactic);
}

void AFlarePlayerController::MatchSpeedWithTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget();
		if (TargetSpacecraft)
		{
			ShipPawn->ForceManual();
			ShipPawn->BrakeToVelocity(TargetSpacecraft->GetLinearVelocity());
		}
	}
}

void AFlarePlayerController::LookAtTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget();
		if (TargetSpacecraft)
		{
			FVector TargetDirection = (TargetSpacecraft->GetActorLocation() - ShipPawn->GetActorLocation());
			ShipPawn->ForceManual();
			ShipPawn->GetNavigationSystem()->PushCommandRotation(TargetDirection, FVector(1, 0, 0));
		}
	}
}

void AFlarePlayerController::UpgradeShip()
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = ShipPawn->GetParent();
	MenuManager->OpenMenu(EFlareMenu::MENU_ShipConfig, Data);
}

void AFlarePlayerController::UndockShip()
{
	if (ShipPawn)
	{
		ShipPawn->GetNavigationSystem()->Undock();
	}
}

void AFlarePlayerController::StartTrading()
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = ShipPawn->GetParent();
	MenuManager->OpenMenu(EFlareMenu::MENU_Trade, Data);
}


/*----------------------------------------------------
	Config
----------------------------------------------------*/

void AFlarePlayerController::SetUseCockpit(bool New)
{
	UseCockpit = New;
	CockpitManager->SetupCockpit(this);
}

void AFlarePlayerController::SetPauseGameInMenus(bool New)
{
	// Unpause if we are disabling the option
	if (New == false)
	{
		SetWorldPause(false);
	}

	PauseGameInMenus = New;

	// Pause if we are setting the option
	if (New == true)
	{
		SetWorldPause(true);
	}
}

void AFlarePlayerController::SetMusicVolume(int32 New)
{
	SoundManager->SetMusicVolume(New);
}

void AFlarePlayerController::SetMasterVolume(int32 New)
{
	SoundManager->SetMasterVolume(New);
}


/*----------------------------------------------------
	Getters for game classes
----------------------------------------------------*/

UFlareSimulatedSpacecraft* AFlarePlayerController::GetPlayerShip()
{
	UFlareSimulatedSpacecraft* Result = NULL;
	UFlareWorld* GameWorld = GetGame()->GetGameWorld();

	if (PlayerShip)
	{
		Result = PlayerShip;
	}
	else if (ShipPawn && ShipPawn->GetParent())
	{
		Result = ShipPawn->GetParent();
	}
	else if (GameWorld)
	{
		Result = GameWorld->FindSpacecraft(PlayerData.LastFlownShipIdentifier);
	}

	return Result;
}


#undef LOCTEXT_NAMESPACE
