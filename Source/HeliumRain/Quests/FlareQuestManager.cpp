
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareQuestCatalogEntry.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuestGenerator.h"
#include "FlareQuestManager.h"
#include "FlareCatalogQuest.h"
#include "QuestCatalog/FlareTutorialQuest.h"

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

	// Init the quest manager
	QuestGenerator = NewObject<UFlareQuestGenerator>(this, UFlareQuestGenerator::StaticClass());
	QuestGenerator->Load(this, Data);

	QuestData = Data;

	ActiveQuestIdentifiers.Empty();
	for (int QuestProgressIndex = 0; QuestProgressIndex <Data.QuestProgresses.Num(); QuestProgressIndex++)
	{
		ActiveQuestIdentifiers.Add(Data.QuestProgresses[QuestProgressIndex].QuestIdentifier);
	}

	LoadBuildinQuest();

	LoadCatalogQuests();

	LoadDynamicQuests();


	for(UFlareQuest* Quest: Quests)
	{
		LoadCallbacks(Quest);
		Quest->UpdateState();
	}

	if (!SelectedQuest)
	{
		AutoSelectQuest();
	}
}

void UFlareQuestManager::LoadBuildinQuest()
{
	AddQuest(UFlareQuestTutorialFlying::Create(this));
	AddQuest(UFlareQuestTutorialNavigation::Create(this));
}

void UFlareQuestManager::LoadCatalogQuests()
{
	for (int QuestIndex = 0; QuestIndex <Game->GetQuestCatalog()->Quests.Num(); QuestIndex++)
	{
		FFlareQuestDescription* QuestDescription = &(Game->GetQuestCatalog()->Quests[QuestIndex]->Data);

		// Create the quest
		UFlareCatalogQuest* Quest = NewObject<UFlareCatalogQuest>(this, UFlareCatalogQuest::StaticClass());
		Quest->Load(this, QuestDescription);

		AddQuest(Quest);
	}
}

void UFlareQuestManager::LoadDynamicQuests()
{
	QuestGenerator->LoadQuests(QuestData);
}

void UFlareQuestManager::AddQuest(UFlareQuest* Quest)
{
	int QuestProgressIndex = ActiveQuestIdentifiers.IndexOfByKey(Quest->GetIdentifier());

	// Setup quest index
	Quest->SetupIndexes();

	// Skip tutorial quests.
	if (Quest->GetQuestCategory() == EFlareQuestCategory::TUTORIAL && !QuestData.PlayTutorial)
	{
		FLOGV("Found skipped tutorial quest %s", *Quest->GetIdentifier().ToString());
		OldQuests.Add(Quest);
		Quest->SetStatus(EFlareQuestStatus::SUCCESSFUL);
	}
	else if (QuestProgressIndex != INDEX_NONE)
	{
		// Active quests
		Quest->Restore(QuestData.QuestProgresses[QuestProgressIndex]);

		if (Quest->GetStatus() == EFlareQuestStatus::PENDING)
		{
			FLOGV("Found pending quest %s", *Quest->GetIdentifier().ToString());
			PendingQuests.Add(Quest);
		}
		else if (Quest->GetStatus() == EFlareQuestStatus::AVAILABLE)
		{
			FLOGV("Found available quest %s", *Quest->GetIdentifier().ToString());
			AvailableQuests.Add(Quest);
		}
		else if(Quest->GetStatus() == EFlareQuestStatus::ONGOING)
		{
			FLOGV("Found ongoing quest %s", *Quest->GetIdentifier().ToString());
			OngoingQuests.Add(Quest);

			if (QuestData.SelectedQuest == Quest->GetIdentifier())
			{
				SelectQuest(Quest);
			}
		}
	}
	else if (QuestData.SuccessfulQuests.Contains(Quest->GetIdentifier()))
	{
		FLOGV("Found completed quest %s", *Quest->GetIdentifier().ToString());
		OldQuests.Add(Quest);
		Quest->SetStatus(EFlareQuestStatus::SUCCESSFUL);
	}
	else if (QuestData.AbandonedQuests.Contains(Quest->GetIdentifier()))
	{
		FLOGV("Found abandoned quest %s", *Quest->GetIdentifier().ToString());
		OldQuests.Add(Quest);
		Quest->SetStatus(EFlareQuestStatus::ABANDONED);
	}
	else if (QuestData.FailedQuests.Contains(Quest->GetIdentifier()))
	{
		FLOGV("Found failed quest %s", *Quest->GetIdentifier().ToString());
		OldQuests.Add(Quest);
		Quest->SetStatus(EFlareQuestStatus::FAILED);
	}
	else
	{
		FLOGV("Found new pending quest %s", *Quest->GetIdentifier().ToString());
		PendingQuests.Add(Quest);
		Quest->SetStatus(EFlareQuestStatus::PENDING);
	}

	Quests.Add(Quest);
}


