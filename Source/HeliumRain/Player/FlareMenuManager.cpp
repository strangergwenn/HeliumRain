
#include "../Flare.h"
#include "FlareMenuManager.h"

#include "../UI/Menus/FlareMainMenu.h"
#include "../UI/Menus/FlareSettingsMenu.h"
#include "../UI/Menus/FlareNewGameMenu.h"
#include "../UI/Menus/FlareDashboard.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareFleetMenu.h"
#include "../UI/Menus/FlareOrbitalMenu.h"
#include "../UI/Menus/FlareLeaderboardMenu.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Menus/FlareTradeMenu.h"
#include "../UI/Menus/FlareTradeRouteMenu.h"
#include "../UI/Menus/FlareCreditsMenu.h"

#include "../Game/FlareSectorInterface.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../HeliumRainLoadingScreen/FlareLoadingScreen.h"


#define LOCTEXT_NAMESPACE "FlareMenuManager"


AFlareMenuManager* AFlareMenuManager::Singleton;


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareMenuManager::AFlareMenuManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, MenuIsOpen(false)
	, FadeDuration(0.25)
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
		SAssignNew(Dashboard, SFlareDashboard).MenuManager(this);
		SAssignNew(CompanyMenu, SFlareCompanyMenu).MenuManager(this);
		SAssignNew(FleetMenu, SFlareFleetMenu).MenuManager(this);
		SAssignNew(ShipMenu, SFlareShipMenu).MenuManager(this);
		SAssignNew(SectorMenu, SFlareSectorMenu).MenuManager(this);
		SAssignNew(TradeMenu, SFlareTradeMenu).MenuManager(this);
		SAssignNew(TradeRouteMenu, SFlareTradeRouteMenu).MenuManager(this);
		SAssignNew(OrbitMenu, SFlareOrbitalMenu).MenuManager(this);
		SAssignNew(LeaderboardMenu, SFlareLeaderboardMenu).MenuManager(this);
		SAssignNew(CreditsMenu, SFlareCreditsMenu).MenuManager(this);

		// Notifier
		SAssignNew(Notifier, SFlareNotifier).MenuManager(this).Visibility(EVisibility::SelfHitTestInvisible);

		// Confirmation overlay
		SAssignNew(Confirmation, SFlareConfirmationOverlay).MenuManager(this);

		// Spacecraft order overlay
		SAssignNew(SpacecraftOrder, SFlareSpacecraftOrderOverlay).MenuManager(this);

		// Tooltip
		SAssignNew(Tooltip, SFlareTooltip).MenuManager(this);

		// Fader
		SAssignNew(Fader, SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"));
		Fader->SetVisibility(EVisibility::Hidden);

		// Register regular menus
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SettingsMenu.ToSharedRef()),     50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(NewGameMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Dashboard.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(FleetMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TradeMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TradeRouteMenu.ToSharedRef()),   50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(OrbitMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(LeaderboardMenu.ToSharedRef()),  50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CreditsMenu.ToSharedRef()),      50);

		// Register special menus
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Notifier.ToSharedRef()),         60);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Confirmation.ToSharedRef()),     60);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Tooltip.ToSharedRef()),          90);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SpacecraftOrder.ToSharedRef()),  70);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),            100);

		// Setup regular menus
		MainMenu->Setup();
		SettingsMenu->Setup();
		NewGameMenu->Setup();
		Dashboard->Setup();
		CompanyMenu->Setup();
		ShipMenu->Setup();
		FleetMenu->Setup();
		SectorMenu->Setup();
		TradeMenu->Setup();
		TradeRouteMenu->Setup();
		OrbitMenu->Setup();
		LeaderboardMenu->Setup();
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
		FLinearColor Color = FLinearColor::Black;
		float Alpha = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);

		// Fade process
		if (Alpha < 1)
		{
			Color.A = FadeFromBlack ? 1 - Alpha : Alpha;
			Fader->SetVisibility(EVisibility::Visible);
			Fader->SetBorderBackgroundColor(Color);
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

void AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, void* Data)
{
	// Filters
	check(!IsSpacecraftMenu(Target));
	if (FadeTarget == Target && FadeTargetData == Data)
	{
		return;
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
	FadeTargetData = Data;
}

void AFlareMenuManager::OpenMenuSpacecraft(EFlareMenu::Type Target, IFlareSpacecraftInterface* Data)
{	
	// Filters
	check(IsSpacecraftMenu(Target));
	if (FadeTarget == Target && FadeTargetData == Data)
	{
		return;
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
	FadeTargetSpacecraft = Cast<IFlareSpacecraftInterface>(Data);
}

void AFlareMenuManager::OpenSpacecraftOrder(UFlareFactory* Factory)
{
	SpacecraftOrder->Open(Factory);
}

void AFlareMenuManager::CloseMenu(bool HardClose)
{
	if (MenuIsOpen)
	{
		if (HardClose)
		{
			ExitMenu();
		}
		else
		{
			OpenMenu(EFlareMenu::MENU_Exit);
		}
	}
	MenuIsOpen = false;
}

void AFlareMenuManager::Back()
{
	FLOG("AFlareMenuManager::Back");
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	if (MenuIsOpen)
	{
		FLOGV("AFlareMenuManager::Back MenuIsOpen %d", (int) CurrentMenu);
		switch (CurrentMenu)
		{
			case EFlareMenu::MENU_NewGame:
				OpenMenu(EFlareMenu::MENU_Main);
				break;

			case EFlareMenu::MENU_Settings:
				FLOGV("AFlareMenuManager::Back MENU_Settings LastNonSettingsMenu %d", (int) LastNonSettingsMenu);

				if (LastNonSettingsMenu == EFlareMenu::MENU_FlyShip
				 || LastNonSettingsMenu == EFlareMenu::MENU_ActivateSector
				 || LastNonSettingsMenu == EFlareMenu::MENU_Exit)
				{
					CloseMenu();
				}
				else
				{
					OpenMenu(LastNonSettingsMenu);
				}
				break;

			case EFlareMenu::MENU_Dashboard:
				CloseMenu();
				break;

			case EFlareMenu::MENU_Company:
				OpenMenu(EFlareMenu::MENU_Orbit);
				break;

			case EFlareMenu::MENU_Ship:
			case EFlareMenu::MENU_ShipConfig:
				if (PC->GetShipPawn())
				{
					OpenMenu(EFlareMenu::MENU_Dashboard);
				}
				else
				{
					OpenMenu(EFlareMenu::MENU_Orbit);
				}
				break;

			case EFlareMenu::MENU_Fleet:
			case EFlareMenu::MENU_Sector:
				OpenMenu(EFlareMenu::MENU_Orbit);
				break;

			case EFlareMenu::MENU_Trade:
				if (PC->GetShipPawn())
				{
					OpenMenu(EFlareMenu::MENU_Dashboard);
				}
				else
				{
					OpenMenu(EFlareMenu::MENU_Orbit);
				}
				break;

			case EFlareMenu::MENU_TradeRoute:
				OpenMenu(EFlareMenu::MENU_Company);
				break;

			case EFlareMenu::MENU_Leaderboard:
				OpenMenu(EFlareMenu::MENU_Orbit);
				break;

			case EFlareMenu::MENU_Credits:
				OpenMenu(EFlareMenu::MENU_Main);
				break;

			case EFlareMenu::MENU_Main:
			case EFlareMenu::MENU_FlyShip:
			case EFlareMenu::MENU_ActivateSector:
			case EFlareMenu::MENU_Orbit:
			case EFlareMenu::MENU_Quit:
			case EFlareMenu::MENU_Exit:
			case EFlareMenu::MENU_None:
			default:
				break;
		}
	}
}

bool AFlareMenuManager::IsMenuOpen() const
{
	return MenuIsOpen;
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

void AFlareMenuManager::Confirm(FText Text, FSimpleDelegate OnConfirmed)
{
	if (Confirmation.IsValid())
	{
		Confirmation->Confirm(Text, OnConfirmed);
	}
}

void AFlareMenuManager::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, float Timeout, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	if (Notifier.IsValid())
	{
		Notifier->Notify(Text, Info, Tag, Type, Timeout, TargetMenu, TargetInfo);
	}
}

void AFlareMenuManager::FlushNotifications()
{
	if (Notifier.IsValid())
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

const FSlateBrush* AFlareMenuManager::GetMenuIcon(EFlareMenu::Type MenuType, bool ButtonVersion)
{
	FString Path;

	switch (MenuType)
	{
		case EFlareMenu::MENU_Main:           Path = "HeliumRain";   break;
		case EFlareMenu::MENU_NewGame:        Path = "HeliumRain";   break;
		case EFlareMenu::MENU_Dashboard:      Path = "Sector";       break;
		case EFlareMenu::MENU_Company:        Path = "Company";      break;
		case EFlareMenu::MENU_Leaderboard:    Path = "Leaderboard";  break;
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
		case EFlareMenu::MENU_Exit:           Path = "Close";        break;
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

	/*if (CurrentMenu != EFlareMenu::MENU_None && CurrentMenu != EFlareMenu::MENU_Settings)
	{
		LastNonSettingsMenu = CurrentMenu;
	}*/

	SpacecraftOrder->Close();

	MainMenu->Exit();
	SettingsMenu->Exit();
	NewGameMenu->Exit();
	Dashboard->Exit();
	CompanyMenu->Exit();
	ShipMenu->Exit();
	FleetMenu->Exit();
	SectorMenu->Exit();
	TradeMenu->Exit();
	TradeRouteMenu->Exit();
	OrbitMenu->Exit();
	LeaderboardMenu->Exit();
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
	/*if (CurrentMenu != EFlareMenu::MENU_None && CurrentMenu != EFlareMenu::MENU_Settings)
	{
		LastNonSettingsMenu = CurrentMenu;
	}*/
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

		case EFlareMenu::MENU_Dashboard:
			OpenDashboard();
			break;

		case EFlareMenu::MENU_Company:
			InspectCompany(static_cast<UFlareCompany*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_FlyShip:
			FlyShip(Cast<AFlareSpacecraft>(FadeTargetSpacecraft));
			break;

		case EFlareMenu::MENU_ActivateSector:
			ActivateSector(static_cast<UFlareSectorInterface*>(FadeTargetData));
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
			OpenSector(static_cast<UFlareSectorInterface*>(FadeTargetData));
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

		case EFlareMenu::MENU_Credits:
			OpenCredits();
			break;

		case EFlareMenu::MENU_Quit:
			PC->ConsoleCommand("quit");
			break;

		case EFlareMenu::MENU_Exit:
			ExitMenu();
			break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	// Reset everything
	FadeTargetData = NULL;
	FadeTarget = EFlareMenu::MENU_None;
	GetPC()->GetNavHUD()->UpdateHUDVisibility();
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
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Main;
	GetPC()->OnEnterMenu();
	MainMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenSettingsMenu()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Settings;
	GetPC()->OnEnterMenu();
	SettingsMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenNewGameMenu()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_NewGame;
	GetPC()->OnEnterMenu();
	NewGameMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenDashboard()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Dashboard;
	GetPC()->OnEnterMenu();
	Dashboard->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::InspectCompany(UFlareCompany* Target)
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Company;
	GetPC()->OnEnterMenu();

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetCompany();
	}
	CompanyMenu->Enter(Target);
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::FlyShip(AFlareSpacecraft* Target)
{
	ExitMenu();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && Target->IsValidLowLevel())
	{
		PC->FlyShip(Target);
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::ActivateSector(UFlareSectorInterface* Target)
{
	ExitMenu();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && Target)
	{
		PC->GetGame()->ActivateSector(PC, Cast<UFlareSimulatedSector>(Target));
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::InspectShip(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	IFlareSpacecraftInterface* MenuTarget = NULL;

	// No target passed - "Inspect" on target ship
	if (Target == NULL && GetPC()->GetShipPawn())
	{
		MenuTarget = GetPC()->GetShipPawn()->GetCurrentTarget();
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

		CurrentMenu = EFlareMenu::MENU_Ship;
		GetPC()->OnEnterMenu();

		ShipMenu->Enter(MenuTarget, IsEditable);
		GetPC()->UpdateMenuTheme();
	}
}

void AFlareMenuManager::OpenFleetMenu(UFlareFleet* TargetFleet)
{
	ResetMenu();

	CurrentMenu = EFlareMenu::MENU_Fleet;
	GetPC()->OnEnterMenu();

	FleetMenu->Enter(TargetFleet);
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenSector(UFlareSectorInterface* Sector)
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Sector;
	GetPC()->OnEnterMenu();


	UFlareSimulatedSector* SimulatedSector = Cast<UFlareSimulatedSector>(Sector);
	if (SimulatedSector)
	{
		SectorMenu->Enter(SimulatedSector);
		GetPC()->UpdateMenuTheme();
	}
	else
	{
		FLOG("Fail to cast to UFlareSimulatedSector");
	}
}

void AFlareMenuManager::OpenTrade(IFlareSpacecraftInterface* Spacecraft)
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Trade;
	GetPC()->OnEnterMenu();

	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Cast<UFlareSimulatedSpacecraft>(Spacecraft);
	if (SimulatedSpacecraft)
	{
		TradeMenu->Enter(SimulatedSpacecraft->GetCurrentSector(), SimulatedSpacecraft, NULL);
	}
	else
	{
		AFlareSpacecraft* ActiveSpacecraft = Cast<AFlareSpacecraft>(Spacecraft);
		TradeMenu->Enter(GetGame()->GetActiveSector(), ActiveSpacecraft, NULL);
	}

	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenTradeRoute(UFlareTradeRoute* TradeRoute)
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_TradeRoute;
	GetPC()->OnEnterMenu();


	TradeRouteMenu->Enter(TradeRoute);
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenOrbit()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Orbit;
	GetPC()->OnEnterMenu();
	OrbitMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenLeaderboard()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Leaderboard;
	GetPC()->OnEnterMenu();
	LeaderboardMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::OpenCredits()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_Credits;
	GetPC()->OnEnterMenu();
	CreditsMenu->Enter();
	GetPC()->UpdateMenuTheme();
}

void AFlareMenuManager::ExitMenu()
{
	ResetMenu();
	CurrentMenu = EFlareMenu::MENU_None;
	GetPC()->OnExitMenu();
}


#undef LOCTEXT_NAMESPACE

