#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../Widgets/FlareShipStatus.h"
#include "../../Ships/FlareShipInterface.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"


class SFlareTargetActions : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTargetActions)
		: _Player(NULL)
		, _NoInspect(false)
		, _MinimizedMode(false)
		, _Translucent(false)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)

	SLATE_ARGUMENT(bool, NoInspect)
	SLATE_ARGUMENT(bool, MinimizedMode)
	SLATE_ARGUMENT(bool, Translucent)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a company as content */
	void SetCompany(UFlareCompany* Target);

	/** Set a station as content */
	void SetStation(IFlareStationInterface* Target);

	/** Set a ship as content */
	void SetShip(IFlareShipInterface* Target);

	/** Set the no-inspect mode */
	void SetNoInspect(bool NewState);

	/** Set the minimized mode */
	void SetMinimized(bool NewState);

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
	FString GetDescription() const;

	/** Get the target icon */
	const FSlateBrush* GetIcon() const;

	/** Get the target class icon */
	const FSlateBrush* GetClassIcon() const;
	
	/** Get the company name */
	FString GetCompanyName() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;
	bool                              NoInspect;
	bool                              MinimizedMode;

	// Target data
	UFlareCompany*                    TargetCompany;
	IFlareStationInterface*           TargetStation;
	FFlareStationDescription*         TargetStationDesc;
	IFlareShipInterface*              TargetShip;
	FFlareShipDescription*            TargetShipDesc;
	FString                           TargetName;

	// Slate data
	TSharedPtr<SHorizontalBox>        CompanyContainer;
	TSharedPtr<SHorizontalBox>        StationContainer;
	TSharedPtr<SHorizontalBox>        ShipContainer;
	TSharedPtr<SFlareButton>          StationInspectButton;
	TSharedPtr<SFlareShipStatus>      ShipStatus;
	TSharedPtr<SFlareButton>          ShipInspectButton;
	TSharedPtr<SFlareButton>          ShipFlyButton;
	TSharedPtr<SFlareButton>          DockButton;
	TSharedPtr<SFlareButton>          UndockButton;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;

};
