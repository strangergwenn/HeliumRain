#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Ships/FlareShipInterface.h"
#include "../../Player/FlarePlayerController.h"


class SFlareTargetActions : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTargetActions)
		: _Player(NULL)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a station as content */
	void SetStation(IFlareStationInterface* Target);

	/** Set a ship as content */
	void SetShip(IFlareShipInterface* Target);

	/** SHow the menu */
	void Show();

	/** Hide the menu */
	void Hide();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the current target */
	void OnInspect();

	/** Fly the current target */
	void OnFly();

	/** Try to dock at the target station */
	void OnDockAt();

	/** Undock */
	void OnUndock();


	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get the target name */
	FString GetName() const;

	/** Get the target class name */
	FString GetClassName() const;

	/** Get the target icon */
	const FSlateBrush* GetIcon() const;

	/** Get the target class icon */
	const FSlateBrush* GetClassIcon() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;

	// Target data
	IFlareStationInterface*           TargetStation;
	FFlareStationDescription*         TargetStationDesc;
	IFlareShipInterface*              TargetShip;
	FFlareShipDescription*            TargetShipDesc;
	FString                           TargetName;

	// Slate data
	TSharedPtr<SHorizontalBox>        StationContainer;
	TSharedPtr<SHorizontalBox>        ShipContainer;
	TSharedPtr<SFlareButton>          DockButton;
	TSharedPtr<SFlareButton>          UndockButton;

};
