#pragma once

#include "../../Flare.h"
#include "../../Ships/FlareShipComponent.h"
#include "FlareButton.h"


class SFlareDashboardButton : public SFlareButton
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareDashboardButton)
		: _Icon(NULL)
	{}

	SLATE_EVENT(FFlareButtonClicked, OnClicked)
		
	SLATE_ARGUMENT(const FSlateBrush*, Icon)
	SLATE_ARGUMENT(FText, Text)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	const FFlareContainerStyle* ContainerStyle;


};
