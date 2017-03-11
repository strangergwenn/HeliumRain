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
		OutBrushes.Add(&NearInvisibleBrush);
		OutBrushes.Add(&SeparatorBrush);
		OutBrushes.Add(&OddBrush);
		OutBrushes.Add(&EvenBrush);

		OutBrushes.Add(&ButtonBackground);
		OutBrushes.Add(&ButtonActiveBackground);
		OutBrushes.Add(&ButtonDecorator);
		OutBrushes.Add(&ButtonActiveDecorator);
		OutBrushes.Add(&ButtonBorderBrush);

		OutBrushes.Add(&RoundButtonCircle);
		OutBrushes.Add(&RoundButtonActiveCircle);
		OutBrushes.Add(&RoundButtonInvertedCircle);
		OutBrushes.Add(&RoundButtonInvertedActiveCircle);
		OutBrushes.Add(&RoundButtonInvertedBackground);

		OutBrushes.Add(&SectorButtonBorder);
		OutBrushes.Add(&SectorButtonBackground);
		OutBrushes.Add(&SectorButtonActiveBackground);

		OutBrushes.Add(&ResourceBackground);

		OutBrushes.Add(&ListBackground);
		OutBrushes.Add(&ListActiveBackground);
	}


	/*----------------------------------------------------
		Style data
	----------------------------------------------------*/

	// Main
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush BackgroundBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush InvertedBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush InvisibleBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush NearInvisibleBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush SeparatorBrush;
	UPROPERTY(EditAnywhere, Category = Main) FMargin TitlePadding;
	UPROPERTY(EditAnywhere, Category = Main) FMargin TitleButtonPadding;
	UPROPERTY(EditAnywhere, Category = Main) FMargin ContentPadding;
	UPROPERTY(EditAnywhere, Category = Main) FMargin SmallContentPadding;
	UPROPERTY(EditAnywhere, Category = Main) float DefaultAlpha;
	UPROPERTY(EditAnywhere, Category = Main) int32 ContentWidth;
	UPROPERTY(EditAnywhere, Category = Main) FScrollBoxStyle ScrollBoxStyle;
	UPROPERTY(EditAnywhere, Category = Main) FScrollBarStyle ScrollBarStyle;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush OddBrush;
	UPROPERTY(EditAnywhere, Category = Main) FSlateBrush EvenBrush;

	// Colors
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor EnemyColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor DamageColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor MidDamageColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor FriendlyColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor DisputedColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor NeutralColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor UnknownColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor ObjectiveColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor TradingColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor QuestColor;
	UPROPERTY(EditAnywhere, Category = Colors) FLinearColor InfoColor;

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
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonDisabledBackground;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonActiveBackground;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonDecorator;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonDisabledDecorator;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonActiveDecorator;
	UPROPERTY(EditAnywhere, Category = Buttons) FSlateBrush ButtonBorderBrush;
	UPROPERTY(EditAnywhere, Category = Buttons) FMargin ButtonPadding;
	UPROPERTY(EditAnywhere, Category = Buttons) int32 ButtonWidth;
	UPROPERTY(EditAnywhere, Category = Buttons) int32 ButtonHeight;

	// Large button style
	UPROPERTY(EditAnywhere, Category = RoundButtons) FSlateBrush RoundButtonCircle;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FSlateBrush RoundButtonActiveCircle;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FSlateBrush RoundButtonInvertedCircle;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FSlateBrush RoundButtonInvertedActiveCircle;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FSlateBrush RoundButtonInvertedBackground;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FMargin RoundButtonPadding;
	UPROPERTY(EditAnywhere, Category = RoundButtons) FMargin RoundButtonTextPadding;
	UPROPERTY(EditAnywhere, Category = RoundButtons) int32 RoundButtonWidth;
	UPROPERTY(EditAnywhere, Category = RoundButtons) int32 RoundButtonHeight;

	// Sector button style
	UPROPERTY(EditAnywhere, Category = SectorButtons) FSlateBrush SectorButtonBorder;
	UPROPERTY(EditAnywhere, Category = SectorButtons) FSlateBrush SectorButtonBackground;
	UPROPERTY(EditAnywhere, Category = SectorButtons) FSlateBrush SectorButtonActiveBackground;
	UPROPERTY(EditAnywhere, Category = SectorButtons) FMargin SectorButtonPadding;
	UPROPERTY(EditAnywhere, Category = SectorButtons) FMargin SectorButtonTextPadding;
	UPROPERTY(EditAnywhere, Category = SectorButtons) int32 SectorButtonWidth;
	UPROPERTY(EditAnywhere, Category = SectorButtons) int32 SectorButtonHeight;

	// Resources
	UPROPERTY(EditAnywhere, Category = Resources) FSlateBrush ResourceBackground;
	UPROPERTY(EditAnywhere, Category = Resources) int32 ResourceWidth;
	UPROPERTY(EditAnywhere, Category = Resources) int32 ResourceHeight;

	// Lists
	UPROPERTY(EditAnywhere, Category = Lists) FSlateBrush ListBackground;
	UPROPERTY(EditAnywhere, Category = Lists) FSlateBrush ListActiveBackground;
	
	// Advanced widgets
	UPROPERTY(EditAnywhere, Category = Advanced) FProgressBarStyle ProgressBarStyle;
	UPROPERTY(EditAnywhere, Category = Advanced) FEditableTextStyle TextInputStyle;
	UPROPERTY(EditAnywhere, Category = Advanced) FComboBoxStyle ComboBoxStyle;
	UPROPERTY(EditAnywhere, Category = Advanced) FSliderStyle SliderStyle;

};


/*----------------------------------------------------
	Wrapper class
----------------------------------------------------*/

UCLASS()
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
