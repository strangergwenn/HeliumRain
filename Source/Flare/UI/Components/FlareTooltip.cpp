
#include "../../Flare.h"
#include "FlareTooltip.h"

#define LOCTEXT_NAMESPACE "FlareTooltip"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTooltip::Construct(const FArguments& InArgs)
{
	// Data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TooltipVisible = false;
	TooltipDelay = 1.0;
	TooltipFadeDuration = 0.5;
	TooltipCurrentTime = 0;
	TooltipCurrentAlpha = 0;

	// Create the layout
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	[
		SNew(SBox)
		[
			SNew(SBorder)
			.BorderImage(&Theme.BackgroundBrush)
			.BorderBackgroundColor(this, &SFlareTooltip::GetTooltipColor)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.Text(this, &SFlareTooltip::GetHelpText)
				.TextStyle(&Theme.NameFont)
				.ShadowColorAndOpacity(this, &SFlareTooltip::GetTooltipShadowColor)
				.ColorAndOpacity(this, &SFlareTooltip::GetTooltipColor)
			]
		]
	];

	SetVisibility(EVisibility::HitTestInvisible);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTooltip::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TooltipCurrentTime += (TooltipVisible ? InDeltaTime : -InDeltaTime);
	TooltipCurrentTime = FMath::Clamp(TooltipCurrentTime, 0.0f, 2 * TooltipDelay + TooltipFadeDuration);
	TooltipCurrentAlpha = FMath::Clamp((TooltipCurrentTime - TooltipDelay) / TooltipFadeDuration, 0.0f, 1.0f);
}

void SFlareTooltip::ShowTooltip(SWidget* TargetWidget, FText Content)
{
	if (TooltipTarget != TargetWidget)
	{
		TooltipTarget = TargetWidget;
		TooltipContent = Content;
		TooltipVisible = !TooltipContent.IsEmptyOrWhitespace();
	}
}

void SFlareTooltip::HideTooltip(SWidget* TargetWidget)
{
	if (TooltipTarget == TargetWidget)
	{
		TooltipVisible = false;
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FSlateColor SFlareTooltip::GetTooltipColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = TooltipCurrentAlpha;
	return Color;
}

FLinearColor SFlareTooltip::GetTooltipShadowColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = TooltipCurrentAlpha;
	return Color;
}

FText SFlareTooltip::GetHelpText() const
{
	return TooltipContent;
}


#undef LOCTEXT_NAMESPACE
