
#include "Flare.h"
#include "../../Game/FlareGame.h"
#include "../FlareQuestCondition.h"
#include "../FlareQuestAction.h"
#include "../FlareQuestStep.h"
#include "../FlareQuest.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../../Spacecrafts/FlareSpacecraft.h"

#include "FlareTutorialQuest.h"

#define LOCTEXT_NAMESPACE "FlareTutorialQuest"

static FName TUTORIAL_CURRENT_PROGRESSION_TAG("current-progression");
static FName TUTORIAL_LAST_TARGET_CHANGE_TS("last-target-change-ts");


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
	if (!Sector)
	{
		return;
	}

	UFlareSimulatedSpacecraft* StationA = Sector->GetSectorStations().Num() > 0 ? Sector->GetSectorStations()[0] : NULL;
	UFlareSimulatedSpacecraft* StationB= Sector->GetSectorStations().Num() > 1 ? Sector->GetSectorStations()[1] : NULL;
	UFlareSimulatedSpacecraft* StationC = Sector->GetSectorStations().Num() > 2 ? Sector->GetSectorStations()[2] : NULL;

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

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetA"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "You can use the targeting system to interact with objects. Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}."),
			UFlareGameTools::DisplaySpacecraftName(StationA));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-a", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationA, 3.0));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetB"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}. The targets are sort by distance to your nose."),
			UFlareGameTools::DisplaySpacecraftName(StationB));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-b", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationB, 3.0));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetC"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Use <input-action:NextTarget> and <input-action:PreviousTarget> to select {0}."),
			UFlareGameTools::DisplaySpacecraftName(StationC));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-C", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationC, 3.0));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CommandDock"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "Press <input-action:Wheel> to select the docking option");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "command-dock", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialCommandDock::Create(this, StationC));
		Steps.Add(Step);
	}
	
	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "Your ship is now docking automatically, but you can brake to disengage the autopilot. Use the wheel menu to undock, trade, or upgrade your ship.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pick-up", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, StationC);
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
	Tutorial technology
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialTechnology"
UFlareQuestTutorialTechnology::UFlareQuestTutorialTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialTechnology::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialTechnology* Quest = NewObject<UFlareQuestTutorialTechnology>(Parent, UFlareQuestTutorialTechnology::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialTechnology::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-technology";
	QuestName = LOCTEXT(QUEST_TAG"Name","Technology tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to use technologies.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GainResearchPoint"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can develop technologies to increase your capabilities and your company's performance. "
									"To develop a technology, you need research points. Companies with a high research value sometimes offer you contracts with research points as a reward. You will learn a more efficient way to get research points in the future !"
									"\nGain some research points to move forward.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "gain-research-points", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialResearchValue::Create(this, 50));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenTechnologyMenu"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You have enough research points to research technology. Open the technology menu with (<input-action:TechnologyMenu>), or from the menu bar with <input-action:ToggleOverlay>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-technology-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Technology));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchTechnology"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can manage research in the technology menu. Select a level 1 technology and research it !"
									"\nChoose wisely, for the price of all technology is increased after each research.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialResearchTechnology::Create(this, 1));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ReachLevelTechnology3"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","High level technology require an increased technology level. Your technology level is incremented every new technology."
																"\nResearch more technologies to unlock level 3 technologies.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "track-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTechnologyLevel::Create(this, 3));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchTechnologyLevel3"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You are now ready to research a level 3 technology.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-technology-3", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialResearchTechnology::Create(this, 3));
		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial build ship
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialBuildShip"
UFlareQuestTutorialBuildShip::UFlareQuestTutorialBuildShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialBuildShip::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialBuildShip* Quest = NewObject<UFlareQuestTutorialBuildShip>(Parent, UFlareQuestTutorialBuildShip::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialBuildShip::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-build-ship";
	QuestName = LOCTEXT(QUEST_TAG"Name","Ship-building tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to build ships.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;


	UFlareSimulatedSector* Sector = FindSector("blue-heart");

	if (!Sector)
	{
		return;
	}

	FName PickUpShipId = "dock-at-ship-id";
	UFlareSimulatedSpacecraft* Shipyard = NULL;
	for(UFlareSimulatedSpacecraft* Station: Sector->GetSectorStations())
	{
			if(Station->IsShipyard())
			{
				Shipyard = Station;
				break;
			}
	}

	if (!Shipyard)
	{
		return;
	}

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GainMoney"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","You can buy ships to increase your trading force or to defend your estate. Ships are expensive, save some money first !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "gain-money", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialMoney::Create(this, 3000000));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "You can order ships at shipyards. Dock your ship at the Blue Heart shipyard.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "dock", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Shipyard);
		Condition->TargetShipSaveId = PickUpShipId;

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OrderShip"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Open the shipyard details, either with the wheel menu or through the sector menu. You will have the details of production lines."
									"\nPick a production line for small ships and order a ship of your choice.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "order-ship", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOrderShip::Create(this, EFlarePartSize::S));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildShip"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Ship construction may be long. Continue playing until your ship is ready !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-ship", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildShip::Create(this, EFlarePartSize::S));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial build station
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialBuildStation"
UFlareQuestTutorialBuildStation::UFlareQuestTutorialBuildStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialBuildStation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialBuildStation* Quest = NewObject<UFlareQuestTutorialBuildStation>(Parent, UFlareQuestTutorialBuildStation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialBuildStation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-build-station";
	QuestName = LOCTEXT(QUEST_TAG"Name","Station-building tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to build stations.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionTutorialResearchValue::Create(this, 50));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-build-ship"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchStationTechnology"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Like other companies, you can have stations. To build them, you need to unlock some stations by researching corresponding technologies.\n"
																"Research one of the technology unlocking station contruction.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-station-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockStation::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"SectorMenu"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "You have now unlock some station contruction possibility. You can build stations on any sector but there is various limitations depending on the kind of station you want to by and the sector."
																 "\nOpen the current sector menu (<input-action:SectorMenu>), or another visited sector from the orbital menu");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "dock", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Sector));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartConstuction"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Open the build station menu using the button on the top left part of the sector menu."
																"\nStart a station construction will cost you a large amount of administrative fees. This cost depend of the number of stations in the sector : the more there is stations, the more the fees will be expensive."
																"\nChoose a station and start the construction");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "order-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialStartStationConstruction::Create(this, false));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildStation"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","New your station is under construction. Bring missing construction resources to finish the construction."
																"\nNote that others companies can sell some resources directly to the station."
																"\nFinish the station building.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, false));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartUpgrade"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Your station is now operational. You can inspect it to see its production needs. "
																"\nYou can upgrade your station to increase its productivity and cargo bay size. Upgrading a station will make it in construction again until you bring the missing resources."
																"\nUpgrade a station (using upgrade button in the station details)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "start-upgrade", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialStartStationConstruction::Create(this, true));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FinishUpgrade"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Bring missing construction resources to finish the upgrade.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "finish-upgrade", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, true));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial research station
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialResearchStation"
UFlareQuestTutorialResearchStation::UFlareQuestTutorialResearchStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialResearchStation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialResearchStation* Quest = NewObject<UFlareQuestTutorialResearchStation>(Parent, UFlareQuestTutorialResearchStation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialResearchStation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-research-station";
	QuestName = LOCTEXT(QUEST_TAG"Name","Reseach station tutorial");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Learn how to build and use research stations.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-technology"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-build-station"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchStationTechnology"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","A good way to get technology is to build research stations. First unlock the technology to build research stations.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-station-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "stations"));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildReasearchStation"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description", "You have the technology to build research stations. Build one research station in the sector of your choose");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-research-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildStation::Create(this, false, "station-research"));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ProduceResearch"
		FText Description = LOCTEXT(QUEST_STEP_TAG"Description","Your research station is ready. Make sure you provide it input resource to produce research points.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "produce-peseach", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialProduceResearch::Create(this, QUEST_TAG"cond1", 50));
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



/*----------------------------------------------------
Tutorial command dock condition
----------------------------------------------------*/
UFlareQuestConditionTutorialCommandDock::UFlareQuestConditionTutorialCommandDock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialCommandDock* UFlareQuestConditionTutorialCommandDock::Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* SpacecraftParam)
{
	UFlareQuestConditionTutorialCommandDock* Condition = NewObject<UFlareQuestConditionTutorialCommandDock>(ParentQuest, UFlareQuestConditionTutorialCommandDock::StaticClass());
	Condition->Load(ParentQuest, SpacecraftParam);
	return Condition;
}

void UFlareQuestConditionTutorialCommandDock::Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* SpacecraftParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	Target = SpacecraftParam;

	InitialLabel = FText::Format(LOCTEXT("StartDocking", "Start docking at {0}"), UFlareGameTools::DisplaySpacecraftName(Target));
}

