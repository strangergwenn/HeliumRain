
#include "FlarePeople.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"

#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "FlareCargoBay.h"


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

static uint32 BIRTH_POINT_TRESHOLD = 3000;
static uint32 DEATH_POINT_TRESHOLD = 29200;
static uint32 MONETARY_CREATION = 10000;

// All in kg and kg/hab

static float FOOD_MIN_CONSUMPTION = 1;
static float FUEL_MIN_CONSUMPTION = 0.6;
static float TOOL_MIN_CONSUMPTION = 0.5;
static float TECH_MIN_CONSUMPTION = 0.4;

static int32 FOOD_NEED_STOCK = 20;
static int32 FUEL_NEED_STOCK = 10;
static int32 TOOL_NEED_STOCK = 10;
static int32 TECH_NEED_STOCK = 10;

static float FOOD_HAPPINESS = 0.1;
static float FUEL_HAPPINESS = 0.6;
static float TOOL_HAPPINESS = 0.5;
static float TECH_HAPPINESS = 0.4;

static float FOOD_SADNESS = 10;
static float FUEL_SADNESS = 4;
static float TOOL_SADNESS = 2;
static float TECH_SADNESS = 1;

static int32 MIN_GENERAL_STOCK = 1000;

void UFlarePeople::Simulate()
{
	if(PeopleData.Population == 0)
	{
		CheckPopulationDisparition();
		return;
	}


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

	// Births : 1 birth for 20 years per inhabitant = 1 birth per 7120 inhabitant days (No more right)
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
	// If an inhabitant don't heat, happiness decrease heavily and  hunger is increase
	// If some inhabitant heat, the hunger deaseapear and some happiness is gain
	uint32 FoodConsumption = PeopleData.Population * PeopleData.FoodConsumption;
	uint32 EatenFood = FMath::Min(FoodConsumption, PeopleData.FoodStock);
	// Reduce stock
	PeopleData.FoodStock -= EatenFood;
	IncreaseHappiness(EatenFood * FOOD_HAPPINESS);

	// Reduce hunger (100% if everybody eat)
	float FeedPeopleRatio = (float) EatenFood / (float) FoodConsumption;

	PeopleData.HungerPoint *= 1 - FeedPeopleRatio;

	// Add hunger (0 if everybody eat)
	uint32 Hunger = FoodConsumption - EatenFood;
	PeopleData.HungerPoint += Hunger + PeopleData.HungerPoint / 10;
	DecreaseHappiness(Hunger * FOOD_SADNESS);

	// Fuel
	int32 FuelConsumption = PeopleData.Population * PeopleData.FuelConsumption;
	int32 EatenFuel = FMath::Min(FuelConsumption, PeopleData.FuelStock);
	// Reduce stock
	PeopleData.FuelStock -= EatenFuel;
	IncreaseHappiness(EatenFuel * FUEL_HAPPINESS);

	// Add hunger (0 if everybody eat)
	Hunger = FuelConsumption - EatenFuel;
	DecreaseHappiness(Hunger * FUEL_SADNESS);

	// Tool
	int32 ToolConsumption = PeopleData.Population * PeopleData.ToolConsumption;
	int32 EatenTool = FMath::Min(ToolConsumption, PeopleData.ToolStock);
	// Reduce stock
	PeopleData.ToolStock -= EatenTool;
	IncreaseHappiness(EatenTool * TOOL_HAPPINESS);


	// Add hunger (0 if everybody eat)
	Hunger = ToolConsumption - EatenTool;
	DecreaseHappiness(Hunger * TOOL_SADNESS);

	// Tech
	int32 TechConsumption = PeopleData.Population * PeopleData.TechConsumption;
	int32 EatenTech = FMath::Min(TechConsumption, PeopleData.TechStock);
	// Reduce stock
	PeopleData.TechStock -= EatenTech;
	IncreaseHappiness(EatenTech * TECH_HAPPINESS);

	Hunger = TechConsumption - EatenTech;
	DecreaseHappiness(Hunger * TECH_SADNESS);
}

