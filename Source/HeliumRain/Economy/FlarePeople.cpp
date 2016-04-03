
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "FlarePeople.h"


#define LOCTEXT_NAMESPACE "FlarePeopleInfo"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlarePeople::UFlarePeople(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*----------------------------------------------------
   Save
----------------------------------------------------*/


void UFlarePeople::Load(UFlareSimulatedSector* ParentSector, const FFlarePeopleSave& Data)
{
	Game = ParentSector->GetGame();

	PeopleData = Data;
	Parent = ParentSector;
}

FFlarePeopleSave* UFlarePeople::Save()
{
	return &PeopleData;
}

/*----------------------------------------------------
   Gameplay
----------------------------------------------------*/

static uint32 BIRTH_POINT_TRESHOLD = 7120;
static uint32 DEATH_POINT_TRESHOLD = 29200;
static uint32 MONETARY_CREATION = 1000;

void UFlarePeople::Simulate()
{
	if(PeopleData.Population == 0)
	{
		return;
	}

	FLOGV("Simulate people for sector %s. Population=%u", *Parent->GetSectorName().ToString(), PeopleData.Population)			

	SimulateResourcePurchase();

	float Happiness = GetHappiness();

	// Death of old age : 1 death for 80 years per inhabitant = 1 death per 29200 inhabitant days
	// Sickness increase with hunger and sadness
	//	- 4 times normal sickness if happiness is 0
	//  - normal sickness if happiness is 1.5
	//  - half sickness if happiness 2
	//  Formula : sickness = 0.5 + (happiness -2)^2
	//
	//  Hunger is add to population death point
	//
	float Sickness = 0.5 + FMath::Square(Happiness - 2);
	PeopleData.DeathPoint += (PeopleData.Population + PeopleData.HungerPoint * 2) * Sickness;
	KillPeople(PeopleData.DeathPoint / DEATH_POINT_TRESHOLD);
	PeopleData.DeathPoint = PeopleData.DeathPoint % DEATH_POINT_TRESHOLD;

	// Births : 1 birth for 20 years per inhabitant = 1 birth per 7120 inhabitant days
	// Fertility increase with happiness :
	//	 - no fertility if hapinness is less of 50%
	//   - normal fertility at 100 % happiness
	//   - triple fertility at 200%
	//   Formula : fertility = 2 * happiness -1
	float Fertility = FMath::Max(2 * Happiness -1, 0.0f);

	PeopleData.BirthPoint += PeopleData.Population * Fertility;
	GiveBirth(PeopleData.BirthPoint / BIRTH_POINT_TRESHOLD);
	PeopleData.BirthPoint = PeopleData.BirthPoint % BIRTH_POINT_TRESHOLD;


	// Eat
	// Each inhabitant eat 1 kg of food a day as vital food.
	// TODO dynamic consumtion
	// If an inhabitant don't heat, happiness decrease heavily and  hunger is increase
	// If some inhabitant heat, the hunger deaseapear and some happiness is gain
	uint32 FoodConsumption = PeopleData.Population;
	uint32 EatenFood = FMath::Min(FoodConsumption, PeopleData.FoodStock);
	// Reduce stock
	PeopleData.FoodStock -= EatenFood;
	IncreaseHappiness(EatenFood / 10);

	// Reduce hunger (100% if everybody eat)
	float FeedPeopleRatio = (float) EatenFood / (float) FoodConsumption;

	PeopleData.HungerPoint *= 1 - FeedPeopleRatio;


	// Add hunger (0 if everybody eat)
	uint32 Hunger = FoodConsumption - EatenFood;
	PeopleData.HungerPoint += Hunger + PeopleData.HungerPoint / 10;
	DecreaseHappiness(Hunger * 10);



	FLOGV(" - happiness: %f", Happiness);
	FLOGV(" - Sickness: %f", Sickness);
	FLOGV(" - Fertility: %f", Fertility);

	FLOGV(" - Food: %u", PeopleData.FoodStock);
	FLOGV(" - FoodConsumption: %u", FoodConsumption);
	FLOGV(" - EatenFood: %u", EatenFood);
	FLOGV(" - FeedPeopleRatio: %f", FeedPeopleRatio);
	FLOGV(" - Delta Hunger: %u", Hunger);
	FLOGV(" - Hunger: %u", PeopleData.HungerPoint);

	FLOGV(" - Money: %u", PeopleData.Money);
	FLOGV(" - Dept: %u", PeopleData.Dept);
}