FFlareQuestSave* UFlareQuestManager::Save()
{
	QuestData.QuestProgresses.Empty();
	QuestData.SuccessfulQuests.Empty();
	QuestData.AbandonedQuests.Empty();
	QuestData.FailedQuests.Empty();

	QuestData.SelectedQuest = (SelectedQuest ? SelectedQuest->GetIdentifier() : NAME_None);

	for (UFlareQuest* Quest: Quests)
	{
		if(!Quest->IsActive())
		{
			continue;
		}

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
			case EFlareQuestStatus::ABANDONED:
				QuestData.AbandonedQuests.Add(Quest->GetIdentifier());
				break;
			case EFlareQuestStatus::FAILED:
				QuestData.FailedQuests.Add(Quest->GetIdentifier());
				break;
			default:
				FLOGV("Bad status %d for quest %s", (int) (Quest->GetStatus() + 0), *Quest->GetIdentifier().ToString());
		}
	}

	QuestGenerator->Save(QuestData);

	return &QuestData;
}


/*----------------------------------------------------
	Quest management
----------------------------------------------------*/

void UFlareQuestManager::AcceptQuest(UFlareQuest* Quest)
{
	FLOGV("Accept quest %s", *Quest->GetIdentifier().ToString());

	Quest->Accept();
}

void UFlareQuestManager::AbandonQuest(UFlareQuest* Quest)
{
	FLOGV("Abandon quest %s", *Quest->GetIdentifier().ToString());

	Quest->Abandon();
}

void UFlareQuestManager::SelectQuest(UFlareQuest* Quest)
{
	FLOGV("Select quest %s", *Quest->GetIdentifier().ToString());
	if (!IsQuestOngoing(Quest))
	{
		FLOGV("ERROR: Fail to select quest %s. The quest to select must be ongoing", *Quest->GetIdentifier().ToString());
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
	if (OngoingQuests.Num()> 1)
	{
		SelectQuest(OngoingQuests[0]);
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
	Callbacks
----------------------------------------------------*/

void UFlareQuestManager::LoadCallbacks(UFlareQuest* Quest)
{
	ClearCallbacks(Quest);

	TArray<EFlareQuestCallback::Type> Callbacks = Quest->GetCurrentCallbacks();

	for (int i = 0; i < Callbacks.Num(); i++)
	{
		CallbacksMap.FindOrAdd(Callbacks[i]).Add(Quest);

		/*switch (Callbacks[i])
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
		case EFlareQuestCallback::SHIP_DOCKED:
			ShipDockedCallback.Add(Quest);
			break;
		case EFlareQuestCallback::QUEST:
			QuestCallback.Add(Quest);
			break;
		default:
			FLOGV("Bad callback type %d for quest %s", (int)(Callbacks[i] + 0), *Quest->GetIdentifier().ToString());
		}*/
	}
}

void UFlareQuestManager::ClearCallbacks(UFlareQuest* Quest)
{
	for (auto& Elem : CallbacksMap)
	{
		Elem.Value.Remove(Quest);
	}
}

void UFlareQuestManager::OnCallbackEvent(EFlareQuestCallback::Type EventType)
{
	if (CallbacksMap.Contains(EventType))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EventType];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->UpdateState();
		}
	}
}

