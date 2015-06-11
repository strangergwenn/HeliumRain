#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../Widgets/FlareSubsystemStatus.h"


class SFlareHUDMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareHUDMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the ship to display data for */
	void SetTargetShip(IFlareSpacecraftInterface* Target);


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the color and alpha of the overheating warning */
	FSlateColor GetOverheatColor(bool Text) const;

	/** Get the color multiplier of the background behind overheat text */
	FSlateColor GetOverheatBackgroundColor() const;

	/** Get the color and alpha of the Outage warning */
	FSlateColor GetOutageColor(bool Text) const;

	/** Get the color multiplier of the background behind overheat text */
	FSlateColor GetOutageBackgroundColor() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>      OwnerHUD;

	// Menu components
	TSharedPtr<SFlareSubsystemStatus>    TemperatureStatus;
	TSharedPtr<SFlareSubsystemStatus>    PowerStatus;
	TSharedPtr<SFlareSubsystemStatus>    PropulsionStatus;
	TSharedPtr<SFlareSubsystemStatus>    RCSStatus;
	TSharedPtr<SFlareSubsystemStatus>    LifeSupportStatus;
	TSharedPtr<SFlareSubsystemStatus>    WeaponStatus;
	TSharedPtr<SHorizontalBox>           WeaponContainer;

	// Target data
	IFlareSpacecraftInterface*           TargetShip;
	bool                                 Overheating;
	bool                                 Burning;
	bool                                 PowerOutage;

	// Effect data
	float                                PresentationFlashTime;
	float                                TimeSinceOverheatChanged;
	float                                TimeSinceOutageChanged;

};