void UFlarePeople::SimulateResourcePurchase()
{
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Tools = Game->GetResourceCatalog()->Get("tools");
	FFlareResourceDescription* Tech = Game->GetResourceCatalog()->Get("tech");

	uint32 BoughtFood = BuyResourcesInSector(Food, GetRessourceConsumption(Food)); // In Tons
	if(BoughtFood)
		FLOGV("People in %s bought %u food", *Parent->GetSectorName().ToString(), BoughtFood);
	PeopleData.FoodStock += BoughtFood * 1000; // In kg


	// Todo stock
	uint32 BoughtFuel = BuyResourcesInSector(Fuel, GetRessourceConsumption(Fuel)); // In Tons
	uint32 BoughtTools = BuyResourcesInSector(Tools, GetRessourceConsumption(Tools)); // In Tons
	uint32 BoughtTech = BuyResourcesInSector(Tech, GetRessourceConsumption(Tech)); // In Tons
	if(BoughtFuel)
		FLOGV("People in %s bought %u fuel", *Parent->GetSectorName().ToString(), BoughtFuel);
	if(BoughtTools)
		FLOGV("People in %s bought %u tools", *Parent->GetSectorName().ToString(), BoughtTools);
	if(BoughtTech)
		FLOGV("People in %s bought %u tech", *Parent->GetSectorName().ToString(), BoughtTech);


	PeopleData.HappinessPoint += BoughtFuel + BoughtTools + BoughtTech;

}

uint32 UFlarePeople::BuyResourcesInSector(FFlareResourceDescription* Resource, uint32 Quantity)
{
	// Find companies selling the ressource

	TArray<UFlareSimulatedSpacecraft*> SellingStations;
	TArray<UFlareCompany*> SellingCompanies;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Parent->GetSectorStations().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Parent->GetSectorStations()[SpacecraftIndex];

		if(!Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			continue;
		}
		SellingStations.Add(Station);
		SellingCompanies.AddUnique(Station->GetCompany());
	}

	// Limit quantity to buy with money
	uint32 BaseQuantity = FMath::Min(Quantity, PeopleData.Money / (uint32) (Parent->GetResourcePrice(Resource)*1.05));
	uint32 ResourceToBuy = BaseQuantity;

	while(ResourceToBuy > 0 && SellingCompanies.Num() > 0)
	{
		uint32 ReputationSum = 0;
		uint32 InitialResourceToBuy = ResourceToBuy;

		// Compute company reputation sum to share market part
		for (int32 CompanyIndex = 0; CompanyIndex < SellingCompanies.Num(); CompanyIndex++)
		{
			FFlareCompanyReputationSave* Reputation = GetCompanyReputation(SellingCompanies[CompanyIndex]);

			ReputationSum += Reputation->Reputation;
		}

		for (int32 CompanyIndex = SellingCompanies.Num()-1; CompanyIndex >= 0; CompanyIndex--)
		{
			FFlareCompanyReputationSave* Reputation = GetCompanyReputation(SellingCompanies[CompanyIndex]);

			uint32 PartToBuy = (InitialResourceToBuy * Reputation->Reputation) / ReputationSum;

			uint32 BoughtQuantity = BuyInStationForCompany(Resource, PartToBuy, SellingCompanies[CompanyIndex], SellingStations);
			ResourceToBuy -= BoughtQuantity;

			if(PartToBuy == 0 || BoughtQuantity < PartToBuy)
			{
				SellingCompanies.RemoveAt(CompanyIndex);
			}
		}
	}

	return BaseQuantity - ResourceToBuy;
}

uint32 UFlarePeople::BuyInStationForCompany(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Company, TArray<UFlareSimulatedSpacecraft*>& Stations)
{
	uint32 RemainingQuantity = Quantity;



	for (int32 StationIndex = 0; StationIndex < Stations.Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Stations[StationIndex];

		if(Station->GetCompany() != Company)
		{
			continue;
		}

		uint32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantity);
		RemainingQuantity -= TakenQuantity;
		uint32 Price = (uint32) (Parent->GetResourcePrice(Resource)*1.05) * TakenQuantity;
		PeopleData.Money -= Price;
		Company->GiveMoney(Price);
	}

	return Quantity - RemainingQuantity;
}

uint32 UFlarePeople::GetRessourceConsumption(FFlareResourceDescription* Resource)
{
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Tools = Game->GetResourceCatalog()->Get("tools");
	FFlareResourceDescription* Tech = Game->GetResourceCatalog()->Get("tech");

	if (Resource == Food)
	{
		// Buy at food for 15 days
		uint32 FoodToHave =  1000 + PeopleData.Population * 15; // In kg
		if(FoodToHave > PeopleData.FoodStock)
		{
			uint32 FoodToBuy = FoodToHave - PeopleData.FoodStock;
			return FoodToBuy / 1000;
		}
	}
	else if (Resource == Fuel)
	{
		return PeopleData.Population / 5000;
	}
	else if (Resource == Tools)
	{
		return PeopleData.Population / 10000;
	}
	else if (Resource == Tech)
	{
		return PeopleData.Population / 10000;
	}

	return 0;
}

