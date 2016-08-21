#pragma once

#include "../UI/Components/FlareTooltip.h"
#include "../UI/Components/FlareNotifier.h"
#include "../UI/Components/FlareMainOverlay.h"
#include "../UI/Components/FlareSpacecraftOrderOverlay.h"
#include "../UI/Components/FlareConfirmationOverlay.h"
#include "FlareMenuManager.generated.h"


/*----------------------------------------------------
	Definitions
----------------------------------------------------*/

// Menus
class SFlareMainMenu;
class SFlareSettingsMenu;
class SFlareNewGameMenu;
class SFlareStoryMenu;
class SFlareShipMenu;
class SFlareFleetMenu;
class SFlareQuestMenu;
class SFlareOrbitalMenu;
class SFlareLeaderboardMenu;
class SFlareCompanyMenu;
class SFlareSectorMenu;
class SFlareTradeMenu;
class SFlareTradeRouteMenu;
class SFlareCreditsMenu;
class SFlareResourcePricesMenu;
class SFlareWorldEconomyMenu;

// Gameplay classes
class AFlarePlayerController;
class AFlareGame;

// Menu state
typedef TPair<EFlareMenu::Type, FFlareMenuParameterData> TFlareMenuData;


/*----------------------------------------------------
	Menu manager code
----------------------------------------------------*/

/** The menu manager is the central UI class for the game, controlling fading, menu transitions and setup */
UCLASS()
class HELIUMRAIN_API AFlareMenuManager : public AHUD
{
public:

	GENERATED_UCLASS_BODY()

public:
		
	/*----------------------------------------------------
		Setup and engine API
	----------------------------------------------------*/
		
	/** Construct the Slate menu interface */
	void SetupMenu();

	void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		Public API for interaction
	----------------------------------------------------*/
	
	/** Open the main overlay */
	void OpenMainOverlay();

	/** Close the main overlay */
	void CloseMainOverlay();

	/** Asynchronously switch to a target menu, with optional data */
	bool OpenMenu(EFlareMenu::Type Target, FFlareMenuParameterData Data = FFlareMenuParameterData(), bool AddToHistory = true);
	
	/** Close the current menu */
	void CloseMenu(bool HardClose = false);

	/** Show the list of spacecraft that can be ordered here */
	void OpenSpacecraftOrder(FFlareMenuParameterData Data, FOrderDelegate ConfirmationCallback);
	
	/** Return to the previous menu */
	void Back();

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type = EFlareNotification::NT_Objective, bool Pinned = false, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, FFlareMenuParameterData TargetInfo = FFlareMenuParameterData());

	/** Remove all notifications from the screen */
	void FlushNotifications();

	/** Show the confirmation overlay */
	void Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed);

	/** Start displaying the tooltip */
	void ShowTooltip(SWidget* TargetWidget, FText Title, FText Content);

	/** Stop displaying the tooltip */
	void HideTooltip(SWidget* TargetWidget);


