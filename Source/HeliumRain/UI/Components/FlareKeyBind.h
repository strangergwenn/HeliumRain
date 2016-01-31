#pragma once

#include "../../Flare.h"
#include "SlateBasics.h"


DECLARE_DELEGATE_TwoParams(FOnKeyBindingChanged, FKey,FKey);


class HELIUMRAIN_API SFlareKeyBind : public SButton
{
public:

	SLATE_BEGIN_ARGS(SFlareKeyBind)
		: _Key(nullptr)
	{}

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