void UFlareQuestManager::OnTick(float DeltaSeconds)
{
	if (GetGame()->GetActiveSector())
	{
		// Tick TickFlying callback only if there is an active sector
		OnCallbackEvent(EFlareQuestCallback::TICK_FLYING);
	}
}

void UFlareQuestManager::OnFlyShip(AFlareSpacecraft* Ship)
{
	OnCallbackEvent(EFlareQuestCallback::FLY_SHIP);
}

void UFlareQuestManager::OnSectorActivation(UFlareSimulatedSector* Sector)
{
	OnCallbackEvent(EFlareQuestCallback::SECTOR_ACTIVE);
}

void UFlareQuestManager::OnSectorVisited(UFlareSimulatedSector* Sector)
{
	OnCallbackEvent(EFlareQuestCallback::SECTOR_VISITED);
}

void UFlareQuestManager::OnShipDocked(UFlareSimulatedSpacecraft* Station, UFlareSimulatedSpacecraft* Ship)
{
	OnCallbackEvent(EFlareQuestCallback::SHIP_DOCKED);
}

void UFlareQuestManager::OnWarStateChanged(UFlareCompany* Company1, UFlareCompany* Company2)
{
	OnCallbackEvent(EFlareQuestCallback::WAR_STATE_CHANGED);
}

void UFlareQuestManager::OnSpacecraftDestroyed(UFlareSimulatedSpacecraft* Spacecraft)
{
	OnCallbackEvent(EFlareQuestCallback::SPACECRAFT_DESTROYED);
}

void UFlareQuestManager::OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 Quantity)
{
	if (CallbacksMap.Contains(EFlareQuestCallback::TRADE_DONE))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EFlareQuestCallback::TRADE_DONE];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->OnTradeDone(SourceSpacecraft, DestinationSpacecraft, Resource, Quantity);
			Quest->UpdateState();
		}
	}

	OnCallbackEvent(EFlareQuestCallback::TRADE_DONE);
}

void UFlareQuestManager::OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter)
{
	if (CallbacksMap.Contains(EFlareQuestCallback::SPACECRAFT_CAPTURED))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EFlareQuestCallback::SPACECRAFT_CAPTURED];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->OnSpacecraftCaptured(CapturedSpacecraftBefore, CapturedSpacecraftAfter);
			Quest->UpdateState();
		}
	}

	OnCallbackEvent(EFlareQuestCallback::SPACECRAFT_CAPTURED);
}


void UFlareQuestManager::OnTravelStarted(UFlareTravel* Travel)
{
	if (CallbacksMap.Contains(EFlareQuestCallback::TRAVEL_STARTED))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EFlareQuestCallback::TRAVEL_STARTED];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->OnTravelStarted(Travel);
			Quest->UpdateState();
		}
	}

	OnCallbackEvent(EFlareQuestCallback::SPACECRAFT_CAPTURED);
}

void UFlareQuestManager::OnNextDay()
{
	OnCallbackEvent(EFlareQuestCallback::NEXT_DAY);
}

void UFlareQuestManager::OnTravelEnded(UFlareFleet* Fleet)
{
	if (Fleet == Game->GetPC()->GetPlayerFleet())
	{
		// Player end travel, try to generate a quest in the destination sector
		QuestGenerator->GenerateSectorQuest(Fleet->GetCurrentSector());
	}
}

void UFlareQuestManager::OnQuestStatusChanged(UFlareQuest* Quest)
{
	LoadCallbacks(Quest);

	OnCallbackEvent(EFlareQuestCallback::QUEST_CHANGED);
}

void UFlareQuestManager::OnQuestSuccess(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now successful", *Quest->GetIdentifier().ToString())
		OngoingQuests.Remove(Quest);
	OldQuests.Add(Quest);

	// Quest successful notification
	if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		FText Text = LOCTEXT("Quest successful", "Contract successful");
		FText Info = Quest->GetQuestName();

		FFlareMenuParameterData Data;
		Data.Quest = Quest;

		GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-status")),
			EFlareNotification::NT_Quest, false, EFlareMenu::MENU_Quest, Data);
	}

	if (Quest == SelectedQuest)
	{
		AutoSelectQuest();
	}

	OnQuestStatusChanged(Quest);

}

