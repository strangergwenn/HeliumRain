
#include "../Flare.h"
#include "FlareCustomizationCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCustomizationCatalog::UFlareCustomizationCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Get & Set
----------------------------------------------------*/

FLinearColor UFlareCustomizationCatalog::GetColor(int32 Index) const
{
	FLinearColor Result = FLinearColor();

	if (Index >= 0 && Index < PaintColors.Num())
	{
		Result = PaintColors[Index];
	}

	return Result;
}

UTexture2D* UFlareCustomizationCatalog::GetEmblem(int32 Index) const
{
	if (Index >= 0 && Index < PlayerEmblems.Num())
	{
		return PlayerEmblems[Index].Texture;
	}
	return NULL;
}

const FSlateBrush* UFlareCustomizationCatalog::GetEmblemBrush(int32 Index) const
{
	if (Index >= 0 && Index < PlayerEmblems.Num())
	{
		return &PlayerEmblems[Index].PatternPreviewBrush;
	}
	return NULL;
}

UTexture2D* UFlareCustomizationCatalog::GetPattern(int32 Index) const
{
	if (Index >= 0 && Index < PaintPatterns.Num())
	{
		return PaintPatterns[Index].Texture;
	}
	return NULL;
}

const FSlateBrush* UFlareCustomizationCatalog::GetPatternBrush(int32 Index) const
{
	if (Index >= 0 && Index < PaintPatterns.Num())
	{
		return &PaintPatterns[Index].PatternPreviewBrush;
	}
	return NULL;
}