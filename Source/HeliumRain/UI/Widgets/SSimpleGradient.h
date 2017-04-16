// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class FPaintArgs;
class FSlateWindowElementList;

/**
 * Implements a Slate widget that renders a simple color gradient.
 */
class SSimpleGradient
	: public SCompoundWidget
{
public:

		SLATE_BEGIN_ARGS(SSimpleGradient)
			: _StartColor(FLinearColor(0.0f, 0.0f, 0.0f))
			, _EndColor(FLinearColor(1.0f, 1.0f, 1.0f))
			, _HasAlphaBackground(false)
			, _Orientation(Orient_Vertical)
			, _UseSRGB(false)
		{ }

		/** The leftmost gradient color */
		SLATE_ATTRIBUTE(FLinearColor, StartColor)
		
		/** The rightmost gradient color */
		SLATE_ATTRIBUTE(FLinearColor, EndColor)

		/** Whether a checker background is displayed for alpha viewing */
		SLATE_ATTRIBUTE(bool, HasAlphaBackground)

		/** Horizontal or vertical gradient */
		SLATE_ATTRIBUTE(EOrientation, Orientation)

		/** Whether to display sRGB color */
		SLATE_ATTRIBUTE(bool, UseSRGB)

	SLATE_END_ARGS()

	/**
	 * Constructs the widget
	 *
	 * @param InArgs The declaration data for this widget
	 */
	void Construct( const FArguments& InArgs );

protected:

	// SCompoundWidget overrides

	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;

private:

	/** The leftmost gradient color */
	TAttribute<FLinearColor> StartColor;

	/** The rightmost gradient color */
	TAttribute<FLinearColor> EndColor;

	/** Whether a checker background is displayed for alpha viewing */
	bool bHasAlphaBackground;
	
	/** Horizontal or vertical gradient */
	EOrientation Orientation;

	/** Whether to display sRGB color */
	TAttribute<bool> UseSRGB;
};
