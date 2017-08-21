
#include "FlareStyleSet.h"
#include "../../Flare.h"


/*----------------------------------------------------
	Static data
----------------------------------------------------*/

#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "Slate/Fonts" / RelativePath + TEXT(".ttf"), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FFlareStyleSet::Instance = NULL;


/*----------------------------------------------------
	Initialization and removal
----------------------------------------------------*/

void FFlareStyleSet::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FFlareStyleSet::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	FCHECK(Instance.IsUnique());
	Instance.Reset();
}


/*----------------------------------------------------
	GUI code
----------------------------------------------------*/

TSharedRef< FSlateStyleSet > FFlareStyleSet::Create()
{
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FName("FlareStyle"), "/Game/Slate", "/Game/Slate");
	FSlateStyleSet& Style = StyleRef.Get();

	Style.Set("Flare.TableRow", FTableRowStyle()
		.SetEvenRowBackgroundBrush(FSlateNoResource())
		.SetEvenRowBackgroundHoveredBrush(FSlateNoResource())
		.SetOddRowBackgroundBrush(FSlateNoResource())
		.SetOddRowBackgroundHoveredBrush(FSlateNoResource())
		.SetSelectorFocusedBrush(FSlateNoResource())
		.SetActiveBrush(FSlateNoResource())
		.SetActiveHoveredBrush(FSlateNoResource())
		.SetInactiveBrush(FSlateNoResource())
		.SetInactiveHoveredBrush(FSlateNoResource())
	);

	Style.Set("Flare.CoreButton", FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetDisabled(FSlateNoResource())
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
	);

	Style.Set("Flare.ActiveCoreButton", FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetDisabled(FSlateNoResource())
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f))
	);

	const FFlareStyleCatalog& DefaultTheme = Style.GetWidgetStyle<FFlareStyleCatalog>("/Style/DefaultTheme");
	Style.Set("WarningText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.EnemyColor)
	);
	Style.Set("TradeText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.TradingColor)
	);
	Style.Set("HighlightText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.FriendlyColor)
	);

	return StyleRef;
}

FLinearColor FFlareStyleSet::GetHealthColor(float Health, bool WithAlpha)
{
	const FFlareStyleCatalog& Theme = GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor MidColor = Theme.MidDamageColor;
	FLinearColor DamageColor = Theme.DamageColor;
	FLinearColor Color;

	// Interpolate the damage color
	if (Health > 0.5)
	{
		float HealthHigh = 2 * (Health - 0.5);
		Color = FMath::Lerp(MidColor, NormalColor, HealthHigh);
	}
	else
	{
		float HealthLow = 2 * Health;
		Color = FMath::Lerp(DamageColor, MidColor, HealthLow);
	}

	// Add alpha if asked for
	if (WithAlpha)
	{
		Color.A *= Theme.DefaultAlpha;
	}
	return Color;
}
