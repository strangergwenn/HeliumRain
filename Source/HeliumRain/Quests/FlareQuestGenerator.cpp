
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
#include "../Economy/FlareCargoBay.h"
#include "../Game/FlareSectorHelper.h"

#define LOCTEXT_NAMESPACE "FlareQuestGenerator"

#define MAX_QUEST_COUNT 15
#define MAX_QUEST_PER_COMPANY_COUNT 5

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
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourceSale::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourceSale>(this, UFlareQuestGeneratedResourceSale::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourcePurchase::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourcePurchase>(this, UFlareQuestGeneratedResourcePurchase::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourceTrade::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourceTrade>(this, UFlareQuestGeneratedResourceTrade::StaticClass());
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

void UFlareQuestGenerator::GenerateSectorQuest(UFlareSimulatedSector* Sector)
{
	TArray<UFlareCompany*> CompaniesToProcess =  Game->GetGameWorld()->GetCompanies();
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	// For each company in random order
	while (CompaniesToProcess.Num() > 0)
	{
		int CompanyIndex = FMath::RandRange(0, CompaniesToProcess.Num()-1);
		UFlareCompany* Company = CompaniesToProcess[CompanyIndex];
		CompaniesToProcess.Remove(Company);

		if(Company == PlayerCompany)
		{
			continue;
		}

		FLOGV("GenerateSectorQuest for %s", *Company->GetCompanyName().ToString());

		// Propability to give a quest is :
		// 100 % if quest count = 0
		// 0 % if quest count = MAX_QUEST_COUNT
		// With early quick drop
		// So pow(1-questCount/MAX_QUEST_COUNT, 2)
		float CompanyQuestProbability = FMath::Pow (1.f - (float) QuestManager->GetVisibleQuestCount(Company)/ (float) MAX_QUEST_PER_COMPANY_COUNT, 2);
		float GlobalQuestProbability = FMath::Pow (1.f - (float) QuestManager->GetVisibleQuestCount()/ (float) MAX_QUEST_COUNT, 2);

		float QuestProbability = GlobalQuestProbability * CompanyQuestProbability;


		FLOGV("CompanyQuestProbability %f", CompanyQuestProbability);
		FLOGV("GlobalQuestProbability %f", GlobalQuestProbability);
		FLOGV("QuestProbability before rep %f", QuestProbability);


		// Then make proba variable with the reputation :
		// if rep > 0 : 100 %
		// if rep < 0 : 100 % + rep (negative, so 0% at 0)

		if(Company->GetReputation(PlayerCompany) < 0)
		{
			QuestProbability += Company->GetReputation(PlayerCompany) / 100.f;
		}

		FLOGV("QuestProbability %f", QuestProbability);

		// Rand
		if (FMath::FRand() > QuestProbability)
		{
			// No luck, no quest this time
			continue;
		}

		// Generate a quest
		UFlareQuestGenerated* Quest = NULL;
		if (FMath::FRand() < 0.15)
		{
			FLOG("VIP");
			Quest = UFlareQuestGeneratedVipTransport::Create(this, Sector, Company);
		}

		// Trade quest
		// Try to create a trade quest,
		// if not, try to create a Purchase quest
		// if not, try to create a Sell quest
		if (!Quest)
		{
			Quest = UFlareQuestGeneratedResourceTrade::Create(this, Sector, Company);
		}

		if (!Quest)
		{
			Quest = UFlareQuestGeneratedResourcePurchase::Create(this, Sector, Company);
		}

		if (!Quest)
		{
			Quest = UFlareQuestGeneratedResourceSale::Create(this, Sector, Company);
		}

		if (!Quest && FMath::FRand() < 0.3)
		{
			Quest = UFlareQuestGeneratedVipTransport::Create(this, Sector, Company);
		}

		// Register quest
		if (Quest)
		{
			QuestManager->AddQuest(Quest);
			GeneratedQuests.Add(Quest);
			QuestManager->LoadCallbacks(Quest);
			Quest->UpdateState();
		}

	}
}

bool UFlareQuestGenerator::FindUniqueTag(FName Tag)
{
	for(UFlareQuestGenerated* Quest : GeneratedQuests)
	{
		EFlareQuestStatus::Type Status = Quest->GetStatus();
		if (Status == EFlareQuestStatus::AVAILABLE || Status == EFlareQuestStatus::ONGOING)
		{
			if (Quest->GetInitData()->HasTag(Tag))
			{
				return true;
			}
		}
	}

	return false;
}

FName UFlareQuestGenerator::GenerateVipTag(UFlareSimulatedSpacecraft* SourceSpacecraft)
{
	return FName(*(FString("vip-")+SourceSpacecraft->GetImmatriculation().ToString()));

}

FName UFlareQuestGenerator::GenerateTradeTag(UFlareSimulatedSpacecraft* SourceSpacecraft, FFlareResourceDescription* Resource)
{
	return FName(*(FString("trade-")+SourceSpacecraft->GetImmatriculation().ToString()+"-"+Resource->Identifier.ToString()));
}

FName UFlareQuestGenerator::GenerateDefenseTag(UFlareSimulatedSector* Sector, UFlareCompany* OwnerCompany, UFlareCompany* HostileCompany)
{
	return FName(*(FString("defense-")+Sector->GetIdentifier().ToString()+"-"+OwnerCompany->GetIdentifier().ToString()+"-"+HostileCompany->GetIdentifier().ToString()));
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

UFlareQuestGenerated* UFlareQuestGeneratedVipTransport::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

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

		// Check unicity
		if (Parent->FindUniqueTag(Parent->GenerateVipTag(CandidateStation)))
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
	Data.PutTag(Parent->GenerateVipTag(Station1));
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
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Pick {0} up at {1} in {2}"), VIPName, FText::FromString(Station1->GetImmatriculation().ToString()), FText::FromString(Sector1->GetSectorName().ToString()));
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
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Drop {0} off at {1} in {2}"), VIPName, FText::FromString(Station2->GetImmatriculation().ToString()), FText::FromString(Sector2->GetSectorName().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "drop-off", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station2);
		Condition->TargetShipMatchId = PickUpShipId;
		Step->GetEndConditions().Add(Condition);

		Step->GetFailConditions().Add(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, NULL, PickUpShipId));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector2));

		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));

	ExpirationConditions.Add(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(Station1->GetCompany(), true);
	SetupGenericReward(Data);
}

