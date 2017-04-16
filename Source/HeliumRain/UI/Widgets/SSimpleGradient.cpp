// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Widgets/Colors/SSimpleGradient.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Application/SlateWindowHelper.h"


/* SSimpleGradient interface
 *****************************************************************************/

void SSimpleGradient::Construct( const FArguments& InArgs )
{
	StartColor = InArgs._StartColor;
	EndColor = InArgs._EndColor;
	bHasAlphaBackground = InArgs._HasAlphaBackground.Get();
	Orientation = InArgs._Orientation.Get();
}


/* SCompoundWidget overrides
 *****************************************************************************/

int32 SSimpleGradient::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	const ESlateDrawEffect::Type DrawEffects = this->ShouldBeEnabled(bParentEnabled) ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	if (bHasAlphaBackground)
	{
		const FSlateBrush* StyleInfo = FCoreStyle::Get().GetBrush("ColorPicker.AlphaBackground");

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			StyleInfo,
			MyClippingRect,
			DrawEffects
		);
	}

	TArray<FSlateGradientStop> GradientStops;

	GradientStops.Add(FSlateGradientStop(FVector2D::ZeroVector, StartColor.Get()));
	GradientStops.Add(FSlateGradientStop(AllottedGeometry.Size, EndColor.Get()));

	FSlateDrawElement::MakeGradient(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		GradientStops,
		Orientation,
		MyClippingRect,
		DrawEffects | ESlateDrawEffect::NoGamma
	);

	return LayerId + 1;
}
