#pragma once

#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareShipList.h"


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

	/** Are we docked */
	EVisibility GetDockedVisibility() const;

	/** Inspect the current ship */
	void OnInspectShip();

	/** Configure the current ship */
	void OnConfigureShip();

	/** Go to orbit */
	void OnOrbit();

	/** Undock */
	void OnUndock();
	
	/** Close the menu */
	void OnExit();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	TWeakObjectPtr<class AFlareMenuManager> MenuManager;

	// Widgets
	TSharedPtr<SFlareShipList>              OwnedShipList;
	TSharedPtr<SFlareShipList>              OtherShipList;
	

};
