
#include "FlareCompanyFlag.h"
#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"


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
				SNew(SColorBlock).Color(this, &SFlareCompanyFlag::GetBasePaintColor)
			]

			// Middle line
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(5)
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
				.HeightOverride(5)
				[
					SNew(SColorBlock).Color(this, &SFlareCompanyFlag::GetOverlayColor)
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

FLinearColor SFlareCompanyFlag::GetBasePaintColor() const
{
	if (PC && Company)
	{
		return Company->GetBasePaintColor();
	}
	return FLinearColor::Black;
}

FLinearColor SFlareCompanyFlag::GetPaintColor() const
{
	if (PC && Company)
	{
		return Company->GetPaintColor();
	}
	return FLinearColor::Black;
}

FLinearColor SFlareCompanyFlag::GetOverlayColor() const
{
	if (PC && Company)
	{
		return Company->GetOverlayColor();
	}
	return FLinearColor::Black;
}
