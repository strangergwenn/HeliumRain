#pragma once

#include "../../Flare.h"
#include "../../Ships/FlareSpacecraftInterface.h"
#include "../../Ships/FlareShipComponent.h"
#include "FlareSubsystemStatus.generated.h"


/** Possible display targets for the subsystem display widgets */
UENUM()
namespace EFlareInfoDisplay
{
	enum Type
	{
		ID_Subsystem,
		ID_Spacer,
		ID_Speed,
		ID_Sector
	};
}


class SFlareSubsystemStatus : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSubsystemStatus)
		: _Type(EFlareInfoDisplay::ID_Subsystem)
		, _Subsystem(EFlareSubsystem::SYS_None)
	{}

	SLATE_ARGUMENT(EFlareInfoDisplay::Type, Type)
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
	void SetTargetComponent(UFlareShipComponent* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Is this icon visible ? */
	EVisibility IsIconVisible() const;

	/** Get the current icon */
	const FSlateBrush* GetIcon() const;

	/** Get the current icon color */
	FSlateColor GetIconColor() const;

	/** Get the current flash color */
	FSlateColor GetFlashColor() const;

	/** Get the current status string */
	FText GetStatusText() const;

	/** Get the subsystem type string */
	FText GetTypeText() const;


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Indicator data
	TEnumAsByte<EFlareInfoDisplay::Type>    DisplayType;
	TEnumAsByte<EFlareSubsystem::Type>      SubsystemType;
	IFlareSpacecraftInterface*                    TargetShip;
	UFlareShipComponent*                    TargetComponent;

	// Health management
	float                                   Health;
	float                                   ComponentHealth;
	float                                   TimeSinceFlash;
	float                                   HealthDropFlashTime;


};