void UFlareQuestConditionTutorialCommandDock::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("start-docking") && Bundle.GetName("target") == Target->GetImmatriculation())
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialCommandDock::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialCommandDock::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	if (Target)
	{
		ObjectiveData->AddTargetSpacecraft(Target);
	}
}

/*----------------------------------------------------
Target spacecraftcondition
----------------------------------------------------*/

UFlareQuestConditionTutorialTargetSpacecraft::UFlareQuestConditionTutorialTargetSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTargetSpacecraft* UFlareQuestConditionTutorialTargetSpacecraft::Create(UFlareQuest* ParentQuest,
	FName ConditionIdentifierParam,
	UFlareSimulatedSpacecraft* Spacecraft,
	float Duration)
{
	UFlareQuestConditionTutorialTargetSpacecraft*Condition = NewObject<UFlareQuestConditionTutorialTargetSpacecraft>(ParentQuest, UFlareQuestConditionTutorialTargetSpacecraft::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, Spacecraft, Duration);
	return Condition;
}

void UFlareQuestConditionTutorialTargetSpacecraft::Load(UFlareQuest* ParentQuest,
	FName ConditionIdentifierParam,
	UFlareSimulatedSpacecraft* Spacecraft,
	float Duration)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionTutorialTargetSpacecraft need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);

	TargetDuration = Duration;
	TargetSpacecraft = Spacecraft;

	InitialLabel = FText::Format(LOCTEXT("FinishQuestLabel", "Target {0} for {1} seconds"),
		UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft),
		FText::AsNumber(int32(TargetDuration)));
}

