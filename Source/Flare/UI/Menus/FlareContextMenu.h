#pragma once

#include "../../Flare.h"
#include "../Widgets/FlareTargetActions.h"


class SFlareContextMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareContextMenu)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a station as content */
	void SetStation(IFlareStationInterface* Target);

	/** Set a ship as content */
	void SetShip(IFlareShipInterface* Target);

	/** Show the menu */
	void Show();

	/** Hide the menu */
	void Hide();

	/** Maximize the menu, showing its contents */
	void Open();

	/** Minimize the menu to its icon state */
	void Close();

	/** Return true if we are open */
	bool IsOpen();

	/** Return true if we are allowed to hide this menu */
	bool CanBeHidden();


protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	/** Get the current position */
	FMargin GetContextMenuPosition() const;

	/** Get the current legend text */
	FText GetLegendText() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>   OwnerHUD;

	// Widget data
	TSharedPtr<SFlareButton>          MinimizedButton;
	TSharedPtr<SVerticalBox>          Container;
	TSharedPtr<SFlareTargetActions>   ActionMenu;
	
	// State data
	bool Visible;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	bool IsVisible() const
	{
		return Visible;
	}


};
