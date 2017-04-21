
#include "Flare.h"
#include "../../Game/FlareGame.h"
#include "../FlareQuestCondition.h"
#include "../FlareQuestAction.h"
#include "../FlareQuestStep.h"
#include "../FlareQuest.h"
#include "FlareTutorialQuest.h"

#define LOCTEXT_NAMESPACE "FlareTutorialQuest"


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
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Spaceships have a lot of small engines that make up the RCS (Reaction Control System), allowing them to move around. To go forward press <input-axis:NormalThrustInput,1.0> slightly. You can modify the key binding in the settings menu (<input-action:SettingsMenu>).");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-forward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 30));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoBackward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","There is no air to brake in space. Your ship will keep its velocity and direction if you don't use your engines. Braking will use as much energy as accelerating, so it can take a long time if you're going fast.<br>Press <input-axis:NormalThrustInput,-1.0> to brake.");
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
		#define QUEST_STEP_TAG QUEST_TAG"Travel"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","To start a travel, open the overlay with <input-action:ToggleOverlay> and click on the orbital map. Select the sector \"The Depths\" and click \"Travel\".<br>Then, use the \"Fast Forward\" button to complete the travel.");
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
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "You can now dock at stations to trade resources or upgrade your spacecraft ! While flying, use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}, and then press <input-action:Wheel> to select the docking option. Your ship will dock automatically - but you can brake to disengage the autopilot. Use the wheel menu to undock too !"), FText::FromString(Station->GetImmatriculation().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pick-up", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station);
		Condition->TargetShipSaveId = PickUpShipId;

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}
}

#undef LOCTEXT_NAMESPACE
