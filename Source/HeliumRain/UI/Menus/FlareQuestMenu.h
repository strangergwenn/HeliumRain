#pragma once

#include "../../Flare.h"


class AFlareMenuManager;


class SFlareQuestMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareQuestMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter(UFlareQuest* TargetQuest);

	/** Exit this menu */
	void Exit();


protected:

	/*----------------------------------------------------
		Internal methods
	----------------------------------------------------*/

	/** Fill the quest list for all active quests */
	void FillActiveQuestList();

	/** Fill the quest list for all previous quests */
	void FillPreviousQuestList();

	/** Show all steps for the current quest */
	void FillQuestDetails();

	/** Add a quest detail line */
	TSharedPtr<SVerticalBox> AddQuestDetail(int32 QuestStepIndex);


	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Get the selected quest title */
	FText GetSelectedQuestTitle() const;

	/** Get the active quest description */
	FText GetQuestStepDescription(int32 QuestStepIndex) const;

	/** Is this quest step visible */
	EVisibility GetQuestStepDescriptionVisibility(int32 QuestStepIndex) const;

	/** Is the "track" button disabled */
	bool IsTrackQuestButtonDisabled(UFlareQuest*  Quest) const;
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** A new quest is tracked */
	void OnQuestTracked(UFlareQuest* Quest);

	/** A new quest is selected for more details */
	void OnQuestSelected(UFlareQuest* Quest);

	/** A new quest step is selected for more details */
	FReply OnQuestStepSelected(int32 QuestStepIndex);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// Slate widgets
	TSharedPtr<SVerticalBox>                        ActiveQuestList;
	TSharedPtr<SVerticalBox>                        PreviousQuestList;
	TSharedPtr<SVerticalBox>                        QuestDetails;

	// Data
	UFlareQuest*                                    SelectedQuest;
	int32                                           CurrentQuestStepIndex;


};
