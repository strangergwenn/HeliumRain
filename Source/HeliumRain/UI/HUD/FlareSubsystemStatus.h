#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/Subsystems/FlareSimulatedSpacecraftDamageSystem.h"
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
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
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
		
	/** Get the subsystem's name */
	FText GetText() const;

	/** Get the subsystem's tooltip text */
	FText GetInfoText() const;

	/** Get the current health color */
	FSlateColor GetHealthColor() const;

	/** Get the current flash color */
	FSlateColor GetFlashColor() const;
	

protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/
	
	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager> MenuManager;

	// Indicator data
	TEnumAsByte<EFlareSubsystem::Type>      SubsystemType;

	// Health management
	float                                   Health;
	float                                   ComponentHealth;
	float                                   TimeSinceFlash;
	float                                   HealthDropFlashTime;


};