void UFlarePeople::SimulateResourcePurchase()
{
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Tool = Game->GetResourceCatalog()->Get("tools");
	FFlareResourceDescription* Tech = Game->GetResourceCatalog()->Get("tech");

	bool LockNext = false;

	uint32 FoodConsumption = GetRessourceConsumption(Food, true);
	uint32 BoughtFood = BuyResourcesInSector(Food, FoodConsumption, 0.001); // In Tons
	//if(BoughtFood)
	//	FLOGV("People in %s bought %u food", *Parent->GetSectorName().ToString(), BoughtFood);
	PeopleData.FoodStock += BoughtFood * 1000; // In kg


	if(FoodConsumption == BoughtFood)
	{
		PeopleData.FoodConsumption += 0.01 * FOOD_MIN_CONSUMPTION / PeopleData.FoodConsumption;
	}
	else
	{
		LockNext = true;
		PeopleData.FoodConsumption -= 1.f /FOOD_NEED_STOCK * FOOD_MIN_CONSUMPTION;
		PeopleData.FuelConsumption -= 1.f /FUEL_NEED_STOCK * FUEL_MIN_CONSUMPTION;
		PeopleData.ToolConsumption -= 1.f /TOOL_NEED_STOCK * TOOL_MIN_CONSUMPTION;
		PeopleData.TechConsumption -= 1.f /TECH_NEED_STOCK * TECH_MIN_CONSUMPTION;

		//TODO reputation
	}


	float FuelPrice = Parent->GetResourcePrice(Fuel, EFlareResourcePriceContext::Default);
	float FuelPriceRatio = (FuelPrice - Fuel->MinPrice) / (float)(Fuel->MaxPrice - Fuel->MinPrice);
	if(!LockNext && FuelPriceRatio < 0.75)
	{

		uint32 FuelConsumption = GetRessourceConsumption(Fuel, true);
		uint32 BoughtFuel = BuyResourcesInSector(Fuel, FuelConsumption, 0.002); // In Tons
		//if(BoughtFuel)
		//	FLOGV("People in %s bought %u fuel", *Parent->GetSectorName().ToString(), BoughtFuel);
		PeopleData.FuelStock += BoughtFuel * 1000; // In kg

		if(FuelConsumption == BoughtFuel)
		{
			PeopleData.FuelConsumption += 0.01 * FUEL_MIN_CONSUMPTION / PeopleData.FuelConsumption;
		}
		else
		{
			LockNext = true;
			PeopleData.FoodConsumption -= 0.1f /FOOD_NEED_STOCK * FOOD_MIN_CONSUMPTION;
			PeopleData.FuelConsumption -= 1.f /FUEL_NEED_STOCK * FUEL_MIN_CONSUMPTION;
			PeopleData.ToolConsumption -= 1.f /TOOL_NEED_STOCK * TOOL_MIN_CONSUMPTION;
			PeopleData.TechConsumption -= 1.f /TECH_NEED_STOCK * TECH_MIN_CONSUMPTION;

			//TODO reputation
		}
	}

	float ToolPrice = Parent->GetResourcePrice(Tool, EFlareResourcePriceContext::Default);
	float ToolPriceRatio = (ToolPrice - Tool->MinPrice) / (float)(Tool->MaxPrice - Tool->MinPrice);
	if(!LockNext && ToolPriceRatio < 0.5)
	{
		uint32 ToolConsumption = GetRessourceConsumption(Tool, true);
		uint32 BoughtTool = BuyResourcesInSector(Tool, ToolConsumption, 0.004); // In Tons
		//if(BoughtTool)
		//	FLOGV("People in %s bought %u tool", *Parent->GetSectorName().ToString(), BoughtTool);
		PeopleData.ToolStock += BoughtTool * 1000; // In kg

		if(ToolConsumption == BoughtTool)
		{
			PeopleData.ToolConsumption += 0.01 * TOOL_MIN_CONSUMPTION / PeopleData.ToolConsumption;
		}
		else
		{
			LockNext = true;
			PeopleData.FoodConsumption -= 0.1f /FOOD_NEED_STOCK * FOOD_MIN_CONSUMPTION;
			PeopleData.FuelConsumption -= 0.1f /FUEL_NEED_STOCK * FUEL_MIN_CONSUMPTION;
			PeopleData.ToolConsumption -= 1.f /TOOL_NEED_STOCK * TOOL_MIN_CONSUMPTION;
			PeopleData.TechConsumption -= 1.f /TECH_NEED_STOCK * TECH_MIN_CONSUMPTION;

			//TODO reputation
		}
	}

	if(!LockNext)
	{
		uint32 TechConsumption = GetRessourceConsumption(Tech, true);
		uint32 BoughtTech = BuyResourcesInSector(Tech, TechConsumption, 0.006); // In Tons
		//if(BoughtTech)
		//	FLOGV("People in %s bought %u tech", *Parent->GetSectorName().ToString(), BoughtTech);
		PeopleData.TechStock += BoughtTech * 1000; // In kg

		if(TechConsumption == BoughtTech)
		{
			PeopleData.TechConsumption += 0.01 * TECH_MIN_CONSUMPTION / PeopleData.TechConsumption;
		}
		else
		{
			PeopleData.FoodConsumption -= 0.1f /FOOD_NEED_STOCK * FOOD_MIN_CONSUMPTION;
			PeopleData.FuelConsumption -= 0.1f /FUEL_NEED_STOCK * FUEL_MIN_CONSUMPTION;
			PeopleData.ToolConsumption -= 0.1f /TOOL_NEED_STOCK * TOOL_MIN_CONSUMPTION;
			PeopleData.TechConsumption -= 1.f /TOOL_NEED_STOCK * TOOL_MIN_CONSUMPTION;
			//TODO reputation
		}
	}

	// TODO use setter to set consumption
	if (PeopleData.FoodConsumption < FOOD_MIN_CONSUMPTION)
	{
		PeopleData.FoodConsumption = FOOD_MIN_CONSUMPTION;
	}
	if (PeopleData.FuelConsumption < FUEL_MIN_CONSUMPTION)
	{
		PeopleData.FuelConsumption = FUEL_MIN_CONSUMPTION;
	}
	if (PeopleData.ToolConsumption < TOOL_MIN_CONSUMPTION)
	{
		PeopleData.ToolConsumption = TOOL_MIN_CONSUMPTION;
	}
	if (PeopleData.TechConsumption < TECH_MIN_CONSUMPTION)
	{
		PeopleData.TechConsumption = TECH_MIN_CONSUMPTION;
	}
}

