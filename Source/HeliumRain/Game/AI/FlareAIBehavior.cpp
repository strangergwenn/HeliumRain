#include "../../Flare.h"
#include "FlareAIBehavior.h"

#include "../FlareGame.h"
#include "../FlareCompany.h"
#include "../FlareScenarioTools.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareAIBehavior::UFlareAIBehavior(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareAIBehavior::Load(UFlareCompany* ParentCompany)
{
	Company = ParentCompany;
	Game = Company->GetGame();
	ST = Game->GetScenarioTools();

	GenerateAffilities();
}

void UFlareAIBehavior::GenerateAffilities()
{
	// Reset resource affilities
	ResourceAffilities.Empty();


	// Default behavior


	SetResourceAffilities(1.f);

	// If above 5, will become exclusive
	SetSectorAffilities(1.f);

	StationCapture = 1.f;
	TradingBuy = 1.f;
	TradingSell = 1.f;
	TradingBoth = 0.5f;

	Shipyard = 1.0;
	Consumer = 1.0;
	Maintenance = 0.5;

	// Budjet
	BudgetTechnology = 1.0;
	BudgetMilitary = 1.0;
	BudgetStation = 1.0;
	BudgetTrade = 1.0;

	ArmySize = 5.0;
	Agressivity = 1.0;
	Bold = 1.0;
	Peaceful = 1.0;


	// Pirate base
	SetSectorAffility(ST->Boneyard, 0.f);


	if(Company == ST->Pirates)
	{
		// Pirates
		// -------
		//
		// Pirate don't trade, and are only interested in getting money by force
		// All resource affilities are set to 0
		// They never build station
		// They never research science

		// They have 2 mode : agressive or moderate

		// They need a shipyard, if they have a shipyard they will steal resources,
		// or buy one to build ship, and fill and defend their base
		// They always want graveyard

		// They need some cargo to buy resources for their shipyard and arsenal

		SetResourceAffilities(0.f);


		SetSectorAffilities(0.f);

		SetSectorAffility(ST->Boneyard, 10.f);


		// Only buy
		TradingSell = 0.f;
		TradingBoth = 0.f;

		// Don't capture station. Change in recovery
		StationCapture = 0.f;

		ArmySize = 50.0;
		Agressivity = 10.0;
		Bold = 10.0;


		// Budjet
		BudgetTechnology = 0.0;
		BudgetMilitary = 1.0;
		BudgetStation = 0.0;
		BudgetTrade = 0.1;

	}
	else if(Company == ST->GhostWorksShipyards)
	{
		// Love Hela and don't like Nema

		Shipyard = 5.0;

		SetSectorAffilitiesByMoon(ST->Nema,0.5f);
		SetSectorAffilitiesByMoon(ST->Hela, 6.f);

		// Budjet
		BudgetTechnology = 1.0;
		BudgetMilitary = 2.0;
		BudgetStation = 1.0;
		BudgetTrade = 1.0;
	}
	else if(Company == ST->MiningSyndicate)
	{
		// Mining specialist.
		// Love raw material
		// Like hard work
		SetResourceAffility(ST->Water, 10.f);
		SetResourceAffility(ST->Silica, 10.f);
		SetResourceAffility(ST->IronOxyde, 10.f);
		SetResourceAffility(ST->Hydrogen, 2.f);

		// Budjet
		BudgetTechnology = 1.0;
		BudgetMilitary = 1.0;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;
	}
	else if(Company == ST->HelixFoundries)
	{
		// Like hard factory, and Anka.
		// Base at Outpost
		SetResourceAffility(ST->Steel, 10.f);
		SetResourceAffility(ST->Tools, 10.f);
		SetResourceAffility(ST->Tech, 5.f);
		SetSectorAffilitiesByMoon(ST->Anka, 6.f);
		SetSectorAffility(ST->Outpost, 10.f);


		// Budjet
		BudgetTechnology = 1.5;
		BudgetMilitary = 1.0;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;
	}
	else if(Company == ST->Sunwatch)
	{
		// Like hard factory, and Anka.
		// Base at Outpost
		SetResourceAffility(ST->Fuel, 10.f);


		// Budjet
		BudgetTechnology = 1.0;
		BudgetMilitary = 1.0;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;
	}
	else if(Company == ST->IonLane)
	{
		BudgetTechnology = 1.0;
		BudgetMilitary = 2.0;
		BudgetStation = 0.1;
		BudgetTrade = 2.0;

		ArmySize = 10.0;
		Agressivity = 2.0;
	}
	else if(Company == ST->UnitedFarmsChemicals)
	{
		// Like chemisty
		SetResourceAffility(ST->Food, 10.f);
		SetResourceAffility(ST->Carbon, 5.f);
		SetResourceAffility(ST->Methane, 5.f);

		// Budjet
		BudgetTechnology = 1.5;
		BudgetMilitary = 1.0;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;
	}
	else if(Company == ST->NemaHeavyWorks)
	{
		// Like Nema and heavy work
		SetResourceAffility(ST->FleetSupply, 2.f);
		SetResourceAffility(ST->Steel, 5.f);
		SetResourceAffility(ST->Tools, 5.f);
		SetResourceAffility(ST->Tech, 5.f);
		SetSectorAffilitiesByMoon(ST->Nema, 5.f);

		Shipyard = 3.0;

		// Budjet
		BudgetTechnology = 1.5;
		BudgetMilitary = 1.0;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;
	}
	else if(Company == ST->AxisSupplies)
	{
		// Assure fleet supply disponibility
		SetResourceAffility(ST->FleetSupply, 2.f);
		SetResourceAffility(ST->Food, 2.f);

		Shipyard = 0.0;
		Consumer = 5.0;
		Maintenance = 10.0;

		// Budjet
		BudgetTechnology = 1.0;
		BudgetMilitary = 0.5;
		BudgetStation = 2.0;
		BudgetTrade = 2.0;

		ArmySize = 1.0;
		Agressivity = 0.0;
		Peaceful = 10.0;
	}

}

void UFlareAIBehavior::SetResourceAffilities(float Value)
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		SetResourceAffility(Resource, Value);
	}
}

void UFlareAIBehavior::SetResourceAffility(FFlareResourceDescription* Resource, float Value)
{
	if(ResourceAffilities.Contains(Resource)){
		ResourceAffilities[Resource] = Value;
	}
	else
	{
		ResourceAffilities.Add(Resource, Value);
	}
}


void UFlareAIBehavior::SetSectorAffilities(float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];
		SetSectorAffility(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffility(UFlareSimulatedSector* Sector, float Value)
{
	if(SectorAffilities.Contains(Sector)){
		SectorAffilities[Sector] = Value;
	}
	else
	{
		SectorAffilities.Add(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffilitiesByMoon(FFlareCelestialBody *CelestialBody, float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		if(Sector->GetOrbitParameters()->CelestialBodyIdentifier == CelestialBody->Identifier)
		{
			SetSectorAffility(Sector, Value);
		}
	}
}
