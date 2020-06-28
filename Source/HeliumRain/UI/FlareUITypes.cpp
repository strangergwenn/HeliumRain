
#include "HeliumRain/UI/FlareUITypes.h"
#include "../Flare.h"

#define LOCTEXT_NAMESPACE "FlareUITypes"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareUITypes::UFlareUITypes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Utility
----------------------------------------------------*/

TSharedRef<SWidget> UFlareUITypes::Header(FText Title)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SHorizontalBox)
			
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		]

		// Title
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(Title)
		]

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SImage)
				.Image(&Theme.NearInvisibleBrush)
			]
		];
}

#undef LOCTEXT_NAMESPACE