void UFlareQuestConditionTutorialTargetSpacecraft::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if (Bundle)
	{
		HasSave &= Bundle->HasFloat(TUTORIAL_LAST_TARGET_CHANGE_TS);
	}
	else
	{
		HasSave = false;
	}

	if (HasSave)
	{
		LastTargetChangeTimestamp = Bundle->GetFloat(TUTORIAL_LAST_TARGET_CHANGE_TS);
	}
	else
	{
		LastTargetChangeTimestamp = FPlatformTime::Seconds();
	}

}

void UFlareQuestConditionTutorialTargetSpacecraft::Save(FFlareBundle* Bundle)
{
	Bundle->PutFloat(TUTORIAL_LAST_TARGET_CHANGE_TS, LastTargetChangeTimestamp);
}


bool UFlareQuestConditionTutorialTargetSpacecraft::IsCompleted()
{
	AFlareSpacecraft* SpacecraftActive = GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetCurrentTarget();
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftActive ? SpacecraftActive->GetParent() : NULL;

	if (TargetSpacecraft == Spacecraft)
	{
		double CurrentDuration =  FPlatformTime::Seconds() - LastTargetChangeTimestamp;
		if (CurrentDuration > TargetDuration)
		{
			return true;
		}
	}
	return false;
}

void UFlareQuestConditionTutorialTargetSpacecraft::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("target-changed"))
	{
		LastTargetChangeTimestamp = FPlatformTime::Seconds();
	}
}

void UFlareQuestConditionTutorialTargetSpacecraft::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxProgress = TargetDuration;
	ObjectiveCondition.MaxCounter = 0;
	ObjectiveCondition.Counter = 0;

	AFlareSpacecraft* SpacecraftActive = GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetCurrentTarget();
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftActive ? SpacecraftActive->GetParent() : NULL;


	if (TargetSpacecraft == Spacecraft)
	{
		double CurrentDuration = FPlatformTime::Seconds() - LastTargetChangeTimestamp;
		if (CurrentDuration > TargetDuration)
		{
			ObjectiveCondition.Progress = TargetDuration;
		}
		else
		{
			ObjectiveCondition.Progress = CurrentDuration;
		}
	}
	else
	{
		ObjectiveCondition.Progress = 0;
	}
	
	ObjectiveData->ConditionList.Add(ObjectiveCondition);

	if (TargetSpacecraft)
	{
		ObjectiveData->AddTargetSpacecraft(TargetSpacecraft);
	}
}

