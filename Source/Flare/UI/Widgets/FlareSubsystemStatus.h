#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Spacecrafts/FlareSpacecraftComponent.h"


class SFlareSubsystemStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSubsystemStatus)
		: _Subsystem(EFlareSubsystem::SYS_None)
	{}

	SLATE_ARGUMENT(EFlareSubsystem::Type, Subsystem)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the ship to display data for */
	void SetTargetShip(IFlareSpacecraftInterface* Target);

	/** Set an optional component to show data for */
	void SetTargetComponent(UFlareSpacecraftComponent* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	/** Get the current icon */
	const FSlateBrush* GetIcon() const;
	
	/** Get the current circling color */
	FSlateColor GetHighlightColor() const;

	/** Get the current icon color */
	FSlateColor GetIconColor() const;

	/** Get the current status string */
	FText GetStatusText() const;

	/** Get the subsystem type string */
	FText GetTypeText() const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Indicator data
	TEnumAsByte<EFlareSubsystem::Type>      SubsystemType;
	IFlareSpacecraftInterface*              TargetShip;
	UFlareSpacecraftComponent*              TargetComponent;

	// Health management
	float                                   Health;
	float                                   ComponentHealth;
	float                                   TimeSinceFlash;
	float                                   HealthDropFlashTime;


};
