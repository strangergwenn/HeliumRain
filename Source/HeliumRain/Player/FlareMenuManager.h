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
class UFlareCompany;
class AFlareGame;
class UFlareFactory;
class UFlareFleet;
class UFlareSimulatedSector;
class AFlareSpacecraft;
class UFlareTradeRoute;
class UFlareTravel;
struct FFlareResourceDescription;


/** Menu parameter structure storing commands + data for async processing */
struct FFlareMenuParameterData
{
	FFlareMenuParameterData()
		: Company(NULL)
		, Factory(NULL)
		, Fleet(NULL)
		, Route(NULL)
		, Sector(NULL)
		, Spacecraft(NULL)
		, Travel(NULL)
		, Resource(NULL)
	{}

	UFlareCompany*                        Company;
	UFlareFactory*                        Factory;
	UFlareFleet*                          Fleet;
	UFlareTradeRoute*                     Route;
	UFlareSimulatedSector*                Sector;
	UFlareSimulatedSpacecraft*            Spacecraft;
	UFlareTravel*                         Travel;
	FFlareResourceDescription*            Resource;
};

typedef TPair<EFlareMenu::Type, FFlareMenuParameterData*> TFlareMenuData;


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
	virtual void SetupMenu();

	virtual void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		Public API for interaction
	----------------------------------------------------*/
	
	/** Open the main overlay */
	void OpenMainOverlay();

	/** Close the main overlay */
	void CloseMainOverlay();

	/** Asynchronously switch to a target menu, with optional data */
	bool OpenMenu(EFlareMenu::Type Target, FFlareMenuParameterData* Data = NULL);
	
	/** Close the current menu */
	void CloseMenu(bool HardClose = false);

	/** Show the list of spacecraft that can be ordered here */
	void OpenSpacecraftOrder(FFlareMenuParameterData* Data, FOrderDelegate ConfirmationCallback);
		
	/** Return to the previous menu */
	void Back();

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type = EFlareNotification::NT_Objective, float Timeout = 5, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, FFlareMenuParameterData* TargetInfo = NULL);
	
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
	void ProcessCurrentTarget();

	/** Pop the previous menu from the stack */
	TFlareMenuData PopPreviousMenu();

	/** Remvoe all notifications from the screen */
	void FlushNotifications();

	/** Start using the light background setting */
	void UseLightBackground();

	/** Start using the dark background setting */
	void UseDarkBackground();


	/*----------------------------------------------------
		Internal menu callbacks
	----------------------------------------------------*/

	/** Load the game */
	virtual void LoadGame(FFlareMenuParameterData* Data);

	/** Fly this ship */
	virtual void FlyShip(FFlareMenuParameterData* Data);

	/** Travel here */
	virtual void Travel(FFlareMenuParameterData* Data);

	/** Open the main menu */
	virtual void OpenMainMenu(FFlareMenuParameterData* Data);

	/** Open the settings menu */
	virtual void OpenSettingsMenu(FFlareMenuParameterData* Data);

	/** Open the new game menu */
	virtual void OpenNewGameMenu(FFlareMenuParameterData* Data);

	/** Open the story menu */
	virtual void OpenStoryMenu(FFlareMenuParameterData* Data);

	/** Open the company menu */
	virtual void InspectCompany(FFlareMenuParameterData* Data);

	/** Show the config menu for a specific ship */
	virtual void InspectShip(FFlareMenuParameterData* Data, bool IsEditable = false);

	/** Show the fleet menu */
	virtual void OpenFleetMenu(FFlareMenuParameterData* Data);

	/** Open the sector menu */
	virtual void OpenSector(FFlareMenuParameterData* Data);

	/** Open the trade menu */
	virtual void OpenTrade(FFlareMenuParameterData* Data);

	/** Open the trade route menu */
	virtual void OpenTradeRoute(FFlareMenuParameterData* Data);

	/** Open the orbital menu */
	virtual void OpenOrbit(FFlareMenuParameterData* Data);

	/** Open the company menu */
	virtual void OpenLeaderboard(FFlareMenuParameterData* Data);

	/** Open the resource prices menu */
	virtual void OpenResourcePrices(FFlareMenuParameterData* Data);

	/** Open the world economy menu */
	virtual void OpenWorldEconomy(FFlareMenuParameterData* Data);

	/** Go to the game's credits */
	virtual void OpenCredits(FFlareMenuParameterData* Data);

	/** Exit the menu */
	virtual void ExitMenu();


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	/** Is UI visible */
	bool IsUIOpen() const;

	/** Is a menu open */
	bool IsMenuOpen() const;

	/** Is the overlay open ? */
	bool IsOverlayOpen() const;

	/** Is a menu being opened or closed */
	bool IsSwitchingMenu() const;

	/** Are we during a fade transition ? */
	bool IsFading();

	/** Which menu, if any, is opened ? */
	EFlareMenu::Type GetCurrentMenu() const;

	/** Get the name text for this menu */
	static FText GetMenuName(EFlareMenu::Type MenuType);

	/** Get the Slate icon brush for this menu */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType, bool ButtonVersion = false);


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
	TFlareMenuData                          CurrentTarget;
	TEnumAsByte<EFlareMenu::Type>           CurrentMenu;
	TEnumAsByte<EFlareMenu::Type>           LastNonSettingsMenu;

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
	TSharedPtr<SFlareOrbitalMenu>           OrbitMenu;
	TSharedPtr<SFlareLeaderboardMenu>       LeaderboardMenu;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;
	TSharedPtr<SFlareTradeMenu>             TradeMenu;
	TSharedPtr<SFlareTradeRouteMenu>        TradeRouteMenu;
	TSharedPtr<SFlareCreditsMenu>           CreditsMenu;
	TSharedPtr<SFlareResourcePricesMenu>    ResourcePricesMenu;
	TSharedPtr<SFlareWorldEconomyMenu>      WorldEconomyMenu;
	

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlarePlayerController* GetPC() const
	{
		return Cast<AFlarePlayerController>(GetOwner());
	}

	AFlareGame* GetGame() const;

	static inline AFlareMenuManager* GetSingleton()
	{
		return Singleton;
	}

	static inline int32 GetMainOverlayHeight()
	{
		return 135;
	}

	TSharedPtr<SFlareShipMenu>	GetShipMenu()
	{
		return ShipMenu;
	}

};
