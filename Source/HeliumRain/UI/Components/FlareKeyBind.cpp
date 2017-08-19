#pragma once

#include "FlareKeyBind.h"
#include "../../Flare.h"
#include "../Style/FlareStyleSet.h"


#define LOCTEXT_NAMESPACE "FlareKeyBind"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareKeyBind::Construct(const FArguments& InArgs)
{
	// Setup
	bWaitingForKey = false;
	DefaultKey = InArgs._DefaultKey;
	OnKeyBindingChanged = InArgs._OnKeyBindingChanged;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Parent constructor
	SButton::Construct(SButton::FArguments()
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.ContentPadding(FMargin(0))
		.DesiredSizeScale(FVector2D(1, 1))
		.ContentScale(FVector2D(1, 1))
		.ButtonColorAndOpacity(FLinearColor::White)
		.ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
	);

	// Layout
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(&Theme.ButtonBackground)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(KeyText, STextBlock)
			.TextStyle(&Theme.SmallFont)
		]
	];

	// Key data
	if (InArgs._Key.IsValid())
	{
		Key = InArgs._Key;
		KeyText->SetText(*Key == FKey() ? FString() : Key->ToString());
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareKeyBind::SetKey(FKey NewKey, bool bCanReset, bool bNotify)
{
	if (Key.IsValid())
	{
		FKey CurrentKey = *Key;
		if (NewKey == *Key && bCanReset)
		{
			NewKey = FKey();
		}

		*Key = NewKey;
		KeyText->SetText(NewKey == FKey() ? FString() : NewKey.ToString());
		bWaitingForKey = false;

		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(true);
		FSlateApplication::Get().ClearKeyboardFocus(EKeyboardFocusCause::SetDirectly);

		if (bNotify)
		{
			OnKeyBindingChanged.ExecuteIfBound( CurrentKey , NewKey, this);
		}
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FReply SFlareKeyBind::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (bWaitingForKey && !InKeyEvent.GetKey().IsGamepadKey())
	{
		SetKey(InKeyEvent.GetKey());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFlareKeyBind::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitingForKey && MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		SetKey(MouseEvent.GetEffectingButton());
		return FReply::Handled();
	}

	else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Get the center of the widget so we can lock our mouse there
		FSlateRect Rect = MyGeometry.GetClippingRect();
		WaitingMousePos.X = (Rect.Left + Rect.Right) * 0.5f;
		WaitingMousePos.Y = (Rect.Top + Rect.Bottom) * 0.5f;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);

		KeyText->SetText(LOCTEXT("SFlareKeyBindPressAnyKey", "Press a key..."));
		bWaitingForKey = true;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(false);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFlareKeyBind::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitingForKey)
	{
		SetKey(MouseEvent.GetWheelDelta() > 0 ? EKeys::MouseScrollUp : EKeys::MouseScrollDown);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

int32 SFlareKeyBind::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Make sure the mouse pointer doesn't leave the button
	if (bWaitingForKey)
	{
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);
	}

	return SButton::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SFlareKeyBind::ComputeDesiredSize(float) const
{
	return FVector2D(200.0f, 46.0f);
}


#undef LOCTEXT_NAMESPACE

