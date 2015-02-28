
#include "../../Flare.h"
#include "FlareStyleSet.h"
#include "FlareContainerWidgetStyle.h"


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
	ensure(Instance.IsUnique());
	Instance.Reset();
}


/*----------------------------------------------------
	GUI code
----------------------------------------------------*/

TSharedRef< FSlateStyleSet > FFlareStyleSet::Create()
{
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FName("FlareStyle"), "/Game/Slate", "/Game/Slate");
	FSlateStyleSet& Style = StyleRef.Get();

	FLinearColor WhiteColor(1.0, 1.0, 1.0, 0.7);
	FLinearColor BlackColor(0.0, 0.0, 0.0, 0.7);

	Style.Set("Flare.SmallText", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato500", 12))
		.SetColorAndOpacity(WhiteColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.SmallTextInverted", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato500", 12))
		.SetColorAndOpacity(BlackColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Text", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato500", 14))
		.SetColorAndOpacity(WhiteColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.TextInverted", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato500", 14))
		.SetColorAndOpacity(BlackColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title3", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato700", 16))
		.SetColorAndOpacity(WhiteColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title3Inverted", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato700", 16))
		.SetColorAndOpacity(BlackColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title2", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato300", 24))
		.SetColorAndOpacity(WhiteColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title2Inverted", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato300", 24))
		.SetColorAndOpacity(BlackColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title1", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato700", 42))
		.SetColorAndOpacity(WhiteColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

	Style.Set("Flare.Title1Inverted", FTextBlockStyle()
		.SetFont(TTF_FONT("Lato700", 42))
		.SetColorAndOpacity(BlackColor)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		);

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

	return StyleRef;
}
