
#include "../../Flare.h"
#include "FlareDashboardButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareDashboardButton::Construct(const FArguments& InArgs)
{
	// Parent constructor call
	SFlareButton::Construct(SFlareButton::FArguments()
		.Toggle(false)
		.OnClicked(InArgs._OnClicked)
		.ButtonStyle(FFlareStyleSet::Get(), "/Style/DashboardButton")
	);

	// Content
	InnerContainer->SetContent(
		SNew(SVerticalBox)

		// Part icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			SNew(SImage).Image(InArgs._Icon)
		]

		// Text box
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(STextBlock)
			.TextStyle(&FFlareStyleSet::GetDefaultTheme().TextFont)
			.Text(InArgs._Text)
		]
	);
}
