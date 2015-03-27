#pragma once

#include "../../Flare.h"
#include "../../Ships/FlareShipInterface.h"


class SFlareShipStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipStatus)
		: _Ship(NULL)
	{}

	SLATE_ARGUMENT(IFlareShipInterface*, Ship)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the ship to display data for */
	void SetTargetShip(IFlareShipInterface* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Is this visible ? */
	EVisibility IsVisible(TSharedPtr<EFlareSubsystem::Type> Type) const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Ship data
	IFlareShipInterface*                    TargetShip;


};
