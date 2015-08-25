#pragma once

#include "../UI/Menus/FlareMainMenu.h"
#include "../UI/Menus/FlareNewGameMenu.h"
#include "../UI/Menus/FlareDashboard.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Menus/FlareOrbitalMenu.h"
#include "../UI/Menus/FlareLeaderboardMenu.h"
#include "../UI/Components/FlareTooltip.h"
#include "../UI/Components/FlareNotifier.h"
#include "../UI/Components/FlareConfirmationOverlay.h"
#include "FlareMenuManager.generated.h"


/** Main HUD class (container for HUD and menus) */
UCLASS()
class FLARE_API AFlareMenuManager : public AHUD
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

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, EFlareNotification::Type Type = EFlareNotification::NT_General, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, void* TargetInfo = NULL);

	/** Remvoe all notifications from the screen */
	void FlushNotifications();

	/** Show the confirmation overlay */
	void Confirm(FText Text, FSimpleDelegate OnConfirmed);

	/** Get a Slate icon brush */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType, bool ButtonVersion = false);

	/** Start displaying the tooltip */
	void ShowTooltip(SWidget* TargetWidget, FText Content);

	/** Stop displaying the tooltip */
	void HideTooltip(SWidget* TargetWidget);


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
	virtual void OpenSector();

	/** Open the orbital menu */
	virtual void OpenOrbit();

	/** Open the company menu */
	virtual void OpenLeaderboard();

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

	// Menus
	TSharedPtr<SFlareTooltip>               Tooltip;
	TSharedPtr<SFlareNotifier>              Notifier;
	TSharedPtr<SFlareConfirmationOverlay>   Confirmation;
	TSharedPtr<SFlareMainMenu>              MainMenu;
	TSharedPtr<SFlareNewGameMenu>           NewGameMenu;
	TSharedPtr<SFlareDashboard>             Dashboard;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareShipMenu>              ShipMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;
	TSharedPtr<SFlareOrbitalMenu>           OrbitMenu;
	TSharedPtr<SFlareLeaderboardMenu>       LeaderboardMenu;
	

	// Menu target data
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;
	void*                                   FadeTargetData;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlarePlayerController* GetPC() const
	{
		return Cast<AFlarePlayerController>(GetOwner());
	}

	inline AFlareGame* GetGame() const
	{
		return GetPC()->GetGame();
	}

	static inline AFlareMenuManager* GetSingleton()
	{
		return Singleton;
	}


};
