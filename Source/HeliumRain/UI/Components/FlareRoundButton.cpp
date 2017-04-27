
#include "FlareRoundButton.h"
#include "../../Flare.h"
#include "../../Player/FlareMenuManager.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareRoundButton::Construct(const FArguments& InArgs)
{
	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Arguments
	IsClickable = InArgs._Clickable;
	OnClicked = InArgs._OnClicked;
	InvertedBackground = InArgs._InvertedBackground;
	IconColor = InArgs._IconColor;
	HighlightColor = InArgs._HighlightColor;
	TextColor = InArgs._TextColor;
	HelpText = InArgs._HelpText;
	Icon = InArgs._Icon;
	Text = InArgs._Text;

	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SVerticalBox)

		// Container
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
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
					.Padding(FMargin(0))
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
			.WrapTextAt(Theme.RoundButtonWidth + Theme.RoundButtonPadding.Left + Theme.RoundButtonPadding.Right - Theme.RoundButtonTextPadding.Left - Theme.RoundButtonTextPadding.Right)
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

void SFlareRoundButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->ShowTooltip(this, Text.Get(), HelpText.Get());
	}
}

void SFlareRoundButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

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
	FLinearColor Color = Theme.SmallFont.ShadowColorAndOpacity;
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
