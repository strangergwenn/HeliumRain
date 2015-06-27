#pragma once

#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"


class SFlareDashboard : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareDashboard){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter();

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the company */
	void OnInspectCompany();

	/** Inspect the current ship */
	void OnInspectShip();

	/** Inspect the current station */
	void OnInspectStation();

	/** Configure the current ship */
	void OnConfigureShip();

	/** Undock */
	void OnUndock();

	/** Open the sector map */
	void OnOpenSector();

	/** Quit the game */
	void OnQuitGame();

	/** Close the dashboard */
	void OnExit();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager> MenuManager;

	// Widgets
	TSharedPtr<SVerticalBox> CompanyBox;
	TSharedPtr<SVerticalBox> StationBox;
	TSharedPtr<SVerticalBox> UniverseBox;
	TSharedPtr<SVerticalBox> SettingsBox;
	

};
