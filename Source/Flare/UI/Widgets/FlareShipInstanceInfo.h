#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Ships/FlareShipInterface.h"
#include "../../Stations/FlareStationInterface.h"


class SFlareShipInstanceInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipInstanceInfo)
		: _Ship(NULL)
		, _Station(NULL)
	{}
	
	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(IFlareShipInterface*, Ship)
	SLATE_ARGUMENT(IFlareStationInterface*, Station)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Show or hide the cost label */
	void SetOwned(bool State);

	/** Show or hide the details */
	void SetMinimized(bool State);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/
	
	// Slate data
	TSharedPtr<SHorizontalBox>         InfoBox;

	// Station target data
	FFlareStationSave*                StationData;
	FFlareStationDescription*         StationDescription;

	// Ship target data
	FFlareShipSave*                   ShipData;
	FFlareShipDescription*            ShipDescription;

};
