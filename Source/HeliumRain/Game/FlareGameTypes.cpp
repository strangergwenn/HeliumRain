
#include "FlareGameTypes.h"
#include "../UI/FlareUITypes.h"
#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Game/FlareGame.h"
#include "../Economy/FlareFactory.h"
#include "../Data/FlareFactoryCatalogEntry.h"
#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Game/FlareGameTools.h"

#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameTypes::UFlareGameTypes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Text
----------------------------------------------------*/

FText UFlareGameTypes::GetCombatGroupDescription(EFlareCombatGroup::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = LOCTEXT("AllMilitary",  "All military ships");   break;
		case EFlareCombatGroup::Capitals:      Result = LOCTEXT("AllCapitals",  "Capital ships");        break;
		case EFlareCombatGroup::Fighters:      Result = LOCTEXT("AllFighters",  "Fighters");             break;
		case EFlareCombatGroup::Civilan:       Result = LOCTEXT("AllCivilians", "Freighters");           break;
	}

	return Result;
}

FText UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatTactic::ProtectMe:        Result = LOCTEXT("ProtectMe",       "Protect me");         break;
		case EFlareCombatTactic::AttackMilitary:   Result = LOCTEXT("AttackMilitary",  "Attack military");    break;
		case EFlareCombatTactic::AttackStations:   Result = LOCTEXT("AttackStations",  "Attack stations");    break;
		case EFlareCombatTactic::AttackCivilians:  Result = LOCTEXT("AttackCivilians", "Attack freighters");  break;
		case EFlareCombatTactic::StandDown:        Result = LOCTEXT("StandDown",       "Stand down");         break;
	}

	return Result;
}

const FSlateBrush* UFlareGameTypes::GetCombatGroupIcon(EFlareCombatGroup::Type Type)
{
	const FSlateBrush* Result = NULL;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = FFlareStyleSet::GetIcon("AllMilitary");   break;
		case EFlareCombatGroup::Capitals:      Result = FFlareStyleSet::GetIcon("AllCapitals");   break;
		case EFlareCombatGroup::Fighters:      Result = FFlareStyleSet::GetIcon("AllFighters");   break;
		case EFlareCombatGroup::Civilan:       Result = FFlareStyleSet::GetIcon("AllFreighters"); break;
	}

	return Result;
}

/*----------------------------------------------------
	Float buffer
----------------------------------------------------*/

void FFlareFloatBuffer::Init(int32 Size)
{
	MaxSize = Size;
	Values.Empty(MaxSize);
	WriteIndex = 0;
}

void FFlareFloatBuffer::Resize(int32 Size)
{
	if(Size <= Values.Num())
	{
		TArray<float> NewValues;
		for (int Age = Size-1; Age >= 0; Age--)
		{
			NewValues.Add(GetValue(Age));
		}
		// Override
		Values = NewValues;
		WriteIndex = 0;
	}

	MaxSize = Size;
}

void FFlareFloatBuffer::Append(float NewValue)
{
	if(Values.Num() <= WriteIndex)
	{
		Values.Add(NewValue);
		WriteIndex = Values.Num();
	}
	else
	{
		Values[WriteIndex] = NewValue;
		WriteIndex++;
	}

	if(WriteIndex >= MaxSize)
	{
		WriteIndex = 0;
	}
}

float FFlareFloatBuffer::GetValue(int32 Age)
{
	if(Values.Num() == 0)
	{
		return 0.f;
	}

	if(Age >= Values.Num())
	{
		Age = Values.Num() - 1;
	}

	int32 ReadIndex = WriteIndex - 1 - Age;
	if (ReadIndex < 0)
	{
		ReadIndex += Values.Num();
	}

	return Values[ReadIndex];
}