uint32 UFlarePeople::BuyResourcesInSector(FFlareResourceDescription* Resource, uint32 Quantity, float MarketingRatio)
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

	int64 SectorResourcePrice = Parent->GetResourcePrice(Resource, EFlareResourcePriceContext::ConsumerConsumption);


	uint32 BaseQuantity = FMath::Min(Quantity, PeopleData.Money / (uint32) (SectorResourcePrice));
	uint32 ResourceToBuy = BaseQuantity;

	int64 AdaptativeResourcePrice = (PeopleData.Money > 0 ? PeopleData.Money * MarketingRatio / ResourceToBuy : 0);

	int64 MarketPrice = FMath::Max(AdaptativeResourcePrice, SectorResourcePrice);

	/*if(Quantity > 0)
	{
		FLOGV("%s in %s, Quantity: %d", *Resource->Name.ToString(), *Parent->GetSectorName().ToString(), Quantity);
		FLOGV("%s in %s, SectorResourcePrice: %lld", *Resource->Name.ToString(), *Parent->GetSectorName().ToString(), UFlareGameTools::DisplayMoney(SectorResourcePrice));
		FLOGV("%s in %s, BaseQuantity: %d", *Resource->Name.ToString(), *Parent->GetSectorName().ToString(), BaseQuantity);
		FLOGV("%s in %s, AdaptativeResourcePrice: %lld", *Resource->Name.ToString(), *Parent->GetSectorName().ToString(), UFlareGameTools::DisplayMoney(AdaptativeResourcePrice));
		FLOGV("%s in %s, MarketPrice: %lld", *Resource->Name.ToString(), *Parent->GetSectorName().ToString(), UFlareGameTools::DisplayMoney(MarketPrice));
	}*/


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

			uint32 PartToBuy = FMath::CeilToInt((InitialResourceToBuy * Reputation->Reputation) / (float) ReputationSum);
			PartToBuy = FMath::Min(ResourceToBuy, PartToBuy);

			uint32 BoughtQuantity = BuyInStationForCompany(Resource, PartToBuy, SellingCompanies[CompanyIndex], SellingStations, MarketPrice);
			ResourceToBuy -= BoughtQuantity;

			if(PartToBuy == 0 || BoughtQuantity < PartToBuy)
			{
				SellingCompanies.RemoveAt(CompanyIndex);
			}
		}
	}

	return BaseQuantity - ResourceToBuy;
}

