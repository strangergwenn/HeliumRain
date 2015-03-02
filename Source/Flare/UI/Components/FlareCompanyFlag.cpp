
#include "../../Flare.h"
#include "FlareCompanyFlag.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCompanyFlag::Construct(const FArguments& InArgs)
{
	// Args
	PC = InArgs._Player;
	Company = InArgs._Company;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.WidthOverride(32)
		.HeightOverride(16)
		[
			SNew(SVerticalBox)

			// Pattern
			+ SVerticalBox::Slot()
			[
				SNew(SImage).Image(this, &SFlareCompanyFlag::GetPattern)
			]

			// Middle line
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(3)
				[
					SNew(SColorBlock).Color(this, &SFlareCompanyFlag::GetPaintColor)
				]
			]

			// Bottom line
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(3)
				[
					SNew(SColorBlock).Color(this, &SFlareCompanyFlag::GetLightColor)
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyFlag::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareCompanyFlag::GetPattern() const
{
	if (PC && Company)
	{
		return PC->GetGame()->GetCustomizationCatalog()->GetPatternBrush(Company->GetPatternIndex());
	}
	return NULL;
}

FLinearColor SFlareCompanyFlag::GetPaintColor() const
{
	if (PC && Company)
	{
		return PC->GetGame()->GetCustomizationCatalog()->GetColor(Company->GetPaintColorIndex());
	}
	return FLinearColor::Black;
}

FLinearColor SFlareCompanyFlag::GetLightColor() const
{
	if (PC && Company)
	{
		return PC->GetGame()->GetCustomizationCatalog()->GetColor(Company->GetLightColorIndex());
	}
	return FLinearColor::Black;
}
