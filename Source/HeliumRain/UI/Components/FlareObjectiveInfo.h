#pragma once

#include "../../Flare.h"


class AFlarePlayerController;


class SFlareObjectiveInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareObjectiveInfo)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, PC)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	
	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Ensure that we don't display empty widgets */
	EVisibility GetVisibility() const;

	/** Get the current objective name */
	FText GetName() const;

	/** Get the current objective description text */
	FText GetDescription() const;

	FText GetInitialLabel(int32 ConditionIndex) const;

	FText GetTerminalLabel(int32 ConditionIndex) const;

	FText GetCounter(int32 ConditionIndex) const;

	EVisibility GetCounterVisibility(int32 ConditionIndex) const;

	/** Get the current color */
	FSlateColor GetColor() const;

	/** Get the current color for text */
	FSlateColor GetTextColor() const;

	/** Get the current color for text shadows */
	FLinearColor GetShadowColor() const;

	/** Get the progress bar */
	TOptional<float> GetProgress(int32 ConditionIndex) const;

	EVisibility GetProgressVisibility(int32 ConditionIndex) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	/** Player reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlarePlayerController>    PC;
	TSharedPtr<SVerticalBox>				        ConditionBox;

	// Data
	float                                           ObjectiveEnterTime;
	float                                           CurrentFadeTime;
	float                                           CurrentAlpha;
	int32                                           LastObjectiveVersion;


};
