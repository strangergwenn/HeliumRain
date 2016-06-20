
#include "../Flare.h"
#include "FlareMenuManager.h"

#include "../UI/Menus/FlareMainMenu.h"
#include "../UI/Menus/FlareSettingsMenu.h"
#include "../UI/Menus/FlareNewGameMenu.h"
#include "../UI/Menus/FlareStoryMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareFleetMenu.h"
#include "../UI/Menus/FlareOrbitalMenu.h"
#include "../UI/Menus/FlareLeaderboardMenu.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Menus/FlareTradeMenu.h"
#include "../UI/Menus/FlareTradeRouteMenu.h"
#include "../UI/Menus/FlareCreditsMenu.h"
#include "../UI/Menus/FlareResourcePricesMenu.h"
#include "../UI/Menus/FlareWorldEconomyMenu.h"

#include "../Player/FlarePlayerController.h"
#include "../HeliumRainLoadingScreen/FlareLoadingScreen.h"


#define LOCTEXT_NAMESPACE "FlareMenuManager"


AFlareMenuManager* AFlareMenuManager::Singleton;


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareMenuManager::AFlareMenuManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, MenuIsOpen(false)
	, FadeFromBlack(true)
	, FadeDuration(0.3)
{
}

void AFlareMenuManager::SetupMenu()
{
	Singleton = this;

	if (GEngine->IsValidLowLevel())
	{
		// Create regular menus
		SAssignNew(MainMenu, SFlareMainMenu).MenuManager(this);
		SAssignNew(SettingsMenu, SFlareSettingsMenu).MenuManager(this);
		SAssignNew(NewGameMenu, SFlareNewGameMenu).MenuManager(this);
		SAssignNew(StoryMenu, SFlareStoryMenu).MenuManager(this);
		SAssignNew(CompanyMenu, SFlareCompanyMenu).MenuManager(this);
		SAssignNew(FleetMenu, SFlareFleetMenu).MenuManager(this);
		SAssignNew(ShipMenu, SFlareShipMenu).MenuManager(this);
		SAssignNew(SectorMenu, SFlareSectorMenu).MenuManager(this);
		SAssignNew(TradeMenu, SFlareTradeMenu).MenuManager(this);
		SAssignNew(TradeRouteMenu, SFlareTradeRouteMenu).MenuManager(this);
		SAssignNew(OrbitMenu, SFlareOrbitalMenu).MenuManager(this);
		SAssignNew(LeaderboardMenu, SFlareLeaderboardMenu).MenuManager(this);
		SAssignNew(ResourcePricesMenu, SFlareResourcePricesMenu).MenuManager(this);
		SAssignNew(WorldEconomyMenu, SFlareWorldEconomyMenu).MenuManager(this);
		SAssignNew(CreditsMenu, SFlareCreditsMenu).MenuManager(this);

		// Create overlays
		SAssignNew(MainOverlay, SFlareMainOverlay).MenuManager(this);
		SAssignNew(Confirmation, SFlareConfirmationOverlay).MenuManager(this);
		SAssignNew(SpacecraftOrder, SFlareSpacecraftOrderOverlay).MenuManager(this);
		SAssignNew(Notifier, SFlareNotifier).MenuManager(this).Visibility(EVisibility::SelfHitTestInvisible);

		// Tooltip
		SAssignNew(Tooltip, SFlareTooltip).MenuManager(this);

		// Fader
		SAssignNew(Fader, SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"));
		Fader->SetVisibility(EVisibility::Hidden);

		// Register regular menus
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainMenu.ToSharedRef()),           50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SettingsMenu.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(NewGameMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(StoryMenu.ToSharedRef()),          50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(FleetMenu.ToSharedRef()),          50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),           50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TradeMenu.ToSharedRef()),          50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TradeRouteMenu.ToSharedRef()),     50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(OrbitMenu.ToSharedRef()),          50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(LeaderboardMenu.ToSharedRef()),    50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ResourcePricesMenu.ToSharedRef()), 50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(WorldEconomyMenu.ToSharedRef()),   50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CreditsMenu.ToSharedRef()),        50);

		// Register special menus
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainOverlay.ToSharedRef()),        55);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Confirmation.ToSharedRef()),       60);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SpacecraftOrder.ToSharedRef()),    70);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Notifier.ToSharedRef()),           80);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Tooltip.ToSharedRef()),            90);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),              100);

		// Setup regular menus
		MainMenu->Setup();
		SettingsMenu->Setup();
		NewGameMenu->Setup();
		StoryMenu->Setup();
		CompanyMenu->Setup();
		ShipMenu->Setup();
		FleetMenu->Setup();
		SectorMenu->Setup();
		TradeMenu->Setup();
		TradeRouteMenu->Setup();
		OrbitMenu->Setup();
		LeaderboardMenu->Setup();
		ResourcePricesMenu->Setup();
		WorldEconomyMenu->Setup();
		CreditsMenu->Setup();

		// Init
		CurrentMenu = EFlareMenu::MENU_None;
		CurrentTarget.Key = EFlareMenu::MENU_None;
		CurrentTarget.Value = NULL;
	}
}

void AFlareMenuManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Fade system
	if (Fader.IsValid() && FadeTimer >= 0)
	{
		FadeTimer += DeltaSeconds;
		float AccelRatio = 1.1;
		float Alpha = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);

		// Apply alpha
		FLinearColor Color = FLinearColor::Black;
		Color.A = FMath::Clamp((FadeFromBlack ? 1 - AccelRatio * Alpha : AccelRatio * Alpha), 0.0f, 1.0f);
		Fader->SetBorderBackgroundColor(Color);

		// Fade process
		if (Alpha < 1)
		{
			Fader->SetVisibility(EVisibility::Visible);
		}

		// Callback
		else if (CurrentTarget.Key != EFlareMenu::MENU_None)
		{
			ProcessCurrentTarget();
		}

		// Done
		else
		{
			Fader->SetVisibility(EVisibility::Hidden);
		}
	}
}


/*----------------------------------------------------
	Public API for interaction
----------------------------------------------------*/

void AFlareMenuManager::OpenMainOverlay()
{
	FLOG("AFlareMenuManager::OpenMainOverlay");

	MainOverlay->Open();
	if (GetPC()->GetNavHUD())
	{
		GetPC()->GetNavHUD()->UpdateHUDVisibility();
	}
}

void AFlareMenuManager::CloseMainOverlay()
{
	FLOG("AFlareMenuManager::CloseMainOverlay");

	MainOverlay->Close();
	if (GetPC()->GetNavHUD())
	{
		GetPC()->GetNavHUD()->UpdateHUDVisibility();
	}
}

bool AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, FFlareMenuParameterData* Data)
{
	// Filters
	if (CurrentTarget.Key == Target && CurrentTarget.Value == Data)
	{
		return false;
	}
	FLOGV("AFlareMenuManager::OpenMenu : %d", (Target - EFlareMenu::MENU_None));

	// Back function
	if (Target != EFlareMenu::MENU_None && Target != EFlareMenu::MENU_Settings)
	{
		LastNonSettingsMenu = Target;
	}

	// Settings
	MenuIsOpen = true;
	FadeOut();
	CurrentTarget.Key = Target;
	CurrentTarget.Value = Data;
	return true;
}

void AFlareMenuManager::CloseMenu(bool HardClose)
{
	FLOGV("AFlareMenuManager::CloseMenu : HardClose = %d", HardClose);
	if (MenuIsOpen && GetPC()->GetShipPawn() && GetGame()->GetActiveSector())
	{
		if (HardClose)
		{
			ExitMenu();
		}
		else
		{
			FFlareMenuParameterData* Data = new FFlareMenuParameterData;
			Data->Spacecraft = GetPC()->GetPlayerShip();
			OpenMenu(EFlareMenu::MENU_FlyShip, Data);
		}
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::OpenSpacecraftOrder(FFlareMenuParameterData* Data, FOrderDelegate ConfirmationCallback)
{
	FLOG("AFlareMenuManager::OpenSpacecraftOrder");
	if (Data->Factory)
	{
		SpacecraftOrder->Open(Data->Factory);
	}
	else if (Data->Sector)
	{
		SpacecraftOrder->Open(Data->Sector, ConfirmationCallback);
	}
}

void AFlareMenuManager::Back()
{
	if (MenuIsOpen)
	{
		FLOG("AFlareMenuManager::Back");
		TFlareMenuData CurrentTarget = PopPreviousMenu();
		if (CurrentTarget.Key != EFlareMenu::MENU_None)
		{
			OpenMenu(CurrentTarget.Key, CurrentTarget.Value);
		}
	}
}

void AFlareMenuManager::Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed)
{
	if (Confirmation.IsValid())
	{
		Confirmation->Confirm(Title, Text, OnConfirmed);
	}
}

void AFlareMenuManager::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, FFlareMenuParameterData* TargetInfo)
{
	if (MainOverlay.IsValid())
	{
		OrbitMenu->StopFastForward();
		Notifier->Notify(Text, Info, Tag, Type, Timeout, TargetMenu, TargetInfo);
	}
}

