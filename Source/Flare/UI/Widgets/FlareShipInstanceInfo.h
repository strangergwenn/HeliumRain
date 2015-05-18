#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../Widgets/FlareTargetActions.h"
#include "../../Ships/FlareShipInterface.h"


class SFlareShipInstanceInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipInstanceInfo)
		: _Ship(NULL)
	{}
	
	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(IFlareShipInterface*, Ship)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Set if we can show the actions or not */
	void SetActionsVisible(bool State);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Ship target data
	IFlareShipInterface*              Ship;
	FFlareShipSave*                   ShipData;
	FFlareShipDescription*            ShipDescription;

	// Widgets
	TSharedPtr<SHorizontalBox>        ListContainer;
	TSharedPtr<SFlareTargetActions>   ActionContainer;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;

};