uint32 UFlarePeople::BuyInStationForCompany(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Company, TArray<UFlareSimulatedSpacecraft*>& Stations, int64 ResourcePrice)
{
	uint32 RemainingQuantity = Quantity;


	while(RemainingQuantity > 0)
	{
		// Always buy in the station with the highter fill ratio
		UFlareSimulatedSpacecraft* BestStation = NULL;
		float BestStationFullRatio = 0;

		for (UFlareSimulatedSpacecraft* Station : Stations)
		{
			if(Station->GetCompany() != Company)
			{
				continue;
			}

			uint32 StationFreeSpace = Station->GetCargoBay()->GetFreeSpaceForResource(Resource, Company);
			uint32 StationResourceQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource, Company);

			if (StationFreeSpace == 0 && StationResourceQuantity == 0)
			{
				continue;
			}

			float FullRatio =  (float) StationResourceQuantity / (float) (StationResourceQuantity + StationFreeSpace);

			if(FullRatio > BestStationFullRatio)
			{
				BestStation = Station;
				BestStationFullRatio = FullRatio;
			}
		}

		if(!BestStation)
		{
			break;
		}

		uint32 TakenQuantity = BestStation->GetCargoBay()->TakeResources(Resource, RemainingQuantity, NULL);
		RemainingQuantity -= TakenQuantity;
		uint32 Price = (uint32) (ResourcePrice) * TakenQuantity;
		PeopleData.Money -= Price;
		Company->GiveMoney(Price);

	}

	return Quantity - RemainingQuantity;
}

float UFlarePeople::GetRessourceConsumption(FFlareResourceDescription* Resource, bool WithStock)
{
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Tools = Game->GetResourceCatalog()->Get("tools");
	FFlareResourceDescription* Tech = Game->GetResourceCatalog()->Get("tech");

	if (PeopleData.Population == 0)
	{
		return 0;
	}

	if (Resource == Food)
	{
		if (PeopleData.FoodConsumption < FOOD_MIN_CONSUMPTION)
		{
			PeopleData.FoodConsumption = FOOD_MIN_CONSUMPTION;
		}

		if(!WithStock)
		{
			return PeopleData.Population * PeopleData.FoodConsumption / 1000.f;
		}

		uint32 FoodToHave =  MIN_GENERAL_STOCK + PeopleData.Population * PeopleData.FoodConsumption * FOOD_NEED_STOCK; // In kg
		if(FoodToHave > PeopleData.FoodStock)
		{
			uint32 FoodToBuy = FoodToHave - PeopleData.FoodStock;
			return FoodToBuy / 1000.f;
		}
	}
	else if (Resource == Fuel)
	{
		if (PeopleData.FuelConsumption < FUEL_MIN_CONSUMPTION)
		{
			PeopleData.FuelConsumption = FUEL_MIN_CONSUMPTION;
		}

		if(!WithStock)
		{
			return PeopleData.Population * PeopleData.FuelConsumption / 1000.f;
		}

		int32 FuelToHave =  MIN_GENERAL_STOCK + PeopleData.Population * PeopleData.FuelConsumption * FUEL_NEED_STOCK; // In kg
		if (FuelToHave > PeopleData.FuelStock)
		{
			int32 FuelToBuy = FuelToHave - PeopleData.FuelStock;
			return FuelToBuy / 1000.f;
		}
	}
	else if (Resource == Tools)
	{
		if (PeopleData.ToolConsumption < TOOL_MIN_CONSUMPTION)
		{
			PeopleData.ToolConsumption = TOOL_MIN_CONSUMPTION;
		}

		if(!WithStock)
		{
			return PeopleData.Population * PeopleData.ToolConsumption / 1000.f;
		}

		int32 ToolToHave =  MIN_GENERAL_STOCK + PeopleData.Population * PeopleData.ToolConsumption * TOOL_NEED_STOCK; // In kg
		if (ToolToHave > PeopleData.ToolStock)
		{
			int32 ToolToBuy = ToolToHave - PeopleData.ToolStock;
			return ToolToBuy / 1000.f;
		}
	}
	else if (Resource == Tech)
	{
		if (PeopleData.TechConsumption < TECH_MIN_CONSUMPTION)
		{
			PeopleData.TechConsumption = TECH_MIN_CONSUMPTION;
		}

		if(!WithStock)
		{
			return PeopleData.Population * PeopleData.TechConsumption / 1000.f;
		}

		int32 TechToHave =  MIN_GENERAL_STOCK + PeopleData.Population * PeopleData.TechConsumption * TECH_NEED_STOCK; // In kg
		if (TechToHave > PeopleData.TechStock)
		{
			int32 TechToBuy = TechToHave - PeopleData.TechStock;
			return TechToBuy / 1000.f;
		}
	}

	return 0;
}

