
#include "../../Flare.h"
#include "FlareStyleSet.h"


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