float FFlareFloatBuffer::GetMean(int32 StartAge, int32 EndAge)
{
	float Count = 0.f;
	float Sum = 0.f;

	if(StartAge >= Values.Num())
	{
		StartAge = Values.Num() - 1;
	}

	if(EndAge >= Values.Num())
	{
		EndAge = Values.Num() - 1;
	}

	for (int Age = StartAge; Age <= EndAge; Age++)
	{
		Sum += GetValue(Age);
		Count += 1.f;
	}

	if(Count == 0.f)
	{
		return 0.f;
	}
	return Sum/Count;
}

bool FFlareBundle::HasFloat(FName Key) const
{
	return FloatValues.Contains(Key);
}

bool FFlareBundle::HasInt32(FName Key) const
{
	return Int32Values.Contains(Key);
}

bool FFlareBundle::HasTransform(FName Key) const
{
	return TransformValues.Contains(Key);
}

bool FFlareBundle::HasVectorArray(FName Key) const
{
	return VectorArrayValues.Contains(Key);
}

bool FFlareBundle::HasName(FName Key) const
{
	return NameValues.Contains(Key);
}

bool FFlareBundle::HasNameArray(FName Key) const
{
	return NameArrayValues.Contains(Key);
}

bool FFlareBundle::HasString(FName Key) const
{
	return StringValues.Contains(Key);
}

bool FFlareBundle::HasTag(FName Tag) const
{
	return Tags.Contains(Tag);
}

bool FFlareBundle::HasPtr(FName Key) const
{
	return PtrValues.Contains(Key);
}

float FFlareBundle::GetFloat(FName Key, float Default) const
{
	if(FloatValues.Contains(Key))
	{
		return FloatValues[Key];
	}
	return Default;
}

int32 FFlareBundle::GetInt32(FName Key, int32 Default) const
{
	if(Int32Values.Contains(Key))
	{
		return Int32Values[Key];
	}
	return Default;
}

FTransform FFlareBundle::GetTransform(FName Key, const FTransform Default) const
{
	if(TransformValues.Contains(Key))
	{
		return TransformValues[Key];
	}
	return Default;
}

TArray<FVector> FFlareBundle::GetVectorArray(FName Key) const
{
	if(VectorArrayValues.Contains(Key))
	{
		return VectorArrayValues[Key].Entries;
	}
	return TArray<FVector>();
}

FName FFlareBundle::GetName(FName Key) const
{
	if(NameValues.Contains(Key))
	{
		return NameValues[Key];
	}
	return "";
}

TArray<FName> FFlareBundle::GetNameArray(FName Key) const
{
	if(NameArrayValues.Contains(Key))
	{
		return NameArrayValues[Key].Entries;
	}
	return TArray<FName>();
}

FString FFlareBundle::GetString(FName Key) const
{
	if (StringValues.Contains(Key))
	{
		return StringValues[Key];
	}
	return "";
}

void* FFlareBundle::GetPtr(FName Key) const
{
	if (PtrValues.Contains(Key))
	{
		return PtrValues[Key].Entry;
	}
	return NULL;
}

