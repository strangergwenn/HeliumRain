
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuestGenerator.h"
#include "FlareQuestManager.h"
#include "FlareQuestCondition.h"
#include "FlareQuest.h"
#include "FlareQuestStep.h"
#include "FlareQuestAction.h"

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

FText UFlareQuestGenerator::GeneratePersonName()
{
	TArray<FText> Names;
	
	Names.Add(FText::FromString("Arya"));
	Names.Add(FText::FromString("Bob"));
	Names.Add(FText::FromString("Curtis"));
	Names.Add(FText::FromString("David"));
	Names.Add(FText::FromString("Elisa"));
	Names.Add(FText::FromString("France"));
	Names.Add(FText::FromString("Gaius"));
	Names.Add(FText::FromString("Hans"));
	Names.Add(FText::FromString("Ivy"));
	Names.Add(FText::FromString("Jebediah"));
	Names.Add(FText::FromString("Katia"));
	Names.Add(FText::FromString("Lucy"));
	Names.Add(FText::FromString("Mary"));
	Names.Add(FText::FromString("Nate"));
	Names.Add(FText::FromString("Olly"));
	Names.Add(FText::FromString("Paula"));
	Names.Add(FText::FromString("Quinn"));
	Names.Add(FText::FromString("Robb"));
	Names.Add(FText::FromString("Saul"));
	Names.Add(FText::FromString("Tilda"));
	Names.Add(FText::FromString("Ulysse"));
	Names.Add(FText::FromString("Viktor"));
	Names.Add(FText::FromString("Walter"));
	Names.Add(FText::FromString("Xavier"));
	Names.Add(FText::FromString("Yann"));
	Names.Add(FText::FromString("Zoe"));

	return Names[FMath::RandHelper(Names.Num() - 1)];
}

void UFlareQuestGenerator::GenerateIdentifer(FName QuestClass, FFlareBundle& Data)
{
	FName Identifier = FName(*(QuestClass.ToString() + "-" + FString::FromInt(NextQuestIndex++)));
	Data.PutName("identifier", Identifier);
}

