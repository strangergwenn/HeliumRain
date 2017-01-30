
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "FlareQuestGenerator.h"
#include "FlareQuestManager.h"
#include "FlareQuest.h"

#define LOCTEXT_NAMESPACE "FlareQuestGenerator"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestGenerator::UFlareQuestGenerator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestGenerator::Load(UFlareQuestManager* Parent, const FFlareQuestSave& Data)
{
	QuestManager = Parent;
	Game = Parent->GetGame();
	NextQuestIndex = Data.NextGeneratedQuestIndex;
}

void UFlareQuestGenerator::LoadQuests(const FFlareQuestSave& Data)
{
	for(const FFlareGeneratedQuestSave& QuestData : Data.GeneratedQuests) {

		UFlareQuestGenerated* Quest = NULL;
		if(QuestData.QuestClass == UFlareQuestGeneratedVipTransport::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedVipTransport>(this, UFlareQuestGeneratedVipTransport::StaticClass());
		}
		else
		{
			continue;
		}
		Quest->Load(this, QuestData.Data);
		QuestManager->AddQuest(Quest);
		GeneratedQuests.Add(Quest);
	}
}

void UFlareQuestGenerator::Save(FFlareQuestSave& Data)
{
	Data.NextGeneratedQuestIndex = NextQuestIndex;
	Data.GeneratedQuests.Empty();
	for(UFlareQuestGenerated* Quest : GeneratedQuests)
	{
		FFlareGeneratedQuestSave QuestData;
		QuestData.QuestClass = Quest->GetQuestClass();
		QuestData.Data = *Quest->GetInitData();
		Data.GeneratedQuests.Add(QuestData);
	}
}

/*----------------------------------------------------
	Quest generation
----------------------------------------------------*/

void UFlareQuestGenerator::GenerateIdentifer(FName QuestClass, FFlareBundle& Data)
{
	FName Identifier = FName(*(QuestClass.ToString() + "-" + FString::FromInt(NextQuestIndex++)));
	Data.PutName("identifier", Identifier);
}

UFlareQuest* UFlareQuestGenerator::GenerateSectorQuest(UFlareSimulatedSector* Sector)
{
	UFlareQuestGenerated* Quest = UFlareQuestGeneratedVipTransport::Create(this);
	QuestManager->AddQuest(Quest);
	GeneratedQuests.Add(Quest);
	QuestManager->LoadCallbacks(Quest);
	Quest->UpdateState();
	return Quest;
}

/*----------------------------------------------------
	Generated quest
----------------------------------------------------*/

UFlareQuestGenerated::UFlareQuestGenerated(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestGenerated::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	LoadInternal(Parent->GetQuestManager());
	QuestGenerator = Parent;
	InitData = Data;
}


/*----------------------------------------------------
	Generated VIP transport quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedVipTransport"
UFlareQuestGeneratedVipTransport::UFlareQuestGeneratedVipTransport(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedVipTransport::Create(UFlareQuestGenerator* Parent)
{
	UFlareQuestGeneratedVipTransport* Quest = NewObject<UFlareQuestGeneratedVipTransport>(Parent, UFlareQuestGeneratedVipTransport::StaticClass());
	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedVipTransport::GetClass(), Data);
	Quest->Load(Parent, Data);
	return Quest;
}

void UFlareQuestGeneratedVipTransport::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);
	QuestClass = UFlareQuestGeneratedVipTransport::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = LOCTEXT(QUEST_TAG"Name","VIP transport");
	QuestDescription = LOCTEXT(QUEST_TAG"Description","Transport a VIP from %s to %s");
	QuestCategory = EFlareQuestCategory::SECONDARY;

	// TODO remove, placeholder
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

}



#undef LOCTEXT_NAMESPACE
