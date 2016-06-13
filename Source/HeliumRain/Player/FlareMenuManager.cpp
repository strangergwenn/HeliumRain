
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

		CurrentMenu = EFlareMenu::MENU_None;
	}
}


/*----------------------------------------------------
	Menu interaction
----------------------------------------------------*/

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
		else if (FadeTarget != EFlareMenu::MENU_None)
		{
			ProcessFadeTarget();
		}

		// Done
		else
		{
			Fader->SetVisibility(EVisibility::Hidden);
		}
	}
}

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

bool AFlareMenuManager::IsOverlayOpen() const
{
	return MainOverlay->IsOpen();
}

bool AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, void* Data)
{
	// Filters
	check(!IsSpacecraftMenu(Target));
	if (FadeTarget == Target && FadeTargetData == Data)
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
	FadeTarget = Target;
	FadeTargetData = Data;
	return true;
}

bool AFlareMenuManager::OpenMenuSpacecraft(EFlareMenu::Type Target, UFlareSimulatedSpacecraft* Data)
{
	FLOGV("AFlareMenuManager::OpenMenuSpacecraft : %d", (Target - EFlareMenu::MENU_None));

	// Filters
	check(IsSpacecraftMenu(Target));
	if (FadeTarget == Target && FadeTargetData == Data)
	{
		return false;
	}

	// Back function
	if (Target != EFlareMenu::MENU_None && Target != EFlareMenu::MENU_Settings)
	{
		LastNonSettingsMenu = Target;
	}

	// Settings
	MenuIsOpen = true;
	FadeOut();
	FadeTarget = Target;
	FadeTargetData = NULL;
	FadeTargetSpacecraft = Data;
	return true;
}

void AFlareMenuManager::OpenSpacecraftOrder(UFlareFactory* Factory)
{
	FLOG("AFlareMenuManager::OpenSpacecraftOrder");
	SpacecraftOrder->Open(Factory);
}

void AFlareMenuManager::OpenSpacecraftOrder(UFlareSimulatedSector* Sector, FOrderDelegate ConfirmationCallback)
{
	FLOG("AFlareMenuManager::OpenSpacecraftOrder");
	SpacecraftOrder->Open(Sector, ConfirmationCallback);
}

bool AFlareMenuManager::IsUIOpen() const
{
	return IsMenuOpen() || IsOverlayOpen();
}

bool AFlareMenuManager::IsMenuOpen() const
{
	return MenuIsOpen;
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
			OpenMenuSpacecraft(EFlareMenu::MENU_FlyShip, GetPC()->GetPlayerShip());
		}
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::Back()
{
	if (MenuIsOpen)
	{
		FLOG("AFlareMenuManager::Back");
		EFlareMenu::Type PreviousMenu = GetPreviousMenu();

		// Menus with dedicated back
		if (CurrentMenu == EFlareMenu::MENU_Trade)
		{
			TradeMenu->Back();
		}
		else if (CurrentMenu == EFlareMenu::MENU_Ship || CurrentMenu == EFlareMenu::MENU_ShipConfig)
		{
			ShipMenu->Back();
		}
		else if (CurrentMenu == EFlareMenu::MENU_ResourcePrices)
		{
			ResourcePricesMenu->Back();
		}
		else if (CurrentMenu == EFlareMenu::MENU_WorldEconomy)
		{
			WorldEconomyMenu->Back();
		}

		// General-purpose back
		else if (PreviousMenu != EFlareMenu::MENU_None)
		{
			if (IsSpacecraftMenu(PreviousMenu))
			{
				OpenMenuSpacecraft(PreviousMenu);
			}
			else
			{
				OpenMenu(PreviousMenu);
			}
		}
	}
}

EFlareMenu::Type AFlareMenuManager::GetCurrentMenu() const
{
	return CurrentMenu;
}

