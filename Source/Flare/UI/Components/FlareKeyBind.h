#pragma once

#include "../../Flare.h"
#include "SlateBasics.h"


DECLARE_DELEGATE_TwoParams(FOnKeyBindingChanged, FKey,FKey);


class FLARE_API SFlareKeyBind : public SButton
{
public:

	SLATE_BEGIN_ARGS(SFlareKeyBind)
		: _TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText"))
		, _HAlign(HAlign_Center)
		, _VAlign(VAlign_Center)
		, _ContentPadding(FMargin(4.0, 2.0))
		, _DesiredSizeScale(FVector2D(1, 1))
		, _ContentScale(FVector2D(1, 1))
		, _ButtonColorAndOpacity(FLinearColor::White)
		, _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
		, _Key(nullptr)
	{}

	SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)
	SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
	SLATE_ARGUMENT(EVerticalAlignment, VAlign)
	SLATE_ATTRIBUTE(FMargin, ContentPadding)
	SLATE_ATTRIBUTE(FVector2D, DesiredSizeScale)
	SLATE_ATTRIBUTE(FVector2D, ContentScale)
	SLATE_ATTRIBUTE(FSlateColor, ButtonColorAndOpacity)
	SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)
	SLATE_ARGUMENT(TSharedPtr<FKey>, Key)
	SLATE_ARGUMENT(FKey, DefaultKey)
	SLATE_EVENT(FOnKeyBindingChanged, OnKeyBindingChanged)
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the key that we will bind or rebind */
	virtual void SetKey(FKey NewKey, bool bCanReset = true, bool bNotify = true);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;


private:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	FOnKeyBindingChanged                         OnKeyBindingChanged;
	TSharedPtr<FKey>                             Key;
	FKey                                         DefaultKey;
	TSharedPtr<STextBlock>                       KeyText;
	FVector2D                                    WaitingMousePos;
	bool                                         bWaitingForKey;

};


