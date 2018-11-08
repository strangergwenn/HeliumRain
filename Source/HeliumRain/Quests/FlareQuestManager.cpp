
#include "FlareQuestManager.h"
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareQuestCatalogEntry.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuestGenerator.h"
#include "FlareCatalogQuest.h"
#include "QuestCatalog/FlareTutorialQuest.h"
#include "QuestCatalog/FlareHistoryQuest.h"

#define LOCTEXT_NAMESPACE "FlareQuestManager"

DECLARE_CYCLE_STAT(TEXT("FlareQuestManager OnCallbackEvent"), STAT_FlareQuestManager_OnCallbackEvent, STATGROUP_Flare);


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
	AddQuest(UFlareQuestTutorialContracts::Create(this));
	AddQuest(UFlareQuestTutorialTechnology::Create(this));
	AddQuest(UFlareQuestTutorialBuildShip::Create(this));
	AddQuest(UFlareQuestTutorialBuildStation::Create(this));
	AddQuest(UFlareQuestTutorialResearchStation::Create(this));
	AddQuest(UFlareQuestTutorialRepairShip::Create(this));
	AddQuest(UFlareQuestTutorialRefillShip::Create(this));
	AddQuest(UFlareQuestTutorialFighter::Create(this));
	AddQuest(UFlareQuestTutorialSplitFleet::Create(this));
	AddQuest(UFlareQuestTutorialDistantFleet::Create(this));
	AddQuest(UFlareQuestTutorialMergeFleet::Create(this));
	AddQuest(UFlareQuestTutorialTradeRoute::Create(this));

	AddQuest(UFlareQuestPendulum::Create(this));
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
		if(Quest->GetQuestCategory() == EFlareQuestCategory::HISTORY
		||Quest->GetQuestCategory() == EFlareQuestCategory::TUTORIAL)
		{
			FLOGV("Found mandatory abandoned quest %s. Set it as pending.", *Quest->GetIdentifier().ToString());
			PendingQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::PENDING);
		}
		else
		{
			FLOGV("Found abandoned quest %s", *Quest->GetIdentifier().ToString());
			OldQuests.Add(Quest);
			Quest->SetStatus(EFlareQuestStatus::ABANDONED);
		}
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

	if(Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		OnEvent(FFlareBundle().PutTag("accept-contract"));
	}
}

void UFlareQuestManager::AbandonQuest(UFlareQuest* Quest)
{
	FLOGV("Abandon quest %s", *Quest->GetIdentifier().ToString());

	if(Quest->GetQuestCategory() != EFlareQuestCategory::HISTORY && Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		Quest->Abandon(false);
	}
	else
	{
		FLOGV("Cannot abandon quest %s", *Quest->GetIdentifier().ToString());
	}
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

	OnEvent(FFlareBundle().PutTag("track-contract"));
}

void UFlareQuestManager::AutoSelectQuest()
{
	if (OngoingQuests.Num()> 1)
	{
		SelectQuest(OngoingQuests[OngoingQuests.Num() - 1]);
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
	SCOPE_CYCLE_COUNTER(STAT_FlareQuestManager_OnCallbackEvent);
	
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

void UFlareQuestManager::OnSpacecraftDestroyed(UFlareSimulatedSpacecraft* Spacecraft, bool Uncontrollable, DamageCause Cause)
{
	if (CallbacksMap.Contains(EFlareQuestCallback::SPACECRAFT_DESTROYED))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EFlareQuestCallback::SPACECRAFT_DESTROYED];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->OnSpacecraftDestroyed(Spacecraft, Uncontrollable, Cause);
			Quest->UpdateState();
		}
	}

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

void UFlareQuestManager::OnEvent(FFlareBundle& Bundle)
{
	if (CallbacksMap.Contains(EFlareQuestCallback::QUEST_EVENT))
	{
		TArray<UFlareQuest*> Callbacks = CallbacksMap[EFlareQuestCallback::QUEST_EVENT];
		for (UFlareQuest* Quest: Callbacks)
		{
			Quest->OnEvent(Bundle);
			Quest->UpdateState();
		}
	}

	OnCallbackEvent(EFlareQuestCallback::SPACECRAFT_CAPTURED);
}


