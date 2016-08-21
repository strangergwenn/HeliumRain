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
	void Enter(UFlareQuest* Sector);

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
	TSharedPtr<SVerticalBox> AddQuestDetail(int32 QuestIndex);


	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Get the active quest title */
	FText GetActiveQuestTitle() const;

	/** Get the active quest description */
	FText GetActiveQuestDescription() const;

	/** Is the "track" button visible ?*/
	bool GetTrackQuestVisibility(UFlareQuest*  Quest) const;
	

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** A new quest is selected */
	void OnQuestSelected(UFlareQuest* Quest);


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


};
