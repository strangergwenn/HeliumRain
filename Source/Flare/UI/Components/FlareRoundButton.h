#pragma once

#include "../../Flare.h"
#include "FlareButton.h"


class SFlareRoundButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareRoundButton)
		: _Clickable(true)
		, _ShowText(true)
		, _InvertedBackground(false)
		, _IconColor(FLinearColor::White)
		, _HighlightColor(FLinearColor::White)
		, _TextColor(FLinearColor::White)
	{}

	SLATE_ARGUMENT(bool, Clickable)
	SLATE_ARGUMENT(bool, ShowText)
	SLATE_ARGUMENT(bool, InvertedBackground)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)

	SLATE_ATTRIBUTE(FText, Text)
	SLATE_ATTRIBUTE(const FSlateBrush*, Icon);
	SLATE_ATTRIBUTE(FSlateColor, IconColor)
	SLATE_ATTRIBUTE(FSlateColor, HighlightColor)
	SLATE_ATTRIBUTE(FSlateColor, TextColor)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
		
	/** Brush callback */
	const FSlateBrush* GetBackgroundBrush() const;
	
	/** Get the color to use for the inverted background */
	FSlateColor GetInvertedBackgroundColor() const;

	/** Get the shadow color */
	FLinearColor GetShadowColor() const;

	/** Mouse clicked */
	FReply OnButtonClicked();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Data
	bool                           IsClickable;
	bool                           InvertedBackground;
	FFlareButtonClicked            OnClicked;

	// Attributes
	TAttribute<FText>              Text;
	TAttribute<const FSlateBrush*> Icon;
	TAttribute<FSlateColor>        IconColor;
	TAttribute<FSlateColor>        HighlightColor;
	TAttribute<FSlateColor>        TextColor;

	// SLate data
	TSharedPtr<STextBlock>         TextBlock;


};
