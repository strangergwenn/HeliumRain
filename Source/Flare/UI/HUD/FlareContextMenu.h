#pragma once

#include "../../Flare.h"
#include "../Components/FlareRoundButton.h"
#include "../Widgets/FlareShipStatus.h"
#include "../Widgets/FlareTargetActions.h"


class SFlareContextMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareContextMenu)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, HUD)
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a station as content */
	void SetStation(IFlareSpacecraftInterface* Target);

	/** Set a ship as content */
	void SetShip(IFlareSpacecraftInterface* Target);

	/** Show the menu */
	void Show();

	/** Hide the menu */
	void Hide();

	/** Open the menu associated to the target */
	void OpenTargetMenu();


protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	/** Get the current position */
	FMargin GetContextMenuPosition() const;
	
	/** Get the current button label */
	FText GetText() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	TWeakObjectPtr<class AFlareHUD>            HUD;
	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;
		
	// State data
	IFlareSpacecraftInterface*                 TargetStation;
	IFlareSpacecraftInterface*                 TargetShip;
	
};
