#pragma once

#include "../../Flare.h"


class AFlareMenuManager;


class SFlareTooltip : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTooltip)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
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
	void ShowTooltip(SWidget* TargetWidget, FText Title, FText Content);

	/** Stop displaying the tooltip unless someone else stole the show (normal, safe version) */
	void HideTooltip(SWidget* TargetWidget);

	/** Stop displaying the tooltip (when you positively need that tooltip gone no matter what)*/
	void HideTooltipForce();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the current position */
	FMargin GetTooltipPosition() const;

	/** Get the current tooltip color/opacity */
	FSlateColor GetTooltipColor() const;

	/** Get the current tooltip color/opacity for text shadows */
	FLinearColor GetTooltipShadowColor() const;

	/** Get the current tooltip title */
	FText GetTitleText() const;

	/** Get the current tooltip text */
	FText GetHelpText() const;
		

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	TWeakObjectPtr<class AFlareMenuManager> MenuManager;
	
	TSharedPtr<SBox>                        ContentBox;

	// Settings
	float                                   TooltipDelay;
	float                                   TooltipFadeDuration;

	// Data
	SWidget*                                TooltipTarget;
	FText                                   TooltipTitle;
	FText                                   TooltipContent;
	bool                                    TooltipVisible;
	float                                   TooltipCurrentTime;
	float                                   TooltipCurrentAlpha;


};
