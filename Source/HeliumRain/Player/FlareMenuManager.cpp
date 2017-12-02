
#include "FlareMenuManager.h"
#include "../Flare.h"

#include "../UI/Components/FlareTooltip.h"
#include "../UI/Components/FlareNotifier.h"
#include "../UI/Components/FlareMainOverlay.h"
#include "../UI/Components/FlareSpacecraftOrderOverlay.h"
#include "../UI/Components/FlareConfirmationOverlay.h"

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
#include "../UI/Menus/FlareEULAMenu.h"
#include "../UI/Menus/FlareResourcePricesMenu.h"
#include "../UI/Menus/FlareWorldEconomyMenu.h"
#include "../UI/Menus/FlareTechnologyMenu.h"
#include "../UI/Menus/FlareGameOverMenu.h"
#include "../UI/Menus/FlareSkirmishSetupMenu.h"
#include "../UI/Menus/FlareSkirmishScoreMenu.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"

#include "FlareHUD.h"
#include "FlareMenuPawn.h"
#include "FlarePlayerController.h"

#include "../HeliumRainLoadingScreen/FlareLoadingScreen.h"

#include "GameFramework/InputSettings.h"


#define LOCTEXT_NAMESPACE "FlareMenuManager"


AFlareMenuManager* AFlareMenuManager::Singleton;


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareMenuManager::AFlareMenuManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, MenuOperationDone(true)
	, MenuIsOpen(false)
	, FadeFromBlack(true)
	, NotifyExitSector(false)
	, FadeDuration(0.3)
	, CurrentSpacecraftInfo(NULL)
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
		SAssignNew(TechnologyMenu, SFlareTechnologyMenu).MenuManager(this);
		SAssignNew(GameOverMenu, SFlareGameOverMenu).MenuManager(this);
		SAssignNew(CreditsMenu, SFlareCreditsMenu).MenuManager(this);
		SAssignNew(EULAMenu, SFlareEULAMenu).MenuManager(this);
		SAssignNew(SkirmishSetupMenu, SFlareSkirmishSetupMenu).MenuManager(this);
		SAssignNew(SkirmishScoreMenu, SFlareSkirmishScoreMenu).MenuManager(this);

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
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TechnologyMenu.ToSharedRef()),     50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(GameOverMenu.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CreditsMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(EULAMenu.ToSharedRef()),           50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SkirmishSetupMenu.ToSharedRef()),  50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SkirmishScoreMenu.ToSharedRef()),  50);

		// Register special menus
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainOverlay.ToSharedRef()),        60);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Notifier.ToSharedRef()),           70);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SpacecraftOrder.ToSharedRef()),    80);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Confirmation.ToSharedRef()),       90);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Tooltip.ToSharedRef()),            100);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),              200);

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
		TechnologyMenu->Setup();
		GameOverMenu->Setup();
		CreditsMenu->Setup();
		EULAMenu->Setup();
		SkirmishSetupMenu->Setup();
		SkirmishScoreMenu->Setup();

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

bool AFlareMenuManager::ToggleMenu(EFlareMenu::Type Target)
{
	if(GetCurrentMenu() != Target)
	{
		return OpenMenu(Target);
	}
	else
	{
		CloseMenu();
		return true;
	}
}



bool AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, FFlareMenuParameterData Data, bool AddToHistory, bool OpenDirectly)
{
	// Filters
	if (NextMenu.Key != EFlareMenu::MENU_None)
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

	// Open the menu via fade or directly
	MenuIsOpen = true;
	if (OpenDirectly)
	{
		SkipNextFade = true;
		ProcessNextMenu();
	}
	else
	{
		FadeOut();
	}

	if(GetGame() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("open-menu").PutInt32("menu", (Target+0)).PutPtr("data", &Data));
	}

	return true;
}

