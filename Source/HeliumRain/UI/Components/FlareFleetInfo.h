#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"


DECLARE_DELEGATE_OneParam(FFlareObjectRemoved, UFlareSimulatedSpacecraft*)


class SFlareFleetInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFleetInfo)
		: _Player(NULL)
		, _Fleet(NULL)
		, _OwnerWidget(NULL)
		, _Minimized(false)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(UFlareFleet*, Fleet)
	SLATE_ARGUMENT(SWidget*, OwnerWidget)
		
	SLATE_ARGUMENT(bool, Minimized)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Set a fleet as content */
	void SetFleet(UFlareFleet* Fleet);
	
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
		

	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get the target name */
	FText GetName() const;

	/** Get the text color */
	FSlateColor GetTextColor() const;

	/** Get the target description */
	FText GetDescription() const;

	/** Hide the company flag if owned */
	EVisibility GetCompanyFlagVisibility() const;
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;
	bool                              Minimized;

	// Target data	
	UFlareFleet*                      TargetFleet;
	FText                             TargetName;

	// Slate data (buttons)
	TSharedPtr<SFlareButton>          InspectButton;

	// Slate data (various)
	TSharedPtr<SWidget>               OwnerWidget;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;

};