EFlareMenu::Type AFlareMenuManager::GetPreviousMenu() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	switch (CurrentMenu)
	{
		// Main
		case EFlareMenu::MENU_NewGame:
		case EFlareMenu::MENU_Credits:
			return EFlareMenu::MENU_Main;
			break;

		// Orbital map
		case EFlareMenu::MENU_Story:
		case EFlareMenu::MENU_Fleet:
		case EFlareMenu::MENU_Sector:
		case EFlareMenu::MENU_Company:
		case EFlareMenu::MENU_Leaderboard:
			return EFlareMenu::MENU_Orbit;
			break;

		// Company
		case EFlareMenu::MENU_TradeRoute:
			return EFlareMenu::MENU_Company;
			break;
		
		// Settings
		case EFlareMenu::MENU_Settings:
			if (LastNonSettingsMenu == EFlareMenu::MENU_FlyShip)
			{
				return EFlareMenu::MENU_FlyShip;
			}
			else
			{
				return LastNonSettingsMenu;
			}
			break;

		// Those menus back themselves
		case EFlareMenu::MENU_Ship:
		case EFlareMenu::MENU_ShipConfig:
		case EFlareMenu::MENU_Trade:
		case EFlareMenu::MENU_ResourcePrices:
		case EFlareMenu::MENU_WorldEconomy:
			break;

		// Those menus have only the main overlay as root, close
		case EFlareMenu::MENU_Orbit:
			return EFlareMenu::MENU_FlyShip;

		// Those menus have no back
		case EFlareMenu::MENU_Main:
		case EFlareMenu::MENU_FlyShip:
		case EFlareMenu::MENU_Quit:
		case EFlareMenu::MENU_None:
		default:
			break;
	}

	return EFlareMenu::MENU_None;
}

bool AFlareMenuManager::IsSwitchingMenu() const
{
	return (Fader->GetVisibility() == EVisibility::Visible);
}

void AFlareMenuManager::ShowLoadingScreen()
{
	IFlareLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IFlareLoadingScreenModule>("FlareLoadingScreen");
	if (LoadingScreenModule)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
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

void AFlareMenuManager::Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed)
{
	if (Confirmation.IsValid())
	{
		Confirmation->Confirm(Title, Text, OnConfirmed);
	}
}

void AFlareMenuManager::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, void* TargetInfo, FName TargetSpacecraft)
{
	if (MainOverlay.IsValid())
	{
		OrbitMenu->StopFastForward();
		Notifier->Notify(Text, Info, Tag, Type, Timeout, TargetMenu, TargetInfo, TargetSpacecraft);
	}
}

