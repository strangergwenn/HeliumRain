
#include "../../Flare.h"
#include "FlareLargeButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareLargeButton::Construct(const FArguments& InArgs)
{
	// Arguments
	IsClickable = InArgs._Clickable;
	OnClicked = InArgs._OnClicked;
	IconColor = InArgs._IconColor;
	HighlightColor = InArgs._HighlightColor;

	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SVerticalBox)

		// Container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Image box
			SNew(SBox)
			.WidthOverride(Theme.LargeButtonWidth)
			.HeightOverride(Theme.LargeButtonHeight)
			[
				// Button (behaviour only, no display)
				SNew(SButton)
				.OnClicked(this, &SFlareLargeButton::OnButtonClicked)
				.ContentPadding(FMargin(0))
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				[
					// Background
					SNew(SBorder)
					.Padding(Theme.LargeButtonPadding)
					.BorderImage(&Theme.LargeButtonBackground)
					.BorderBackgroundColor(HighlightColor)
					[
						// Icon
						SNew(SImage)
						.Image(InArgs._Icon)
						.ColorAndOpacity(IconColor)
					]
				]
			]
		]

		// Button text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(Text, STextBlock)
			.Text(InArgs._Text)
			.TextStyle(&Theme.SmallFont)
			.Justification(ETextJustify::Center)
		]
	];

	// Text management
	if (!InArgs._ShowText)
	{
		Text->SetVisibility(EVisibility::Collapsed);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareLargeButton::Show()
{
}

void SFlareLargeButton::Hide()
{
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareLargeButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() && IsClickable ? &Theme.ButtonActiveBackground : &Theme.ButtonBackground);
}

FReply SFlareLargeButton::OnButtonClicked()
{
	if (IsClickable && OnClicked.IsBound())
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}
