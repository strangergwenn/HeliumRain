
#include "FlareCustomizationCatalog.h"
#include "../Flare.h"
#include "../Game/FlareGameTools.h"

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

FLinearColor UFlareCustomizationCatalog::GetColorByIndex(int32 Index) const
{
	FLinearColor Result = FLinearColor();

	if (Index >= 0 && Index < PaintColors.Num())
	{
		Result = PaintColors[Index];
	}

	return Result;
}

int32 UFlareCustomizationCatalog::FindColor(FLinearColor Color) const
{
	float MinDistance = 0;
	int32 MinDistanceIndex = -1;
	FVector SearchVector = UFlareGameTools::ColorToVector(Color);

	int32 Index = 0;
	for(const FLinearColor& PaintColor : PaintColors)
	{
		FVector CandidateVector = UFlareGameTools::ColorToVector(PaintColor);

		float Distance = (CandidateVector-SearchVector).SizeSquared();
		if(MinDistanceIndex < 0 || Distance < MinDistance)
		{
			MinDistanceIndex =  Index;
			MinDistance = Distance;
		}

		Index++;
	}
	return MinDistanceIndex;
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