void UFlareQuestManager::OnNextDay()
{
	QuestGenerator->GenerateMilitaryQuests();

	NotifyNewQuests(NewQuestAccumulator);
	NewQuestAccumulator.Empty();

	OnCallbackEvent(EFlareQuestCallback::NEXT_DAY);
}

void UFlareQuestManager::NotifyNewQuests(TArray<UFlareQuest*>& QuestsToNotify)
{
	// New quest notification
	if (QuestsToNotify.Num() == 1)
	{
		UFlareQuest* Quest = QuestsToNotify[0];

		FText Text = LOCTEXT("New quest", "New contract available");
		FText Info = Quest->GetQuestName();

		FFlareMenuParameterData Data;
		Data.Quest = Quest;

		GetGame()->GetPC()->Notify(Text, Info, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-status")),
			EFlareNotification::NT_NewQuest, false, EFlareMenu::MENU_Quest, Data);
	}
	else if(QuestsToNotify.Num() > 1)
	{
		FText Text = LOCTEXT("New quests", "New contracts available");
		FText Info = FText::Format(LOCTEXT("NewQuestsInfos", "{0} new contracts available"), QuestsToNotify.Num());

		FFlareMenuParameterData Data;
		Data.Quest = QuestsToNotify[0];

		GetGame()->GetPC()->Notify(Text, Info, FName("new-quest"),
			EFlareNotification::NT_NewQuest, false, EFlareMenu::MENU_Quest, Data);
	}
}


void UFlareQuestManager::OnTravelEnded(UFlareFleet* Fleet)
{
	if (Fleet == Game->GetPC()->GetPlayerFleet())
	{
		// Player end travel, try to generate a quest in the destination sector
		QuestGenerator->GenerateSectorQuest(Fleet->GetCurrentSector());

		NotifyNewQuests(NewQuestAccumulator);
		NewQuestAccumulator.Empty();

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
		OnEvent(FFlareBundle().PutTag("success-contract"));
	}

	if (Quest == SelectedQuest)
	{
		AutoSelectQuest();
	}

	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestFail(UFlareQuest* Quest, bool Notify)
{
	FLOGV("Quest %s is now failed", *Quest->GetIdentifier().ToString())
	OngoingQuests.Remove(Quest);
	AvailableQuests.Remove(Quest);
	PendingQuests.Remove(Quest);
	OldQuests.Add(Quest);

	// Quest failed notification
	if (Notify && Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
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
	if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL && Quest->GetQuestCategory() != EFlareQuestCategory::SECONDARY)
	{
		TArray<UFlareQuest*> QuestsToNotify;
		QuestsToNotify.Add(Quest);
		NotifyNewQuests(QuestsToNotify);
	}
	else if (Quest->GetQuestCategory() != EFlareQuestCategory::TUTORIAL)
	{
		NewQuestAccumulator.Add(Quest);
	}

	OnQuestStatusChanged(Quest);
}

void UFlareQuestManager::OnQuestOngoing(UFlareQuest* Quest)
{
	FLOGV("Quest %s is now ongoing", *Quest->GetIdentifier().ToString())
	AvailableQuests.Remove(Quest);
	OngoingQuests.Add(Quest);
	
	if (!SelectedQuest)
	{
		SelectQuest(Quest);
	}
	OnQuestStatusChanged(Quest);
}

int32 UFlareQuestManager::GetReservedCapacity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource)
{
	int32 ReservedCapacity = 0;

	for(UFlareQuest* Quest: OngoingQuests)
	{
		ReservedCapacity += Quest->GetReservedCapacity(Station, Resource);
	}

	for(UFlareQuest* Quest: AvailableQuests)
	{
		ReservedCapacity += Quest->GetReservedCapacity(Station, Resource);
	}

	return ReservedCapacity;
}

