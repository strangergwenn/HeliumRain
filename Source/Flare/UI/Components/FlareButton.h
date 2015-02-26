#pragma once

#include "../../Flare.h"
#include "../Style/FlareButtonWidgetStyle.h"
#include "../Style/FlareContainerWidgetStyle.h"


DECLARE_DELEGATE(FFlareButtonClicked)


class SFlareButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareButton)
	: _Toggle(false)
	, _TextStyle(&FFlareStyleSet::Get().GetWidgetStyle<FTextBlockStyle>("Flare.Text"))
	, _ContainerStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle"))
	, _ButtonStyle(&FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/DefaultButton"))
	, _BackgroundImage(NULL)
	, _DecoratorImage(NULL)
	{}

	SLATE_ARGUMENT(bool, Toggle)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)

	SLATE_ARGUMENT(FText, Text)
	SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

	SLATE_STYLE_ARGUMENT(FFlareContainerStyle, ContainerStyle)
	SLATE_STYLE_ARGUMENT(FFlareButtonStyle, ButtonStyle)
	SLATE_ARGUMENT(const FSlateBrush*, BackgroundImage)
	SLATE_ARGUMENT(const FSlateBrush*, DecoratorImage)
	
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

	bool IsToggle;
	bool IsPressed;

	FFlareButtonClicked OnClicked;

	const FSlateBrush* DecoratorImage;
	const FSlateBrush* BackgroundImage;

	const FFlareContainerStyle* ContainerStyle;
	const FFlareButtonStyle* ButtonStyle;

	UPROPERTY()
	TSharedPtr<SBorder> InnerContainer;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	TSharedPtr<SBorder> GetContainer() const
	{
		return InnerContainer;
	}


};
