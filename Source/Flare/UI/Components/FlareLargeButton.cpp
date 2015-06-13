
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
	TextColor = InArgs._TextColor;
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
		.Padding(Theme.LargeButtonTextPadding)
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
