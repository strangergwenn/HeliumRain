#pragma once

#include "../UI/Components/FlareTooltip.h"
#include "../UI/Components/FlareNotifier.h"
#include "../UI/Components/FlareSpacecraftOrderOverlay.h"
#include "../UI/Components/FlareConfirmationOverlay.h"
#include "FlareMenuManager.generated.h"


class SFlareFactoryInfo;

class SFlareMainMenu;
class SFlareSettingsMenu;
class SFlareNewGameMenu;
class SFlareDashboard;
class SFlareShipMenu;
class SFlareOrbitalMenu;
class SFlareLeaderboardMenu;
class SFlareCompanyMenu;
class SFlareSectorMenu;
class SFlareTradeMenu;
class SFlareTradeRouteMenu;
class SFlareCreditsMenu;

class AFlarePlayerController;
class AFlareSpacecraft;
class AFlareGame;
class IFlareSpacecraftInterface;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareSectorInterface;
class UFlareTradeRoute;

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
	
	/** Open a menu asynchronously, from a target and user data */
	void OpenMenu(EFlareMenu::Type Target, void* Data = NULL);

	/** Open a menu asynchronously, from a target and user data */
	void OpenMenuSpacecraft(EFlareMenu::Type Target, IFlareSpacecraftInterface* Data = NULL);

	/** Show the list of spacecraft that can be ordered here */
	void OpenSpacecraftOrder(SFlareFactoryInfo* FactoryMenu);

	/** Close the current menu */
	void CloseMenu(bool HardClose = false);
	
	/** Return to top menu */
	void Back();

	/** Is a menu open */
	bool IsMenuOpen() const;

	/** Is a menu being opened or closed */
	bool IsSwitchingMenu() const;

	/** Start the loading screen */
	void ShowLoadingScreen();

	/** Start using the light background setting */
	void UseLightBackground();

	/** Start using the dark background setting */
	void UseDarkBackground();

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type = EFlareNotification::NT_General, float Timeout = 5, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, void* TargetInfo = NULL);

	/** Remvoe all notifications from the screen */
	void FlushNotifications();

	/** Show the confirmation overlay */
	void Confirm(FText Text, FSimpleDelegate OnConfirmed);

	/** Get a Slate icon brush */
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

	/** Open the main menu */
	virtual void OpenDashboard();

	/** Open the company menu */
	virtual void InspectCompany(UFlareCompany* Target);

	/** Fly this ship */
	virtual void FlyShip(AFlareSpacecraft* Target);

	/** Show the config menu for a specific ship */
	virtual void InspectShip(IFlareSpacecraftInterface* Target = NULL, bool IsEditable = false);

	/** Open the sector menu */
	virtual void OpenSector(UFlareSectorInterface* Sector);

	/** Open the trade menu */
	virtual void OpenTrade(IFlareSpacecraftInterface* Spacecraft);

	/** Open the trade route menu */
	virtual void OpenTradeRoute(UFlareTradeRoute* TradeRoute);

	/** Open the orbital menu */
	virtual void OpenOrbit();

	/** Open the company menu */
	virtual void OpenLeaderboard();

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
	TSharedPtr<SFlareNotifier>              Notifier;
	TSharedPtr<SFlareConfirmationOverlay>   Confirmation;
	TSharedPtr<SFlareSpacecraftOrderOverlay>SpacecraftOrder;

	// Menus
	TSharedPtr<SFlareMainMenu>              MainMenu;
	TSharedPtr<SFlareSettingsMenu>          SettingsMenu;
	TSharedPtr<SFlareNewGameMenu>           NewGameMenu;
	TSharedPtr<SFlareDashboard>             Dashboard;
	TSharedPtr<SFlareShipMenu>              ShipMenu;
	TSharedPtr<SFlareOrbitalMenu>           OrbitMenu;
	TSharedPtr<SFlareLeaderboardMenu>       LeaderboardMenu;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;
	TSharedPtr<SFlareTradeMenu>             TradeMenu;
	TSharedPtr<SFlareTradeRouteMenu>        TradeRouteMenu;
	TSharedPtr<SFlareCreditsMenu>           CreditsMenu;


	/*----------------------------------------------------
		Menu target data
	----------------------------------------------------*/

	// Menu
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;

	// Ship data
	IFlareSpacecraftInterface*              FadeTargetSpacecraft;

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


};
