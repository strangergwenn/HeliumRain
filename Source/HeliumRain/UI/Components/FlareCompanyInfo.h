#pragma once

#include "../../Flare.h"


class UFlareCompany;
class AFlarePlayerController;


class SFlareCompanyInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCompanyInfo)
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

	/** Get the hostility data towards us, from this company */
	FText GetCompanyHostility() const;

	/** Get the company emblem */
	const FSlateBrush* GetCompanyEmblem() const;

	/** Hide the war button on ourselves */
	EVisibility GetToggleHostilityVisibility() const;

	/** Get the war/peace button text */
	FText GetToggleHostilityText() const;

	/** Toggle player hostility toward this company */
	void OnToggleHostility();

	
protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*                    Player;
	UFlareCompany*                             Company;


};
