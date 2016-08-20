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

	/** Fill the quest lists with all active quests */
	void FillQuestList();

	/** Show all steps for the current quest */
	void FillQuestDetails();


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
	TSharedPtr<SVerticalBox>                        QuestList;
	TSharedPtr<SVerticalBox>                        QuestDetails;


};