void UFlarePeople::GiveBirth(uint32 BirthCount)
{
	if(BirthCount == 0)
	{
		return;
	}

	//FLOGV("Give birth %u people for sector %s", BirthCount, *Parent->GetSectorName().ToString());

	// Increase population
	PeopleData.Population += BirthCount;

	// Money creation
	uint32 NewMoney = BirthCount * MONETARY_CREATION;
	PeopleData.Money += NewMoney;
	Game->GetGameWorld()->WorldMoneyReference += NewMoney;

	IncreaseHappiness(BirthCount * 100 * 2);
	PeopleData.HappinessPoint += BirthCount * 100 * 2; // Birth happiness bonus
}

int32 UFlarePeople::GetBasePopulation()
{
	int32 BasePopulation = 0;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Parent->GetSectorStations().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Parent->GetSectorStations()[SpacecraftIndex];

		if(Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			BasePopulation += 1000 * Station->GetLevel();
		}
		else
		{
			BasePopulation += 100 * Station->GetLevel();
		}
	}
	return BasePopulation;
}

void UFlarePeople::KillPeople(uint32 KillCount)
{
	// Never kill people below the base population

	int32 PeopleToKill = FMath::Min((int32) KillCount, (int32)PeopleData.Population - GetBasePopulation());

	if (PeopleToKill <= 0)
	{
		return;
	}



	//FLOGV("Kill %u people for sector %s", KillCount, *Parent->GetSectorName().ToString());


	float KillRatio = (float) PeopleToKill / (float)PeopleData.Population;
	// Decrease population
	PeopleData.Population -= PeopleToKill;

	// Money destruction (delayed, really destroy on Pay)
	uint32 DestroyedMoney = PeopleToKill * MONETARY_CREATION;
	PeopleData.Dept += DestroyedMoney;
	Game->GetGameWorld()->WorldMoneyReference -= DestroyedMoney;

	DecreaseHappiness(PeopleToKill * 100 * 2); // Death happiness malus

	//Cancel dead hunger
	PeopleData.HungerPoint = (1 - KillRatio) * PeopleData.HungerPoint;

	if(PeopleData.Population == 0)
	{
		ResetPeople();
	}
}

void UFlarePeople::Migrate(UFlareSimulatedSector* DestinationSector, int32 ShipCount)
{
	UFlarePeople* DestinationPeople = DestinationSector->GetPeople();

	if (DestinationPeople->GetBasePopulation() == 0)
	{
		// Empty destination sector: no migration
		return;
	}

	if (DestinationPeople->GetPopulation() == 0
	 || (DestinationPeople->GetWealth() > GetWealth() &&
			DestinationPeople->GetHappiness() > GetHappiness()))
	{
		// Migrate
		int32 MigratingPopulation = FMath::Min(ShipCount * 10, (int32) PeopleData.Population / 2);

		int32 MigratingHappiness = MigratingPopulation * GetHappiness();

		PeopleData.HappinessPoint -= MigratingHappiness;
		PeopleData.Population -= MigratingPopulation;

		DestinationPeople->GetData()->Population += MigratingPopulation;
		DestinationPeople->GetData()->HappinessPoint += MigratingHappiness;
	}
}

