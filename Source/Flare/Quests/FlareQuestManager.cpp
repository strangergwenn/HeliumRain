
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareQuestCatalogEntry.h"
#include "FlareQuestManager.h"

#define LOCTEXT_NAMESPACE "FlareQuestManager"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestManager::UFlareQuestManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

void UFlareQuestManager::Load(const FFlareQuestSave& Data)
{
	Game = Cast<AFlareGame>(GetOuter());

	QuestData = Data;

	TArray<FName> CurrentQuestIdentifiers;
	for (int QuestProgressIndex = 0; QuestProgressIndex <Data.QuestProgresses.Num(); QuestProgressIndex++)
	{
		CurrentQuestIdentifiers.Add(Data.QuestProgresses[QuestProgressIndex].QuestIdentifier);
	}

	// Load quests
	for (int QuestIndex = 0; QuestIndex <Game->GetQuestCatalog()->Quests.Num(); QuestIndex++)
	{

		FFlareQuestDescription* QuestDescription = &(Game->GetQuestCatalog()->Quests[QuestIndex]->Data);

		if(QuestDescription->Category == EFlareQuestCategory::TUTORIAL && !QuestData.PlayTutorial)
		{
			// Skip tutorial quests.
			continue;
		}


		// Create the quest
		UFlareQuest* Quest = NewObject<UFlareQuest>(this, UFlareQuest::StaticClass());
		Quest->Load(QuestDescription);

		// Find quest status
		int QuestProgressIndex = CurrentQuestIdentifiers.IndexOfByKey(QuestDescription->Identifier);
		if (QuestProgressIndex != INDEX_NONE)
		{
			// Current quests
			AvailableQuests.Add(Quest);
			Quest->Restore(Data.QuestProgresses[QuestProgressIndex]);
			if (Data.SelectedQuest == QuestDescription->Identifier)
			{
				SelectedQuest = Quest;
			}
		}
		else if (Data.SuccessfulQuests.Contains(QuestDescription->Identifier))
		{
			OldQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::SUCCESSFUL);
		}
		else if (Data.AbandonnedQuests.Contains(QuestDescription->Identifier))
		{
			OldQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::ABANDONNED);
		}
		else if (Data.FailedQuests.Contains(QuestDescription->Identifier))
		{
			OldQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::FAILED);
		}
		else
		{
			AvailableQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::AVAILABLE);
		}

		LoadCallbacks(Quest);
	}
}


FFlareQuestSave* UFlareQuestManager::Save()
{
	QuestData.QuestProgresses.Empty();
	QuestData.SuccessfulQuests.Empty();
	QuestData.AbandonnedQuests.Empty();
	QuestData.FailedQuests.Empty();

	QuestData.SelectedQuest = (SelectedQuest ? SelectedQuest->GetIdentifier() : NAME_None);

	for (int QuestIndex = 0; QuestIndex < ActiveQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = ActiveQuests[QuestIndex];
		FFlareQuestProgressSave* QuestProgressSave = Quest->Save();
		QuestData.QuestProgresses.Add(*QuestProgressSave);
	}

	for (int QuestIndex = 0; QuestIndex < OldQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = OldQuests[QuestIndex];
		switch (Quest->GetStatus())
		{
			case EFlareQuestStatus::SUCCESSFUL:
				QuestData.SuccessfulQuests.Add(Quest->GetIdentifier());
				break;
			case EFlareQuestStatus::ABANDONNED:
				QuestData.AbandonnedQuests.Add(Quest->GetIdentifier());
				break;
			case EFlareQuestStatus::FAILED:
				QuestData.FailedQuests.Add(Quest->GetIdentifier());
				break;
			default:
				FLOGV("Bad status %d for quest %s", (int) (Quest->GetStatus() + 0), *Quest->GetIdentifier().ToString());
		}
	}

	return &QuestData;
}

/*----------------------------------------------------
	Callback
----------------------------------------------------*/

void UFlareQuestManager::LoadCallbacks(UFlareQuest* Quest)
{
	ClearCallbacks(Quest);

	TArray<EFlareQuestCallback::Type> Callbacks = Quest->GetCurrentCallbacks();

	for (int i = 0; i < Callbacks.Num(); i++)
	{
		switch(Callbacks[i])
		{
			case EFlareQuestCallback::FLY_SHIP:
				FlyShipCallback.Add(Quest);
				break;
			case EFlareQuestCallback::TICK_FLYING:
				TickFlying.Add(Quest);
				break;
		case EFlareQuestCallback::QUEST:
				QuestCallback.Add(Quest);
				break;
			default:
				FLOGV("Bad callback type %d for quest %s", (int) (Callbacks[i] + 0), *Quest->GetIdentifier().ToString());
		}
	}
}

void UFlareQuestManager::ClearCallbacks(UFlareQuest* Quest)
{
	TickFlying.Remove(Quest);
	FlyShipCallback.Remove(Quest);
}

void UFlareQuestManager::OnTick(float DeltaSeconds)
{
	if(GetGame()->GetActiveSector())
	{
		// Tick TickFlying callback only if there is an active sector
		for (int i = 0; i < TickFlying.Num(); i++)
		{
			TickFlying[i]->OnTick(DeltaSeconds);
		}
	}
}

void UFlareQuestManager::OnFlyShip(AFlareSpacecraft* Ship)
{
	for (int i = 0; i < FlyShipCallback.Num(); i++)
	{
		FlyShipCallback[i]->OnFlyShip(Ship);
	}
}

void UFlareQuestManager::OnQuestStatusChanged(UFlareQuest* Quest)
{
	LoadCallbacks(Quest);

	for (int i = 0; i < QuestCallback.Num(); i++)
	{
		QuestCallback[i]->OnQuestStatusChanged(Quest);
	}
}

void UFlareQuestManager::OnQuestSuccess(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now successful", *Quest->GetIdentifier().ToString())
	ActiveQuests.Remove(Quest);
	OldQuests.Add(Quest);
	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestFail(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now failed", *Quest->GetIdentifier().ToString())
	ActiveQuests.Remove(Quest);
	OldQuests.Add(Quest);
	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestActivation(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now active", *Quest->GetIdentifier().ToString())
	AvailableQuests.Remove(Quest);
	ActiveQuests.Add(Quest);
	OnQuestStatusChanged(Quest);
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


bool UFlareQuestManager::IsQuestSuccesfull(FName QuestIdentifier)
{
	for (int QuestIndex = 0; QuestIndex < OldQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = OldQuests[QuestIndex];
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			if (Quest->GetStatus() == EFlareQuestStatus::SUCCESSFUL)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

bool UFlareQuestManager::IsQuestFailed(FName QuestIdentifier)
{
	for (int QuestIndex = 0; QuestIndex < OldQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = OldQuests[QuestIndex];
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			if (Quest->GetStatus() == EFlareQuestStatus::FAILED)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}


#undef LOCTEXT_NAMESPACE