void AFlareMenuManager::CloseMenu(bool HardClose)
{
	FLOGV("AFlareMenuManager::CloseMenu : HardClose = %d", HardClose);
	if (MenuIsOpen && GetPC()->GetPlayerShip() && GetGame()->GetActiveSector())
	{
		FCHECK(GetPC()->GetPlayerShip()->GetActive());

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

	if (Data.ComplexConnectorName != NAME_None)
	{
		SpacecraftOrder->Open(Data.Spacecraft, Data.ComplexConnectorName, ConfirmationCallback);
	}
	else if (Data.Spacecraft)
	{
		SpacecraftOrder->Open(Data.Spacecraft, Data.SpacecraftOrderHeavy);
	}
	else if (Data.Sector)
	{
		SpacecraftOrder->Open(Data.Sector, ConfirmationCallback);
	}
	else if (Data.Skirmish)
	{
		SpacecraftOrder->Open(GetGame()->GetSkirmishManager(), Data.OrderForPlayer, ConfirmationCallback);
	}
}

void AFlareMenuManager::Back()
{
	// Close confirmation
	if (Confirmation->IsOpen())
	{
		Confirmation->Close();
	}

	// Close spacecraft order
	else if (SpacecraftOrder->IsOpen())
	{
		SpacecraftOrder->Close();
	}

	// Is in menu
	else if (MenuIsOpen)
	{
		// Back
		if (HasPreviousMenu())
		{
			FLOG("AFlarePlayerController::Back Back");

			while (MenuHistory.Num())
			{
				// Pop from stack
				TFlareMenuData PreviousMenu = MenuHistory.Last();
				MenuHistory.RemoveAt(MenuHistory.Num() - 1);

				// Check consistency of target, open the previous menu if nothing looks wrong
				if ((PreviousMenu.Value.Spacecraft && !PreviousMenu.Value.Spacecraft->IsValidLowLevel())
					|| (PreviousMenu.Value.Fleet && !PreviousMenu.Value.Fleet->IsValidLowLevel())
					|| (PreviousMenu.Value.Route && !PreviousMenu.Value.Route->IsValidLowLevel()))
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

			GetPC()->ClientPlaySound(GetPC()->GetSoundManager()->NegativeClickSound);
		}

		// Close menu
		else
		{
			FLOG("AFlarePlayerController::Back Close");
			CloseMenu();
			GetPC()->ClientPlaySound(GetPC()->GetSoundManager()->NegativeClickSound);
		}
	}

	// Is in overlay
	else if (IsOverlayOpen())
	{
		FLOG("AFlarePlayerController::Back Toggle");
		CloseMainOverlay();
	}
}

void AFlareMenuManager::Reload()
{
	if (MenuIsOpen)
	{
		FLOGV("AFlareMenuManager::Reload : reloading to '%s'", *GetMenuName(CurrentMenu.Key).ToString());
		OpenMenu(CurrentMenu.Key, CurrentMenu.Value, false, true);
	}
}

void AFlareMenuManager::Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed, FSimpleDelegate OnCancel, FSimpleDelegate OnIgnore)
{
	if (Confirmation.IsValid())
	{
		Confirmation->Confirm(Title, Text, OnConfirmed, OnCancel, OnIgnore);
	}
}

bool AFlareMenuManager::Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	if (MainOverlay.IsValid())
	{
		if (!UFlareGameTools::FastFastForward && Type != EFlareNotification::NT_NewQuest)
		{
			OrbitMenu->RequestStopFastForward();
		}
		return Notifier->Notify(Text, Info, Tag, Type, Pinned, TargetMenu, TargetInfo);
	}
	return false;
}

