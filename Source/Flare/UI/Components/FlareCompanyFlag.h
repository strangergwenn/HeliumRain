#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"


class AFlarePlayerController;

class SFlareCompanyFlag : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCompanyFlag)
		: _Player(NULL)
		, _Company(NULL)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(UFlareCompany*, Company)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the company to show the flag of */
	void SetCompany(UFlareCompany* NewCompany);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the base color */
	FLinearColor GetBasePaintColor() const;

	/** Get the paint color */
	FLinearColor GetPaintColor() const;

	/** Get the light color */
	FLinearColor GetOverlayColor() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	AFlarePlayerController*           PC;
	UFlareCompany*                    Company;


};
