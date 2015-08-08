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

	/** Get the current objective info text */
	FText GetInfo() const;

	/** Get the current color */
	FSlateColor GetColor() const;

	/** Get the current color for text */
	FSlateColor GetTextColor() const;

	/** Get the current color for text shadows */
	FLinearColor GetShadowColor() const;

	/** Get the progress bar */
	TOptional<float> GetProgress() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	/** Player reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlarePlayerController>    PC;

	// Data
	float                                           ObjectiveEnterTime;
	float                                           CurrentFadeTime;
	float                                           CurrentAlpha;


};
