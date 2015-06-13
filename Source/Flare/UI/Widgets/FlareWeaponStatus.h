#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Spacecrafts/FlareSpacecraftComponent.h"


class SFlareWeaponStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareWeaponStatus)
	{}

	SLATE_ARGUMENT(IFlareSpacecraftInterface*, TargetShip)
	SLATE_ARGUMENT(UFlareWeapon*, TargetWeapon)
	
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
		
	/** Get the Weapon's name */
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
	float                                   CurrentAlpha;
	float                                   FadeInTime;
	float                                   FadeOutTime;

	// Target data
	IFlareSpacecraftInterface*              TargetShip;
	UFlareWeapon*                           TargetWeapon;


};
