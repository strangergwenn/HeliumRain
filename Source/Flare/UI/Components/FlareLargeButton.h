#pragma once

#include "../../Flare.h"
#include "FlareButton.h"


class SFlareLargeButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareLargeButton)
		: _Clickable(true)
		, _ShowText(true)
		, _IconColor(FLinearColor::White)
		, _HighlightColor(FLinearColor::White)
		, _TextColor(FLinearColor::White)
	{}

	SLATE_ARGUMENT(bool, Clickable)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)

	SLATE_ARGUMENT(bool, ShowText)
	SLATE_ARGUMENT(FText, Text)
	SLATE_ARGUMENT(const FSlateBrush*, Icon);
	
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
	
	/** Mouse clicked */
	FReply OnButtonClicked();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Data
	bool                           IsClickable;
	FFlareButtonClicked            OnClicked;
	TAttribute<FSlateColor>        IconColor;
	TAttribute<FSlateColor>        HighlightColor;
	TAttribute<FSlateColor>        TextColor;
	TSharedPtr<STextBlock>         Text;


};
