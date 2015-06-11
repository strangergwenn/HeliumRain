#pragma once

#include "../../Flare.h"
#include "FlareWidgetStyleCatalog.generated.h"


USTRUCT()
struct FFlareStyleCatalog : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	/*----------------------------------------------------
	Class defaults
	----------------------------------------------------*/

	static const FName TypeName;
	const FName GetTypeName() const override
	{
		return TypeName;
	}

	static const FFlareStyleCatalog& GetDefault()
	{
		static FFlareStyleCatalog Default;
		return Default;
	}

	void GetResources(TArray<const FSlateBrush*>& OutBrushes) const
	{
		OutBrushes.Add(&BackgroundBrush);
		OutBrushes.Add(&InvertedBrush);
		OutBrushes.Add(&InvisibleBrush);
		OutBrushes.Add(&BorderBrush);

		OutBrushes.Add(&ButtonBackground);
		OutBrushes.Add(&ButtonActiveBackground);
		OutBrushes.Add(&ButtonDecorator);
		OutBrushes.Add(&ButtonActiveDecorator);

		OutBrushes.Add(&LargeButtonBackground);
		OutBrushes.Add(&LargeButtonActiveBackground);
	}


	/*----------------------------------------------------
		Style data
	----------------------------------------------------*/

	// Main
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush BackgroundBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush InvertedBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush InvisibleBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush BorderBrush;
	UPROPERTY(EditAnywhere, Category = Main) FMargin TitlePadding;
	UPROPERTY(EditAnywhere, Category = Main) FMargin ContentPadding;
	UPROPERTY(EditAnywhere, Category = Main) float DefaultAlpha;

	// Colors
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor EnemyColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor FriendlyColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor NeutralColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor InvertedColor;

	// Fonts
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle TitleFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle InvertedTitleFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle SubTitleFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle InvertedSubTitleFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle NameFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle InvertedNameFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle TextFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle InvertedTextFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle SmallFont;
	UPROPERTY(EditAnywhere, Category = Fonts) FTextBlockStyle InvertedSmallFont;
	
	// Default button style
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonBackground;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonActiveBackground;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonDecorator;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonActiveDecorator;
	UPROPERTY(EditAnywhere, Category = Buttons) FMargin ButtonPadding;
	UPROPERTY(EditAnywhere, Category = Buttons) FMargin ButtonBorderPadding;
	UPROPERTY(EditAnywhere, Category = Buttons) int32 ButtonWidth;
	UPROPERTY(EditAnywhere, Category = Buttons) int32 ButtonHeight;

	// Large button style
	UPROPERTY(EditAnywhere, Category = LargeButtons) FSlateBrush LargeButtonBackground;
	UPROPERTY(EditAnywhere, Category = LargeButtons) FSlateBrush LargeButtonActiveBackground;
	UPROPERTY(EditAnywhere, Category = LargeButtons) FMargin LargeButtonPadding;
	UPROPERTY(EditAnywhere, Category = LargeButtons) int32 LargeButtonWidth;
	UPROPERTY(EditAnywhere, Category = LargeButtons) int32 LargeButtonHeight;
	
	
};


/*----------------------------------------------------
	Wrapper class
----------------------------------------------------*/

UCLASS(hidecategories=Object, MinimalAPI)
class UFlareWidgetStyleCatalog : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&m_style);
	}

protected:

	UPROPERTY(Category = Appearance, EditAnywhere, meta = (ShowOnlyInnerProperties))
		FFlareStyleCatalog m_style;

};