FFlareBundle& FFlareBundle::PutFloat(FName Key, float Value)
{
	FloatValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutInt32(FName Key, int32 Value)
{
	Int32Values.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutTransform(FName Key, const FTransform Value)
{
	TransformValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutVectorArray(FName Key, const TArray<FVector> Value)
{
	FVectorArray Array;
	Array.Entries = Value;
	VectorArrayValues.Add(Key, Array);
	return *this;
}

FFlareBundle& FFlareBundle::PutName(FName Key, FName Value)
{
	NameValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutNameArray(FName Key, const TArray<FName> Value)
{
	FNameArray Array;
	Array.Entries = Value;
	NameArrayValues.Add(Key, Array);
	return *this;
}

FFlareBundle& FFlareBundle::PutString(FName Key, FString Value)
{
	StringValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutTag(FName Tag)
{
	Tags.Add(Tag);
	return *this;
}


FFlareBundle& FFlareBundle::PutPtr(FName Key, void* Value)
{
	FPtr Ptr;
	Ptr.Entry = Value;
	PtrValues.Add(Key, Ptr);
	return *this;
}

void FFlareBundle::Clear()
{
	FloatValues.Empty();
	Int32Values.Empty();
	TransformValues.Empty();
	VectorArrayValues.Empty();
	NameValues.Empty();
	NameArrayValues.Empty();
	StringValues.Empty();
	Tags.Empty();
	PtrValues.Empty();
}

DamageCause::DamageCause():
	Spacecraft(NULL),
	Company(NULL),
	DamageType(EFlareDamage::DAM_None)
{

}

DamageCause::DamageCause(EFlareDamage::Type DamageTypeParam):
	Spacecraft(NULL),
	Company(NULL),
	DamageType(DamageTypeParam)
{

}

DamageCause::DamageCause(UFlareSimulatedSpacecraft* SpacecraftParam, EFlareDamage::Type DamageTypeParam):
	Spacecraft(SpacecraftParam),
	Company(NULL),
	DamageType(DamageTypeParam)
{
	if(Spacecraft)
	{
		Company = Spacecraft->GetCompany();
	}
}

DamageCause::DamageCause(UFlareCompany* CompanyParam, EFlareDamage::Type DamageTypeParam):
	Spacecraft(NULL),
	Company(CompanyParam),
	DamageType(DamageTypeParam)
{
}

bool FFlareResourceUsage::HasAnyUsage() const
{
	return Usages.Num() > 0;
}

bool FFlareResourceUsage::HasUsage(EFlareResourcePriceContext::Type Usage) const
{
	return Usages.Contains(Usage);
}

void FFlareResourceUsage::AddUsage(EFlareResourcePriceContext::Type Usage)
{
	Usages.AddUnique(Usage);
}



FFlareTransactionLogEntry FFlareTransactionLogEntry::LogUpgradeShipPart(UFlareSimulatedSpacecraft* Spacecraft)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::UpgradeShipPart;
	Entry.Spacecraft = Spacecraft->GetImmatriculation();
	Entry.Sector = Spacecraft->GetCurrentSector()->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::OrderShip;
	Entry.Spacecraft = Shipyard->GetImmatriculation();
	Entry.Sector = Shipyard->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = Shipyard->GetCompany()->GetIdentifier();
	Entry.ExtraIdentifier1 = OrderShipClass;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogCancelOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::CancelOrderShip;
	Entry.Spacecraft = Shipyard->GetImmatriculation();
	Entry.Sector = Shipyard->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = Shipyard->GetCompany()->GetIdentifier();
	Entry.ExtraIdentifier1 = OrderShipClass;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogShipOrderAdvance(UFlareSimulatedSpacecraft* Shipyard, FName Company, FName ShipClass)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::OrderShipAdvance;
	Entry.Spacecraft = Shipyard->GetImmatriculation();
	Entry.Sector = Shipyard->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = Company;
	Entry.ExtraIdentifier1 = ShipClass;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogFactoryWages(UFlareFactory* Factory)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::FactoryWages;
	Entry.Spacecraft = Factory->GetParent()->IsComplexElement() ? Factory->GetParent()->GetComplexMaster()->GetImmatriculation() : Factory->GetParent()->GetImmatriculation();
	Entry.Sector = Factory->GetParent()->GetCurrentSector()->GetIdentifier();
	Entry.ExtraIdentifier1 = Factory->GetDescription()->Identifier;
	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogCancelFactoryWages(UFlareFactory* Factory)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::CancelFactoryWages;
	Entry.Spacecraft = Factory->GetParent()->IsComplexElement() ? Factory->GetParent()->GetComplexMaster()->GetImmatriculation() : Factory->GetParent()->GetImmatriculation();
	Entry.Sector = Factory->GetParent()->GetCurrentSector()->GetIdentifier();
	Entry.ExtraIdentifier1 = Factory->GetDescription()->Identifier;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogPeoplePurchase(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource, int32 Quantity)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::PeoplePurchase;
	Entry.Spacecraft = Station->GetImmatriculation();
	Entry.Sector = Station->GetCurrentSector()->GetIdentifier();
	Entry.Resource = Resource->Identifier;
	Entry.ResourceQuantity = Quantity;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogInitialMoney()
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::InitialMoney;



	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogBuyResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute)
{
	FFlareTransactionLogEntry Entry;
	if(TradeRoute)
	{
		Entry.Type = EFlareTransactionLogEntry::TradeRouteResourcePurchase;
		Entry.ExtraIdentifier2 = TradeRoute->GetIdentifier();
	}
	else
	{
		Entry.Type = EFlareTransactionLogEntry::ManualResourcePurchase;
	}

	Entry.Spacecraft = DestinationSpacecraft->GetImmatriculation();
	Entry.Sector = DestinationSpacecraft->GetCurrentSector()->GetIdentifier();
	Entry.Resource = Resource->Identifier;
	Entry.ResourceQuantity = GivenResources;
	Entry.ExtraIdentifier1 = SourceSpacecraft->GetImmatriculation();
	Entry.OtherCompany = SourceSpacecraft->GetCompany()->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogSellResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute)
{
	FFlareTransactionLogEntry Entry;
	if(TradeRoute)
	{
		Entry.Type = EFlareTransactionLogEntry::TradeRouteResourceSell;
		Entry.ExtraIdentifier2 = TradeRoute->GetIdentifier();
	}
	else
	{
		Entry.Type = EFlareTransactionLogEntry::ManualResourceSell;
	}

	Entry.Spacecraft = SourceSpacecraft->GetImmatriculation();
	Entry.Sector = SourceSpacecraft->GetCurrentSector()->GetIdentifier();
	Entry.Resource = Resource->Identifier;
	Entry.ResourceQuantity = GivenResources;
	Entry.ExtraIdentifier1 = DestinationSpacecraft->GetImmatriculation();
	Entry.OtherCompany = DestinationSpacecraft->GetCompany()->GetIdentifier();



	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogPayMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, int32 TakenQuantity, bool ForRepair)
{
	FFlareTransactionLogEntry Entry;
	if(ForRepair)
	{
		Entry.Type = EFlareTransactionLogEntry::PayRepair;
	}
	else
	{
		Entry.Type = EFlareTransactionLogEntry::PayRefill;
	}

	Entry.Spacecraft = SellerSpacecraft->GetImmatriculation();
	Entry.Sector = SellerSpacecraft->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = SellerSpacecraft->GetCompany()->GetIdentifier();
	Entry.ResourceQuantity = TakenQuantity;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogPaidForMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, UFlareCompany* Company, int32 TakenQuantity, bool ForRepair)
{
	FFlareTransactionLogEntry Entry;
	if(ForRepair)
	{
		Entry.Type = EFlareTransactionLogEntry::PaidForRepair;
	}
	else
	{
		Entry.Type = EFlareTransactionLogEntry::PaidForRefill;
	}

	Entry.Spacecraft = SellerSpacecraft->GetImmatriculation();
	Entry.Sector = SellerSpacecraft->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = Company->GetIdentifier();
	Entry.ResourceQuantity = TakenQuantity;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogSendTribute(UFlareCompany* Company)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::SendTribute;
	Entry.OtherCompany = Company->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogReceiveTribute(UFlareCompany* Company)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::ReceiveTribute;
	Entry.OtherCompany = Company->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogRecoveryFees()
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::RecoveryFees;
	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogBuildStationFees(FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSector* Sector)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::StationConstructionFees;
	Entry.Sector = Sector->GetIdentifier();
	Entry.ExtraIdentifier1 = StationDescription->Identifier;

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogUpgradeStationFees(UFlareSimulatedSpacecraft* Station)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::StationUpgradeFees;
	Entry.Spacecraft = Station->GetImmatriculation();
	Entry.Sector = Station->GetCurrentSector()->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogScrapCost(UFlareCompany* Company)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::ScrapCost;
	Entry.OtherCompany = Company->GetIdentifier();

	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogScrapGain(UFlareSimulatedSpacecraft* ScrappedShip, UFlareCompany* Company)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::ScrapGain;
	Entry.Spacecraft = ScrappedShip->GetImmatriculation();
	Entry.Sector = ScrappedShip->GetCurrentSector()->GetIdentifier();
	Entry.OtherCompany = Company->GetIdentifier();


	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogMutualAssistance()
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::MutualAssistance;
	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogCheat()
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::Cheat;
	return Entry;
}

FFlareTransactionLogEntry FFlareTransactionLogEntry::LogQuestReward(UFlareCompany* Company)
{
	FFlareTransactionLogEntry Entry;
	Entry.Type = EFlareTransactionLogEntry::QuestReward;
	if(Company)
	{
		Entry.OtherCompany = Company->GetIdentifier();
	}

	return Entry;
}

FText FFlareTransactionLogEntry::GetCategoryDescription(EFlareTransactionLogEntry::Type Type)
{
	switch (Type)
	{		
	case EFlareTransactionLogEntry::ManualResourcePurchase:     return LOCTEXT("ManualResourcePurchase",     "Resources purchases");        break;
	case EFlareTransactionLogEntry::ManualResourceSell:         return LOCTEXT("ManualResourceSell",         "Resources sales");            break;
	case EFlareTransactionLogEntry::TradeRouteResourcePurchase: return LOCTEXT("TradeRouteResourcePurchase", "Trade route purchases");      break;
	case EFlareTransactionLogEntry::TradeRouteResourceSell:     return LOCTEXT("TradeRouteResourceSell",     "Trade route sales");          break;
	case EFlareTransactionLogEntry::FactoryWages:               return LOCTEXT("FactoryWages",               "Factory wages");              break;
	case EFlareTransactionLogEntry::CancelFactoryWages:         return LOCTEXT("CancelFactoryWages",         "Factory wages cancelled");    break;
	case EFlareTransactionLogEntry::StationConstructionFees:    return LOCTEXT("StationConstructionFees",    "Station construction");       break;
	case EFlareTransactionLogEntry::StationUpgradeFees:         return LOCTEXT("StationUpgradeFees",         "Station upgrades");           break;
	case EFlareTransactionLogEntry::UpgradeShipPart:            return LOCTEXT("UpgradeShipPart",            "Ship upgrades");              break;
	case EFlareTransactionLogEntry::OrderShip:                  return LOCTEXT("OrderShip",                  "Ship construction");          break;
	case EFlareTransactionLogEntry::CancelOrderShip:            return LOCTEXT("CancelOrderShip",            "Ship construction refunds");  break;
	case EFlareTransactionLogEntry::OrderShipAdvance:           return LOCTEXT("OrderShipAdvance",           "Ship construction advances"); break;
	case EFlareTransactionLogEntry::PeoplePurchase:             return LOCTEXT("PeoplePurchase",             "Population purchases");       break;
	case EFlareTransactionLogEntry::InitialMoney:               return LOCTEXT("InitialMoney",               "Initial capital");            break;
	case EFlareTransactionLogEntry::PayRepair:                  return LOCTEXT("PayRepair",                  "Ship repairs");               break;
	case EFlareTransactionLogEntry::PayRefill:                  return LOCTEXT("PayRefill",                  "Ship refilling");             break;
	case EFlareTransactionLogEntry::PaidForRepair:              return LOCTEXT("PaidForRepair",              "Ship repair services");       break;
	case EFlareTransactionLogEntry::PaidForRefill:              return LOCTEXT("PaidForRefill",              "Ship refilling services");    break;
	case EFlareTransactionLogEntry::SendTribute:                return LOCTEXT("SendTribute",                "Tributes paid");              break;
	case EFlareTransactionLogEntry::ReceiveTribute:             return LOCTEXT("ReceiveTribute",             "Tributes received");          break;
	case EFlareTransactionLogEntry::RecoveryFees:               return LOCTEXT("RecoveryFees",               "Ship recovery");              break;
	case EFlareTransactionLogEntry::ScrapGain:                  return LOCTEXT("ScrapGain",                  "Ship scrapping");             break;
	case EFlareTransactionLogEntry::Cheat:                      return LOCTEXT("Cheat",                      "Unlawful transactions");      break;
	case EFlareTransactionLogEntry::QuestReward:                return LOCTEXT("QuestReward",                "Contracts");                  break;
	case EFlareTransactionLogEntry::MutualAssistance:           return LOCTEXT("MutualAssistance",           "Mutual assistance");          break;
	case EFlareTransactionLogEntry::ScrapCost:                  return LOCTEXT("ScrapCost",                  "Ship scrapping costs");       break;
	}

	return FText();
}

UFlareCompany* FFlareTransactionLogEntry::GetOtherCompany(AFlareGame* Game) const
{
	if(OtherCompany != NAME_None)
	{
		return Game->GetGameWorld()->FindCompany(OtherCompany);
	}
	else
	{
		return nullptr;
	}
}

UFlareSimulatedSector* FFlareTransactionLogEntry::GetSector(AFlareGame* Game) const
{
	if(Sector != NAME_None)
	{
		return Game->GetGameWorld()->FindSector(Sector);
	}
	else
	{
		return nullptr;
	}
}

UFlareSimulatedSpacecraft* FFlareTransactionLogEntry::GetSpacecraft(AFlareGame* Game) const
{
	if(Spacecraft != NAME_None)
	{
		return Game->GetGameWorld()->FindSpacecraft(Spacecraft);
	}
	else
	{
		return nullptr;
	}
}

FText FFlareTransactionLogEntry::GetComment(AFlareGame* Game) const
{
	FText Comment;


	FLOGV("--- Type %d", int(Type));
	FLOGV("  Spacecraft %s", *Spacecraft.ToString());
	FLOGV("  Sector %s", *Sector.ToString());
	FLOGV("  ExtraIdentifier1 %s", *ExtraIdentifier1.ToString());
	FLOGV("  ExtraIdentifier2 %s", *ExtraIdentifier2.ToString());


	UFlareSimulatedSpacecraft* SpacecraftCache = nullptr;
	FFlareResourceDescription* ResourceCache = nullptr;

	if(Spacecraft != NAME_None)
	{
		SpacecraftCache = Game->GetGameWorld()->FindSpacecraft(Spacecraft);
	}

	if(Resource != NAME_None)
	{
		ResourceCache = Game->GetResourceCatalog()->Get(Resource);
	}

	auto FindFactoryDescription = [](UFlareSimulatedSpacecraft* SpacecraftWithFactory, FName Identifier) -> FFlareFactoryDescription const*
	{
		for(UFlareFactory* Factory : SpacecraftWithFactory->GetFactories())
		{
			FFlareFactoryDescription const* FactoryDescription = Factory->GetDescription();
			if(FactoryDescription->Identifier == Identifier)
			{
				return FactoryDescription;
			}
		}
		return nullptr;
	};

	auto GetFactoryName = [this, &SpacecraftCache, &FindFactoryDescription]()
	{
		FFlareFactoryDescription const* FactoryDescription = FindFactoryDescription(SpacecraftCache, ExtraIdentifier1);

		if(FactoryDescription)
		{
			return FactoryDescription->Name;
		}

		return FText();
	};


	auto IsAITransaction = [](UFlareSimulatedSpacecraft* SpacecraftCache, UFlareSimulatedSpacecraft* OtherSpacecraftCache)
	{
		if(SpacecraftCache && OtherSpacecraftCache)
		{
			if((!SpacecraftCache->IsStation() && !SpacecraftCache->GetCompany()->IsPlayerCompany()) || (!OtherSpacecraftCache->IsStation() && !OtherSpacecraftCache->GetCompany()->IsPlayerCompany()))
			{
				return true;
			}
		}
		return false;
	};


	switch(Type)
	{
	case EFlareTransactionLogEntry::ManualResourcePurchase:
	{
		UFlareSimulatedSpacecraft* OtherSpacecraftCache = Game->GetGameWorld()->FindSpacecraft(ExtraIdentifier1);

		if(OtherSpacecraftCache && ResourceCache)
		{
			FText ResourceComment = FText::Format(LOCTEXT("ManualResourcePurchaseResources", "{0} {1}"), ResourceQuantity, ResourceCache->Name);

			if(IsAITransaction(SpacecraftCache, OtherSpacecraftCache))
			{
				Comment = FText::Format(LOCTEXT("AIManualResourcePurchase", "{0} sold {1} with {2}"), OtherSpacecraftCache->GetCompany()->GetCompanyName(), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
			else
			{
				Comment = FText::Format(LOCTEXT("ManualResourcePurchase", "Bought {0} from {1}"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
		}
		break;
	}
	case EFlareTransactionLogEntry::ManualResourceSell:
	{
		UFlareSimulatedSpacecraft* OtherSpacecraftCache = Game->GetGameWorld()->FindSpacecraft(ExtraIdentifier1);

		if(OtherSpacecraftCache && ResourceCache)
		{
			FText ResourceComment = FText::Format(LOCTEXT("ManualResourceSellResources", "{0} {1}"), ResourceQuantity, ResourceCache->Name);

			if(IsAITransaction(SpacecraftCache, OtherSpacecraftCache))
			{
				Comment = FText::Format(LOCTEXT("AIManualResourceSell", "{0} bought {1} with {2}"), OtherSpacecraftCache->GetCompany()->GetCompanyName(), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
			else
			{
				Comment = FText::Format(LOCTEXT("ManualResourceSell", "Sold {0} to {1}"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
		}
		break;
	}
	case EFlareTransactionLogEntry::TradeRouteResourcePurchase:
	{
		UFlareSimulatedSpacecraft* OtherSpacecraftCache = Game->GetGameWorld()->FindSpacecraft(ExtraIdentifier1);
		UFlareTradeRoute* TradeRouteCache = Game->GetGameWorld()->FindTradeRoute(ExtraIdentifier2);

		if(OtherSpacecraftCache && ResourceCache)
		{
			FText ResourceComment = FText::Format(LOCTEXT("TradeRouteResourcePurchase", "{0} {1}"), ResourceQuantity, ResourceCache->Name);

			if(TradeRouteCache)
			{
				Comment = FText::Format(LOCTEXT("TradeRouteResourcePurchase", "Bought {0} from {1} on trade route {2}"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache), TradeRouteCache->GetTradeRouteName());
			}
			else
			{
				Comment = FText::Format(LOCTEXT("NotTradeRouteResourcePurchase", "Bought {0} from {1} on an old trade route"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
		}
		break;
	}
	case EFlareTransactionLogEntry::TradeRouteResourceSell:
	{
		UFlareSimulatedSpacecraft* OtherSpacecraftCache = Game->GetGameWorld()->FindSpacecraft(ExtraIdentifier1);
		UFlareTradeRoute* TradeRouteCache = Game->GetGameWorld()->FindTradeRoute(ExtraIdentifier2);

		if(OtherSpacecraftCache && ResourceCache)
		{
			FText ResourceComment = FText::Format(LOCTEXT("TradeRouteResourceSellResources", "{0} {1}"), ResourceQuantity, ResourceCache->Name);

			if(TradeRouteCache)
			{
				Comment = FText::Format(LOCTEXT("TradeRouteResourceSell", "Sold {0} to {1} on trade route {2}"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache), TradeRouteCache->GetTradeRouteName());
			}
			else
			{
				Comment = FText::Format(LOCTEXT("NotTradeRouteResourceSell", "Sold {0} to {1} on an old trade route"), ResourceComment, UFlareGameTools::DisplaySpacecraftName(OtherSpacecraftCache));
			}
		}
		break;
	}
	case EFlareTransactionLogEntry::FactoryWages:
	{
		if(SpacecraftCache)
		{
			Comment = FText::Format(LOCTEXT("FactoryWages", "Factory wages for {0}"), GetFactoryName());
		}
		break;
	}
	case EFlareTransactionLogEntry::CancelFactoryWages:
	{
		if(SpacecraftCache)
		{
			Comment = FText::Format(LOCTEXT("CancelFactoryWages", "Cancelled factory wages for {0}"), GetFactoryName());
		}
		break;
	}
	case EFlareTransactionLogEntry::StationConstructionFees:
	{
		FFlareSpacecraftDescription* Description = Game->GetSpacecraftCatalog()->Get(ExtraIdentifier1);

		if(Description)
		{
			Comment = FText::Format(LOCTEXT("StationConstructionFees", "Contruction fees for {0}"), Description->Name);
		}
		break;
	}
	case EFlareTransactionLogEntry::StationUpgradeFees:
	{
		Comment = LOCTEXT("StationUpgradeFees", "Station upgrade fees");
		break;
	}
	case EFlareTransactionLogEntry::UpgradeShipPart:
	{
		Comment = LOCTEXT("UpgradeShipFees", "Upgrade ship fees");
		break;
	}
	case EFlareTransactionLogEntry::OrderShip:
	{
		FFlareSpacecraftDescription* Description = Game->GetSpacecraftCatalog()->Get(ExtraIdentifier1);
		if(Description)
		{
			Comment = FText::Format(LOCTEXT("OrderShip", "Ordered ship ({0}})"), Description->Name);
		}
		break;
	}
	case EFlareTransactionLogEntry::CancelOrderShip:
	{
		FFlareSpacecraftDescription* Description = Game->GetSpacecraftCatalog()->Get(ExtraIdentifier1);
		if(Description)
		{
			Comment = FText::Format(LOCTEXT("CancelOrderShip", "Cancelled ship order ({0}})"), Description->Name);
		}
		break;
	}
	case EFlareTransactionLogEntry::OrderShipAdvance:
	{
		FFlareSpacecraftDescription* Description = Game->GetSpacecraftCatalog()->Get(ExtraIdentifier1);
		if(Description)
		{
			Comment = FText::Format(LOCTEXT("OrderShipAdvance", "Paid for ship order ({0}})"), Description->Name);
		}
		break;
	}
	case EFlareTransactionLogEntry::PeoplePurchase:
	{
		//UFlareSimulatedSpacecraft* OtherSpacecraftCache = Game->GetGameWorld()->FindSpacecraft(ExtraIdentifier1);

		if(ResourceCache)
		{
			FText ResourceComment = FText::Format(LOCTEXT("PeoplePurchaseResources", "{0} {1}"), ResourceQuantity, ResourceCache->Name);
			Comment = FText::Format(LOCTEXT("PeoplePurchase", "Population bought {0}"), ResourceComment);
		}
		break;
	}
	case EFlareTransactionLogEntry::InitialMoney:
	{
		Comment = LOCTEXT("InitialMoney", "Initial company capital");
		break;
	}

	//PayRepair,
	//PayRefill,
	//PaidForRepair,
	//PaidForRefill,
	//SendTribute,
	//ReceiveTribute,
	//RecoveryFees,
	case EFlareTransactionLogEntry::ScrapGain:
	{
		Comment = LOCTEXT("ScrapGain", "Scrap money");
		break;
	}
	case EFlareTransactionLogEntry::Cheat:
	{
		Comment = LOCTEXT("Cheat", "Unlawful transaction");
		break;
	}
	case EFlareTransactionLogEntry::QuestReward:
	{
		Comment = LOCTEXT("QuestReward", "Contract reward");
		break;
	}

	// AI only:
	// MutualAssistance,
	// ScrapCost

	default:

		Comment = FText::Format(LOCTEXT("Dummy", "Dummy comment {0}"), int(Type));
	}

	/*FName Spacecraft;
	FName Sector;
	FName OtherCompany;
	FName OtherSpacecraft;
	FName Resource;
	int32 ResourceQuantity;
*/
	return Comment;
}


#undef LOCTEXT_NAMESPACE