/*----------------------------------------------------
	Generated resource sale quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedResourceSale"
UFlareQuestGeneratedResourceSale::UFlareQuestGeneratedResourceSale(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourceSale::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

		// Check friendlyness
		if (CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company
			continue;
		}

		// Check if have too much stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Output)
			{
				// Not an output resource
				continue;
			}

			if (Slot.Quantity <= CandidateStation->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO)
			{
				// Not enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}

			// Check if player kwown at least one buyer of this resource
			if(!PlayerCompany->HasKnowResourceInput(Slot.Resource))
			{
				continue;
			}

			// It's a good candidate
			CandidateStations.Add(CandidateStation);
		}
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station = CandidateStations[CandidateIndex];

	// Find a resource

	int32 BestResourceExcessQuantity = 0;
	int32 BestResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;

	for (FFlareCargo& Slot : Station->GetCargoBay()->GetSlots())
	{
		if (Slot.Lock != EFlareResourceLock::Output)
		{
			// Not an output resource
			continue;
		}


		int32 ExcessResourceQuantity = Slot.Quantity - Station->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO;

		if (ExcessResourceQuantity <= 0)
		{
			// Not enought resources
			continue;
		}

		// Check if player kwown at least one buyer of this resource
		if(!PlayerCompany->HasKnowResourceInput(Slot.Resource))
		{
			continue;
		}

		if (ExcessResourceQuantity > BestResourceExcessQuantity)
		{
			BestResourceExcessQuantity = ExcessResourceQuantity;
			BestResourceQuantity = FMath::Min(Slot.Quantity, int32(Station->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO));
			BestResource = Slot.Resource;
		}
	}

	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PlayerCompany->GetTransportCapacity());

	// Setup reward
	int64 QuestValue = 1000 * QuestResourceQuantity;

	// Create the quest
	UFlareQuestGeneratedResourceSale* Quest = NewObject<UFlareQuestGeneratedResourceSale>(Parent, UFlareQuestGeneratedResourceSale::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourceSale::GetClass(), Data);

	Data.PutName("station", Station->GetImmatriculation());
	Data.PutName("sector", Station->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station, BestResource));

	CreateGenericReward(Data, QuestValue);

	Quest->Load(Parent, Data);

	return Quest;
}

void UFlareQuestGeneratedResourceSale::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));
	UFlareSimulatedSpacecraft* Station = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourceSale::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT(QUEST_TAG"Name","{0} sale in {1}"), Resource->Name, Sector->GetSectorName());
	QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionFormat","Buy {0} {1} from {2} at {3}"),
									 FText::AsNumber(Quantity), Resource->Name, FText::FromName(Station->GetImmatriculation()), Sector->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuyResource"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Buy {0} {1} from {2} at {3}"),
										  FText::AsNumber(Quantity), Resource->Name, FText::FromName(Station->GetImmatriculation()), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "buy", Description);

		UFlareQuestConditionBuyAtStation* Condition = UFlareQuestConditionBuyAtStation::Create(this, QUEST_TAG"cond1", Station, Resource, Quantity);

		Step->GetEndConditions().Add(Condition);
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	ExpirationConditions.Add(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(Station->GetCompany(), true);
	SetupGenericReward(Data);
}

/*----------------------------------------------------
	Generated resource purchase quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedResourcePurchase"
UFlareQuestGeneratedResourcePurchase::UFlareQuestGeneratedResourcePurchase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourcePurchase::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

		// Check friendlyness
		if (CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company
			continue;
		}

		// Check if have too low stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Input)
			{
				// Not an input resource
				continue;
			}

			if (Slot.Quantity >= CandidateStation->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO)
			{
				// Enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}

			// Check if player kwown at least one seller of this resource
			if(!PlayerCompany->HasKnowResourceOutput(Slot.Resource))
			{
				continue;
			}

			// It's a good candidate
			CandidateStations.Add(CandidateStation);
		}
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station = CandidateStations[CandidateIndex];

	// Find a resource

	int32 BestResourceMissingQuantity = 0;
	int32 BestResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;

	for (FFlareCargo& Slot : Station->GetCargoBay()->GetSlots())
	{
		if (Slot.Lock != EFlareResourceLock::Input)
		{
			// Not an input resource
			continue;
		}


		int32 MissingResourceQuantity = Station->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO - Slot.Quantity;

		if (MissingResourceQuantity <= 0)
		{
			// Enought resources
			continue;
		}

		// Check if player kwown at least one seller of this resource
		if(!PlayerCompany->HasKnowResourceOutput(Slot.Resource))
		{
			continue;
		}

		if (MissingResourceQuantity > BestResourceQuantity)
		{
			BestResourceMissingQuantity = MissingResourceQuantity;
			BestResourceQuantity = FMath::Min(Station->GetCargoBay()->GetSlotCapacity() - Slot.Quantity, int32(Station->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO));
			BestResource = Slot.Resource;
		}


	}

	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PlayerCompany->GetTransportCapacity());


	// Setup reward
	int64 QuestValue = 1000 * QuestResourceQuantity;

	// Create the quest
	UFlareQuestGeneratedResourcePurchase* Quest = NewObject<UFlareQuestGeneratedResourcePurchase>(Parent, UFlareQuestGeneratedResourcePurchase::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourcePurchase::GetClass(), Data);

	Data.PutName("station", Station->GetImmatriculation());
	Data.PutName("sector", Station->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station, BestResource));
	CreateGenericReward(Data, QuestValue);

	Quest->Load(Parent, Data);

	return Quest;
}

void UFlareQuestGeneratedResourcePurchase::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));
	UFlareSimulatedSpacecraft* Station = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourcePurchase::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT(QUEST_TAG"Name","{0} purchase in {1}"), Resource->Name, Sector->GetSectorName());
	QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionFormat","Sell {0} {1} to {2} at {3}"),
									 FText::AsNumber(Quantity), Resource->Name, FText::FromName(Station->GetImmatriculation()), Sector->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuyResource"
		FText Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"Description", "Sell {0} {1} to {2} at {3}"),
										  FText::AsNumber(Quantity), Resource->Name, FText::FromName(Station->GetImmatriculation()), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "sell", Description);

		UFlareQuestConditionSellAtStation* Condition = UFlareQuestConditionSellAtStation::Create(this, QUEST_TAG"cond1", Station, Resource, Quantity);

		Step->GetEndConditions().Add(Condition);
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	ExpirationConditions.Add(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(Station->GetCompany(), true);
	SetupGenericReward(Data);
}

/*----------------------------------------------------
	Generated resource trade quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedResourceTrade"
UFlareQuestGeneratedResourceTrade::UFlareQuestGeneratedResourceTrade(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourceTrade::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Find a best friendly station in sector
	TMap<FFlareResourceDescription*, int32> BestQuantityToBuyPerResource;
	TMap<FFlareResourceDescription*, UFlareSimulatedSpacecraft*> BestStationToBuyPerResource;
	TMap<FFlareResourceDescription*, int32> BestQuantityToSellPerResource;
	TMap<FFlareResourceDescription*, UFlareSimulatedSpacecraft*> BestStationToSellPerResource;

	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		// Check friendlyness
		if (CandidateStation->GetCompany()->GetWarState(Company) != EFlareHostility::Owned)
		{
			continue;
		}

		// Check if have too much stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Output)
			{
				// Not an output resource
				continue;
			}

			int32 AvailableResourceQuantity = Slot.Quantity - CandidateStation->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO;

			if (AvailableResourceQuantity <= 0)
			{
				// Not enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}

			// It's a good candidate
			if(!BestQuantityToBuyPerResource.Contains(Slot.Resource) || BestQuantityToBuyPerResource[Slot.Resource] > AvailableResourceQuantity)
			{
				// Best candidate
				BestQuantityToBuyPerResource.Add(Slot.Resource, AvailableResourceQuantity);
				BestStationToBuyPerResource.Add(Slot.Resource, CandidateStation);
			}
		}
	}

	for(UFlareCompany* Company2: Parent->GetGame()->GetGameWorld()->GetCompanies())
	{
		if (Company2 == PlayerCompany || Company2->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		for (UFlareSimulatedSpacecraft* CandidateStation : Company2->GetCompanyStations())
		{

			// Check if have too much stock there is another distant station

			for (FFlareCargo& Slot : CandidateStation->GetCargoBay()->GetSlots())
			{
				if (Slot.Lock != EFlareResourceLock::Input)
				{
					// Not an input resource
					continue;
				}


				int32 MissingResourceQuantity = CandidateStation->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO - Slot.Quantity;

				if (MissingResourceQuantity <= 0)
				{
					// Enought resources
					continue;
				}

				// Check unicity
				if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
				{
					continue;
				}

				// It's a good candidate
				if(!BestQuantityToSellPerResource.Contains(Slot.Resource) || BestQuantityToSellPerResource[Slot.Resource] > MissingResourceQuantity)
				{
					// Best candidate
					BestQuantityToSellPerResource.Add(Slot.Resource, MissingResourceQuantity);
					BestStationToSellPerResource.Add(Slot.Resource, CandidateStation);
				}
			}
		}
	}


	// All maps are full, now, pick the best candidate
	int32 BestNeededResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;
	UFlareSimulatedSpacecraft* Station1 = NULL;
	UFlareSimulatedSpacecraft* Station2 = NULL;

	for (UFlareResourceCatalogEntry* ResourcePointer : Parent->GetGame()->GetResourceCatalog()->GetResourceList())
	{
		FFlareResourceDescription* Resource = &ResourcePointer->Data;

		if (BestQuantityToBuyPerResource.Contains(Resource) && BestQuantityToSellPerResource.Contains(Resource))
		{
			int32 MinQuantity = FMath::Min(BestQuantityToBuyPerResource[Resource], BestQuantityToSellPerResource[Resource]);
			if(MinQuantity > BestNeededResourceQuantity)
			{
				BestNeededResourceQuantity = MinQuantity;
				BestResource = Resource;
			}
		}
	}
	if(BestResource == NULL)
	{
		return NULL;
	}
	Station1 = BestStationToBuyPerResource[BestResource];
	Station2 = BestStationToSellPerResource[BestResource];
	int32 BestBuyResourceQuantity = FMath::Min(Station1->GetCargoBay()->GetResourceQuantity(BestResource, PlayerCompany), int32(Station1->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO));
	int32 BestSellResourceQuantity = FMath::Min(Station2->GetCargoBay()->GetSlotCapacity() - Station2->GetCargoBay()->GetResourceQuantity(BestResource, PlayerCompany), int32(Station2->GetCargoBay()->GetSlotCapacity() * AI_NERF_RATIO));

	int32 BestResourceQuantity = FMath::Min(BestBuyResourceQuantity, BestSellResourceQuantity);
	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PlayerCompany->GetTransportCapacity());



	// Setup reward
	int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Parent->GetGame()->GetGameWorld(), Station1->GetCurrentSector(), Station2->GetCurrentSector());
	int64 QuestValue = 2000 * QuestResourceQuantity * TravelDuration;

	// Create the quest
	UFlareQuestGeneratedResourceTrade* Quest = NewObject<UFlareQuestGeneratedResourceTrade>(Parent, UFlareQuestGeneratedResourceTrade::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourceTrade::GetClass(), Data);

	Data.PutName("station1", Station1->GetImmatriculation());
	Data.PutName("sector1", Station1->GetCurrentSector()->GetIdentifier());
	Data.PutName("station2", Station2->GetImmatriculation());
	Data.PutName("sector2", Station2->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station1->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station1, BestResource));
	Data.PutTag(Parent->GenerateTradeTag(Station2, BestResource));
	CreateGenericReward(Data, QuestValue);

	Quest->Load(Parent, Data);

	return Quest;
}

void UFlareQuestGeneratedResourceTrade::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector1 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector1"));
	UFlareSimulatedSector* Sector2 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector2"));
	UFlareSimulatedSpacecraft* Station1 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station1"));
	UFlareSimulatedSpacecraft* Station2 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station2"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourceTrade::GetClass();
	Identifier = InitData.GetName("identifier");
	if(Sector1 == Sector2)
	{
		QuestName = FText::Format(LOCTEXT(QUEST_TAG"NameLocal","{0} trade in {1}"), Resource->Name, Sector1->GetSectorName());
		QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionLocalFormat","Trade {0} {1} in {2}"),
									 FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName());
	}
	else
	{
		QuestName = FText::Format(LOCTEXT(QUEST_TAG"NameDistant","{0} trade to {1}"), Resource->Name,
								  Sector2->GetSectorName());
		QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionDistantFormat","Trade {0} {1} from {2} to {3}"),
									 FText::AsNumber(Quantity), Resource->Name,
										 Sector1->GetSectorName(), Sector2->GetSectorName());
	}
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TradeResource"
		FText Description;
		if(Sector1 == Sector2)
		{
			Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"DescriptionLocal", "Trade {0} {1} in {2}"),
										  FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName());
		}
		else
		{
			Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"DescriptionDistant", "Trade {0} {1} from {2} to {3}"),
										  FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName(), Sector2->GetSectorName());
		}
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "trade", Description);

		UFlareQuestConditionBuyAtStation* Condition1 = UFlareQuestConditionBuyAtStation::Create(this, QUEST_TAG"cond1", Station1, Resource, Quantity);
		Step->GetEndConditions().Add(Condition1);
		UFlareQuestConditionSellAtStation* Condition2 = UFlareQuestConditionSellAtStation::Create(this, QUEST_TAG"cond2", Station2, Resource, Quantity);
		Step->GetEndConditions().Add(Condition2);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector1));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector2));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));
	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));
	AddGlobalFailCondition(UFlareQuestConditionAtWar::Create(this, Station2->GetCompany()));

	ExpirationConditions.Add(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(Station1->GetCompany(), true);
	SetupGenericReward(Data);
}

/*----------------------------------------------------
	Generated station defense quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedStationDefense"
UFlareQuestGeneratedStationDefense::UFlareQuestGeneratedStationDefense(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedStationDefense::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateDefenseTag(Sector, Company, HostileCompany)))
	{
		return NULL;
	}

	int64 WarPrice;

	if (HostileCompany->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		WarPrice = 0;
	}
	else
	{
		WarPrice = 1000 * (HostileCompany->GetReputation(PlayerCompany) + 100);
	}

	int32 PreferredPlayerCombatPoints= FMath::Max(1, int32(PlayerCompany->GetCompanyValue().ArmyCombatPoints /3));


	int32 NeedArmyCombatPoints= SectorHelper::GetHostileArmyCombatPoints(Sector, Company) - SectorHelper::GetCompanyArmyCombatPoints(Sector, Company);


	int32 RequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);

	int64 ArmyPrice = RequestedArmyCombatPoints  * 250;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice;

	// Create the quest
	UFlareQuestGeneratedStationDefense* Quest = NewObject<UFlareQuestGeneratedStationDefense>(Parent, UFlareQuestGeneratedStationDefense::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedStationDefense::GetClass(), Data);
	Data.PutName("sector", Sector->GetIdentifier());
	Data.PutInt32("army-combat-points", RequestedArmyCombatPoints );
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutName("hostile-company", HostileCompany->GetIdentifier());
	Data.PutTag(Parent->GenerateDefenseTag(Sector, Company, HostileCompany));
	CreateGenericReward(Data, QuestValue);

	Quest->Load(Parent, Data);

	return Quest;
}

void UFlareQuestGeneratedStationDefense::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");

	QuestClass = UFlareQuestGeneratedStationDefense::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT(QUEST_TAG"NameLocal","Defend {0} against {1}"), Sector->GetSectorName(), HostileCompany->GetCompanyName());
	QuestDescription = FText::Format(LOCTEXT(QUEST_TAG"DescriptionLocalFormat","Defend stations of {0} in {1} againts {2} with a army of {3} combat points"),
								 FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName(), FText::AsNumber(ArmyCombatPoints ));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"DescriptionDistant", "Attack {0} in {1} with a force of {2}"),
										  HostileCompany->GetCompanyName(), Sector->GetSectorName(), FText::AsNumber(ArmyCombatPoints ));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);

		UFlareQuestConditionMinArmyCombatPointsInSector* Condition1 = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
		Step->GetEndConditions().Add(Condition1);
		UFlareQuestConditionAtWar* Condition2 = UFlareQuestConditionAtWar::Create(this, HostileCompany);
		Step->GetEndConditions().Add(Condition2);

		UFlareQuestConditionStationLostInSector* FailCondition1 = UFlareQuestConditionStationLostInSector::Create(this, Sector, FriendlyCompany);
		Step->GetFailConditions().Add(FailCondition1);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		// TODO if peace, sucesse

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Defend"
		FText Description;

		Description = FText::Format(LOCTEXT(QUEST_STEP_TAG"DescriptionDistant", "Defend {0} in {1} with your force againts {2}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "defend", Description);

		UFlareQuestConditionNoCapturingStationInSector* Condition1 = UFlareQuestConditionNoCapturingStationInSector::Create(this, Sector, FriendlyCompany, HostileCompany);
		Step->GetEndConditions().Add(Condition1);

		UFlareQuestConditionMaxArmyCombatPointsInSector* Condition2 = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
		Condition2->SetOr(true);
		Step->GetEndConditions().Add(Condition2);
		// TODO OR !


		// TODO if peace, sucesse

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, FriendlyCompany);
		Step->GetFailConditions().Add(FailCondition1);
		UFlareQuestConditionAtPeace* FailCondition2 = UFlareQuestConditionAtPeace::Create(this, HostileCompany);
		Step->GetFailConditions().Add(FailCondition2);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}


	AddGlobalFailCondition(UFlareQuestConditionAtWar::Create(this, FriendlyCompany));

	ExpirationConditions.Add(UFlareQuestConditionStationLostInSector::Create(this,  Sector, FriendlyCompany));
	ExpirationConditions.Add(UFlareQuestConditionNoStationInCapture::Create(this,  Sector, FriendlyCompany));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);
}

#undef LOCTEXT_NAMESPACE
