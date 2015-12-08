
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Game/FlareGameTools.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
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
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlarePlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnableCheats();

	// Menu manager
	SetupMenu();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);

	// Sound manager
	SoundManager = NewObject<UFlareSoundManager>(this, UFlareSoundManager::StaticClass());
	if (SoundManager)
	{
		SoundManager->Setup(this);
	}
}

void AFlarePlayerController::PlayerTick(float DeltaSeconds)
{
	Super::PlayerTick(DeltaSeconds);
	AFlareHUD* HUD = GetNavHUD();
	TimeSinceWeaponSwitch += DeltaSeconds;

	if (ShipPawn)
	{
		HUD->SetInteractive(ShipPawn->GetStateManager()->IsWantContextMenu());
	}

	// Mouse cursor
	bool NewShowMouseCursor = !HUD->IsMouseMenuOpen() ;
	if (!MenuManager->IsMenuOpen() && ShipPawn && !ShipPawn->GetStateManager()->IsWantCursor())
	{
		NewShowMouseCursor = false;
	}

	if (NewShowMouseCursor != bShowMouseCursor)
	{
		// Set the mouse status
		FLOGV("AFlarePlayerController::PlayerTick : New mouse cursor state is %d", NewShowMouseCursor);
		bShowMouseCursor = NewShowMouseCursor;

		ResetMousePosition();

#if PLATFORM_LINUX
		// This code fix missing cursor bug but cause focus regression.
		if (NewShowMouseCursor)
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
#endif

		// Force focus to UI
		if (NewShowMouseCursor || HUD->IsMouseMenuOpen())
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
		FVector ViewLocation;
		FRotator ViewRotation;

		GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewRotation.Normalize();
		ViewLocation += ViewRotation.RotateVector(500 * FVector(1, 0, 0));

		FVector Velocity = ShipPawn->GetLinearVelocity();
		FLinearColor Color = FLinearColor::White;
		Color.R= FMath::Clamp(Velocity.Size() / 100, 0.0f, 1.0f);
		DustEffect->SetColorParameter("Intensity", Color);

		FVector Direction = Velocity;
		Direction.Normalize();
		DustEffect->SetWorldLocation(ViewLocation);
		DustEffect->SetVectorParameter("Direction", -Direction);
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
	// Reset the current ship to auto
	if (ShipPawn)
	{
		ShipPawn->GetStateManager()->EnablePilot(true);
	}

	// Fly the new ship
	if (PossessNow)
	{
		Possess(Ship);
	}
	ShipPawn = Ship;
	SetExternalCamera(true);
	ShipPawn->GetStateManager()->EnablePilot(false);
	ShipPawn->GetWeaponsSystem()->DeactivateWeapons();

	// Register ambient sounds
	if (SoundManager)
	{
		SoundManager->SetCurrentSpacecraft(Ship);
	}

	// Inform the player
	if (Ship)
	{
		// Notification
		FText Text = FText::Format(LOCTEXT("FlyingFormat", "Now flying {0}"), FText::FromName(Ship->GetImmatriculation()));
		FText Info = LOCTEXT("FlyingInfo", "You can switch to nearby ships with N.");
		Notify(Text, Info, "flying-info", EFlareNotification::NT_Help);

		// HUD update
		GetNavHUD()->OnTargetShipChanged();
		SetSelectingWeapon();

		GetGame()->GetQuestManager()->OnFlyShip(Ship);
	}
}

void AFlarePlayerController::PrepareForExit()
{
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
	SelectedFleet = GetGame()->GetGameWorld()->FindFleet(PlayerData.SelectedFleetIdentifier);
	Company = GetGame()->GetGameWorld()->FindCompany(PlayerData.CompanyIdentifier);
}

void AFlarePlayerController::OnLoadComplete()
{
	SetWorldPause(true);
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::OnSectorActivated()
{
	FLOGV("AFlarePlayerController::OnSectorActivated LastFlownShip=%s", *GetGame()->GetActiveSector()->GetData()->LastFlownShip.ToString());
	bool CandidateFound = false;

	if (GetGame()->GetActiveSector()->GetData()->LastFlownShip != "")
	{
		FLOG("AFlarePlayerController::OnSectorActivated not null last ship");
		AFlareSpacecraft* Candidate = GetGame()->GetActiveSector()->FindSpacecraft(GetGame()->GetActiveSector()->GetData()->LastFlownShip);
		if (Candidate)
		{
			FLOG("AFlarePlayerController::OnSectorActivated last ship found");
			CandidateFound = true;

			// Disable pilot during the switch
			Candidate->GetStateManager()->EnablePilot(false);
			MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Candidate);
		}
	}

	if (!CandidateFound)
	{
		FLOG("AFlarePlayerController::OnSectorActivated no candidate");
		QuickSwitch();
	}
}

void AFlarePlayerController::OnSectorDeactivated()
{
	ShipPawn = NULL;
	MenuManager->OpenMenu(EFlareMenu::MENU_Orbit);
}

void AFlarePlayerController::Save(FFlarePlayerSave& SavePlayerData, FFlareCompanyDescription& SaveCompanyData)
{
	if (SelectedFleet)
	{
		PlayerData.SelectedFleetIdentifier = SelectedFleet->GetIdentifier();
	}
	else
	{
		PlayerData.SelectedFleetIdentifier = NAME_None;
	}
	SavePlayerData = PlayerData;
	SaveCompanyData = CompanyData;
}

void AFlarePlayerController::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}