/*----------------------------------------------------
	Tutorial research value condition
----------------------------------------------------*/
UFlareQuestConditionTutorialResearchValue::UFlareQuestConditionTutorialResearchValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialResearchValue* UFlareQuestConditionTutorialResearchValue::Create(UFlareQuest* ParentQuest, int32 Count)
{
	UFlareQuestConditionTutorialResearchValue* Condition = NewObject<UFlareQuestConditionTutorialResearchValue>(ParentQuest, UFlareQuestConditionTutorialResearchValue::StaticClass());
	Condition->Load(ParentQuest, Count);
	return Condition;
}

void UFlareQuestConditionTutorialResearchValue::Load(UFlareQuest* ParentQuest, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetResearchPoints = Count;

	FText InitialLabelText = LOCTEXT("PlayerResearchPoints", "Gain {0} research points");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetResearchPoints));
}

bool UFlareQuestConditionTutorialResearchValue::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetResearchValue() >= TargetResearchPoints;
}

void UFlareQuestConditionTutorialResearchValue::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = PlayerCompany->GetResearchValue();
	ObjectiveCondition.MaxCounter = TargetResearchPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial technology level condition
----------------------------------------------------*/
UFlareQuestConditionTutorialTechnologyLevel::UFlareQuestConditionTutorialTechnologyLevel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTechnologyLevel* UFlareQuestConditionTutorialTechnologyLevel::Create(UFlareQuest* ParentQuest, int32 Level)
{
	UFlareQuestConditionTutorialTechnologyLevel* Condition = NewObject<UFlareQuestConditionTutorialTechnologyLevel>(ParentQuest, UFlareQuestConditionTutorialTechnologyLevel::StaticClass());
	Condition->Load(ParentQuest, Level);
	return Condition;
}

void UFlareQuestConditionTutorialTechnologyLevel::Load(UFlareQuest* ParentQuest, int32 Level)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetLevel = Level;

	FText InitialLabelText = LOCTEXT("PlayerTechnologyLevel", "Reach technnology level {0}");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetLevel));
}

bool UFlareQuestConditionTutorialTechnologyLevel::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetTechnologyLevel() >= TargetLevel;
}

void UFlareQuestConditionTutorialTechnologyLevel::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = PlayerCompany->GetTechnologyLevel();
	ObjectiveCondition.MaxCounter = TargetLevel;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial research technology condition
----------------------------------------------------*/
UFlareQuestConditionTutorialResearchTechnology::UFlareQuestConditionTutorialResearchTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialResearchTechnology* UFlareQuestConditionTutorialResearchTechnology::Create(UFlareQuest* ParentQuest, int32 MinLevel)
{
	UFlareQuestConditionTutorialResearchTechnology* Condition = NewObject<UFlareQuestConditionTutorialResearchTechnology>(ParentQuest, UFlareQuestConditionTutorialResearchTechnology::StaticClass());
	Condition->Load(ParentQuest, MinLevel);
	return Condition;
}

void UFlareQuestConditionTutorialResearchTechnology::Load(UFlareQuest* ParentQuest, int32 MinLevel)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetMinLevel = MinLevel;

	if(MinLevel <= 1)
	{
		InitialLabel = LOCTEXT("ResearchTechnologyLowLevel", "Research a technology");
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("ResearchTechnologyHighLevel", "Research a technology of level {0} or more"), TargetMinLevel);
	}
}

void UFlareQuestConditionTutorialResearchTechnology::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("unlock-technology") && Bundle.GetInt32("level") >= TargetMinLevel)
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialResearchTechnology::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialResearchTechnology::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial money value condition
----------------------------------------------------*/
UFlareQuestConditionTutorialMoney::UFlareQuestConditionTutorialMoney(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialMoney* UFlareQuestConditionTutorialMoney::Create(UFlareQuest* ParentQuest, int32 Count)
{
	UFlareQuestConditionTutorialMoney* Condition = NewObject<UFlareQuestConditionTutorialMoney>(ParentQuest, UFlareQuestConditionTutorialMoney::StaticClass());
	Condition->Load(ParentQuest, Count);
	return Condition;
}

void UFlareQuestConditionTutorialMoney::Load(UFlareQuest* ParentQuest, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetMoney = Count;

	FText InitialLabelText = LOCTEXT("PlayerMoney", "Save {0} credits");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(UFlareGameTools::DisplayMoney(TargetMoney)));
}

