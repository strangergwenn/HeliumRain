#pragma once

#include "../Flare.h"
#include "Engine/DataAsset.h"
#include "FlareCustomizationCatalog.generated.h"


USTRUCT()
struct FFlareCustomizationPatternDescription
{
	GENERATED_USTRUCT_BODY()

	/** Pattern name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Pattern texture */
	UPROPERTY(EditAnywhere, Category = Content) UTexture2D* Texture;

	/** Pattern preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush PatternPreviewBrush;

};


UCLASS()
class HELIUMRAIN_API UFlareCustomizationCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Pattern storage */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareCustomizationPatternDescription> PaintPatterns;

	/** Color storage */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FLinearColor> PaintColors;

	/** Emblems storage */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareCustomizationPatternDescription> PlayerEmblems;


	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline int32 GetColorCount() const
	{
		return PaintColors.Num();
	}

	inline int32 GetPatternCount() const
	{
		return PaintPatterns.Num();
	}

	inline int32 GetEmblemCount() const
	{
		return PlayerEmblems.Num();
	}

	FLinearColor GetColorByIndex(int32 Index) const;

	int32 FindColor(FLinearColor Color) const;

	UTexture2D* GetEmblem(int32 Index) const;

	const FSlateBrush* GetEmblemBrush(int32 Index) const;

	UTexture2D* GetPattern(int32 Index) const;

	const FSlateBrush* GetPatternBrush(int32 Index) const;


};