void AFlarePlayerController::SetLastFlownShip(FName LastFlownShipIdentifier)
{
	PlayerData.LastFlownShipIdentifier = LastFlownShipIdentifier;
}

/*----------------------------------------------------
	Menus
----------------------------------------------------*/

void AFlarePlayerController::Notify(FText Title, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	FLOGV("AFlarePlayerController::Notify : '%s'", *Title.ToString());
	MenuManager->Notify(Title, Info, Tag, Type, Timeout, TargetMenu, TargetInfo);
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
	GetNavHUD()->UpdateHUDVisibility();
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
		SetSelectingWeapon();
	}

	GetNavHUD()->UpdateHUDVisibility();
}

void AFlarePlayerController::SetWorldPause(bool Pause)
{
	FLOGV("AFlarePlayerController::SetWorldPause world %d", Pause);

	if (GetGame()->GetActiveSector())
	{
		GetGame()->GetActiveSector()->SetPause(Pause);
	}
}

void AFlarePlayerController::SelectFleet(UFlareFleet* Fleet)
{
	if (Fleet == NULL)
	{
		FLOG("Select no fleet");
	}
	else
	{
		FLOGV("Select fleet %s : %s", *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName());
	}
	SelectedFleet = Fleet;
}

UFlareFleet* AFlarePlayerController::GetSelectedFleet()
{
	return SelectedFleet;
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
	if (ShipPawn)
	{
		SetExternalCamera(!ShipPawn->GetStateManager()->IsExternalCamera());
	}
}

void AFlarePlayerController::ToggleMenu()
{
	if (GetGame()->IsLoadedOrCreated() && GetGame()->GetActiveSector())
	{
		if (IsInMenu())
		{
			MenuManager->CloseMenu();
		}
		else
		{
			MenuManager->OpenMenu(EFlareMenu::MENU_Dashboard);
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

void AFlarePlayerController::SettingsMenu()
{
	FLOG("AFlarePlayerController::SettingsMenu");
	MenuManager->OpenMenu(EFlareMenu::MENU_Settings);
}

void AFlarePlayerController::ToggleCombat()
{
	if (ShipPawn && ShipPawn->IsMilitary() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
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
	FLOG("AFlarePlayerController::QuickSwitch");

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

				if (Candidate->GetDamageSystem()->IsAlive()
					&& Candidate->IsMilitary()
					&& Candidate->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
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
					if (Candidate && Candidate != ShipPawn && Candidate->GetDamageSystem()->IsAlive())
					{
						SeletedCandidate = Candidate;
						break;
					}
				}
			}

			// Switch to the found ship
			if (SeletedCandidate)
			{
				FLOG("AFlarePlayerController::QuickSwitch : found new ship");
				QuickSwitchNextOffset = OffsetIndex + 1;
				// Disable pilot during the switch
				SeletedCandidate->GetStateManager()->EnablePilot(false);
				MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, SeletedCandidate);
			}
			else
			{
				FLOG("AFlarePlayerController::QuickSwitch : no ship found");
			}
		}
		else
		{
			FLOG("AFlarePlayerController::QuickSwitch : no ships in company !");
		}
	}
	else
	{
		FLOG("AFlarePlayerController::QuickSwitch : company !");
	}
}

void AFlarePlayerController::MouseInputX(float Val)
{
	if (MenuManager->IsMenuOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->YawInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsMouseMenuOpen())
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
	if (MenuManager->IsMenuOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->PitchInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsMouseMenuOpen())
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
}

void AFlarePlayerController::Test2()
{
}


/*----------------------------------------------------
	Wheel menu
----------------------------------------------------*/

