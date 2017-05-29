#pragma once

#include "../../Flare.h"

class UFlareQuest;
class UFlareQuestStep;
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

	/** Fill the quest list for all available quests */
	void FillAvailableQuestList();

	/** Fill the quest list for all ongoing quests */
	void FillOngoingQuestList();

	/** Fill the quest list for all previous quests */
	void FillPreviousQuestList();

	/** Show all steps for the current quest */
	void FillQuestDetails();

	/** Add a quest detail line */
	TSharedPtr<SVerticalBox> AddQuestDetail(UFlareQuestStep* QuestStep);


	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/
	
	/** Get the active quest description */
	FText GetQuestStepDescription(UFlareQuestStep* QuestStep) const;

	/** Is this quest step visible */
	EVisibility GetQuestStepDescriptionVisibility(UFlareQuestStep* QuestStep) const;

	/** Get the quest color */
	FSlateColor GetQuestColor(UFlareQuest* Quest) const;

	/** Is the "track" button disabled */
	bool IsTrackButtonDisabled(UFlareQuest* Quest) const;
	
	/** Is abandon quest button visible */
	EVisibility GetAbandonButtonVisibility(UFlareQuest* Quest) const;

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** A new quest is accepted */
	void OnQuestAccepted(UFlareQuest* Quest);
	
	/** A quest is abandoned */
	void OnQuestAbandoned(UFlareQuest* Quest);

	/** A quest is abandoned (confirmed) */
	void OnQuestAbandonedConfirmed(UFlareQuest* Quest);

	/** A new quest is tracked */
	void OnQuestTracked(UFlareQuest* Quest);

	/** A new quest is selected for more details */
	void OnQuestSelected(UFlareQuest* Quest);

	/** A new quest step is selected for more details */
	FReply OnQuestStepSelected(UFlareQuestStep* QuestStep);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// Slate widgets
	TSharedPtr<SVerticalBox>                        AvailableQuestList;
	TSharedPtr<SVerticalBox>                        OngoingQuestList;
	TSharedPtr<SVerticalBox>                        PreviousQuestList;
	TSharedPtr<SVerticalBox>                        QuestDetails;

	// Data
	UFlareQuest*                                    SelectedQuest;
	UFlareQuestStep*                                CurrentQuestStep;


};