bool UFlareQuestConditionTutorialMoney::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetMoney() >= TargetMoney;
}

void UFlareQuestConditionTutorialMoney::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = UFlareGameTools::DisplayMoney(PlayerCompany->GetMoney());
	ObjectiveCondition.MaxCounter = UFlareGameTools::DisplayMoney(TargetMoney);

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial order ship condition
----------------------------------------------------*/
UFlareQuestConditionTutorialOrderShip::UFlareQuestConditionTutorialOrderShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialOrderShip* UFlareQuestConditionTutorialOrderShip::Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	UFlareQuestConditionTutorialOrderShip* Condition = NewObject<UFlareQuestConditionTutorialOrderShip>(ParentQuest, UFlareQuestConditionTutorialOrderShip::StaticClass());
	Condition->Load(ParentQuest, Size);
	return Condition;
}

void UFlareQuestConditionTutorialOrderShip::Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSize = Size;

	if(Size == EFlarePartSize::S)
	{
		InitialLabel = LOCTEXT("OrderSmallShip", "Order a small ship");
	}
	else
	{
		InitialLabel = LOCTEXT("OrderLargeShip", "Order a large ship");
	}
}

void UFlareQuestConditionTutorialOrderShip::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("order-ship") && Bundle.GetInt32("size") == int32(TargetSize))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialOrderShip::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialOrderShip::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial build ship condition
----------------------------------------------------*/
UFlareQuestConditionTutorialBuildShip::UFlareQuestConditionTutorialBuildShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialBuildShip* UFlareQuestConditionTutorialBuildShip::Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	UFlareQuestConditionTutorialBuildShip* Condition = NewObject<UFlareQuestConditionTutorialBuildShip>(ParentQuest, UFlareQuestConditionTutorialBuildShip::StaticClass());
	Condition->Load(ParentQuest, Size);
	return Condition;
}

void UFlareQuestConditionTutorialBuildShip::Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSize = Size;

	if(Size == EFlarePartSize::S)
	{
		InitialLabel = LOCTEXT("BuildSmallShip", "Build a small ship");
	}
	else
	{
		InitialLabel = LOCTEXT("BuildLargeShip", "Build a large ship");
	}
}

void UFlareQuestConditionTutorialBuildShip::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("build-ship") && Bundle.GetInt32("size") == int32(TargetSize))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialBuildShip::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialBuildShip::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial unlock station condition
----------------------------------------------------*/
UFlareQuestConditionTutorialUnlockStation::UFlareQuestConditionTutorialUnlockStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialUnlockStation* UFlareQuestConditionTutorialUnlockStation::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialUnlockStation* Condition = NewObject<UFlareQuestConditionTutorialUnlockStation>(ParentQuest, UFlareQuestConditionTutorialUnlockStation::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialUnlockStation::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);

	FText InitialLabelText = LOCTEXT("PlayerUnloackStationTech", "Unlock at least one station  technology");
	InitialLabel = InitialLabelText;
}

bool UFlareQuestConditionTutorialUnlockStation::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	return PlayerCompany->HasStationTechnologyUnlocked();
}

void UFlareQuestConditionTutorialUnlockStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial start station construction condition
----------------------------------------------------*/
UFlareQuestConditionTutorialStartStationConstruction::UFlareQuestConditionTutorialStartStationConstruction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialStartStationConstruction* UFlareQuestConditionTutorialStartStationConstruction::Create(UFlareQuest* ParentQuest, bool Upgrade)
{
	UFlareQuestConditionTutorialStartStationConstruction* Condition = NewObject<UFlareQuestConditionTutorialStartStationConstruction>(ParentQuest, UFlareQuestConditionTutorialStartStationConstruction::StaticClass());
	Condition->Load(ParentQuest, Upgrade);
	return Condition;
}

void UFlareQuestConditionTutorialStartStationConstruction::Load(UFlareQuest* ParentQuest, bool Upgrade)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetUpgrade = Upgrade;

	if(!Upgrade)
	{
		InitialLabel = LOCTEXT("StartStationConstructionNew", "Start a station construction");
	}
	else
	{
		InitialLabel = LOCTEXT("StartStationConstructionUprade", "Start a station upgrade");
	}
}