void AFlareMenuManager::FlushNotifications()
{
	if (MainOverlay.IsValid())
	{
		Notifier->FlushNotifications();
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

bool AFlareMenuManager::IsSpacecraftMenu(EFlareMenu::Type Type) const
{
	if (Type == EFlareMenu::MENU_FlyShip
	 || Type == EFlareMenu::MENU_Ship
     || Type == EFlareMenu::MENU_ShipConfig
     || Type == EFlareMenu::MENU_Trade)
	{
		return true;
	}
	else
	{
		return false;
	}
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
		case EFlareMenu::MENU_Undock:         Name = LOCTEXT("UndockMenuName", "Undock");                  break;
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
		case EFlareMenu::MENU_Undock:         Path = "Undock";       break;
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


/*----------------------------------------------------
	Menu management
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

bool AFlareMenuManager::IsFading()
{
	return (FadeTimer < FadeDuration);
}

void AFlareMenuManager::ProcessFadeTarget()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FLOGV("ProcessFadeTarget %d", FadeTarget + 0)
	switch (FadeTarget)
	{
		case EFlareMenu::MENU_Main:
			OpenMainMenu();
			break;

		case EFlareMenu::MENU_Settings:
			OpenSettingsMenu();
			break;

		case EFlareMenu::MENU_NewGame:
			OpenNewGameMenu();
			break;

		case EFlareMenu::MENU_LoadGame:
			LoadGame();
			break;

		case EFlareMenu::MENU_Story:
			OpenStoryMenu();
			break;

		case EFlareMenu::MENU_Company:
			InspectCompany(static_cast<UFlareCompany*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_FlyShip:
			FlyShip(FadeTargetSpacecraft);
			break;

		case EFlareMenu::MENU_Fleet:
			OpenFleetMenu(static_cast<UFlareFleet*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Ship:
			InspectShip(FadeTargetSpacecraft);
			break;

		case EFlareMenu::MENU_ShipConfig:
			InspectShip(FadeTargetSpacecraft, true);
			break;

		case EFlareMenu::MENU_Sector:
			OpenSector(static_cast<UFlareSimulatedSector*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Trade:
			OpenTrade(FadeTargetSpacecraft);
			break;

		case EFlareMenu::MENU_TradeRoute:
			OpenTradeRoute(static_cast<UFlareTradeRoute*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Orbit:
			OpenOrbit();
			break;

		case EFlareMenu::MENU_Leaderboard:
			OpenLeaderboard();
			break;

		case EFlareMenu::MENU_ResourcePrices:
			OpenResourcePrices(static_cast<UFlareSimulatedSector*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_WorldEconomy:
			OpenWorldEconomy(static_cast<FFlareWorldEconomyMenuParam*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Credits:
			OpenCredits();
			break;

		case EFlareMenu::MENU_Quit:
			PC->ConsoleCommand("quit");
			break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	// Reset everything
	FadeTargetData = NULL;
	FadeTarget = EFlareMenu::MENU_None;
	if (GetPC()->GetNavHUD())
	{
		GetPC()->GetNavHUD()->UpdateHUDVisibility();
	}
}

AFlareGame* AFlareMenuManager::GetGame() const
{
	return GetPC()->GetGame();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void AFlareMenuManager::OpenMainMenu()
{
	GetPC()->ExitShip();

	ResetMenu();

	CloseMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Main;
	GetPC()->OnEnterMenu();
	MainMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::OpenSettingsMenu()
{
	ResetMenu();

	CloseMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Settings;
	GetPC()->OnEnterMenu();
	SettingsMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::OpenNewGameMenu()
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_NewGame;
	GetPC()->OnEnterMenu();
	NewGameMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::LoadGame()
{
	ExitMenu();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	PC->GetGame()->LoadGame(PC);
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();

	if (CurrentShip && CurrentShip->GetCurrentSector())
	{
		UFlareSimulatedSector* Sector = CurrentShip->GetCurrentSector();
		Sector->SetShipToFly(CurrentShip);
		PC->GetGame()->ActivateCurrentSector();
		FlyShip(CurrentShip);
	}
}

void AFlareMenuManager::OpenStoryMenu()
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_Story;
	GetPC()->OnEnterMenu();
	StoryMenu->Enter();
	UseDarkBackground();
}

void AFlareMenuManager::InspectCompany(UFlareCompany* Target)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Company;
	GetPC()->OnEnterMenu();

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetCompany();
	}
	CompanyMenu->Enter(Target);
	UseLightBackground();
}

void AFlareMenuManager::FlyShip(UFlareSimulatedSpacecraft* Target)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		if(Target)
		{
			PC->FlyShip(Target->GetActive());
		}

		ExitMenu();
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::InspectShip(UFlareSimulatedSpacecraft* Target, bool IsEditable)
{
	UFlareSimulatedSpacecraft* MenuTarget = NULL;

	// No target passed - "Inspect" on target ship
	if (Target == NULL && GetPC()->GetShipPawn())
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
		MenuTarget = Target;
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

void AFlareMenuManager::OpenFleetMenu(UFlareFleet* TargetFleet)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Fleet;
	GetPC()->OnEnterMenu();

	FleetMenu->Enter(TargetFleet);
	UseLightBackground();
}

void AFlareMenuManager::OpenSector(UFlareSimulatedSector* Sector)
{
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
		OpenOrbit();
	}
}

void AFlareMenuManager::OpenTrade(UFlareSimulatedSpacecraft* Spacecraft)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Trade;
	GetPC()->OnEnterMenu();

	TradeMenu->Enter(Spacecraft->GetCurrentSector(), Spacecraft, NULL);

	UseLightBackground();
}

void AFlareMenuManager::OpenTradeRoute(UFlareTradeRoute* TradeRoute)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_TradeRoute;
	GetPC()->OnEnterMenu();
	
	TradeRouteMenu->Enter(TradeRoute);
	UseDarkBackground();
}

void AFlareMenuManager::OpenOrbit()
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Orbit;
	GetPC()->OnEnterMenu();

	OrbitMenu->Enter();

	UseDarkBackground();
}

void AFlareMenuManager::OpenLeaderboard()
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_Leaderboard;
	GetPC()->OnEnterMenu();
	LeaderboardMenu->Enter();
	UseDarkBackground();
}

void AFlareMenuManager::OpenResourcePrices(UFlareSimulatedSector* Sector)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_ResourcePrices;
	GetPC()->OnEnterMenu();
	ResourcePricesMenu->Enter(Sector);
	UseLightBackground();
}

void AFlareMenuManager::OpenWorldEconomy(FFlareWorldEconomyMenuParam* Params)
{
	ResetMenu();

	OpenMainOverlay();
	CurrentMenu = EFlareMenu::MENU_WorldEconomy;
	GetPC()->OnEnterMenu();
	WorldEconomyMenu->Enter(Params->Resource, Params->Sector);
	UseLightBackground();
	delete Params;
}

void AFlareMenuManager::OpenCredits()
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_Credits;
	GetPC()->OnEnterMenu();
	CreditsMenu->Enter();
	UseLightBackground();
}

void AFlareMenuManager::ExitMenu()
{
	FLOG("AFlareMenuManager::ExitMenu");

	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_None;
	CloseMainOverlay();
	GetPC()->OnExitMenu();
}


#undef LOCTEXT_NAMESPACE

