
#include "../../Flare.h"
#include "FlareConfirmationOverlay.h"

#define LOCTEXT_NAMESPACE "FlareConfirmationOverlay"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareConfirmationOverlay::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	InfoText.Set(LOCTEXT("AreYouSureInfo", "Please confirm this action."));

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(0, 10))
			.BorderImage(&Theme.BackgroundBrush)
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				.BorderImage(&Theme.BackgroundBrush)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AreYouSure", "ARE YOU SURE ?"))
						.TextStyle(&Theme.TitleFont)
						.Justification(ETextJustify::Center)
					]

					// Info text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.Text(InfoText)
						.TextStyle(&Theme.SubTitleFont)
						.Justification(ETextJustify::Center)
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("Confirm", "Confirm"))
							.HelpText(LOCTEXT("ConfirmInfo", "Confirm this action"))
							.Icon(FFlareStyleSet::GetIcon("OK"))
							.OnClicked(this, &SFlareConfirmationOverlay::OnConfirmed)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("Cancel", "Cancel"))
							.HelpText(LOCTEXT("CancelInfo", "Cancel this action"))
							.Icon(FFlareStyleSet::GetIcon("Delete"))
							.OnClicked(this, &SFlareConfirmationOverlay::OnCancelled)
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareConfirmationOverlay::Confirm(FText Text, FSimpleDelegate OnConfirmed)
{
	InfoText.Set(Text);
	OnConfirmedCB = OnConfirmed;
	SetVisibility(EVisibility::Visible);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareConfirmationOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

void SFlareConfirmationOverlay::OnConfirmed()
{
	OnConfirmedCB.ExecuteIfBound();
	SetVisibility(EVisibility::Collapsed);
}

void SFlareConfirmationOverlay::OnCancelled()
{
	SetVisibility(EVisibility::Collapsed);
}

#undef LOCTEXT_NAMESPACE
