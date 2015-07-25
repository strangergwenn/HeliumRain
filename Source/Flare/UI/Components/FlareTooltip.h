#pragma once

#include "../../Flare.h"
#include "../Components/FlareNotification.h"


class AFlareMenuManager;


class SFlareTooltip : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTooltip)
	{}
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Start displaying the tooltip */
	void ShowTooltip(SWidget* TargetWidget, FText Content);

	/** Stop displaying the tooltip */
	void HideTooltip(SWidget* TargetWidget);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the current tooltip color/opacity */
	FSlateColor GetTooltipColor() const;

	/** Get the current tooltip color/opacity for text shadows */
	FLinearColor GetTooltipShadowColor() const;

	/** Get the current tooltip text */
	FText GetHelpText() const;
		

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Settings
	float                                   TooltipDelay;
	float                                   TooltipFadeDuration;

	// Data
	SWidget*                                TooltipTarget;
	FText                                   TooltipContent;
	bool                                    TooltipVisible;
	float                                   TooltipCurrentTime;
	float                                   TooltipCurrentAlpha;


};
