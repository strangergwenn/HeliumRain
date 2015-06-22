
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
	InvertedBackground = InArgs._InvertedBackground;
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
					// Inverted background
					SNew(SBorder)
					.BorderImage(&Theme.RoundButtonInvertedBackground)
					.BorderBackgroundColor(this, &SFlareRoundButton::GetInvertedBackgroundColor)
					[
						// Background
						SNew(SBorder)
						.Padding(Theme.RoundButtonPadding)
						.BorderImage(this, &SFlareRoundButton::GetBackgroundBrush)
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
		]

		// Button text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.RoundButtonTextPadding)
		[
			SAssignNew(TextBlock, STextBlock)
			.Text(Text)
			.WrapTextAt(Theme.RoundButtonWidth)
			.TextStyle(&Theme.SmallFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(TextColor)
			.ShadowColorAndOpacity(this, &SFlareRoundButton::GetShadowColor)
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

	if (InvertedBackground)
	{
		return (IsHovered() && IsClickable ? &Theme.RoundButtonInvertedActiveCircle : &Theme.RoundButtonInvertedCircle);
	}
	else
	{
		return (IsHovered() && IsClickable ? &Theme.RoundButtonActiveCircle : &Theme.RoundButtonCircle);
	}
}

FSlateColor SFlareRoundButton::GetInvertedBackgroundColor() const
{
	FLinearColor Color = FLinearColor::White;

	if (InvertedBackground)
	{
		FLinearColor AlphaColor = HighlightColor.Get().GetSpecifiedColor();
		Color.A = AlphaColor.A;
	}
	else
	{
		Color.A = 0;
	}

	return FSlateColor(Color);
}

FLinearColor SFlareRoundButton::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.InvertedColor;
	FLinearColor AlphaColor = TextColor.Get().GetSpecifiedColor();
	Color.A = AlphaColor.A;
	return Color;
}

FReply SFlareRoundButton::OnButtonClicked()
{
	if (IsClickable && OnClicked.IsBound())
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}