void AFlareMenuManager::ShowTooltip(SWidget* TargetWidget, FText Title, FText Content)
{
	if (Tooltip.IsValid() && !IsFading())
	{
		Tooltip->ShowTooltip(TargetWidget, Title, Content);
	}
}

void AFlareMenuManager::HideTooltip(SWidget* TargetWidget)
{
	if (Tooltip.IsValid() && !IsFading())
	{
		Tooltip->HideTooltip(TargetWidget);
	}
}


/*----------------------------------------------------
	Internal management
----------------------------------------------------*/

void AFlareMenuManager::ResetMenu()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	
	SpacecraftOrder->Close();

	MainMenu->Exit();
	SettingsMenu->Exit();
	NewGameMenu->Exit();
	StoryMenu->Exit();
	CompanyMenu->Exit();
	ShipMenu->Exit();
	FleetMenu->Exit();
	SectorMenu->Exit();
	TradeMenu->Exit();
	TradeRouteMenu->Exit();
	OrbitMenu->Exit();
	LeaderboardMenu->Exit();
	ResourcePricesMenu->Exit();
	WorldEconomyMenu->Exit();
	CreditsMenu->Exit();

	if (PC)
	{
		PC->GetMenuPawn()->ResetContent();
	}

	FadeIn();
}

void AFlareMenuManager::FadeIn()
{
	FadeFromBlack = true;
	FadeTimer = 0;
}

void AFlareMenuManager::FadeOut()
{
	FadeFromBlack = false;
	FadeTimer = 0;
	CurrentMenu = EFlareMenu::MENU_None;
	Tooltip->HideTooltipForce();
}