void UFlareQuestConditionTutorialStartStationConstruction::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("start-station-construction") && Bundle.GetInt32("upgrade") == int32(TargetUpgrade))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialStartStationConstruction::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialStartStationConstruction::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial build station condition
----------------------------------------------------*/
UFlareQuestConditionTutorialBuildStation::UFlareQuestConditionTutorialBuildStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialBuildStation* UFlareQuestConditionTutorialBuildStation::Create(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier)
{
	UFlareQuestConditionTutorialBuildStation* Condition = NewObject<UFlareQuestConditionTutorialBuildStation>(ParentQuest, UFlareQuestConditionTutorialBuildStation::StaticClass());
	Condition->Load(ParentQuest, Upgrade, StationIdentifier);
	return Condition;
}

void UFlareQuestConditionTutorialBuildStation::Load(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetUpgrade = Upgrade;
	TargetStationIdentifier = StationIdentifier;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationIdentifier);


	if(!Upgrade)
	{
		if(Desc)
		{
			InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionNew", "Build a {0} "), Desc->Name);
		}
		else
		{
			InitialLabel = LOCTEXT("FinishStationConstructionNew", "Finish a station construction");
		}
	}
	else
	{
		if(Desc)
		{
				InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionUpgrade", "Upgrade a {0} "), Desc->Name);
		}
		else
		{
			InitialLabel = LOCTEXT("FinishStationConstructionUpgrade", "Fishish a station upgrade");
		}
	}
}

void UFlareQuestConditionTutorialBuildStation::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("finish-station-construction") && Bundle.GetInt32("upgrade") == int32(TargetUpgrade))
	{
		if(TargetStationIdentifier == NAME_None || TargetStationIdentifier == Bundle.GetName("identifier"))
		{
			Completed = true;
		}
	}
}
bool UFlareQuestConditionTutorialBuildStation::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialBuildStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
	Tutorial unlock technology condition
----------------------------------------------------*/
UFlareQuestConditionTutorialUnlockTechnology::UFlareQuestConditionTutorialUnlockTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialUnlockTechnology* UFlareQuestConditionTutorialUnlockTechnology::Create(UFlareQuest* ParentQuest, FName Identifier)
{
	UFlareQuestConditionTutorialUnlockTechnology* Condition = NewObject<UFlareQuestConditionTutorialUnlockTechnology>(ParentQuest, UFlareQuestConditionTutorialUnlockTechnology::StaticClass());
	Condition->Load(ParentQuest, Identifier);
	return Condition;
}

void UFlareQuestConditionTutorialUnlockTechnology::Load(UFlareQuest* ParentQuest, FName TechIdentifier)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetIdentifier = TechIdentifier;

	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(TargetIdentifier);

	if(Technology)
	{
		FText InitialLabelText = LOCTEXT("PlayerUnlockTech", "Unlock '{0}' technology");
		InitialLabel = FText::Format(InitialLabelText, Technology->Name);
	}
}

bool UFlareQuestConditionTutorialUnlockTechnology::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	return PlayerCompany->IsTechnologyUnlocked(TargetIdentifier);
}

void UFlareQuestConditionTutorialUnlockTechnology::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
	Buy at station condition
----------------------------------------------------*/

UFlareQuestConditionTutorialProduceResearch::UFlareQuestConditionTutorialProduceResearch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialProduceResearch* UFlareQuestConditionTutorialProduceResearch::Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam)
{
	UFlareQuestConditionTutorialProduceResearch*Condition = NewObject<UFlareQuestConditionTutorialProduceResearch>(ParentQuest, UFlareQuestConditionTutorialProduceResearch::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, QuantityParam);
	return Condition;
}

void UFlareQuestConditionTutorialProduceResearch::Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionBuyAtStation need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Quantity = QuantityParam;


	InitialLabel = FText::Format(LOCTEXT("BuyAtStationDestroyed", "Produce {0} research points"),
								 FText::AsNumber(Quantity));
}

void UFlareQuestConditionTutorialProduceResearch::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionTutorialProduceResearch::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionTutorialProduceResearch::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionTutorialProduceResearch::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

void UFlareQuestConditionTutorialProduceResearch::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("produce-research"))
	{
		CurrentProgression+=Bundle.GetInt32("amount");
	}

	if (CurrentProgression >= Quantity)
	{
		CurrentProgression = Quantity;
	}
}


#undef LOCTEXT_NAMESPACE
