#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Ships/FlareShipComponent.h"


class SFlareShipInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipInfo)
	: _IsOwned(false)
	, _ShowOwnershipInfo(false)
	{}

	SLATE_ARGUMENT(bool, IsOwned)

	SLATE_ARGUMENT(bool, ShowOwnershipInfo)

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(IFlareShipInterface*, Ship)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Show or hide the cost label */
	void SetOwned(bool State);


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/



protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Behaviour data
	bool                               IsOwned;
	bool                               IsMinimized;
	bool                               ShowOwnershipInfo;

	// Content data
	float                              ShipCost;

	// Slate data
	TSharedPtr<SImage>                 CostImage;
	TSharedPtr<STextBlock>             CostLabel;
	TSharedPtr<SHorizontalBox>         InfoBox;
	TSharedPtr<SVerticalBox>           Details;


};
