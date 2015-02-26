#pragma once

#include "../../Flare.h"
#include "../Style/FlareButtonWidgetStyle.h"
#include "../Style/FlareContainerWidgetStyle.h"


DECLARE_DELEGATE(FFlareHeaderButtonClicked)


class SFlareHeaderButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareHeaderButton)
	: _TextStyle(&FFlareStyleSet::Get().GetWidgetStyle<FTextBlockStyle>("Flare.Title3"))
	, _ContainerStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/HeaderContainerStyle"))
	, _ButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/HeaderButtonStyle"))
	{}

	SLATE_EVENT(FFlareHeaderButtonClicked, OnClicked)

	SLATE_ARGUMENT(FText, Text)
	SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

	SLATE_STYLE_ARGUMENT(FFlareContainerStyle, ContainerStyle)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ButtonStyle)
	
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
	
	/** Brush callback */
	const FSlateBrush* GetDecoratorBrush() const;

	/** Brush callback*/
	const FSlateBrush* GetBackgroundBrush() const;

	/** Mouse click	*/
	FReply OnButtonClicked();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	bool IsPressed;

	FFlareHeaderButtonClicked OnClicked;

	const FFlareContainerStyle* ContainerStyle;
	const FFlareButtonStyle* ButtonStyle;

	UPROPERTY()
	TSharedPtr<SBorder> InnerContainer;


};