void AFlareMenuManager::ProcessCurrentTarget()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FLOGV("ProcessCurrentTarget %d", CurrentTarget.Key + 0)

	// Process the target
	switch (CurrentTarget.Key)
	{
		case EFlareMenu::MENU_Main:
			OpenMainMenu(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Settings:
			OpenSettingsMenu(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_NewGame:
			OpenNewGameMenu(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_LoadGame:
			LoadGame(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Story:
			OpenStoryMenu(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Company:
			InspectCompany(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_FlyShip:
			FlyShip(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Travel:
			Travel(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Fleet:
			OpenFleetMenu(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Ship:
			InspectShip(CurrentTarget.Value, false);
			break;

		case EFlareMenu::MENU_ShipConfig:
			InspectShip(CurrentTarget.Value, true);
			break;

		case EFlareMenu::MENU_Sector:
			OpenSector(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Trade:
			OpenTrade(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_TradeRoute:
			OpenTradeRoute(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Orbit:
			OpenOrbit(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Leaderboard:
			OpenLeaderboard(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_ResourcePrices:
			OpenResourcePrices(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_WorldEconomy:
			OpenWorldEconomy(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Credits:
			OpenCredits(CurrentTarget.Value);
			break;

		case EFlareMenu::MENU_Quit:
			PC->ConsoleCommand("quit");
			break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	// Reset everything
	delete CurrentTarget.Value;
	CurrentTarget.Value = NULL;
	CurrentTarget.Key = EFlareMenu::MENU_None;

	// Signal the HUD
	if (GetPC()->GetNavHUD())
	{
		GetPC()->GetNavHUD()->UpdateHUDVisibility();
	}
}

TFlareMenuData AFlareMenuManager::PopPreviousMenu()
{
	return TFlareMenuData();
}

void AFlareMenuManager::FlushNotifications()
{
	if (MainOverlay.IsValid())
	{
		Notifier->FlushNotifications();
	}
}

void AFlareMenuManager::UseLightBackground()
{
	GetPC()->GetMenuPawn()->UseLightBackground();
}

void AFlareMenuManager::UseDarkBackground()
{
	GetPC()->GetMenuPawn()->UseDarkBackground();
}

AFlareGame* AFlareMenuManager::GetGame() const
{
	return GetPC()->GetGame();
}


/*----------------------------------------------------
	Internal menu callbacks
----------------------------------------------------*/

void AFlareMenuManager::LoadGame(FFlareMenuParameterData* Data)
{
	ExitMenu();

	// Load the game
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	PC->GetGame()->LoadGame(PC);
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();

	if (CurrentShip && CurrentShip->GetCurrentSector())
	{
		// Activate sector
		UFlareSimulatedSector* Sector = CurrentShip->GetCurrentSector();
		Sector->SetShipToFly(CurrentShip);
		PC->GetGame()->ActivateCurrentSector();

		// Fly the ship - we create another set of data here to keep with the convention :) 
		FFlareMenuParameterData FlyData;
		FlyData.Spacecraft = CurrentShip;
		FlyShip(&FlyData);
	}
}

void AFlareMenuManager::FlyShip(FFlareMenuParameterData* Data)
{
	check(Data);
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		if (Data->Spacecraft)
		{
			PC->FlyShip(Data->Spacecraft->GetActive());
		}

		ExitMenu();
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::Travel(FFlareMenuParameterData* Data)
{
	check(Data);
	UFlareFleet* PlayerFleet = GetGame()->GetPC()->GetPlayerFleet();
	UFlareFleet* SelectedFleet = GetGame()->GetPC()->GetSelectedFleet();

	if (Data->Travel && PlayerFleet)
	{
		// Player flying : activate the travel sector
		if (PlayerFleet == Data->Travel->GetFleet())
		{
			GetGame()->ActivateCurrentSector();
		}

		// Reload sector to update it after the departure of a fleet
		else if (PlayerFleet->GetCurrentSector() == Data->Travel->GetSourceSector())
		{
			GetGame()->DeactivateSector();
			GetGame()->ActivateCurrentSector();
		}
	}

	OpenOrbit(NULL);
}

void AFlareMenuManager::OpenMainMenu(FFlareMenuParameterData* Data)
{
	GetPC()->ExitShip();

	ResetMenu();

	CloseMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Main;
	GetPC()->GetGame()->SaveGame(GetPC());
	GetPC()->OnEnterMenu();
	MainMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::OpenSettingsMenu(FFlareMenuParameterData* Data)
{
	ResetMenu();

	CloseMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Settings;
	GetPC()->OnEnterMenu();
	SettingsMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::OpenNewGameMenu(FFlareMenuParameterData* Data)
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_NewGame;
	GetPC()->OnEnterMenu();
	NewGameMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::OpenStoryMenu(FFlareMenuParameterData* Data)
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_Story;
	GetPC()->OnEnterMenu();
	StoryMenu->Enter();
	UseDarkBackground();
}

void AFlareMenuManager::InspectCompany(FFlareMenuParameterData* Data)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Company;
	GetPC()->OnEnterMenu();

	if (Data && Data->Company)
	{
		CompanyMenu->Enter(Data->Company);
	}
	else
	{
		CompanyMenu->Enter(GetPC()->GetCompany());
	}

	UseLightBackground();
}

void AFlareMenuManager::InspectShip(FFlareMenuParameterData* Data, bool IsEditable)
{
	UFlareSimulatedSpacecraft* MenuTarget = NULL;

	// No target passed - "Inspect" on target ship
	if ((Data == NULL || Data->Spacecraft == NULL) && GetPC()->GetShipPawn())
	{
		MenuTarget = GetPC()->GetShipPawn()->GetCurrentTarget()->GetParent();
		if (MenuTarget)
		{
			FLOGV("AFlareMenuManager::InspectShip : No ship passed, using selection : %s", *MenuTarget->GetImmatriculation().ToString());
		}
		else
		{
			FLOG("AFlareMenuManager::InspectShip : No ship, aborting");
		}
	}
	else
	{
		MenuTarget = Data->Spacecraft;
	}

	// Open the menu for good
	if (MenuTarget)
	{
		ResetMenu();

		OpenMainOverlay();
		CurrentMenu = EFlareMenu::MENU_Ship;
		GetPC()->OnEnterMenu();

		ShipMenu->Enter(MenuTarget, IsEditable);
		UseLightBackground();
	}
}

void AFlareMenuManager::OpenFleetMenu(FFlareMenuParameterData* Data)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Fleet;
	GetPC()->OnEnterMenu();

	FleetMenu->Enter(Data ? Data->Fleet : NULL);
	UseLightBackground();
}

void AFlareMenuManager::OpenSector(FFlareMenuParameterData* Data)
{
	UFlareSimulatedSector* Sector = Data ? Data->Sector : NULL;

	// Do everythong we can to find the active sector
	if (!Sector)
	{
		if (GetGame()->GetActiveSector())
		{
			Sector = GetGame()->GetActiveSector()->GetSimulatedSector();
		}
		else if (GetPC()->GetPlayerShip())
		{
			Sector = GetPC()->GetPlayerShip()->GetCurrentSector();
		}
	}

	if (Sector)
	{
		ResetMenu();

		OpenMainOverlay();
		CurrentMenu = EFlareMenu::MENU_Sector;
		GetPC()->OnEnterMenu();
		SectorMenu->Enter(Sector);
		UseLightBackground();
	}
	else
	{
		OpenOrbit(NULL);
	}
}

void AFlareMenuManager::OpenTrade(FFlareMenuParameterData* Data)
{
	check(Data);
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Trade;
	GetPC()->OnEnterMenu();

	TradeMenu->Enter(Data->Spacecraft->GetCurrentSector(), Data->Spacecraft, NULL);

	UseLightBackground();
}

void AFlareMenuManager::OpenTradeRoute(FFlareMenuParameterData* Data)
{
	check(Data);
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_TradeRoute;
	GetPC()->OnEnterMenu();
	
	TradeRouteMenu->Enter(Data->Route);
	UseDarkBackground();
}

void AFlareMenuManager::OpenOrbit(FFlareMenuParameterData* Data)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Orbit;
	GetPC()->OnEnterMenu();

	OrbitMenu->Enter();

	UseDarkBackground();
}

void AFlareMenuManager::OpenLeaderboard(FFlareMenuParameterData* Data)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Leaderboard;
	GetPC()->OnEnterMenu();
	LeaderboardMenu->Enter();
	UseDarkBackground();
}

void AFlareMenuManager::OpenResourcePrices(FFlareMenuParameterData* Data)
{
	check(Data);
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_ResourcePrices;
	GetPC()->OnEnterMenu();
	ResourcePricesMenu->Enter(Data->Sector);
	UseDarkBackground();
}

void AFlareMenuManager::OpenWorldEconomy(FFlareMenuParameterData* Data)
{
	check(Data);
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_WorldEconomy;
	GetPC()->OnEnterMenu();
	WorldEconomyMenu->Enter(Data->Resource, Data->Sector);
	UseDarkBackground();
}

void AFlareMenuManager::OpenCredits(FFlareMenuParameterData* Data)
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_Credits;
	GetPC()->OnEnterMenu();
	CreditsMenu->Enter();
	UseDarkBackground();
}

void AFlareMenuManager::ExitMenu()
{
	FLOG("AFlareMenuManager::ExitMenu");
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_None;
	CloseMainOverlay();
	GetPC()->OnExitMenu();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

bool AFlareMenuManager::IsUIOpen() const
{
	return IsMenuOpen() || IsOverlayOpen();
}

bool AFlareMenuManager::IsMenuOpen() const
{
	return MenuIsOpen;
}

bool AFlareMenuManager::IsOverlayOpen() const
{
	return MainOverlay->IsOpen();
}

bool AFlareMenuManager::IsFading()
{
	return (FadeTimer < FadeDuration);
}

bool AFlareMenuManager::IsSwitchingMenu() const
{
	return (Fader->GetVisibility() == EVisibility::Visible);
}

EFlareMenu::Type AFlareMenuManager::GetCurrentMenu() const
{
	return CurrentMenu;
}

FText AFlareMenuManager::GetMenuName(EFlareMenu::Type MenuType)
{
	FText Name;

	switch (MenuType)
	{
		case EFlareMenu::MENU_None:           Name = LOCTEXT("NoneMenuName", "");                          break;
		case EFlareMenu::MENU_Main:           Name = LOCTEXT("MainMenuName", "Save & quit");               break;
		case EFlareMenu::MENU_NewGame:        Name = LOCTEXT("NewGameMenuName", "New game");               break;
		case EFlareMenu::MENU_Company:        Name = LOCTEXT("CompanyMenuName", "Company");                break;
		case EFlareMenu::MENU_Leaderboard:    Name = LOCTEXT("LeaderboardMenuName", "Leaderboard");        break;
		case EFlareMenu::MENU_ResourcePrices: Name = LOCTEXT("ResourcePricesMenuName", "Local economy");   break;
		case EFlareMenu::MENU_WorldEconomy:   Name = LOCTEXT("WorldEconomyMenuName", "World economy");     break;
		case EFlareMenu::MENU_Ship:           Name = LOCTEXT("ShipMenuName", "Ship");                      break;
		case EFlareMenu::MENU_Fleet:          Name = LOCTEXT("FleetMenuName", "Fleets");                   break;
		case EFlareMenu::MENU_Station:        Name = LOCTEXT("StationMenuName", "Station");                break;
		case EFlareMenu::MENU_ShipConfig:     Name = LOCTEXT("ShipConfigMenuName", "Ship upgrade");        break;
		case EFlareMenu::MENU_Travel:         Name = LOCTEXT("TravelMenuName", "Travel");                  break;
		case EFlareMenu::MENU_Sector:         Name = LOCTEXT("SectorMenuName", "Sector info");             break;
		case EFlareMenu::MENU_Trade:          Name = LOCTEXT("TradeMenuName", "Trade");                    break;
		case EFlareMenu::MENU_TradeRoute:     Name = LOCTEXT("TradeRouteMenuName", "Trade route");         break;
		case EFlareMenu::MENU_Orbit:          Name = LOCTEXT("OrbitMenuName", "Orbital map");              break;
		case EFlareMenu::MENU_Settings:       Name = LOCTEXT("SettingsMenuName", "Settings");              break;
		case EFlareMenu::MENU_Quit:           Name = LOCTEXT("QuitMenuName", "Quit");                      break;		
		default:                                                                                           break;
	}

	return Name;
}

const FSlateBrush* AFlareMenuManager::GetMenuIcon(EFlareMenu::Type MenuType, bool ButtonVersion)
{
	FString Path;

	switch (MenuType)
	{
		case EFlareMenu::MENU_Main:           Path = "HeliumRain";   break;
		case EFlareMenu::MENU_NewGame:        Path = "HeliumRain";   break;
		case EFlareMenu::MENU_Company:        Path = "Company";      break;
		case EFlareMenu::MENU_Leaderboard:    Path = "Leaderboard";  break;
		case EFlareMenu::MENU_ResourcePrices: Path = "Sector";       break;
		case EFlareMenu::MENU_WorldEconomy:   Path = "Sector";       break;
		case EFlareMenu::MENU_Ship:           Path = "Ship";         break;
		case EFlareMenu::MENU_Fleet:          Path = "Fleet";        break;
		case EFlareMenu::MENU_Station:        Path = "Station";      break;
		case EFlareMenu::MENU_ShipConfig:     Path = "ShipUpgrade";  break;
		case EFlareMenu::MENU_Travel:         Path = "Travel";       break;
		case EFlareMenu::MENU_Sector:         Path = "Sector";       break;
		case EFlareMenu::MENU_Trade:          Path = "Trade";        break;
		case EFlareMenu::MENU_TradeRoute:     Path = "TradeRoute";   break;
		case EFlareMenu::MENU_Orbit:          Path = "Orbit";        break;
		case EFlareMenu::MENU_Settings:       Path = "Settings";     break;
		case EFlareMenu::MENU_Quit:           Path = "Quit";         break;
		case EFlareMenu::MENU_FlyShip:        Path = "Close";        break;
		default:                              Path = "Empty";
	}

	if (ButtonVersion)
	{
		Path += "_Button";
	}

	return FFlareStyleSet::GetIcon(Path);
}

#undef LOCTEXT_NAMESPACE

