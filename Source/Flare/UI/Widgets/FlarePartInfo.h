#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../../Ships/FlareShipComponent.h"


class SFlarePartInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlarePartInfo)
	: _IsOwned(false)
	, _ShowOwnershipInfo(false)
	{}

	SLATE_ARGUMENT(bool, IsOwned)

	SLATE_ARGUMENT(bool, ShowOwnershipInfo)

	SLATE_ARGUMENT(const FFlareShipModuleDescription*, Description)
	
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
	static void BuildInfoBlock(TSharedPtr<SHorizontalBox>& Box, const FFlareShipModuleDescription* Desc, bool ShowHelpers = true);

	/** Get a readable info string */
	static FString GetCharacteristicInfo(const FFlareShipModuleDescription* Desc, EFlarePartCharacteristicType::Type Type);

	/** Get a readable info string */
	static FString GetCharacteristicInfo(const FFlarePartCharacteristic& Characteristic);

	/** Get a readable label string */
	static FString GetCharacteristicLabel(EFlarePartCharacteristicType::Type Type);

	/** Get a Slate brush */
	static const FSlateBrush* GetCharacteristicBrush(const FFlarePartCharacteristic& Characteristic);
	

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