void UFlareQuestManager::OnQuestFail(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now failed", *Quest->GetIdentifier().ToString())
	OngoingQuests.Remove(Quest);
	AvailableQuests.Remove(Quest);
	PendingQuests.Remove(Quest);
	OldQuests.Add(Quest);

	// Quest failed notification
	if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		FText Text = LOCTEXT("Quest failed", "Contract failed");
		FText Info = Quest->GetQuestName();

		FFlareMenuParameterData Data;
		Data.Quest = Quest;

		GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-status")),
			EFlareNotification::NT_Quest, false, EFlareMenu::MENU_Quest, Data);
	}

	if (Quest == SelectedQuest)
	{
		AutoSelectQuest();
	}
	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestAvailable(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now available", *Quest->GetIdentifier().ToString())
	PendingQuests.Remove(Quest);
	AvailableQuests.Add(Quest);

	// New quest notification
	if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		FText Text = LOCTEXT("New quest", "New contract available");
		FText Info = Quest->GetQuestName();

		FFlareMenuParameterData Data;
		Data.Quest = Quest;

		GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-status")),
			EFlareNotification::NT_Quest, false, EFlareMenu::MENU_Quest, Data);
	}

	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestOngoing(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now ongoing", *Quest->GetIdentifier().ToString())
	AvailableQuests.Remove(Quest);
	OngoingQuests.Add(Quest);

	// New quest notification
	if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		FText Text = LOCTEXT("New quest", "New contract started");
		FText Info = Quest->GetQuestName();

		FFlareMenuParameterData Data;
		Data.Quest = Quest;

		GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-status")),
			EFlareNotification::NT_Quest, false, EFlareMenu::MENU_Quest, Data);
	}

	if (!SelectedQuest)
	{
		SelectQuest(Quest);
	}
	OnQuestStatusChanged(Quest);
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

bool UFlareQuestManager::IsQuestOngoing(UFlareQuest* Quest)
{
	return OngoingQuests.Contains(Quest);
}

bool UFlareQuestManager::IsOldQuest(UFlareQuest* Quest)
{
	return IsQuestSuccessfull(Quest) || IsQuestFailed(Quest);
}

bool UFlareQuestManager::IsQuestAvailable(UFlareQuest* Quest)
{
	return AvailableQuests.Contains(Quest);
}

bool UFlareQuestManager::IsQuestSuccessfull(UFlareQuest* Quest)
{
	for(UFlareQuest* OldQuest: OldQuests)
	{
		if (OldQuest == Quest)
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

bool UFlareQuestManager::IsQuestFailed(UFlareQuest* Quest)
{
	for(UFlareQuest* OldQuest: OldQuests)
	{
		if (OldQuest == Quest)
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

UFlareQuest* UFlareQuestManager::FindQuest(FName QuestIdentifier)
{
	for(UFlareQuest* Quest: OngoingQuests)
	{
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			return Quest;
		}
	}

	for(UFlareQuest* Quest: OldQuests)
	{
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			return Quest;
		}
	}

	for(UFlareQuest* Quest: AvailableQuests)
	{
		if (Quest->GetIdentifier() == QuestIdentifier)
		{
			return Quest;
		}
	}

	return NULL;
}

int32 UFlareQuestManager::GetVisibleQuestCount()
{
	return AvailableQuests.Num() + OngoingQuests.Num();
}

int32 UFlareQuestManager::GetVisibleQuestCount(UFlareCompany* Client)
{
	int32 QuestCount = 0;

	for(UFlareQuest* Quest: AvailableQuests)
	{
		if (Quest->GetClient() == Client)
		{
			QuestCount++;
		}
	}

	for(UFlareQuest* Quest: OngoingQuests)
	{
		if (Quest->GetClient() == Client)
		{
			QuestCount++;
		}
	}

	return QuestCount;
}


#undef LOCTEXT_NAMESPACE
