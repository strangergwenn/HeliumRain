
#include "Flare.h"
#include "../../Game/FlareGame.h"
#include "../FlareQuestCondition.h"
#include "../FlareQuestAction.h"
#include "../FlareQuestStep.h"
#include "../FlareQuest.h"
#include "FlareTutorialQuest.h"

#define LOCTEXT_NAMESPACE "FlareTutorialQuest"

static FName TUTORIAL_CURRENT_PROGRESSION_TAG("current-progression");

/*----------------------------------------------------
	Tutorial Flying
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialFlying"
UFlareQuestTutorialFlying::UFlareQuestTutorialFlying(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialFlying::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialFlying* Quest = NewObject<UFlareQuestTutorialFlying>(Parent, UFlareQuestTutorialFlying::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialFlying::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-flying";
	QuestName = LOCTEXT(QUEST_TAG"Name","Spaceflight tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Let's learn how to fly this thing !");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	// Common condition
	UFlareQuestCondition* FlyShip = UFlareQuestConditionFlyingShipClass::Create(this, NAME_None);

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoForward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Spaceships feature small maneuvering engines called the RCS, and large orbital engines for accelerating. To go forward press <input-axis:NormalThrustInput,1.0> slightly. You can modify the key binding in the settings menu (<input-action:SettingsMenu>).");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-forward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 30));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoBackward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","There is no air to brake in space. Your ship will keep its velocity and direction if you don't brake with your RCS. Press <input-axis:NormalThrustInput,-1.0> to brake.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-backward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxCollinearVelocity::Create(this, QUEST_TAG"cond1", -20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchUp"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the top of the screen to pitch up.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-up", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,1,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchDown"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the bottom of the screen to pitch down.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-down", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,1,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawLeft"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the left of the screen to turn left.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-left", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,0,1) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawRight"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the right of the screen to turn right.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-right", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,0,1) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollLeft"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Roll your spacescraft left with <input-axis:NormalRollInput,-1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-left", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(1,0,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollRight"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Roll your spacescraft right with <input-axis:NormalRollInput,1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-right", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(1,0,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Forward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your ship forward again.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "forward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FollowAdvancedPath"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can use the prograde vector to follow a path. Keep a constant velocity and aim for your target, your ship's engine controller will automatically align your velocity with your ship's orientation.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "follow-advanced-path", Description);


		// TODO force first light


		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionFollowRandomWaypoints::Create(this, QUEST_TAG"cond1"));
		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial Navigation
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialNavigation"
UFlareQuestTutorialNavigation::UFlareQuestTutorialNavigation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialNavigation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialNavigation* Quest = NewObject<UFlareQuestTutorialNavigation>(Parent, UFlareQuestTutorialNavigation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialNavigation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-navigation";
	QuestName = LOCTEXT(QUEST_TAG"Name","Orbital navigation tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to travel from one sector to another.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-flying"));

	FName PickUpShipId = "dock-at-ship-id";
	UFlareSimulatedSector* Sector = FindSector("the-depths");
	UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations().Num() ? Sector->GetSectorStations()[0] : NULL;

	if (!Sector)
	{
		return;
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenTravelMenu"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You need to learn how to travel between sectors. To start a travel, open the menu bar with <input-action:ToggleOverlay> and click on the orbital map.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-travel-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Orbit));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Travel"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Select the sector \"The Depths\" and click \"Travel\". Then, on the orbital map, use the \"Fast Forward\" button to wait for the end of the travel.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, Sector));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Activate"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Your ship arrived at destination !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "activate", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorActive::Create(this, Sector));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CloseMenu"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can now dock at stations to trade resources or upgrade your spacecraft ! Close the menu to go back to your ship.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "close-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_FlyShip));
		Steps.Add(Step);
	}

	// TODO #682 Split up this last step into 5 different steps
	// 
	// TODO quest step condition = select station A. 
	//					message = "You can use the targeting system to interact with objects. Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}."
	// TODO quest step condition = select station B. 
	//					message = "Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}."
	// TODO quest step condition = select station C. 
	//					message = "Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}."
	// TODO quest step condition = start docking. 
	//					message = "Press <input-action:Wheel> to select the docking option"
	// TODO quest step condition = wait for docking to end. 
	//					message = "Your ship is now docking automatically, but you can brake to disengage the autopilot. Use the wheel menu to undock, trade, or upgrade your ship."

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "You can use the targeting system to interact with objects. Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}, and then press <input-action:Wheel> to select the docking option. Your ship will dock automatically, but you can brake to disengage the autopilot. Use the wheel menu to undock, trade, or upgrade your ship"),
			UFlareGameTools::DisplaySpacecraftName(Station));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pick-up", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station);
		Condition->TargetShipSaveId = PickUpShipId;

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial Contracts
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialContracts"
UFlareQuestTutorialContracts::UFlareQuestTutorialContracts(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialContracts::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialContracts* Quest = NewObject<UFlareQuestTutorialContracts>(Parent, UFlareQuestTutorialContracts::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialContracts::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-contracts";
	QuestName = LOCTEXT(QUEST_TAG"Name","Contracts tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to complete contracts for other companies.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-navigation"));


	UFlareSimulatedSector* Sector = FindSector("blue-heart");

	if (!Sector)
	{
		return;
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FindContract"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Other companies in this system can give you contracts to carry out. You can ignore them, but they are a very good way to earn money or research, or to discover new sectors. Keep traveling between sectors to find contracts.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "find-contract", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGetContrat::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenQuestMenu"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","A company has made a new contract available to you.\nOpen the contracts menu with (<input-action:QuestMenu>), or from the menu bar with <input-action:ToggleOverlay>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-quest-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Quest));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"AcceptQuest"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can manage contracts here. You have only a few days to accept a contract, but once you do, you can usually take your time to finish it. Accept your first contract now !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "accept-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialAcceptQuest::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TrackQuest"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Tracking a contract makes its instructions visible permanently. Track a contract now - you can return to this menu to track the tutorial contract at any time if you are lost.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "track-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTrackQuest::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FinishQuest"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Complete five contracts to start your mercenary career.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "complete-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialFinishQuest::Create(this, QUEST_TAG"cond1", 5));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial get contrat condition
----------------------------------------------------*/
UFlareQuestConditionTutorialGetContrat::UFlareQuestConditionTutorialGetContrat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialGetContrat* UFlareQuestConditionTutorialGetContrat::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialGetContrat* Condition = NewObject<UFlareQuestConditionTutorialGetContrat>(ParentQuest, UFlareQuestConditionTutorialGetContrat::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialGetContrat::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialGetContrat", "Accept a contract");
}

