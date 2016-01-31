#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Spacecrafts/FlareSpacecraftComponent.h"


class SFlarePartInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlarePartInfo)
	: _IsOwned(false)
	, _IsMinimized(false)
	, _ShowOwnershipInfo(false)
	{}

	SLATE_ARGUMENT(bool, IsOwned)

	SLATE_ARGUMENT(bool, IsMinimized)

	SLATE_ARGUMENT(bool, ShowOwnershipInfo)

	SLATE_ARGUMENT(const FFlareSpacecraftComponentDescription*, Description)
	
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
		Content
	----------------------------------------------------*/

	/** Get a Slate info block */
	static void BuildInfoBlock(TSharedPtr<SHorizontalBox>& Box, const FFlareSpacecraftComponentDescription* Desc, bool ShowHelpers = true);

	/** Add data for a single characteristic to an horizontal box */
	static void AddCharacteristicToBlock(TSharedPtr<SHorizontalBox>& Box, FText Label, FText Value, const FSlateBrush* Icon, bool ShowHelpers);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Behaviour data
	bool                               IsOwned;
	bool                               ShowOwnershipInfo;

	// Content data
	float                              PartCost;

	// Slate data
	TSharedPtr<SImage>                 CostImage;
	TSharedPtr<STextBlock>             CostLabel;
	TSharedPtr<SHorizontalBox>         InfoBox;
	TSharedPtr<SVerticalBox>           Details;


};
