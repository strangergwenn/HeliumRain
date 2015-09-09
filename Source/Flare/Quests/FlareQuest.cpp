
#include "Flare.h"
#include "FlareQuest.h"

#define LOCTEXT_NAMESPACE "FlareQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuest::UFlareQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/


void UFlareQuest::Load(const FFlareQuestDescription* Description)
{
	QuestDescription = Description;
	QuestStatus = EFlareQuestStatus::AVAILABLE;
}

void UFlareQuest::Restore(const FFlareQuestProgressSave& Data)
{
	QuestData = Data;
	QuestStatus = EFlareQuestStatus::ACTIVE;
}

FFlareQuestProgressSave* UFlareQuest::Save()
{
	// TODO Save
	return &QuestData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareQuest::SetStatus(EFlareQuestStatus::Type Status)
{
	QuestStatus = Status;
}

#undef LOCTEXT_NAMESPACE
