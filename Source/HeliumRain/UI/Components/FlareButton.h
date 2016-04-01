#pragma once

#include "../../Flare.h"


DECLARE_DELEGATE(FFlareButtonClicked)


class SFlareButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareButton)
		: _Color(FLinearColor::White)
		, _Toggle(false)
		, _Transparent(false)
		, _Width(5)
		, _Height(1)
	{}

	SLATE_ATTRIBUTE(FText, Text)
	SLATE_ATTRIBUTE(FText, HelpText)
	SLATE_ATTRIBUTE(const FSlateBrush*, Icon)
	SLATE_ATTRIBUTE(FSlateColor, Color)
	SLATE_ATTRIBUTE(bool, IsDisabled)

	SLATE_EVENT(FFlareButtonClicked, OnClicked)

	SLATE_ARGUMENT(bool, Toggle)
	SLATE_ARGUMENT(bool, Transparent)
	SLATE_ARGUMENT(int32, Width)
	SLATE_ARGUMENT(int32, Height)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the toggle state */
	void SetActive(bool State);

	/** Get the toggle state */
	bool IsActive() const;

	/** Set the disabled state */
	void SetDisabled(bool State);
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Brush callback */
	const FSlateBrush* GetDecoratorBrush() const;

	/** Brush callback*/
	const FSlateBrush* GetIconBrush() const;

	/** Brush callback*/
	const FSlateBrush* GetBackgroundBrush() const;

	/** Color callback */
	FSlateColor GetMainColor() const;

	/** Get the text style */
	FSlateFontInfo GetTextStyle() const;

	/** Mouse click	*/
	FReply OnButtonClicked();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/
	
	// State data
	bool                           IsToggle;
	bool                           IsPressed;
	bool                           IsTransparent;
	int32                          Width;
	int32                          Height;
	
	// Slate data
	FFlareButtonClicked            OnClicked;
	TSharedPtr<SBorder>            InnerContainer;

	// Attributes
	TAttribute<const FSlateBrush*> Icon;
	TAttribute<FSlateColor>        Color;
	TAttribute<FText>              Text;
	TAttribute<FText>              HelpText;
	TAttribute<bool>               IsDisabled;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	TSharedPtr<SBorder> GetContainer() const
	{
		return InnerContainer;
	}


};