void AFlareMenuManager::ClearNotifications(FName Tag)
{
	if (MainOverlay.IsValid())
	{
		Notifier->ClearNotifications(Tag, false);
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
	if (IsValidLowLevel() && Tooltip.IsValid() && !IsFading())
	{
		Tooltip->HideTooltip(TargetWidget);
	}
}

void AFlareMenuManager::RegisterSpacecraftInfo(SFlareSpacecraftInfo* Info)
{
	CurrentSpacecraftInfo = Info;
}

void AFlareMenuManager::UnregisterSpacecraftInfo(SFlareSpacecraftInfo* Info)
{
	if (CurrentSpacecraftInfo == Info)
	{
		CurrentSpacecraftInfo = NULL;
	}
}

void AFlareMenuManager::SpacecraftInfoHotkey(int32 Index)
{
	if (CurrentSpacecraftInfo)
	{
		CurrentSpacecraftInfo->Hotkey(Index);
	}
}

void AFlareMenuManager::JoystickCursorMove(FVector2D Move)
{
	if (Move.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		auto Cursor = FSlateApplication::Get().GetPlatformApplication().Get()->Cursor;
		JoystickCursorPosition = Cursor->GetPosition();

		float Power = 2;
		float Intensity = 15;

		JoystickCursorPosition.X += Intensity * FMath::Sign(Move.X) * FMath::Pow(Move.X, Power);
		JoystickCursorPosition.Y += Intensity * FMath::Sign(Move.Y) * FMath::Pow(Move.Y, Power);

		Cursor->SetPosition(JoystickCursorPosition.X, JoystickCursorPosition.Y);
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
	TechnologyMenu->Exit();
	GameOverMenu->Exit();
	CreditsMenu->Exit();
	EULAMenu->Exit();
	SkirmishSetupMenu->Exit();
	SkirmishScoreMenu->Exit();

	Tooltip->HideTooltipForce();

	if (PC)
	{
		PC->GetMenuPawn()->ResetContent();
	}

	if (SkipNextFade)
	{
		SkipNextFade = false;
	}
	else
	{
		FadeIn();
	}
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

	if (Tooltip.IsValid())
	{
		Tooltip->HideTooltipForce();
	}
}

void AFlareMenuManager::ProcessNextMenu()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FLOGV("AFlareMenuManager::ProcessNextMenu : '%s'", *GetMenuName(NextMenu.Key).ToString());

	// Process the target
	switch (NextMenu.Key)
	{
		case EFlareMenu::MENU_CreateGame:         MenuOperationDone = CreateGame();         break;
		case EFlareMenu::MENU_LoadGame:           MenuOperationDone = LoadGame();           break;
		case EFlareMenu::MENU_FlyShip:            MenuOperationDone = FlyShip();            break;
		case EFlareMenu::MENU_ReloadSector:       MenuOperationDone = ReloadSector();       break;
		case EFlareMenu::MENU_FastForwardSingle:  MenuOperationDone = FastForwardSingle();  break;

		case EFlareMenu::MENU_Travel:             Travel();                    break;
		case EFlareMenu::MENU_GameOver:           GameOver();                  break;

		case EFlareMenu::MENU_Main:               OpenMainMenu();              break;
		case EFlareMenu::MENU_Settings:           OpenSettingsMenu();          break;
		case EFlareMenu::MENU_NewGame:            OpenNewGameMenu();           break;
		case EFlareMenu::MENU_Story:              OpenStoryMenu();             break;
		case EFlareMenu::MENU_Company:            InspectCompany();            break;
		case EFlareMenu::MENU_Fleet:              OpenFleetMenu();             break;
		case EFlareMenu::MENU_Quest:              OpenQuestMenu();             break;
		case EFlareMenu::MENU_Ship:
		case EFlareMenu::MENU_Station:            InspectShip(false);          break;
		case EFlareMenu::MENU_ShipConfig:         InspectShip(true);           break;
		case EFlareMenu::MENU_Sector:             OpenSector();                break;
		case EFlareMenu::MENU_Trade:              OpenTrade();                 break;
		case EFlareMenu::MENU_TradeRoute:         OpenTradeRoute();            break;
		case EFlareMenu::MENU_Orbit:              OpenOrbit();                 break;
		case EFlareMenu::MENU_Leaderboard:        OpenLeaderboard();           break;
		case EFlareMenu::MENU_ResourcePrices:     OpenResourcePrices();        break;
		case EFlareMenu::MENU_WorldEconomy:       OpenWorldEconomy();          break;
		case EFlareMenu::MENU_Technology:         OpenTechnology();            break;
		case EFlareMenu::MENU_Credits:            OpenCredits();               break;
		case EFlareMenu::MENU_EULA:               OpenEULA();                  break;
		case EFlareMenu::MENU_SkirmishSetup:      OpenSkirmishSetup();         break;
		case EFlareMenu::MENU_SkirmishScore:      OpenSkirmishScore();         break;

		case EFlareMenu::MENU_Quit:               PC->ConsoleCommand("quit");  break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	// Reset target
	if (MenuOperationDone)
	{
		NextMenu.Value = FFlareMenuParameterData();
		NextMenu.Key = EFlareMenu::MENU_None;
	}

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

bool AFlareMenuManager::CreateGame()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	
	// Create a new skirmish game
	if (NextMenu.Value.Skirmish)
	{
		PC->GetGame()->CreateSkirmishGame(NextMenu.Value.Skirmish);
		PC->GetGame()->ActivateCurrentSector();

		NextMenu.Value.Skirmish = NULL;
	}

	// Create a new sandbox game
	else if (NextMenu.Value.ScenarioIndex >= 0)
	{
		PC->GetGame()->CreateGame(*NextMenu.Value.CompanyDescription, NextMenu.Value.ScenarioIndex, NextMenu.Value.PlayerEmblemIndex, NextMenu.Value.PlayTutorial);
		PC->GetGame()->ActivateCurrentSector();

		NextMenu.Value.ScenarioIndex = -1;
	}

	// Load ship
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();
	NextMenu.Key = EFlareMenu::MENU_FlyShip;
	NextMenu.Value.Spacecraft = CurrentShip;
	return FlyShip();
}

bool AFlareMenuManager::LoadGame()
{
	FText Reason;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	PC->GetGame()->LoadGame(PC);
	
	if(!PC->GetPlayerFleet())
	{
		// No player fleet, create recovery ship
		PC->GetGame()->Recovery();
	}

	// No player ship ? Get one !
	UFlareSimulatedSpacecraft* CurrentShip = PC->GetPlayerShip();
	if (CurrentShip)
	{
		// Do nothing
	}
	else
	{
		// A new ship in player fleet
		PC->SetPlayerShip(PC->GetPlayerFleet()->GetShips()[0]);
	}

	// We got a valid ship here
	CurrentShip = PC->GetPlayerShip();
	if (CurrentShip)
	{
		// Activate sector
		FLOGV("AFlareMenuManager::LoadGame : found player ship '%s'", *CurrentShip->GetImmatriculation().ToString());
		PC->GetGame()->ActivateCurrentSector();

		// Fly the ship - we create another set of data here to keep with the convention :) 
		NextMenu.Key = EFlareMenu::MENU_FlyShip;
		NextMenu.Value.Spacecraft = CurrentShip;
		return FlyShip();
	}

	// TODO : handle game over
	else
	{
		FLOG("AFlareMenuManager::LoadGame : still no player ship, player is fucked :) ");
		OpenMenu(EFlareMenu::MENU_Main);
	}

	return true;
}

bool AFlareMenuManager::FlyShip()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		// Fly ship
		if (NextMenu.Value.Spacecraft)
		{
			AFlareSpacecraft* OldShip = PC->GetShipPawn();
			UFlareSimulatedSpacecraft* Ship = NextMenu.Value.Spacecraft;

			// Fly the ship - retry at new tick if not possible
			if (!Ship->IsActive())
			{
				FLOGV("AFlareMenuManager::FlyShip: need ship %s to be active", *Ship->GetImmatriculation().ToString());
				return false;
			}
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
					Title = FText::Format(LOCTEXT("FlyingTravelFormat", "Traveling with {0}"), UFlareGameTools::DisplaySpacecraftName(Ship));
				}
				else
				{
					Title = FText::Format(LOCTEXT("FlyingFormat", "Now flying {0}"), UFlareGameTools::DisplaySpacecraftName(Ship));
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

	return true;
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

			if (NextMenu.Value.Travel->GetSourceSector() == NextMenu.Value.Travel->GetDestinationSector())
			{
				NotifyExitSector = true;
			}
			else
			{
				NotifyExitSector = false;
			}
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

void AFlareMenuManager::GameOver()
{
	OnEnterMenu(false, false);

	GetGame()->DeactivateSector();
	GetGame()->Recovery();
	GetGame()->GetGameWorld()->Simulate();
	GetGame()->ActivateCurrentSector();

	GameOverMenu->Enter();
}

bool AFlareMenuManager::ReloadSector()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		// Reload the sector (only when not reentrant)
		if (MenuOperationDone)
		{
			PC->GetGame()->DeactivateSector();
			PC->GetGame()->ActivateCurrentSector();
		}

		if (NextMenu.Value.Spacecraft)
		{
			// Fly the ship - retry at new tick if not possible
			if (!NextMenu.Value.Spacecraft->IsActive())
			{
				FLOGV("AFlareMenuManager::ReloadSector: need ship %s to be active", *NextMenu.Value.Spacecraft->GetImmatriculation().ToString());
				return false;
			}
			PC->FlyShip(NextMenu.Value.Spacecraft->GetActive());
		}

		ExitMenu();
		MenuIsOpen = false;
	}
	return true;
}

bool AFlareMenuManager::FastForwardSingle()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		// Exit sector (only when not reentrant)
		if (MenuOperationDone)
		{
			PC->GetGame()->DeactivateSector();
			PC->GetGame()->GetGameWorld()->Simulate();
			PC->GetGame()->ActivateCurrentSector();
		}

		// Fly the ship - retry at new tick if not possible
		if (!PC->GetShipPawn())
		{
			FLOG("AFlareMenuManager::FastForwardSingle: need player ship to be active");
			return false;
		}
		PC->FlyShip(PC->GetShipPawn());

		// Notify date
		PC->Notify(LOCTEXT("NewDate", "A day passed by..."),
			UFlareGameTools::GetDisplayDate(GetGame()->GetGameWorld()->GetDate()),
			FName("new-date-ff"));

		ExitMenu();
		MenuIsOpen = false;
	}
	return true;
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
	OnEnterMenu(false);
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
	FCHECK(Company);

	CompanyMenu->Enter(Company);
}