void UFlareQuestConditionTutorialGetContrat::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("get-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialGetContrat::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialGetContrat::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial open menu condition
----------------------------------------------------*/
UFlareQuestConditionTutorialOpenMenu::UFlareQuestConditionTutorialOpenMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialOpenMenu* UFlareQuestConditionTutorialOpenMenu::Create(UFlareQuest* ParentQuest, EFlareMenu::Type Menu)
{
	UFlareQuestConditionTutorialOpenMenu* Condition = NewObject<UFlareQuestConditionTutorialOpenMenu>(ParentQuest, UFlareQuestConditionTutorialOpenMenu::StaticClass());
	Condition->Load(ParentQuest, Menu);
	return Condition;
}

void UFlareQuestConditionTutorialOpenMenu::Load(UFlareQuest* ParentQuest, EFlareMenu::Type Menu)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	MenuType = Menu;
	
	switch (MenuType) {
	case EFlareMenu::MENU_FlyShip:
		InitialLabel = LOCTEXT("MenuNone", "Close the menu");
		break;
	case EFlareMenu::MENU_Orbit:
		InitialLabel = LOCTEXT("MenuOrbit", "Open the orbital map");
		break;
	case EFlareMenu::MENU_Quest:
		InitialLabel =  LOCTEXT("MenuQuest", "Open the contract menu");
		break;
	default:
		break;
	}
}

void UFlareQuestConditionTutorialOpenMenu::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("open-menu") && Bundle.GetInt32("menu") == (MenuType+0))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialOpenMenu::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialOpenMenu::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Accept quest condition
----------------------------------------------------*/
UFlareQuestConditionTutorialAcceptQuest::UFlareQuestConditionTutorialAcceptQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialAcceptQuest* UFlareQuestConditionTutorialAcceptQuest::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialAcceptQuest* Condition = NewObject<UFlareQuestConditionTutorialAcceptQuest>(ParentQuest, UFlareQuestConditionTutorialAcceptQuest::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialAcceptQuest::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialAcceptContrat", "Accept a contract");
}

void UFlareQuestConditionTutorialAcceptQuest::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("accept-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialAcceptQuest::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialAcceptQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Track quest condition
----------------------------------------------------*/
UFlareQuestConditionTutorialTrackQuest::UFlareQuestConditionTutorialTrackQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTrackQuest* UFlareQuestConditionTutorialTrackQuest::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialTrackQuest* Condition = NewObject<UFlareQuestConditionTutorialTrackQuest>(ParentQuest, UFlareQuestConditionTutorialTrackQuest::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialTrackQuest::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialTrackContrat", "Track a contract");
}

void UFlareQuestConditionTutorialTrackQuest::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("track-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialTrackQuest::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialTrackQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Finish quest condition
----------------------------------------------------*/

UFlareQuestConditionTutorialFinishQuest::UFlareQuestConditionTutorialFinishQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialFinishQuest* UFlareQuestConditionTutorialFinishQuest::Create(UFlareQuest* ParentQuest,
																					 FName ConditionIdentifierParam,
																					 int32 Count)
{
	UFlareQuestConditionTutorialFinishQuest*Condition = NewObject<UFlareQuestConditionTutorialFinishQuest>(ParentQuest, UFlareQuestConditionTutorialFinishQuest::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, Count);
	return Condition;
}

void UFlareQuestConditionTutorialFinishQuest::Load(UFlareQuest* ParentQuest,
												 FName ConditionIdentifierParam,
												 int32 Count)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionTutorialFinishQuest need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);

	QuestCount = Count;

	InitialLabel = FText::Format(LOCTEXT("FinishQuestLabel","Complete {0} contracts"),
									 FText::AsNumber(QuestCount));
}

void UFlareQuestConditionTutorialFinishQuest::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionTutorialFinishQuest::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(TUTORIAL_CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionTutorialFinishQuest::IsCompleted()
{
	return CurrentProgression >= QuestCount;
}

void UFlareQuestConditionTutorialFinishQuest::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("success-contract"))
	{
		CurrentProgression++;
	}
}

void UFlareQuestConditionTutorialFinishQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = QuestCount;
	ObjectiveCondition.MaxProgress = QuestCount;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}



#undef LOCTEXT_NAMESPACE
