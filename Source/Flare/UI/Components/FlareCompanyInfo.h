#pragma once

#include "../../Flare.h"


class UFlareCompany;


class SFlareCompanyInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCompanyInfo)
		: _Company(NULL)
	{}

	SLATE_ARGUMENT(UFlareCompany*, Company)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Set the company to display */
	void SetCompany(UFlareCompany* NewCompany);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the name of the company */
	FText GetCompanyName() const;

	/** Get the company info text */
	FText GetCompanyInfo() const;

	/** Get the company emblem */
	const FSlateBrush* GetCompanyEmblem() const;

	
protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	UFlareCompany*                    Company;


};