void AFlareMenuManager::InspectShip(bool IsEditable)
{
	UFlareSimulatedSpacecraft* MenuTarget = NextMenu.Value.Spacecraft;

	// Make sure we're not simulating anymore when going from orbit to ship, as we use player pawn
	OrbitMenu->StopFastForward();

	// No target passed - "Inspect" on target ship
	if (MenuTarget == NULL)
	{
		if (GetPC()->GetPlayerShip())
		{
			MenuTarget = GetPC()->GetPlayerShip();
			FLOGV("AFlareMenuManager::InspectShip : No ship passed, using player ship : %s", *MenuTarget->GetImmatriculation().ToString());
		}
		else
		{
			FLOG("AFlareMenuManager::InspectShip : No ship, aborting");
		}
	}

	// Open the menu for good
	if (MenuTarget)
	{
		NextMenu.Value = FFlareMenuParameterData();
		NextMenu.Value.Spacecraft = MenuTarget;

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

	if (NotifyExitSector)
	{
		GetPC()->NotifyExitSector();
		NotifyExitSector = false;
	}

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

void AFlareMenuManager::OpenTechnology()
{
	OnEnterMenu(false);
	TechnologyMenu->Enter();
}

void AFlareMenuManager::OpenCredits()
{
	OnEnterMenu(false, false, false);
	CreditsMenu->Enter();
}

void AFlareMenuManager::OpenEULA()
{
	OnEnterMenu(false, false, false);
	EULAMenu->Enter();
}

void AFlareMenuManager::OpenSkirmishSetup()
{
	OnEnterMenu(false, false, false);
	SkirmishSetupMenu->Enter();
}

void AFlareMenuManager::OpenSkirmishScore()
{
	OnEnterMenu(false, false, false);
	SkirmishScoreMenu->Enter();
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

FText AFlareMenuManager::GetMenuName(EFlareMenu::Type MenuType, bool Uppercase)
{
	FText Name;

	if (Uppercase)
	{
		switch (MenuType)
		{
			case EFlareMenu::MENU_None:           Name = LOCTEXT("UppercaseNoneMenuName", "");                          break;
			case EFlareMenu::MENU_Main:           Name = LOCTEXT("UppercaseMainMenuName", "MAIN MENU");                 break;
			case EFlareMenu::MENU_NewGame:        Name = LOCTEXT("UppercaseNewGameMenuName", "NEW GAME");               break;
			case EFlareMenu::MENU_Company:        Name = LOCTEXT("UppercaseCompanyMenuName", "COMPANY");                break;
			case EFlareMenu::MENU_Leaderboard:    Name = LOCTEXT("UppercaseLeaderboardMenuName", "DIPLOMACY");          break;
			case EFlareMenu::MENU_ResourcePrices: Name = LOCTEXT("UppercaseResourcePricesMenuName", "LOCAL PRICES");    break;
			case EFlareMenu::MENU_WorldEconomy:   Name = LOCTEXT("UppercaseWorldEconomyMenuName", "ECONOMY");           break;
			case EFlareMenu::MENU_Technology:     Name = LOCTEXT("UppercaseTechnologyMenuName", "TECHNOLOGY");          break;
			case EFlareMenu::MENU_Ship:           Name = LOCTEXT("UppercaseShipMenuName", "SHIP");                      break;
			case EFlareMenu::MENU_Fleet:          Name = LOCTEXT("UppercaseFleetMenuName", "FLEETS");                   break;
			case EFlareMenu::MENU_Quest:          Name = LOCTEXT("UppercaseQuestMenuName", "CONTRACTS");                break;
			case EFlareMenu::MENU_Station:        Name = LOCTEXT("UppercaseStationMenuName", "STATION");                break;
			case EFlareMenu::MENU_ShipConfig:     Name = LOCTEXT("UppercaseShipConfigMenuName", "SHIP UPGRADE");        break;
			case EFlareMenu::MENU_Travel:         Name = LOCTEXT("UppercaseTravelMenuName", "TRAVEL");                  break;
			case EFlareMenu::MENU_GameOver:       Name = LOCTEXT("UppercaseGameOverMenuName", "GAME OVER");             break;
			case EFlareMenu::MENU_Sector:         Name = LOCTEXT("UppercaseSectorMenuName", "SECTOR INFO");             break;
			case EFlareMenu::MENU_Trade:          Name = LOCTEXT("UppercaseTradeMenuName", "TRADE");                    break;
			case EFlareMenu::MENU_TradeRoute:     Name = LOCTEXT("UppercaseTradeRouteMenuName", "TRADE ROUTE");         break;
			case EFlareMenu::MENU_Orbit:          Name = LOCTEXT("UppercaseOrbitMenuName", "ORBITAL MAP");              break;
			case EFlareMenu::MENU_Settings:       Name = LOCTEXT("UppercaseSettingsMenuName", "SETTINGS");              break;
			case EFlareMenu::MENU_Quit:           Name = LOCTEXT("UppercaseQuitMenuName", "QUIT");                      break;		
			default:                                                                                                    break;
		}
	}
	else
	{
		switch (MenuType)
		{
			case EFlareMenu::MENU_None:           Name = LOCTEXT("LowercaseNoneMenuName", "");                          break;
			case EFlareMenu::MENU_Main:           Name = LOCTEXT("LowercaseMainMenuName", "Main menu");                 break;
			case EFlareMenu::MENU_NewGame:        Name = LOCTEXT("LowercaseNewGameMenuName", "New game");               break;
			case EFlareMenu::MENU_Company:        Name = LOCTEXT("LowercaseCompanyMenuName", "Company");                break;
			case EFlareMenu::MENU_Leaderboard:    Name = LOCTEXT("LowercaseLeaderboardMenuName", "Diplomacy");          break;
			case EFlareMenu::MENU_ResourcePrices: Name = LOCTEXT("LowercaseResourcePricesMenuName", "Local prices");    break;
			case EFlareMenu::MENU_WorldEconomy:   Name = LOCTEXT("LowercaseWorldEconomyMenuName", "Economy");           break;
			case EFlareMenu::MENU_Technology:     Name = LOCTEXT("LowercaseTechnologyMenuName", "Technology");          break;
			case EFlareMenu::MENU_Ship:           Name = LOCTEXT("LowercaseShipMenuName", "Ship");                      break;
			case EFlareMenu::MENU_Fleet:          Name = LOCTEXT("LowercaseFleetMenuName", "Fleets");                   break;
			case EFlareMenu::MENU_Quest:          Name = LOCTEXT("LowercaseQuestMenuName", "Contracts");                break;
			case EFlareMenu::MENU_Station:        Name = LOCTEXT("LowercaseStationMenuName", "Station");                break;
			case EFlareMenu::MENU_ShipConfig:     Name = LOCTEXT("LowercaseShipConfigMenuName", "Ship upgrade");        break;
			case EFlareMenu::MENU_Travel:         Name = LOCTEXT("LowercaseTravelMenuName", "Travel");                  break;
			case EFlareMenu::MENU_GameOver:       Name = LOCTEXT("LowercaseGameOverMenuName", "Game over");             break;
			case EFlareMenu::MENU_Sector:         Name = LOCTEXT("LowercaseSectorMenuName", "Sector info");             break;
			case EFlareMenu::MENU_Trade:          Name = LOCTEXT("LowercaseTradeMenuName", "Trade");                    break;
			case EFlareMenu::MENU_TradeRoute:     Name = LOCTEXT("LowercaseTradeRouteMenuName", "Trade route");         break;
			case EFlareMenu::MENU_Orbit:          Name = LOCTEXT("LowercaseOrbitMenuName", "Orbital map");              break;
			case EFlareMenu::MENU_Settings:       Name = LOCTEXT("LowercaseSettingsMenuName", "Settings");              break;
			case EFlareMenu::MENU_Quit:           Name = LOCTEXT("LowercaseQuitMenuName", "Quit");                      break;		
			default:                                                                                                    break;
		}
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
		case EFlareMenu::MENU_Technology:     Path = "Technology";   break;
		case EFlareMenu::MENU_Ship:           Path = "Ship";         break;
		case EFlareMenu::MENU_Fleet:          Path = "Fleet";        break;
		case EFlareMenu::MENU_Quest:          Path = "Quest";        break;
		case EFlareMenu::MENU_Station:        Path = "Station";      break;
		case EFlareMenu::MENU_ShipConfig:     Path = "ShipUpgrade";  break;
		case EFlareMenu::MENU_Travel:         Path = "Travel";       break;
		case EFlareMenu::MENU_GameOver:       Path = "HeliumRain";   break;
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

FString AFlareMenuManager::GetMenuKey(EFlareMenu::Type MenuType)
{
	FName Key;

	switch (MenuType)
	{
		case EFlareMenu::MENU_Ship:           Key = "ShipMenu";         break;
		case EFlareMenu::MENU_Sector:         Key = "SectorMenu";       break;
		case EFlareMenu::MENU_Orbit:          Key = "OrbitMenu";        break;
		case EFlareMenu::MENU_Leaderboard:    Key = "LeaderboardMenu";  break;
		case EFlareMenu::MENU_Company:        Key = "CompanyMenu";      break;
		case EFlareMenu::MENU_Fleet:          Key = "FleetMenu";        break;
		case EFlareMenu::MENU_Technology:     Key = "TechnologyMenu";   break;
		case EFlareMenu::MENU_Quest:          Key = "QuestMenu";        break;
		case EFlareMenu::MENU_Main:           Key = "MainMenu";         break;
		case EFlareMenu::MENU_Settings:       Key = "SettingsMenu";     break;
		default:                              Key = "NoKey";
	}
	
	return GetKeyNameFromActionName(Key);
}

FString AFlareMenuManager::GetKeyNameFromActionName(FName ActionName)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	FCHECK(InputSettings);

	for (int32 i = 0; i < InputSettings->ActionMappings.Num(); i++)
	{
		FInputActionKeyMapping Action = InputSettings->ActionMappings[i];
		if (Action.ActionName == ActionName && !Action.Key.IsGamepadKey())
		{
			FString KeyName = Action.Key.ToString();

			if (KeyName == "Zero" || KeyName == "NumPadZero") return "0";
			if (KeyName == "One" || KeyName == "NumPadOne") return "1";
			if (KeyName == "Two" || KeyName == "NumPadTwo") return "2";
			if (KeyName == "Three" || KeyName == "NumPadThree") return "3";
			if (KeyName == "Four" || KeyName == "NumPadFour") return "4";
			if (KeyName == "Five" || KeyName == "NumPadFive") return "5";
			if (KeyName == "Six" || KeyName == "NumPadSix") return "6";
			if (KeyName == "Seven" || KeyName == "NumPadSeven") return "7";
			if (KeyName == "Eight" || KeyName == "NumPadEight") return "8";
			if (KeyName == "Nine" || KeyName == "NumPadNine") return "9";

			return KeyName;
		}
	}

	return LOCTEXT("NoKey", "").ToString();
}

bool AFlareMenuManager::IsUIOpen() const
{
	return IsMenuOpen() || IsOverlayOpen() || Confirmation->IsOpen();
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

bool AFlareMenuManager::IsFading() const
{
	return (FadeTimer < FadeDuration);
}

bool AFlareMenuManager::IsFadingFromBlack() const
{
	return FadeFromBlack;
}

float AFlareMenuManager::GetFadeAlpha() const
{
	return FadeTimer / FadeDuration;
}

bool AFlareMenuManager::IsSwitchingMenu() const
{
	return (Fader->GetVisibility() == EVisibility::Visible);
}

EFlareMenu::Type AFlareMenuManager::GetCurrentMenu() const
{
	return CurrentMenu.Key;
}

EFlareMenu::Type AFlareMenuManager::GetPreviousMenu() const
{
	if (MenuHistory.Num())
	{
		TFlareMenuData PreviousMenu = MenuHistory.Last();
		return PreviousMenu.Key;
	}
	else
	{
		return EFlareMenu::MENU_None;
	}
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
