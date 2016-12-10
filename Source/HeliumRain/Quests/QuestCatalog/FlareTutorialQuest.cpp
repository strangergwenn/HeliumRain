
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

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 30));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoBackward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","There is no air to brake in space. Your ship will keep its velocity and direction if you don't use your engines. Braking will use as much energy as accelerating, so it can take a long time if you're going fast.<br>Press <input-axis:NormalThrustInput,-1.0> to brake.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-backward", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMaxCollinearVelocity::Create(this, QUEST_TAG"cond1", -20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchUp"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the top or bottom part of the screen to pitch up.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-up", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,1,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchDown"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the top or bottom part of the screen to pitch down.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-down", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,1,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawLeft"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the left or right part of the screen to turn left.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-left", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,0,1) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawRight"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your cursor to the left or right part of the screen to turn right.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-right", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,0,1) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollLeft"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Roll your spacescraft left <input-axis:NormalRollInput,-1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-left", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(1,0,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollRight"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Roll your spacescraft right with <input-axis:NormalRollInput,1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-right", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(1,0,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Forward"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Move your ship forward again.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "forward", Description);

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FollowPath"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can use the prograde vector to follow a path. Keep a constant velocity and just aim your target, your ship's engine controller will automatically align yout velicity with your ship orientation.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "follow-path", Description);

		TArray<FVector> Waypoints;
		Waypoints.Add(FVector(2000, 50, 150));
		Waypoints.Add(FVector(2500, 200, 150));
		Waypoints.Add(FVector(3000, 200, 0));
		Waypoints.Add(FVector(3500, 250, 100));
		Waypoints.Add(FVector(4000, 400, 200));

		Step->GetEnableConditions().Add(FlyShip);
		Step->GetEndConditions().Add(UFlareQuestConditionFollowRelativeWaypoints::Create(this, QUEST_TAG"cond1", Waypoints));
		Steps.Add(Step);
	}
}


#undef LOCTEXT_NAMESPACE
