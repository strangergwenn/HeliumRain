#pragma once

#include "../../Flare.h"
#include "../Widgets/FlareShipStatus.h"
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
	void SetStation(IFlareShipInterface* Target);

	/** Set a ship as content */
	void SetShip(IFlareShipInterface* Target);

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
	TSharedPtr<SFlareShipStatus>      ShipStatus;
	TSharedPtr<SVerticalBox>          Container;
	
	// State data
	TSharedPtr<SFlareButton>          MinimizedButton;
	IFlareShipInterface*           TargetStation;
	IFlareShipInterface*              TargetShip;
	
};