int32 UFlareQuestManager::GetReservedQuantity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource)
{
	int32 ReservedQuantity = 0;

	for(UFlareQuest* Quest: OngoingQuests)
	{
		ReservedQuantity += Quest->GetReservedQuantity(Station, Resource);
	}

	for(UFlareQuest* Quest: AvailableQuests)
	{
		ReservedQuantity += Quest->GetReservedQuantity(Station, Resource);
	}

	return ReservedQuantity;
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

	for(UFlareQuest* Quest: PendingQuests)
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

bool UFlareQuestManager::IsInterestingMeteorite(FFlareMeteoriteSave& Meteorite)
{
	UFlareSimulatedSpacecraft* Station =  Game->GetGameWorld()->FindSpacecraft(Meteorite.TargetStation);

	if(!Station)
	{
		return false;
	}

	if(Station->GetCompany() == Game->GetPC()->GetCompany())
	{
		return true;
	}

	for(UFlareQuest* OngoingQuest : GetOngoingQuests())
	{
		UFlareQuestGeneratedMeteoriteInterception* MeteoriteQuest = Cast<UFlareQuestGeneratedMeteoriteInterception>(OngoingQuest);
		if(MeteoriteQuest)
		{
			if(MeteoriteQuest->GetInitData()->GetName("target-station") == Meteorite.TargetStation)
			{
				return true;
			}
		}
	}

	return false;
}

bool UFlareQuestManager::IsTradeQuestUseStation(UFlareSimulatedSpacecraft* Station)
{
	if(!Station)
	{
		return false;
	}

	FName StationId = Station->GetImmatriculation();

	auto IsUseStation = [&](UFlareQuest* Quest)
	{
		UFlareQuestGeneratedResourceTrade* TradeQuest = Cast<UFlareQuestGeneratedResourceTrade>(Quest);
		if(TradeQuest)
		{
			return TradeQuest->GetInitData()->GetName("station1") == StationId || TradeQuest->GetInitData()->GetName("station2") == StationId;
		}

		UFlareQuestGeneratedResourceSale* SaleQuest = Cast<UFlareQuestGeneratedResourceSale>(Quest);
		if(SaleQuest)
		{
			return SaleQuest->GetInitData()->GetName("station") == StationId;
		}

		UFlareQuestGeneratedResourcePurchase* PurchaseQuest = Cast<UFlareQuestGeneratedResourcePurchase>(Quest);
		if(PurchaseQuest)
		{
			return PurchaseQuest->GetInitData()->GetName("station") == StationId;
		}
		return false;
	};

	for(UFlareQuest* OngoingQuest : GetOngoingQuests())
	{
		if(IsUseStation(OngoingQuest))
		{
			return true;
		}
	}

	for(UFlareQuest* AvailableQuest : GetAvailableQuests())
	{
		if(IsUseStation(AvailableQuest))
		{
			return true;
		}
	}

	return false;
}

bool UFlareQuestManager::IsUnderMilitaryContract(UFlareSimulatedSector* Sector,  UFlareCompany* Company, bool IncludeCache)
{
	if(IsUnderMilitaryContractNoCache(Sector, Company))
	{
		for(IsUnderMilitaryContractCacheEntry& CacheEntry : IsUnderMilitaryContractCache)
		{
			if(CacheEntry.Sector == Sector && CacheEntry.Company == Company)
			{
				CacheEntry.ExpireTime = FPlatformTime::Seconds() + 10.f;
				return true;
			}
		}

		IsUnderMilitaryContractCacheEntry NewEntry;
		NewEntry.ExpireTime = FPlatformTime::Seconds() + 10.f;
		NewEntry.Sector = Sector;
		NewEntry.Company = Company;
		IsUnderMilitaryContractCache.Add(NewEntry);

		return true;
	}
	else if(!IncludeCache)
	{
		return false;
	}
	else
	{
		int Index = 0;

		for(IsUnderMilitaryContractCacheEntry& CacheEntry : IsUnderMilitaryContractCache)
		{
			if(CacheEntry.Sector == Sector && CacheEntry.Company == Company)
			{
				if(CacheEntry.ExpireTime > FPlatformTime::Seconds())
				{
					return true;
				}
				else
				{
					IsUnderMilitaryContractCache.RemoveAt(Index);
					break;
				}
			}
			Index++;
		}


		return false;
	}
}

bool UFlareQuestManager::IsMilitaryTarget(UFlareSimulatedSpacecraft const* Spacecraft, bool IncludeCache)
{
	if(IsMilitaryTargetNoCache(Spacecraft))
	{
		for(IsMilitaryTargetCacheEntry& CacheEntry : IsMilitaryTargetCache)
		{
			if(CacheEntry.Spacecraft == Spacecraft)
			{
				CacheEntry.ExpireTime = FPlatformTime::Seconds() + 10.f;
				return true;
			}
		}

		IsMilitaryTargetCacheEntry NewEntry;
		NewEntry.ExpireTime = FPlatformTime::Seconds();
		NewEntry.Spacecraft = Spacecraft;
		IsMilitaryTargetCache.Add(NewEntry);

		return true;
	}
	else if(!IncludeCache)
	{
		return false;
	}
	else
	{
		int Index = 0;
		for(IsMilitaryTargetCacheEntry& CacheEntry : IsMilitaryTargetCache)
		{
			if(CacheEntry.Spacecraft == Spacecraft)
			{
				if(CacheEntry.ExpireTime > FPlatformTime::Seconds())
				{
					return true;
				}
				else
				{
					IsMilitaryTargetCache.RemoveAt(Index);
					break;
				}
			}
			Index++;
		}
		return false;
	}
}

bool UFlareQuestManager::IsUnderMilitaryContractNoCache(UFlareSimulatedSector* Sector,  UFlareCompany* Company)
{
	for(UFlareQuest* OngoingQuest : GetOngoingQuests())
	{
		UFlareQuestGeneratedCargoHunt2* CargoHunt = Cast<UFlareQuestGeneratedCargoHunt2>(OngoingQuest);
		if(CargoHunt)
		{

			if(CargoHunt->GetInitData()->GetName("hostile-company") != Company->GetIdentifier())
			{
				continue;
			}

			bool TargetLargeCargo = CargoHunt->GetInitData()->GetInt32("large-cargo") > 0;

			for(UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
			{
				if(Ship->GetCompany() != Company)
				{
					continue;
				}

				if(Ship->IsMilitary())
				{
					continue;
				}

				if(TargetLargeCargo == (Ship->GetSize() == EFlarePartSize::L))
				{
					return true;
				}
			}
		}

		UFlareQuestGeneratedMilitaryHunt2* MilitaryHunt = Cast<UFlareQuestGeneratedMilitaryHunt2>(OngoingQuest);
		if(MilitaryHunt)
		{
			if(MilitaryHunt->GetInitData()->GetName("hostile-company") != Company->GetIdentifier())
			{
				continue;
			}


			for(UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
			{
				if(Ship->GetCompany() != Company)
				{
					continue;
				}

				if(!Ship->IsMilitary())
				{
					continue;
				}

				return true;
			}
		}

		UFlareQuestGeneratedStationDefense2* StationDefense = Cast<UFlareQuestGeneratedStationDefense2>(OngoingQuest);
		if(StationDefense)
		{
			if(StationDefense->GetInitData()->GetName("hostile-company") != Company->GetIdentifier())
			{
				continue;
			}

			if(StationDefense->GetInitData()->GetName("sector") != Sector->GetIdentifier())
			{
				continue;
			}

			return true;
		}

		UFlareQuestGeneratedJoinAttack2* JoinAttack = Cast<UFlareQuestGeneratedJoinAttack2>(OngoingQuest);
		if(JoinAttack)
		{

			if(JoinAttack->GetInitData()->GetName("sector") != Sector->GetIdentifier())
			{
				continue;
			}

			TArray<FName> HostileCompanyNames = JoinAttack->GetInitData()->GetNameArray("hostile-companies");
			for(FName HostileCompanyName : HostileCompanyNames)
			{
				if(HostileCompanyName  == Company->GetIdentifier())
				{
					return true;
				}
			}

			continue;
		}

		UFlareQuestGeneratedSectorDefense2* SectorDefense = Cast<UFlareQuestGeneratedSectorDefense2>(OngoingQuest);
		if(SectorDefense)
		{
			if(SectorDefense->GetInitData()->GetName("hostile-company") != Company->GetIdentifier())
			{
				continue;
			}

			if(SectorDefense->GetInitData()->GetName("sector") != Sector->GetIdentifier())
			{
				continue;
			}

			return true;
		}
	}

	return false;
}

bool UFlareQuestManager::IsMilitaryTargetNoCache(UFlareSimulatedSpacecraft const* Spacecraft)
{
	for(UFlareQuest* OngoingQuest : GetOngoingQuests())
	{
		UFlareQuestGeneratedCargoHunt2* CargoHunt = Cast<UFlareQuestGeneratedCargoHunt2>(OngoingQuest);
		if(CargoHunt)
		{
			if(Spacecraft->IsMilitary())
			{
		continue;
			}

			if(CargoHunt->GetInitData()->GetName("hostile-company") != Spacecraft->GetCompany()->GetIdentifier())
			{
				continue;
			}

			bool TargetLargeCargo = CargoHunt->GetInitData()->GetInt32("large-cargo") > 0;

			if(TargetLargeCargo == (Spacecraft->GetSize() == EFlarePartSize::L))
			{
				return true;
			}
		}

		UFlareQuestGeneratedMilitaryHunt2* MilitaryHunt = Cast<UFlareQuestGeneratedMilitaryHunt2>(OngoingQuest);
		if(MilitaryHunt)
		{
			if(!Spacecraft->IsMilitary())
			{
				continue;
			}

			if(MilitaryHunt->GetInitData()->GetName("hostile-company") != Spacecraft->GetCompany()->GetIdentifier())
			{
				continue;
			}

			return true;
		}
	}
	return false;
}

bool UFlareQuestManager::IsAllowedToDestroy(UFlareSimulatedSpacecraft const* Spacecraft)
{
	for(UFlareQuest* OngoingQuest : GetOngoingQuests())
	{
		UFlareQuestGeneratedCargoHunt2* CargoHunt = Cast<UFlareQuestGeneratedCargoHunt2>(OngoingQuest);
		if(CargoHunt)
		{
			if(Spacecraft->IsMilitary())
			{
		continue;
			}

			if(CargoHunt->GetInitData()->GetName("hostile-company") != Spacecraft->GetCompany()->GetIdentifier())
			{
				continue;
			}

			bool TargetLargeCargo = CargoHunt->GetInitData()->GetInt32("large-cargo") > 0;
			bool RequestDestroyTarget = CargoHunt->GetInitData()->GetInt32("destroy-cargo") > 0;


			if(RequestDestroyTarget && TargetLargeCargo == (Spacecraft->GetSize() == EFlarePartSize::L))
			{
				return true;
			}
		}

		UFlareQuestGeneratedMilitaryHunt2* MilitaryHunt = Cast<UFlareQuestGeneratedMilitaryHunt2>(OngoingQuest);
		if(MilitaryHunt)
		{
			if(!Spacecraft->IsMilitary())
			{
				continue;
			}

			if(MilitaryHunt->GetInitData()->GetName("hostile-company") != Spacecraft->GetCompany()->GetIdentifier())
			{
				continue;
			}

			bool RequestDestroyTarget =  MilitaryHunt->GetInitData()->GetInt32("destroy") > 0;

			return RequestDestroyTarget;
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
