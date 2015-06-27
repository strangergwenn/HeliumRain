#pragma once

#include "GameFramework/HUD.h"
#include "../Spacecrafts/FlareSpacecraftPawn.h"
#include "../UI/Menus/FlareMainMenu.h"
#include "../UI/Menus/FlareDashboard.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareStationMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Widgets/FlareNotifier.h"
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

	virtual void BeginPlay() override;
	
	/** Construct the Slate menu interface */
	virtual void SetupMenu(struct FFlarePlayerSave& PlayerData);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;
	
	/** Open a menu asynchronously, from a target and user data */
	void OpenMenu(EFlareMenu::Type Target, void* Data = NULL);

	/** Close the current menu */
	void CloseMenu(bool HardClose = false);

	/** Is a menu open */
	bool IsMenuOpen() const;

	/** Start the loading screen */
	void ShowLoadingScreen();

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo);

	/** Get a Slate icon brush */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType);


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
	virtual void OpenDashboard();

	/** Open the company menu */
	virtual void InspectCompany(UFlareCompany* Target);

	/** Fly this ship */
	virtual void FlyShip(AFlareSpacecraft* Target);

	/** Show the config menu for a specific ship */
	virtual void InspectShip(IFlareSpacecraftInterface* Target = NULL, bool IsEditable = false);

	/** Show the config menu for a specific station */
	virtual void InspectStation(IFlareSpacecraftInterface* Target = NULL, bool IsEditable = false);

	/** Open the sector menu */
	virtual void OpenSector();

	/** Exit the menu */
	virtual void ExitMenu();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// General data
	bool                                    MenuIsOpen;
	bool                                    FadeFromBlack;
	float                                   FadeDuration;
	float                                   FadeTimer;
	TSharedPtr<SBorder>                     Fader;
	
	// Menus
	TSharedPtr<SFlareNotifier>              Notifier;
	TSharedPtr<SFlareMainMenu>              MainMenu;
	TSharedPtr<SFlareDashboard>             Dashboard;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareShipMenu>              ShipMenu;
	TSharedPtr<SFlareStationMenu>           StationMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;

	// Menu target data
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;
	void*                                   FadeTargetData;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlarePlayerController* GetPC()
	{
		return Cast<AFlarePlayerController>(GetOwner());
	}


};
