
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareQuestCatalogEntry.h"
#include "../Player/FlarePlayerController.h"
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

	TArray<FName> ActiveQuestIdentifiers;
	for (int QuestProgressIndex = 0; QuestProgressIndex <Data.QuestProgresses.Num(); QuestProgressIndex++)
	{
		ActiveQuestIdentifiers.Add(Data.QuestProgresses[QuestProgressIndex].QuestIdentifier);
	}

	// Load quests
	for (int QuestIndex = 0; QuestIndex <Game->GetQuestCatalog()->Quests.Num(); QuestIndex++)
	{

		FFlareQuestDescription* QuestDescription = &(Game->GetQuestCatalog()->Quests[QuestIndex]->Data);

		if (QuestDescription->Category == EFlareQuestCategory::TUTORIAL && !QuestData.PlayTutorial)
		{
			// Skip tutorial quests.
			continue;
		}


		// Create the quest
		UFlareQuest* Quest = NewObject<UFlareQuest>(this, UFlareQuest::StaticClass());
		Quest->Load(QuestDescription);

		// Find quest status
		int QuestProgressIndex = ActiveQuestIdentifiers.IndexOfByKey(QuestDescription->Identifier);
		if (QuestProgressIndex != INDEX_NONE)
		{
			// Current quests
			ActiveQuests.Add(Quest);
			Quest->Restore(Data.QuestProgresses[QuestProgressIndex]);
			if (Data.SelectedQuest == QuestDescription->Identifier)
			{
				SelectQuest(Quest);
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
		Quest->UpdateState();
	}
	if (!SelectedQuest)
	{
		AutoSelectQuest();
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
				TickFlyingCallback.Add(Quest);
				break;
			case EFlareQuestCallback::SECTOR_VISITED:
				SectorVisitedCallback.Add(Quest);
				break;
			case EFlareQuestCallback::SECTOR_ACTIVE:
				SectorActiveCallback.Add(Quest);
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
	TickFlyingCallback.Remove(Quest);
	FlyShipCallback.Remove(Quest);
}

void UFlareQuestManager::OnTick(float DeltaSeconds)
{
	if (GetGame()->GetActiveSector())
	{
		// Tick TickFlying callback only if there is an active sector
		for (int i = 0; i < TickFlyingCallback.Num(); i++)
		{
			TickFlyingCallback[i]->OnTick(DeltaSeconds);
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

void UFlareQuestManager::OnSectorActivation(UFlareSimulatedSector* Sector)
{
	for (int i = 0; i < SectorActiveCallback.Num(); i++)
	{
		SectorActiveCallback[i]->OnSectorActivation(Sector);
	}
}

void UFlareQuestManager::OnSectorVisited(UFlareSimulatedSector* Sector)
{
	for (int i = 0; i < SectorVisitedCallback.Num(); i++)
	{
		SectorVisitedCallback[i]->OnSectorVisited(Sector);
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

	// Quest successful notification
	FText Text = LOCTEXT("Quest successful", "Quest successful");
	FText Info = Quest->GetQuestName();
	GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-")+Quest->GetIdentifier().ToString()+"-status"), EFlareNotification::NT_Quest));

	if (Quest == SelectedQuest)
	{
		AutoSelectQuest();
	}

	OnQuestStatusChanged(Quest);

}

void UFlareQuestManager::OnQuestFail(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now failed", *Quest->GetIdentifier().ToString())
	ActiveQuests.Remove(Quest);
	OldQuests.Add(Quest);

	// Quest failed notification
	FText Text = LOCTEXT("Quest failed", "Quest failed");
	FText Info = Quest->GetQuestName();
	GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-")+Quest->GetIdentifier().ToString()+"-status"), EFlareNotification::NT_Quest));

	if (Quest == SelectedQuest)
	{
		AutoSelectQuest();
	}
	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestActivation(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now active", *Quest->GetIdentifier().ToString())
	AvailableQuests.Remove(Quest);
	ActiveQuests.Add(Quest);

	// New quest notification
	FText Text = LOCTEXT("New quest", "New quest available");
	FText Info = Quest->GetQuestName();
	GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-")+Quest->GetIdentifier().ToString()+"-status"), EFlareNotification::NT_Quest));

	if (!SelectedQuest)
	{
		SelectQuest(Quest);
	}
	OnQuestStatusChanged(Quest);
}


/*----------------------------------------------------
	Quest management
----------------------------------------------------*/

void UFlareQuestManager::SelectQuest(UFlareQuest* Quest)
{
	FLOGV("Select quest %s", *Quest->GetIdentifier().ToString());
	if (!IsQuestActive(Quest->GetIdentifier()))
	{
		FLOGV("ERROR: Fail to select quest %s. The quest to select must be active", *Quest->GetIdentifier().ToString());
		return;
	}

	if (SelectedQuest)
	{
		SelectedQuest->StopObjectiveTracking();
	}

	SelectedQuest = Quest;
	SelectedQuest->StartObjectiveTracking();
}

void UFlareQuestManager::AutoSelectQuest()
{
	if (ActiveQuests.Num()> 1)
	{
		SelectQuest(ActiveQuests[0]);
	}
	else
	{
		if (SelectedQuest)
		{
			SelectedQuest->StopObjectiveTracking();
		}
		SelectedQuest = NULL;
	}

}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

bool UFlareQuestManager::IsQuestActive(FName QuestIdentifier)
{
	for (int QuestIndex = 0; QuestIndex < ActiveQuests.Num(); QuestIndex++)
	{
		UFlareQuest* Quest = ActiveQuests[QuestIndex];
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			return true;
		}
	}
	return false;
}

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
