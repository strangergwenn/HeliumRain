#pragma once

#include "../UI/Components/FlareTooltip.h"
#include "../UI/Components/FlareMainOverlay.h"
#include "../UI/Components/FlareSpacecraftOrderOverlay.h"
#include "../UI/Components/FlareConfirmationOverlay.h"
#include "FlareMenuManager.generated.h"


class SFlareFactoryInfo;

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

class AFlarePlayerController;
class AFlareSpacecraft;
class AFlareGame;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareTradeRoute;
class UFlareFactory;
class UFlareFleet;
struct FFlareWorldEconomyMenuParam;

/** Main HUD class (container for HUD and menus) */
UCLASS()
class HELIUMRAIN_API AFlareMenuManager : public AHUD
{
public:

	GENERATED_UCLASS_BODY()

public:
		
	/*----------------------------------------------------
		Setup
	----------------------------------------------------*/
		
	/** Construct the Slate menu interface */
	virtual void SetupMenu();


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;

	/** Open the main overlay */
	void OpenMainOverlay();

	/** Close the main overlay */
	void CloseMainOverlay();
	
	/** Is the overlay open ? */
	bool IsOverlayOpen() const;
	
	/** Open a menu asynchronously, from a target and user data */
	void OpenMenu(EFlareMenu::Type Target, void* Data = NULL);

	/** Open a menu asynchronously, from a target and user data */
	void OpenMenuSpacecraft(EFlareMenu::Type Target, UFlareSimulatedSpacecraft* Data = NULL);

	/** Show the list of spacecraft that can be ordered here */
	void OpenSpacecraftOrder(UFlareFactory* Factory);

	/** Show the list of stations that can be ordered here */
	void OpenSpacecraftOrder(UFlareSimulatedSector* Sector, FOrderDelegate ConfirmationCallback);

	/** Is UI visible */
	bool IsUIOpen() const;

	/** Is a menu open */
	bool IsMenuOpen() const;

	/** Close the current menu */
	void CloseMenu(bool HardClose = false);
	
	/** Return to the previous menu */
	void Back();

	/** Which menu, if any, is opened ? */
	EFlareMenu::Type GetCurrentMenu() const;

	/** Which menu, if any, was opened ? */
	EFlareMenu::Type GetPreviousMenu() const;
	
	/** Is a menu being opened or closed */
	bool IsSwitchingMenu() const;

	/** Start the loading screen */
	void ShowLoadingScreen();

	/** Start using the light background setting */
	void UseLightBackground();

	/** Start using the dark background setting */
	void UseDarkBackground();

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type = EFlareNotification::NT_Objective, float Timeout = 5, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, void* TargetInfo = NULL, FName TargetSpacecraft = NAME_None);

	/** Remvoe all notifications from the screen */
	void FlushNotifications();

	/** Show the confirmation overlay */
	void Confirm(FText Title, FText Text, FSimpleDelegate OnConfirmed);

	/** Get the name text for this menu */
	static FText GetMenuName(EFlareMenu::Type MenuType);

	/** Get the Slate icon brush for this menu */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType, bool ButtonVersion = false);

	/** Start displaying the tooltip */
	void ShowTooltip(SWidget* TargetWidget, FText Title, FText Content);

	/** Stop displaying the tooltip */
	void HideTooltip(SWidget* TargetWidget);

	// Is this a spacecraft menu
	bool IsSpacecraftMenu(EFlareMenu::Type Type) const;


protected:

	/*----------------------------------------------------
		Menu management
	----------------------------------------------------*/

	/** Hide the menu */
	void ResetMenu();

	/** Fade from black */
	void FadeIn();

	/** Fade to black */
	void FadeOut();

	/** Are we during a fade transition ? */
	bool IsFading();
	
	/** After a fading process has completed, proceed */
	virtual void ProcessFadeTarget();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Open the main menu */
	virtual void OpenMainMenu();

	/** Open the settings menu */
	virtual void OpenSettingsMenu();

	/** Open the new game menu */
	virtual void OpenNewGameMenu();

	/** Open the story menu */
	virtual void OpenStoryMenu();

	/** Open the company menu */
	virtual void InspectCompany(UFlareCompany* Target);

	/** Fly this ship */
	virtual void FlyShip(UFlareSimulatedSpacecraft* Target);

	/** Activate a sector */
	virtual void ActivateSector(UFlareSimulatedSector* Target);

	/** Show the config menu for a specific ship */
	virtual void InspectShip(UFlareSimulatedSpacecraft* Target = NULL, bool IsEditable = false);

	/** Show the fleet menu */
	virtual void OpenFleetMenu(UFlareFleet* TargetFleet);

	/** Open the sector menu */
	virtual void OpenSector(UFlareSimulatedSector* Sector);

	/** Open the trade menu */
	virtual void OpenTrade(UFlareSimulatedSpacecraft* Spacecraft);

	/** Open the trade route menu */
	virtual void OpenTradeRoute(UFlareTradeRoute* TradeRoute);

	/** Open the orbital menu */
	virtual void OpenOrbit();

	/** Open the company menu */
	virtual void OpenLeaderboard();

	/** Open the resource prices menu */
	virtual void OpenResourcePrices(UFlareSimulatedSector* Sector);

	/** Open the world economy menu */
	virtual void OpenWorldEconomy(FFlareWorldEconomyMenuParam* Params);

	/** Go to the game's credits */
	virtual void OpenCredits();

	/** Exit the menu */
	virtual void ExitMenu();


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
	TSharedPtr<SBorder>                     Fader;
	TEnumAsByte<EFlareMenu::Type>           CurrentMenu;
	TEnumAsByte<EFlareMenu::Type>           LastNonSettingsMenu;

	// Menu tools
	TSharedPtr<SFlareTooltip>               Tooltip;
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


	/*----------------------------------------------------
		Menu target data
	----------------------------------------------------*/

	// Menu
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;

	// Ship data
	UFlareSimulatedSpacecraft*              FadeTargetSpacecraft;

	// Generic data
	void*                                   FadeTargetData;


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
		return 128;
	}

	TSharedPtr<SFlareShipMenu>	GetShipMenu()
	{
		return ShipMenu;
	}

};
