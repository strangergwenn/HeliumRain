#pragma once

#include "../../Flare.h"
#include "FlareButton.h"


class SFlareRoundButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareRoundButton)
		: _IconColor(FLinearColor::White)
		, _HighlightColor(FLinearColor::White)
		, _TextColor(FLinearColor::White)
		, _Clickable(true)
		, _ShowText(true)
		, _InvertedBackground(false)
	{}

	SLATE_ATTRIBUTE(FText, Text)
	SLATE_ATTRIBUTE(FText, HelpText)
	SLATE_ATTRIBUTE(const FSlateBrush*, Icon);
	SLATE_ATTRIBUTE(FSlateColor, IconColor)
	SLATE_ATTRIBUTE(FSlateColor, HighlightColor)
	SLATE_ATTRIBUTE(FSlateColor, TextColor)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)

	SLATE_ARGUMENT(bool, Clickable)
	SLATE_ARGUMENT(bool, ShowText)
	SLATE_ARGUMENT(bool, InvertedBackground)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

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
	TAttribute<const FSlateBrush*> Icon;
	TAttribute<FSlateColor>        IconColor;
	TAttribute<FSlateColor>        HighlightColor;
	TAttribute<FSlateColor>        TextColor;
	TAttribute<FText>              HelpText;
	TAttribute<FText>              Text;

	// Slate data
	TSharedPtr<STextBlock>         TextBlock;


};
