
#include "../../Flare.h"
#include "FlareRoundButton.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareRoundButton::Construct(const FArguments& InArgs)
{
	// Arguments
	IsClickable = InArgs._Clickable;
	OnClicked = InArgs._OnClicked;
	IconColor = InArgs._IconColor;
	HighlightColor = InArgs._HighlightColor;
	TextColor = InArgs._TextColor;
	Icon = InArgs._Icon;
	Text = InArgs._Text;

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
			.WidthOverride(Theme.RoundButtonWidth)
			.HeightOverride(Theme.RoundButtonHeight)
			[
				// Button (behaviour only, no display)
				SNew(SButton)
				.OnClicked(this, &SFlareRoundButton::OnButtonClicked)
				.ContentPadding(FMargin(0))
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				[
					// Background
					SNew(SBorder)
					.Padding(Theme.RoundButtonPadding)
					.BorderImage(&Theme.RoundButtonBackground)
					.BorderBackgroundColor(HighlightColor)
					[
						// Icon
						SNew(SImage)
						.Image(Icon)
						.ColorAndOpacity(IconColor)
					]
				]
			]
		]

		// Button text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.RoundButtonTextPadding)
		[
			SAssignNew(TextBlock, STextBlock)
			.Text(Text)
			.TextStyle(&Theme.SmallFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(TextColor)
		]
	];

	// Text management
	if (!InArgs._ShowText)
	{
		TextBlock->SetVisibility(EVisibility::Collapsed);
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareRoundButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() && IsClickable ? &Theme.ButtonActiveBackground : &Theme.ButtonBackground);
}

FReply SFlareRoundButton::OnButtonClicked()
{
	if (IsClickable && OnClicked.IsBound())
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}
