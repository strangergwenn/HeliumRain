#pragma once

#include "../../Flare.h"


class SFlareShipStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareShipStatus)
		: _Ship(NULL)
		, _Center(false)
	{}

	SLATE_ARGUMENT(UFlareSimulatedSpacecraft*, Ship)
	SLATE_ARGUMENT(bool, Center)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the ship to display data for */
	void SetTargetShip(UFlareSimulatedSpacecraft* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Get the current color */
	FSlateColor GetIconColor(EFlareSubsystem::Type Type) const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Ship data
	UFlareSimulatedSpacecraft*                    TargetShip;
	bool                                    CenterIcons;

	// Slate data
	TSharedPtr<SImage>                      WeaponIndicator;


};
