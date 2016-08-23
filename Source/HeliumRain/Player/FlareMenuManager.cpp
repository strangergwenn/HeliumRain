
#include "../Flare.h"
#include "FlareMenuManager.h"

#include "../UI/Menus/FlareMainMenu.h"
#include "../UI/Menus/FlareSettingsMenu.h"
#include "../UI/Menus/FlareNewGameMenu.h"
#include "../UI/Menus/FlareStoryMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareFleetMenu.h"
#include "../UI/Menus/FlareQuestMenu.h"
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
		SAssignNew(QuestMenu, SFlareQuestMenu).MenuManager(this);
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
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(QuestMenu.ToSharedRef()),          50);
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
		QuestMenu->Setup();
		SectorMenu->Setup();
		TradeMenu->Setup();
		TradeRouteMenu->Setup();
		OrbitMenu->Setup();
		LeaderboardMenu->Setup();
		ResourcePricesMenu->Setup();
		WorldEconomyMenu->Setup();
		CreditsMenu->Setup();

		// Init
		CurrentMenu.Key = EFlareMenu::MENU_None;
		NextMenu.Key = EFlareMenu::MENU_None;
		CurrentMenu.Value = FFlareMenuParameterData();
		NextMenu.Value = FFlareMenuParameterData();
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
		else if (NextMenu.Key != EFlareMenu::MENU_None)
		{
			ProcessNextMenu();
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

bool AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, FFlareMenuParameterData Data, bool AddToHistory)
{
	// Filters
	if (NextMenu.Key == Target)
	{
		return false;
	}
	FLOGV("AFlareMenuManager::OpenMenu : '%s'", *GetMenuName(Target).ToString());
	
	// Store current menu in history
	if (CurrentMenu.Key != EFlareMenu::MENU_None && AddToHistory)
	{
		FLOGV("AFlareMenuManager::OnEnterMenu : previous menu is now '%s'", *GetMenuName(CurrentMenu.Key).ToString());
		MenuHistory.Add(CurrentMenu);
	}

	// Reset current menu, set next menu
	CurrentMenu.Key = EFlareMenu::MENU_None;
	CurrentMenu.Value = FFlareMenuParameterData();
	NextMenu.Key = Target;
	NextMenu.Value = Data;

	// Start fading out
	MenuIsOpen = true;
	FadeOut();
	return true;
}