UFlareQuest* UFlareQuestGenerator::GenerateSectorQuest(UFlareSimulatedSector* Sector)
{
	UFlareQuestGenerated* Quest = UFlareQuestGeneratedVipTransport::Create(this, Sector);
	if (Quest)
	{
		QuestManager->AddQuest(Quest);
		GeneratedQuests.Add(Quest);
		QuestManager->LoadCallbacks(Quest);
		Quest->UpdateState();
	}
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

void UFlareQuestGenerated::CreateGenericReward(FFlareBundle& Data, int64 QuestValue)
{
	// TODO more reward
	Data.PutInt32("reward-money", QuestValue);

	// Reputation penalty
	Data.PutInt32("penalty-reputation", -FMath::Sqrt(QuestValue / 1000));
}

void UFlareQuestGenerated::AddGlobalFailCondition(UFlareQuestCondition* Condition)
{
	for(UFlareQuestStep* Step : GetSteps())
	{
		Step->GetFailConditions().Add(Condition);
	}
}

void UFlareQuestGenerated::SetupQuestGiver(UFlareCompany* Company, bool AddWarCondition)
{
	Client = Company;
	if (AddWarCondition)
	{
		AddGlobalFailCondition(UFlareQuestConditionAtWar::Create(this, Company));
	}
}

void UFlareQuestGenerated::SetupGenericReward(const FFlareBundle& Data)
{

	if (Data.HasInt32("reward-money"))
	{
		int64 Amount = Data.GetInt32("reward-money");
		GetSuccessActions().Add(UFlareQuestActionGiveMoney::Create(this, Client, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Amount));
	}

	if (Data.HasInt32("penalty-reputation"))
	{
		int64 Amount = Data.GetInt32("penalty-reputation");
		GetFailActions().Add(UFlareQuestActionReputationChange::Create(this, Client, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Amount));
	}
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

UFlareQuestGenerated* UFlareQuestGeneratedVipTransport::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		// Check friendlyness
		if (CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company
			continue;
		}

		// Check if there is another distant station
		bool HasDistantSector = false;
		for(UFlareSimulatedSpacecraft* CompanyStation : CandidateStation->GetCompany()->GetCompanyStations())
		{
			if(CompanyStation->GetCurrentSector() != Sector)
			{
				HasDistantSector = true;
				break;
			}
		}

		if (!HasDistantSector)
		{
			continue;
		}

		// It's a good
		CandidateStations.Add(CandidateStation);
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station1 = CandidateStations[CandidateIndex];

	// Find second station candidate
	TArray<UFlareSimulatedSpacecraft*> CandidateStations2;
	for(UFlareSimulatedSpacecraft* CompanyStation : Station1->GetCompany()->GetCompanyStations())
	{
		if(CompanyStation->GetCurrentSector() != Sector)
		{
			CandidateStations2.Add(CompanyStation);
		}
	}

	int32 Candidate2Index = FMath::RandRange(0, CandidateStations2.Num()-1);
	UFlareSimulatedSpacecraft* Station2 = CandidateStations2[Candidate2Index];

	// Setup reward
	int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Parent->GetGame()->GetGameWorld(), Station1->GetCurrentSector(), Station2->GetCurrentSector());
	int64 QuestValue = 20000 * TravelDuration;

	// Create the quest
	UFlareQuestGeneratedVipTransport* Quest = NewObject<UFlareQuestGeneratedVipTransport>(Parent, UFlareQuestGeneratedVipTransport::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedVipTransport::GetClass(), Data);
	Data.PutString("vip-name", UFlareQuestGenerator::GeneratePersonName().ToString());
	Data.PutName("station-1", Station1->GetImmatriculation());
	Data.PutName("station-2", Station2->GetImmatriculation());
	Data.PutName("sector-1", Station1->GetCurrentSector()->GetIdentifier());
	Data.PutName("sector-2", Station2->GetCurrentSector()->GetIdentifier());
	Data.PutName("client", Station1->GetCompany()->GetIdentifier());
	CreateGenericReward(Data, QuestValue);

	Quest->Load(Parent, Data);

	return Quest;
}

void UFlareQuestGeneratedVipTransport::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector1 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector-1"));
	UFlareSimulatedSector* Sector2 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector-2"));
	UFlareSimulatedSpacecraft* Station1 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station-1"));
	UFlareSimulatedSpacecraft* Station2 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station-2"));


	FText VIPName = FText::FromString(Data.GetString("vip-name"));

	QuestClass = UFlareQuestGeneratedVipTransport::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT(QUEST_TAG"Name","VIP transport : {0}"), VIPName);
	QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionFormat","Transport {0} from {1} to {2}"), VIPName, Sector1->GetSectorName(), Sector2->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;


	FName PickUpShipId = "pick-up-ship-id";

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PickUp"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Pick-up {0} at {1} in {2}"), VIPName, FText::FromString(Station1->GetImmatriculation().ToString()), FText::FromString(Sector1->GetSectorName().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pick-up", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station1);
		Condition->TargetShipSaveId = PickUpShipId;

		Step->GetEndConditions().Add(Condition);
		Step->GetFailConditions().Add(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector1));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Drop-off"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Drop-off {0} at {1} in {2}"), VIPName, FText::FromString(Station2->GetImmatriculation().ToString()), FText::FromString(Sector2->GetSectorName().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "drop-off", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station2);
		Condition->TargetShipMatchId = PickUpShipId;
		Step->GetEndConditions().Add(Condition);

		Step->GetFailConditions().Add(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, NULL, PickUpShipId));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector2));

		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));

	SetupQuestGiver(Station1->GetCompany(), true);
	SetupGenericReward(Data);
}

#undef LOCTEXT_NAMESPACE