protected:

	/*----------------------------------------------------
		Internal management
	----------------------------------------------------*/

	/** Hide the menu */
	void ResetMenu();

	/** Fade from black */
	void FadeIn();

	/** Fade to black */
	void FadeOut();
	
	/** After a fading process has completed, proceed */
	void ProcessNextMenu();

	/** Target menu was correctly entered */
	void OnEnterMenu(bool LightBackground = true, bool ShowOverlay = true, bool TellPlayer = true);

	/** Start using the light background setting */
	void UseLightBackground();

	/** Start using the dark background setting */
	void UseDarkBackground();


	/*----------------------------------------------------
		Internal menu callbacks
	----------------------------------------------------*/

	/** Load the game */
	void LoadGame();

	/** Fly this ship */
	void FlyShip();

	/** Travel here */
	void Travel();

	/** Reload the sector */
	void ReloadSector();

	/** Open the main menu */
	void OpenMainMenu();

	/** Open the settings menu */
	void OpenSettingsMenu();

	/** Open the new game menu */
	void OpenNewGameMenu();

	/** Open the story menu */
	void OpenStoryMenu();

	/** Open the company menu */
	void InspectCompany();

	/** Show the config menu for a specific ship */
	void InspectShip(bool IsEditable = false);

	/** Show the fleet menu */
	void OpenFleetMenu();

	/** Show the quest menu */
	void OpenQuestMenu();

	/** Open the sector menu */
	void OpenSector();

	/** Open the trade menu */
	void OpenTrade();

	/** Open the trade route menu */
	void OpenTradeRoute();

	/** Open the orbital menu */
	void OpenOrbit();

	/** Open the company menu */
	void OpenLeaderboard();

	/** Open the resource prices menu */
	void OpenResourcePrices();

	/** Open the world economy menu */
	void OpenWorldEconomy();

	/** Go to the game's credits */
	void OpenCredits();

	/** Exit the menu */
	void ExitMenu();


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	/** Get the name text for this menu */
	static FText GetMenuName(EFlareMenu::Type MenuType);

	/** Get the Slate icon brush for this menu */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType);

	/** Is UI visible */
	bool IsUIOpen() const;

	/** Is a menu open */
	bool IsMenuOpen() const;

	/** Can we go back ? */
	bool HasPreviousMenu() const;

	/** Is the overlay open ? */
	bool IsOverlayOpen() const;

	/** Is a menu being opened or closed */
	bool IsSwitchingMenu() const;

	/** Are we during a fade transition ? */
	bool IsFading();

	/** Which menu, if any, is opened ? */
	EFlareMenu::Type GetCurrentMenu() const;

	/** Which menu, if any, is coming next ? */
	EFlareMenu::Type GetNextMenu() const;

	/** Get the PC */
	AFlarePlayerController* GetPC() const;

	/** Get the game */
	AFlareGame* GetGame() const;

	/** Get the spacecraft menu */
	TSharedPtr<SFlareShipMenu> GetShipMenu() const;

	/** Get the height of the main overlay */
	static int32 GetMainOverlayHeight();

	/** Get the menu manager */
	static AFlareMenuManager* GetSingleton();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Singleton pointer, because we reference this class from the entire world
	static AFlareMenuManager*               Singleton;

	// General data
	bool                                    MenuIsOpen;
	bool                                    FadeFromBlack;
	float                                   FadeDuration;
	float                                   FadeTimer;
	TFlareMenuData                          CurrentMenu;
	TFlareMenuData                          NextMenu;
	TArray<TFlareMenuData>                  MenuHistory;

	// Menu tools
	TSharedPtr<SBorder>                     Fader;
	TSharedPtr<SFlareTooltip>               Tooltip;
	TSharedPtr<SFlareNotifier>              Notifier;
	TSharedPtr<SFlareMainOverlay>           MainOverlay;
	TSharedPtr<SFlareConfirmationOverlay>   Confirmation;
	TSharedPtr<SFlareSpacecraftOrderOverlay>SpacecraftOrder;

	// Menus
	TSharedPtr<SFlareMainMenu>              MainMenu;
	TSharedPtr<SFlareSettingsMenu>          SettingsMenu;
	TSharedPtr<SFlareNewGameMenu>           NewGameMenu;
	TSharedPtr<SFlareStoryMenu>             StoryMenu;
	TSharedPtr<SFlareShipMenu>              ShipMenu;
	TSharedPtr<SFlareFleetMenu>             FleetMenu;
	TSharedPtr<SFlareQuestMenu>             QuestMenu;
	TSharedPtr<SFlareOrbitalMenu>           OrbitMenu;
	TSharedPtr<SFlareLeaderboardMenu>       LeaderboardMenu;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;
	TSharedPtr<SFlareTradeMenu>             TradeMenu;
	TSharedPtr<SFlareTradeRouteMenu>        TradeRouteMenu;
	TSharedPtr<SFlareCreditsMenu>           CreditsMenu;
	TSharedPtr<SFlareResourcePricesMenu>    ResourcePricesMenu;
	TSharedPtr<SFlareWorldEconomyMenu>      WorldEconomyMenu;
	

};
