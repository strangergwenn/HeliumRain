#pragma once

#include "../../Flare.h"


class UFlareQuestStep;
class AFlarePlayerController;


class SFlareObjectiveInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareObjectiveInfo)
		: _ConditionsOnly(false)
		, _QuestStep(NULL)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, PC)
	SLATE_ARGUMENT(UFlareQuestStep*, QuestStep)
	SLATE_ARGUMENT(int32, Width)
	SLATE_ARGUMENT(bool, ConditionsOnly)

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
	
	/** Get the current objective name */
	FText GetName() const;
	
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


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	/** Player reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlarePlayerController>    PC;

	// Slate data
	TSharedPtr<SVerticalBox>				        ConditionBox;

	// Settings
	int32                                           Width;
	UFlareQuestStep*                                QuestStep;
	bool                                            ConditionsOnly;

	// Data
	float                                           ObjectiveEnterTime;
	float                                           CurrentFadeTime;
	float                                           CurrentAlpha;
	int32                                           LastObjectiveVersion;
	FFlarePlayerObjective                           QuestObjective;


};
