#pragma once

#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../Widgets/FlareSubsystemStatus.h"


class SFlareMouseMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareMouseMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareHUD>, OwnerHUD)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Open the menu */
	void Open();

	/** Hide the menu */
	void Close();

	/** Add a widget */
	void AddWidget();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get a widget's position on the screen */
	FVector2D GetWidgetPosition(int32 Index) const;

	/** Get a widget's size on the screen */
	FVector2D GetWidgetSize(int32 Index) const;

	/** Get a widget's color multiplier */
	FSlateColor GetWidgetColor(int32 Index) const;


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Get the widget's direction */
	FVector2D GetDirection(int32 Index) const;

	/** Get colinearity */
	float GetColinearity(int32 Index) const;

	/** Check if a widget has been selected */
	bool HasSelection() const;

	/** Get the slected index */
	int32 GetSelectedIndex() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareHUD>      OwnerHUD;

	/** Player reference */
	UPROPERTY()
	AFlarePlayerController*              PC;

	// HUD content
	TSharedPtr<SFlareSubsystemStatus>    SectorStatus;
	TSharedPtr<SCanvas>                  HUDCanvas;

	// HUD settings
	int32                                WidgetDistance;
	int32                                WidgetSize;
	float                                AnimTime;

	// HUD data
	FVector2D                            ViewportCenter;
	FVector2D                            InitialMousePosition;
	FVector2D                            MouseOffset;
	int32                                WidgetCount;
	float                                CurrentTime;
	float                                IsOpen;


};
