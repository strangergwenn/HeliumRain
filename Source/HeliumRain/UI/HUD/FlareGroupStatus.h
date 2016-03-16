#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Spacecrafts/FlareSpacecraftComponent.h"


class AFlarePlayerController;


class SFlareGroupStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareGroupStatus)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, PC)
	SLATE_ARGUMENT(EFlareCombatGroup::Type, TargetShipGroup)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
		
	/** Get the Group's name */
	FText GetText() const;

	/** Get the circle's color */
	FSlateColor GetHighlightColor() const;

	/** Get the current icon color */
	FSlateColor GetIconColor() const;

	/** Get the current text color */
	FSlateColor GetTextColor() const;

	/** Get the current text shadow color */
	FLinearColor GetShadowColor() const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// State data
	float                                   GroupHealth;
	float                                   CurrentAlpha;
	float                                   FadeInTime;
	float                                   FadeOutTime;

	// Target data
	AFlarePlayerController*                 PC;
	TEnumAsByte<EFlareCombatGroup::Type>    TargetShipGroup;


};
