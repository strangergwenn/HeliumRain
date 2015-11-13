#pragma once

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
class FLARE_API UFlareCustomizationCatalog : public UDataAsset
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

	FLinearColor GetColor(int32 Index) const;

	UTexture2D* GetPattern(int32 Index) const;

	const FSlateBrush* GetPatternBrush(int32 Index) const;


};
