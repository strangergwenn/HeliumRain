#pragma once

#include "../../Flare.h"

class UFlareSimulatedSpacecraft;


class SFlareCargoInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareCargoInfo)
		: _Spacecraft(NULL)
	{}

	SLATE_ARGUMENT(UFlareSimulatedSpacecraft*, Spacecraft)
	SLATE_ARGUMENT(int32, CargoIndex)
	
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

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Get the resource icon */
	const FSlateBrush* GetResourceIcon() const;

	/** Get the resource small name */
	FText GetResourceAcronym() const;

	/** Get the quantity text */
	FText GetResourceQuantity() const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Spacecraft data
	UFlareSimulatedSpacecraft*                      TargetSpacecraft;
	int32                                           CargoIndex;

};