void AFlareMenuManager::CloseMenu(bool HardClose)
{
	FLOGV("AFlareMenuManager::CloseMenu : HardClose = %d", HardClose);
	if (MenuIsOpen && GetPC()->GetPlayerShip() && GetGame()->GetActiveSector())
	{
		check(GetPC()->GetPlayerShip()->GetActive());

		if (HardClose)
		{
			ExitMenu();
		}
		else
		{
			FFlareMenuParameterData Data;
			Data.Spacecraft = GetPC()->GetPlayerShip();
			OpenMenu(EFlareMenu::MENU_FlyShip, Data);
		}
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::OpenSpacecraftOrder(FFlareMenuParameterData Data, FOrderDelegate ConfirmationCallback)
{
	FLOG("AFlareMenuManager::OpenSpacecraftOrder");
	if (Data.Factory)
	{
		SpacecraftOrder->Open(Data.Factory);
	}
	else if (Data.Sector)
	{
		SpacecraftOrder->Open(Data.Sector, ConfirmationCallback);
	}
}

void AFlareMenuManager::Back()
{
	if (MenuIsOpen)
	{
		FLOG("AFlareMenuManager::Back");

		while (MenuHistory.Num())
		{
			// Pop from stack
			TFlareMenuData PreviousMenu = MenuHistory.Last();
			MenuHistory.RemoveAt(MenuHistory.Num() - 1);

			// Check consistency of target, open the previous menu if nothing looks wrong
			if ((PreviousMenu.Value.Spacecraft && !PreviousMenu.Value.Spacecraft->IsValidLowLevel())
			 || (PreviousMenu.Value.Fleet      && !PreviousMenu.Value.Fleet->IsValidLowLevel())
			 || (PreviousMenu.Value.Route      && !PreviousMenu.Value.Route->IsValidLowLevel()))
			{
				FLOGV("AFlareMenuManager::Back : ignore corrupted target '%s'", *GetMenuName(PreviousMenu.Key).ToString());
				continue;
			}
			else
			{
				FLOGV("AFlareMenuManager::Back : backing to '%s'", *GetMenuName(PreviousMenu.Key).ToString());
				OpenMenu(PreviousMenu.Key, PreviousMenu.Value, false);
				return;
			}
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

void AFlareMenuManager::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	if (MainOverlay.IsValid())
	{
		OrbitMenu->RequestStopFastForward();
		Notifier->Notify(Text, Info, Tag, Type, Pinned, TargetMenu, TargetInfo);
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
	QuestMenu->Exit();
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
	Tooltip->HideTooltipForce();
}

void AFlareMenuManager::ProcessNextMenu()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FLOGV("AFlareMenuManager::ProcessNextMenu : '%s'", *GetMenuName(NextMenu.Key).ToString());

	// Process the target
	switch (NextMenu.Key)
	{
		case EFlareMenu::MENU_LoadGame:           LoadGame();                  break;
		case EFlareMenu::MENU_FlyShip:            FlyShip();                   break;
		case EFlareMenu::MENU_ReloadSector:       ReloadSector();              break;
		case EFlareMenu::MENU_Travel:             Travel();                    break;

		case EFlareMenu::MENU_Main:               OpenMainMenu();              break;
		case EFlareMenu::MENU_Settings:           OpenSettingsMenu();          break;
		case EFlareMenu::MENU_NewGame:            OpenNewGameMenu();           break;
		case EFlareMenu::MENU_Story:              OpenStoryMenu();             break;
		case EFlareMenu::MENU_Company:            InspectCompany();            break;
		case EFlareMenu::MENU_Fleet:              OpenFleetMenu();             break;
		case EFlareMenu::MENU_Quest:              OpenQuestMenu();             break;
		case EFlareMenu::MENU_Ship:               InspectShip(false);          break;
		case EFlareMenu::MENU_ShipConfig:         InspectShip(true);           break;
		case EFlareMenu::MENU_Sector:             OpenSector();                break;
		case EFlareMenu::MENU_Trade:              OpenTrade();                 break;
		case EFlareMenu::MENU_TradeRoute:         OpenTradeRoute();            break;
		case EFlareMenu::MENU_Orbit:              OpenOrbit();                 break;
		case EFlareMenu::MENU_Leaderboard:        OpenLeaderboard();           break;
		case EFlareMenu::MENU_ResourcePrices:     OpenResourcePrices();        break;
		case EFlareMenu::MENU_WorldEconomy:       OpenWorldEconomy();          break;
		case EFlareMenu::MENU_Credits:            OpenCredits();               break;

		case EFlareMenu::MENU_Quit:               PC->ConsoleCommand("quit");  break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	// Reset target
	NextMenu.Value = FFlareMenuParameterData();
	NextMenu.Key = EFlareMenu::MENU_None;

	// Signal the HUD
	if (GetPC()->GetNavHUD())
	{
		GetPC()->GetNavHUD()->UpdateHUDVisibility();
	}
}

void AFlareMenuManager::OnEnterMenu(bool LightBackground, bool ShowOverlay, bool TellPlayer)
{
	ResetMenu();
	CurrentMenu = NextMenu;

	if (LightBackground)
	{
		UseLightBackground();
	}
	else
	{
		UseDarkBackground();
	}

	if (ShowOverlay)
	{
		OpenMainOverlay();
	}
	else
	{
		CloseMainOverlay();
	}

	if (TellPlayer)
	{
		GetPC()->OnEnterMenu();
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


/*----------------------------------------------------
	Internal menu callbacks
----------------------------------------------------*/

void AFlareMenuManager::LoadGame()
{
	ExitMenu();

	FText Reason;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	PC->GetGame()->LoadGame(PC);
	
	// No player ship ? Get one !
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();
	if (CurrentShip && CurrentShip->CanBeFlown(Reason))
	{
		// Do nothing
	}
	else
	{
		FLOG("AFlareMenuManager::LoadGame : no player ship");
		TArray<UFlareSimulatedSpacecraft*> Ships = PC->GetCompany()->GetCompanyShips();
		if (Ships.Num())
		{
			for (int32 ShipIndex = 0; ShipIndex < Ships.Num(); ShipIndex++)
			{
				if (Ships[ShipIndex]->CanBeFlown(Reason))
				{
					PC->SetPlayerShip(Ships[ShipIndex]);
					break;
				}
			}
		}
	}

	// We got a valid ship here
	CurrentShip = PC->GetPlayerShip();
	if (CurrentShip && CurrentShip->CanBeFlown(Reason))
	{
		// Activate sector
		FLOGV("AFlareMenuManager::LoadGame : found player ship '%s'", *CurrentShip->GetImmatriculation().ToString());
		PC->GetGame()->ActivateCurrentSector();

		check(CurrentShip->GetActive());

		// Fly the ship - we create another set of data here to keep with the convention :) 
		NextMenu.Key = EFlareMenu::MENU_FlyShip;
		NextMenu.Value.Spacecraft = CurrentShip;
		FlyShip();
		NextMenu.Value = FFlareMenuParameterData();
	}

	// TODO : handle game over
	else
	{
		FLOG("AFlareMenuManager::LoadGame : still no player ship, player is fucked :) ");
		OpenMenu(EFlareMenu::MENU_Main);
	}
}

void AFlareMenuManager::FlyShip()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		// Fly ship
		if (NextMenu.Value.Spacecraft)
		{
			AFlareSpacecraft* OldShip = PC->GetShipPawn();
			UFlareSimulatedSpacecraft* Ship = NextMenu.Value.Spacecraft;
			PC->FlyShip(Ship->GetActive());

			if (OldShip != Ship->GetActive())
			{
				// Count owned ships
				int32 OwnedSpacecraftCount = 0;
				TArray<AFlareSpacecraft*>& SectorSpacecrafts = GetGame()->GetActiveSector()->GetSpacecrafts();
				for (int SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
				{
					AFlareSpacecraft* OtherSpacecraft = SectorSpacecrafts[SpacecraftIndex];
					if (OtherSpacecraft->GetParent()->GetCompany() == PC->GetCompany())
					{
						OwnedSpacecraftCount++;
					}
				}

				// Notification title
				FText Title;
				if (PC->GetPlayerFleet()->IsTraveling())
				{
					Title = FText::Format(LOCTEXT("FlyingTravelFormat", "Travelling with {0}"), FText::FromName(Ship->GetImmatriculation()));
				}
				else
				{
					Title = FText::Format(LOCTEXT("FlyingFormat", "Now flying {0}"), FText::FromName(Ship->GetImmatriculation()));
				}

				// Notification body
				FText Info;
				if (PC->GetPlayerFleet()->IsTraveling())
				{
					Info = LOCTEXT("FlyingTravelInfo", "Complete travels with the \"Fast forward\" button on the orbital map.");
				}
				else if (OwnedSpacecraftCount > 1)
				{
					Info = LOCTEXT("FlyingMultipleInfo", "You can switch to nearby ships with N.");
				}
				else
				{
					Info = LOCTEXT("FlyingInfo", "You are now flying your personal ship.");
				}

				// Notify
				FFlareMenuParameterData Data;
				Data.Spacecraft = Ship;
				Notify(Title, Info, "flying-info", EFlareNotification::NT_Info, false, EFlareMenu::MENU_Ship, Data);
			}
		}

		ExitMenu();
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::Travel()
{
	UFlareFleet* PlayerFleet = GetGame()->GetPC()->GetPlayerFleet();

	if (NextMenu.Value.Travel && PlayerFleet)
	{
		// Player flying : activate the travel sector
		if (PlayerFleet == NextMenu.Value.Travel->GetFleet())
		{
			GetGame()->ActivateCurrentSector();
		}

		// Reload sector to update it after the departure of a fleet
		else if (PlayerFleet->GetCurrentSector() == NextMenu.Value.Travel->GetSourceSector())
		{
			GetGame()->DeactivateSector();
			GetGame()->ActivateCurrentSector();
		}
	}

	// Redirect to the orbit menu
	NextMenu.Key = EFlareMenu::MENU_Orbit;
	NextMenu.Value = FFlareMenuParameterData();
	OpenOrbit();
}

void AFlareMenuManager::ReloadSector()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		PC->GetGame()->DeactivateSector();
		PC->GetGame()->ActivateCurrentSector();

		if (NextMenu.Value.Spacecraft)
		{
			PC->FlyShip(NextMenu.Value.Spacecraft->GetActive());
		}

		ExitMenu();
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::OpenMainMenu()
{
	OnEnterMenu(true);
	GetPC()->ExitShip();
	GetPC()->GetGame()->SaveGame(GetPC(), false);
	MainMenu->Enter();
}

void AFlareMenuManager::OpenSettingsMenu()
{
	OnEnterMenu(true);
	SettingsMenu->Enter();
}

void AFlareMenuManager::OpenNewGameMenu()
{
	OnEnterMenu(true);
	NewGameMenu->Enter();
}

void AFlareMenuManager::OpenStoryMenu()
{
	OnEnterMenu(false, false);
	StoryMenu->Enter();
}

void AFlareMenuManager::InspectCompany()
{
	OnEnterMenu(false);

	UFlareCompany* Company = (NextMenu.Value.Company) ? NextMenu.Value.Company : GetPC()->GetCompany();
	check(Company);

	CompanyMenu->Enter(Company);
}

void AFlareMenuManager::InspectShip(bool IsEditable)
{
	UFlareSimulatedSpacecraft* MenuTarget = NULL;

	// No target passed - "Inspect" on target ship
	if (NextMenu.Value.Spacecraft == NULL && GetPC()->GetShipPawn())
	{
		MenuTarget = GetPC()->GetShipPawn()->GetParent();
		if (MenuTarget)
		{
			FLOGV("AFlareMenuManager::InspectShip : No ship passed, using player ship : %s", *MenuTarget->GetImmatriculation().ToString());
		}
		else
		{
			FLOG("AFlareMenuManager::InspectShip : No ship, aborting");
		}
	}
	else
	{
		MenuTarget = NextMenu.Value.Spacecraft;
	}

	// Open the menu for good
	if (MenuTarget)
	{
		OnEnterMenu();
		ShipMenu->Enter(MenuTarget, IsEditable);
	}
}

void AFlareMenuManager::OpenFleetMenu()
{
	OnEnterMenu(false);
	FleetMenu->Enter(NextMenu.Value.Fleet);
}

void AFlareMenuManager::OpenQuestMenu()
{
	OnEnterMenu(false);
	QuestMenu->Enter(NextMenu.Value.Quest);
}

void AFlareMenuManager::OpenSector()
{
	UFlareSimulatedSector* Sector = NextMenu.Value.Sector;

	// Do everything we can to find the active sector
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
		// We found the sector, open the menu
		OnEnterMenu();
		SectorMenu->Enter(Sector);
	}
	else
	{
		// Redirect to the orbit menu
		NextMenu.Key = EFlareMenu::MENU_Orbit;
		NextMenu.Value = FFlareMenuParameterData();
		OpenOrbit();
	}
}

void AFlareMenuManager::OpenTrade()
{
	OnEnterMenu();
	TradeMenu->Enter(NextMenu.Value.Spacecraft->GetCurrentSector(), NextMenu.Value.Spacecraft, NULL);
}

void AFlareMenuManager::OpenTradeRoute()
{
	OnEnterMenu(false);
	TradeRouteMenu->Enter(NextMenu.Value.Route);
}

void AFlareMenuManager::OpenOrbit()
{
	OnEnterMenu(false);
	OrbitMenu->Enter();
}

void AFlareMenuManager::OpenLeaderboard()
{
	OnEnterMenu(false);
	LeaderboardMenu->Enter();
}

void AFlareMenuManager::OpenResourcePrices()
{
	OnEnterMenu();
	ResourcePricesMenu->Enter(NextMenu.Value.Sector);
}

void AFlareMenuManager::OpenWorldEconomy()
{
	OnEnterMenu(false);
	WorldEconomyMenu->Enter(NextMenu.Value.Resource, NextMenu.Value.Sector);
}

void AFlareMenuManager::OpenCredits()
{
	OnEnterMenu(false, false, false);
	CreditsMenu->Enter();
}

void AFlareMenuManager::ExitMenu()
{
	// Reset menu
	ResetMenu();
	CloseMainOverlay();

	// Reset data
	MenuHistory.Empty();
	CurrentMenu.Key = EFlareMenu::MENU_None;
	CurrentMenu.Value = FFlareMenuParameterData();

	// Signal PC
	GetPC()->OnExitMenu();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

FText AFlareMenuManager::GetMenuName(EFlareMenu::Type MenuType)
{
	FText Name;

	switch (MenuType)
	{
		case EFlareMenu::MENU_None:           Name = LOCTEXT("NoneMenuName", "");                          break;
		case EFlareMenu::MENU_Main:           Name = LOCTEXT("MainMenuName", "Load game");                 break;
		case EFlareMenu::MENU_NewGame:        Name = LOCTEXT("NewGameMenuName", "New game");               break;
		case EFlareMenu::MENU_Company:        Name = LOCTEXT("CompanyMenuName", "Company");                break;
		case EFlareMenu::MENU_Leaderboard:    Name = LOCTEXT("LeaderboardMenuName", "Leaderboard");        break;
		case EFlareMenu::MENU_ResourcePrices: Name = LOCTEXT("ResourcePricesMenuName", "Local prices");    break;
		case EFlareMenu::MENU_WorldEconomy:   Name = LOCTEXT("WorldEconomyMenuName", "World prices");      break;
		case EFlareMenu::MENU_Ship:           Name = LOCTEXT("ShipMenuName", "Ship");                      break;
		case EFlareMenu::MENU_Fleet:          Name = LOCTEXT("FleetMenuName", "Fleets");                   break;
		case EFlareMenu::MENU_Quest:          Name = LOCTEXT("QuestMenuName", "Quests");                   break;
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

const FSlateBrush* AFlareMenuManager::GetMenuIcon(EFlareMenu::Type MenuType)
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
		case EFlareMenu::MENU_Quest:          Path = "Quest";        break;
		case EFlareMenu::MENU_Station:        Path = "Station";      break;
		case EFlareMenu::MENU_ShipConfig:     Path = "ShipUpgrade";  break;
		case EFlareMenu::MENU_Travel:         Path = "Travel";       break;
		case EFlareMenu::MENU_Sector:         Path = "Sector";       break;
		case EFlareMenu::MENU_Trade:          Path = "Trade";        break;
		case EFlareMenu::MENU_TradeRoute:     Path = "Trade";        break;
		case EFlareMenu::MENU_Orbit:          Path = "Orbit";        break;
		case EFlareMenu::MENU_Settings:       Path = "Settings";     break;
		case EFlareMenu::MENU_Quit:           Path = "Quit";         break;
		case EFlareMenu::MENU_FlyShip:        Path = "Close";        break;
		default:                              Path = "Empty";
	}

	return FFlareStyleSet::GetIcon(Path);
}

bool AFlareMenuManager::IsUIOpen() const
{
	return IsMenuOpen() || IsOverlayOpen();
}

bool AFlareMenuManager::IsMenuOpen() const
{
	return MenuIsOpen;
}

bool AFlareMenuManager::HasPreviousMenu() const
{
	return (MenuHistory.Num() > 0);
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
	return CurrentMenu.Key;
}

EFlareMenu::Type AFlareMenuManager::GetNextMenu() const
{
	return NextMenu.Key;
}

AFlareGame* AFlareMenuManager::GetGame() const
{
	return GetPC()->GetGame();
}

AFlarePlayerController* AFlareMenuManager::GetPC() const
{
	return Cast<AFlarePlayerController>(GetOwner());
}

TSharedPtr<SFlareShipMenu> AFlareMenuManager::GetShipMenu() const
{
	return ShipMenu;
}

int32 AFlareMenuManager::GetMainOverlayHeight()
{
	return 150;
}

AFlareMenuManager* AFlareMenuManager::GetSingleton()
{
	return Singleton;
}


#undef LOCTEXT_NAMESPACE