void UFlarePeople::IncreaseHappiness(float HappinessPoints)
{
	// Max happiness is 2 so Happiness max is 200 x population
	// Gain happiness is boost when sad and difficult when happy
	//  - Normal gain for 1 as happiness
	//  - 4 times gain for 0 as happiness
	//  - No gain for 2 as happiness
	// Formula: gain = (happiness - 2) ^ 2

	if(HappinessPoints > 0 && HappinessPoints < 1)
	{
		HappinessPoints = 1;
	}

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
	//FLOGV("Pay to people for sector %s Amount=%f", *Parent->GetSectorName().ToString(), Amount/100.)

	uint32 Repayment = 0;
	if(PeopleData.Dept > 0)
	{
		Repayment = FMath::Min(PeopleData.Dept, Amount / 10);
		PeopleData.Dept -= Repayment;
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

void UFlarePeople::PrintInfo()
{
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Tools = Game->GetResourceCatalog()->Get("tools");
	FFlareResourceDescription* Tech = Game->GetResourceCatalog()->Get("tech");



	FLOGV("People for sector %s. ", *Parent->GetSectorName().ToString())
	FLOGV(" - population: %u", PeopleData.Population);
	FLOGV(" - base population: %d", GetBasePopulation());
	FLOGV(" - happiness: %f", GetHappiness());
	//FLOGV(" - Sickness: %f", Sickness);
	//FLOGV(" - Fertility: %f", Fertility);
	FLOG(" - Stocks");
	FLOGV("   - Food: %u", PeopleData.FoodStock);
	FLOGV("   - Fuel: %u", PeopleData.FuelStock);
	FLOGV("   - Tool: %u", PeopleData.ToolStock);
	FLOGV("   - Tech: %u", PeopleData.TechStock);
	FLOG(" - Consumptions (sector)");
	FLOGV("   - Food: %f", GetRessourceConsumption(Food, false));
	FLOGV("   - Fuel: %f", GetRessourceConsumption(Fuel, false));
	FLOGV("   - Tool: %f", GetRessourceConsumption(Tools, false));
	FLOGV("   - Tech: %f", GetRessourceConsumption(Tech, false));
	FLOG(" - Consumptions (per inhabitant)");
	FLOGV("   - Food: %f", PeopleData.FoodConsumption);
	FLOGV("   - Fuel: %f", PeopleData.FuelConsumption);
	FLOGV("   - Tool: %f", PeopleData.ToolConsumption);
	FLOGV("   - Tech: %f", PeopleData.TechConsumption);
	//FLOGV(" - FoodConsumption: %u", FoodConsumption);
	//FLOGV(" - EatenFood: %u", EatenFood);
	//FLOGV(" - FeedPeopleRatio: %f", FeedPeopleRatio);
	//FLOGV(" - Delta Hunger: %u", Hunger);
	FLOGV(" - Hunger: %u", PeopleData.HungerPoint);

	FLOGV(" - Money: %f", PeopleData.Money / 100.);
	FLOGV(" - Dept: %f", PeopleData.Dept / 100.);
}

void UFlarePeople::CheckPopulationDisparition()
{
	// if world population is zero and this sector has habitation. Spawn some people
	if(Game->GetGameWorld()->GetWorldPopulation() > 0)
	{
		return;
	}

	FLOG("WARNING: world people died !!!");

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Parent->GetSectorStations().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Station = Parent->GetSectorStations()[SpacecraftIndex];

		if(Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			// Find an habitation, spawn
			GiveBirth(2000);
			FLOGV("WARNING: give birth at %s", *Parent->GetSectorName().ToString());
			return;
		}
	}
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

float UFlarePeople::GetWealth()
{
	if(PeopleData.Population == 0)
	{
		return 0;
	}
	return (float) PeopleData.Money / ((float) PeopleData.Population);
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