void UFlarePeople::GiveBirth(uint32 BirthCount)
{
	if(BirthCount == 0)
	{
		return;
	}

	FLOGV("Give birth %u people for sector %s", BirthCount, *Parent->GetSectorName().ToString());

	// Increase population
	PeopleData.Population += BirthCount;

	// Money creation
	uint32 NewMoney = BirthCount * MONETARY_CREATION;
	PeopleData.Money += NewMoney;
	Game->GetGameWorld()->WorldMoneyReference +=NewMoney;

	IncreaseHappiness(BirthCount * 100 * 2);
	PeopleData.HappinessPoint += BirthCount * 100 * 2; // Birth happiness bonus
}

void UFlarePeople::KillPeople(uint32 KillCount)
{

	uint32 PeopleToKill = FMath::Min(KillCount, PeopleData.Population);
	if(PeopleToKill == 0)
	{
		return;
	}

	FLOGV("Kill %u people for sector %s", KillCount, *Parent->GetSectorName().ToString());


	float KillRatio = (float) PeopleToKill / (float)PeopleData.Population;
	// Decrease population
	PeopleData.Population -= KillCount;

	// Money destruction (delayed, really destroy on Pay)
	PeopleData.Dept += KillCount * MONETARY_CREATION;

	DecreaseHappiness(KillCount * 100 * 2); // Death happiness malus

	//Cancel dead hunger
	PeopleData.HungerPoint = (1 - KillRatio) * PeopleData.HungerPoint;

	if(PeopleToKill == 0)
	{
		ResetPeople();
	}
}

void UFlarePeople::IncreaseHappiness(uint32 HappinessPoints)
{
	// Max happiness is 2 so Happiness max is 200 x population
	// Gain happiness is boost when sad and difficult when happy
	//  - Normal gain for 1 as happiness
	//  - 4 times gain for 0 as happiness
	//  - No gain for 2 as happiness
	// Formula: gain = (happiness - 2) ^ 2
	float Happiness = GetHappiness();
	float Gain = FMath::Square(Happiness - 2);
	PeopleData.HappinessPoint += HappinessPoints * Gain;
	PeopleData.HappinessPoint = FMath::Min(PeopleData.HappinessPoint, PeopleData.Population * 200);
}

void UFlarePeople::DecreaseHappiness(uint32 SadnessPoints)
{
	// Same as for increase but gain are inverted
	float Happiness = GetHappiness();
	float Gain = FMath::Square(Happiness);
	PeopleData.HappinessPoint -= SadnessPoints * Gain;
	PeopleData.HappinessPoint = FMath::Max(PeopleData.HappinessPoint, (uint32) 0);
}

void UFlarePeople::SetHappiness(float Happiness)
{
	PeopleData.HappinessPoint = PeopleData.Population * 100 * Happiness;
}

void UFlarePeople::Pay(uint32 Amount)
{
	//FLOGV("Pay to people for sector %s Amount=%u", *Parent->GetSectorName().ToString(), Amount)

	uint32 Repayment = 0;
	if(PeopleData.Dept > 0)
	{
		Repayment = FMath::Min(PeopleData.Dept, Amount / 10);
		PeopleData.Dept -= Repayment;
		Game->GetGameWorld()->WorldMoneyReference -=Repayment;
	}
	PeopleData.Money += Amount - Repayment;
}

void UFlarePeople::TakeMoney(uint32 Amount)
{
	uint32 TakenMoney = FMath::Min(PeopleData.Money, Amount);
	PeopleData.Money -=  TakenMoney;

	PeopleData.Dept += Amount - TakenMoney;
}

void UFlarePeople::ResetPeople()
{
	PeopleData.Population = 0;
	PeopleData.BirthPoint = 0;
	PeopleData.DeathPoint = 0;
	PeopleData.FoodStock = 0;
	PeopleData.HappinessPoint = 0;
	PeopleData.HungerPoint = 0;

	// Don't reset money to avoid moyen lost

}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

float UFlarePeople::GetHappiness()
{
	if(PeopleData.Population == 0)
	{
		return 0;
	}
	return (float) PeopleData.HappinessPoint / (100 * (float) PeopleData.Population);
}

FFlareCompanyReputationSave* UFlarePeople::GetCompanyReputation(UFlareCompany* Company)
{
	for(int ReputationIndex = 0; ReputationIndex < PeopleData.CompanyReputations.Num(); ReputationIndex++)
	{
		if(PeopleData.CompanyReputations[ReputationIndex].CompanyIdentifier == Company->GetIdentifier())
		{
			return &PeopleData.CompanyReputations[ReputationIndex];
		}
	}

	//Init Reputation
	FFlareCompanyReputationSave NewReputation;
	NewReputation.CompanyIdentifier = Company->GetIdentifier();
	NewReputation.Reputation = 1000 * PeopleData.Population;
	PeopleData.CompanyReputations.Add(NewReputation);

	return GetCompanyReputation(Company);
}

#undef LOCTEXT_NAMESPACE