void AFlarePlayerController::WheelPressed()
{
	if (GetGame()->IsLoadedOrCreated() && MenuManager && !MenuManager->IsMenuOpen() && !GetNavHUD()->IsMouseMenuOpen())
	{
		TSharedPtr<SFlareMouseMenu> MouseMenu = GetNavHUD()->GetMouseMenu();

		// Setup mouse menu
		MouseMenu->ClearWidgets();
		MouseMenu->AddDefaultWidget("Mouse_Nothing", LOCTEXT("Cancel", "Cancel"));

		// Docked controls
		if (ShipPawn->GetNavigationSystem()->IsDocked())
		{
			MouseMenu->AddWidget("ShipUpgrade_Button", LOCTEXT("Upgrade", "Upgrade"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UpgradeShip));
			MouseMenu->AddWidget("Undock_Button", LOCTEXT("Undock", "Undock"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UndockShip));
		}

		// Flying controls
		else
		{
			if (ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_BOMB)
			{
				MouseMenu->AddWidget("Mouse_Align", LOCTEXT("Align", "Forward"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::AlignToSpeed));
			}

			MouseMenu->AddWidget("Mouse_Brake", LOCTEXT("Brake", "Brake"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::Brake));

			if (ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_BOMB)
			{
				MouseMenu->AddWidget("Mouse_Reverse", LOCTEXT("Backward", "Backward"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::AlignToReverse));
			}

			AFlareSpacecraft* Nearest = GetNearestSpacecraft(true);
			if (Nearest)
			{
				FText Text = FText::Format(LOCTEXT("MatchSpeedFormat", "Match speed with {0}"), FText::FromName(Nearest->GetImmatriculation()));
				MouseMenu->AddWidget("Mouse_MatchSpeed", Text,
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::MatchSpeedWithNearestSpacecraft));
			}
			else
			{
				MouseMenu->AddWidget("Mouse_LookAt", LOCTEXT("FindNearest", "Look at nearest spacecraft"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::LookAtNearestSpacecraft));
			}

		}

		GetNavHUD()->SetWheelMenu(true);
	}
}

void AFlarePlayerController::WheelReleased()
{
	if (GetGame()->IsLoadedOrCreated())
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

AFlareSpacecraft* AFlarePlayerController::GetNearestSpacecraft(bool OnScreenRequired)
{
	AFlareSpacecraft* TargetSpacecraft = NULL;
	if (ShipPawn)
	{
		float TargetDistance = 0;
		TArray<AFlareSpacecraft*> CompanySpacecrafs = GetGame()->GetActiveSector()->GetCompanySpacecrafts(Company);
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < CompanySpacecrafs.Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Spacecraft = CompanySpacecrafs[SpacecraftIndex];
			if (Spacecraft && Spacecraft != ShipPawn)
			{
				float Distance = (Spacecraft->GetActorLocation() - ShipPawn->GetActorLocation()).Size();
				if (!TargetSpacecraft || Distance < TargetDistance)
				{
					FVector2D ScreenPosition;
					if (!OnScreenRequired || ProjectWorldLocationToScreen(Spacecraft->GetActorLocation(), ScreenPosition))
					{
						TargetSpacecraft = Spacecraft;
						TargetDistance = Distance;
					}
				}
			}
		}
	}

	if (TargetSpacecraft)
	{
		FLOGV("AFlarePlayerController::GetNearestSpacecraft : Found %s", *TargetSpacecraft->GetImmatriculation().ToString());
	}
	return TargetSpacecraft;
}

void AFlarePlayerController::MatchSpeedWithNearestSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = GetNearestSpacecraft(true);
		if (TargetSpacecraft)
		{
			ShipPawn->ForceManual();
			ShipPawn->BrakeToVelocity(TargetSpacecraft->GetLinearVelocity());
		}
	}
}

void AFlarePlayerController::LookAtNearestSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = GetNearestSpacecraft(false);
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
	MenuManager->OpenMenu(EFlareMenu::MENU_ShipConfig);
}

void AFlarePlayerController::UndockShip()
{
	if (ShipPawn)
	{
		ShipPawn->GetNavigationSystem()->Undock();
	}
}

/*----------------------------------------------------
	Getters for game classes
----------------------------------------------------*/

UFlareSimulatedSpacecraft* AFlarePlayerController::GetLastFlownShip()
{
	UFlareWorld* GameWorld = GetGame()->GetGameWorld();
	if (GameWorld)
	{
		return GameWorld->FindSpacecraft(PlayerData.LastFlownShipIdentifier);
	}
	else
	{
		return NULL;
	}
}


#undef LOCTEXT_NAMESPACE
