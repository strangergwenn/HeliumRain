
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareSector.h"
#include "FlareTravel.h"
#include "FlareFleet.h"


/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

UFlareWorld::UFlareWorld(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareWorld::Load(const FFlareWorldSave& Data)
{
	FLOG("UFlareWorld::Load");
	Game = Cast<AFlareGame>(GetOuter());
    WorldData = Data;

	// Init planetarium
	Planetarium = NewObject<UFlareSimulatedPlanetarium>(this, UFlareSimulatedPlanetarium::StaticClass());
	Planetarium->Load();

    // Load all companies
    for (int32 i = 0; i < WorldData.CompanyData.Num(); i++)
    {
		LoadCompany(WorldData.CompanyData[i]);
    }

	// Load sectors
	for (int32 OrbitalBodyIndex = 0; OrbitalBodyIndex < Game->GetSectorCatalog()->OrbitalBodies.Num(); OrbitalBodyIndex++)
	{
		FFlareSectorCelestialBodyDescription* SectorCelestialBodyDescription = &Game->GetSectorCatalog()->OrbitalBodies[OrbitalBodyIndex];
		for (int32 OrbitIndex = 0; OrbitIndex < SectorCelestialBodyDescription->Orbits.Num(); OrbitIndex++)
		{
			FFlareSectorOrbitDescription* SectorOrbitDescription = &SectorCelestialBodyDescription->Orbits[OrbitIndex];
			for (int32 SectorIndex = 0; SectorIndex < SectorOrbitDescription->Sectors.Num(); SectorIndex++)
			{
				const FFlareSectorDescription* SectorDescription = &SectorOrbitDescription->Sectors[SectorIndex];

				// Find save if exist
				FFlareSectorSave* SectorSave = NULL;
				for (int32 i = 0; i < WorldData.SectorData.Num(); i++)
				{
					if (WorldData.SectorData[i].Identifier == SectorDescription->Identifier)
					{
						// Old save found
						SectorSave = &WorldData.SectorData[i];
						break;
					}
				}

				FFlareSectorSave NewSectorData;
				if (!SectorSave)
				{
					// No save, init new sector
					NewSectorData.GivenName = FText();
					NewSectorData.Identifier = SectorDescription->Identifier;
					NewSectorData.LocalTime = 0;

					// Init population
					NewSectorData.PeopleData.Population = 0;
					NewSectorData.PeopleData.BirthPoint = 0;
					NewSectorData.PeopleData.DeathPoint = 0;
					NewSectorData.PeopleData.FoodStock = 0;
					NewSectorData.PeopleData.HappinessPoint = 0;
					NewSectorData.PeopleData.HungerPoint = 0;
					NewSectorData.PeopleData.Money = 0;
					NewSectorData.PeopleData.Dept = 0;



					SectorSave = &NewSectorData;
				}

				FFlareSectorOrbitParameters OrbitParameters;
				OrbitParameters.CelestialBodyIdentifier = SectorCelestialBodyDescription->CelestialBodyIdentifier;
				OrbitParameters.Altitude = SectorOrbitDescription->Altitude;
				OrbitParameters.Phase = SectorDescription->Phase;

				LoadSector(SectorDescription, *SectorSave, OrbitParameters);
			}
		}
	}

	// Load all travels
	for (int32 i = 0; i < WorldData.TravelData.Num(); i++)
	{
		LoadTravel(WorldData.TravelData[i]);
	}

	// Companies post load
	for (int i = 0; i < Companies.Num(); i++)
	{
		Companies[i]->PostLoad();
	}

	WorldMoneyReferenceInit = false;
}


UFlareCompany* UFlareWorld::LoadCompany(const FFlareCompanySave& CompanyData)
{
    UFlareCompany* Company = NULL;

    // Create the new company
	Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
    Company->Load(CompanyData);
    Companies.AddUnique(Company);

	FLOGV("UFlareWorld::LoadCompany : loaded '%s'", *Company->GetCompanyName().ToString());

    return Company;
}


UFlareSimulatedSector* UFlareWorld::LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters)
{
	UFlareSimulatedSector* Sector = NULL;

	// Create the new sector
	Sector = NewObject<UFlareSimulatedSector>(this, UFlareSimulatedSector::StaticClass(), SectorData.Identifier);
	Sector->Load(Description, SectorData, OrbitParameters);
	Sectors.AddUnique(Sector);

	FLOGV("UFlareWorld::LoadSector : loaded '%s'", *Sector->GetSectorName().ToString());

	return Sector;
}


UFlareTravel* UFlareWorld::LoadTravel(const FFlareTravelSave& TravelData)
{
	UFlareTravel* Travel = NULL;

	// Create the new travel
	Travel = NewObject<UFlareTravel>(this, UFlareTravel::StaticClass());
	Travel->Load(TravelData);
	Travels.AddUnique(Travel);

	//FLOGV("UFlareWorld::LoadTravel : loaded travel for fleet '%s'", *Travel->GetFleet()->GetFleetName().ToString());

	return Travel;
}


FFlareWorldSave* UFlareWorld::Save(UFlareSector* ActiveSector)
{
	WorldData.CompanyData.Empty();
	WorldData.SectorData.Empty();
	WorldData.TravelData.Empty();

	TArray<FFlareSpacecraftSave> SpacecraftData;
	if (ActiveSector)
	{
		ActiveSector->Save(SpacecraftData);

		// Reload  spacecrafts. Have to be done before companies save
		for (int i = 0 ; i < SpacecraftData.Num(); i++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = FindSpacecraft(SpacecraftData[i].Immatriculation);
			Spacecraft->Load(SpacecraftData[i]);
		}
	}

	// Companies
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];

		//FLOGV("UFlareWorld::Save : saving company ('%s')", *Company->GetName());
		FFlareCompanySave* TempData = Company->Save();
		WorldData.CompanyData.Add(*TempData);
	}

	// Sectors
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		//FLOGV("UFlareWorld::Save : saving sector ('%s')", *Sector->GetName());

		FFlareSectorSave* TempData;
		if (ActiveSector && Sector->GetIdentifier() == ActiveSector->GetIdentifier())
		{
			//FLOG("  sector saved as active sector");
			TempData = ActiveSector->Save(SpacecraftData);
		}
		else
		{
			TempData = Sector->Save();
		}
		WorldData.SectorData.Add(*TempData);
	}

	// Travels
	for (int i = 0; i < Travels.Num(); i++)
	{
		UFlareTravel* Travel = Travels[i];

		//FLOGV("UFlareWorld::Save : saving travel for ('%s')", *Travel->GetFleet()->GetFleetName().ToString());
		FFlareTravelSave* TempData = Travel->Save();
		WorldData.TravelData.Add(*TempData);
	}

	return &WorldData;
}


bool UFlareWorld::CheckIntegrity()
{
	bool Integrity = true;
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];

		for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];
			if (!Station->IsStation())
			{
				FLOGV(" !!! Integrity error: Station %s in %s is not a station", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());
				Integrity = false;
			}
		}
	}

	return Integrity;
}

void UFlareWorld::Simulate()
{

	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	/**
	 *  End previous day
	 */

	// Finish player battles
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		EFlareSectorBattleState::Type BattleState = Sector->GetSectorBattleState(PlayerCompany);

		if(BattleState != EFlareSectorBattleState::NoBattle && BattleState != EFlareSectorBattleState::BattleWon)
		{
			// Destroy all player ships
			TArray<UFlareSimulatedSpacecraft*> ShipToDestroy;

			for (int ShipIndex = 0; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
			{
				if(Sector->GetSectorShips()[ShipIndex]->GetCompany() == PlayerCompany)
				{
					ShipToDestroy.Add(Sector->GetSectorShips()[ShipIndex]);
				}
			}

			for (int ShipIndex = 0; ShipIndex < ShipToDestroy.Num(); ShipIndex++)
			{
				PlayerCompany->DestroySpacecraft(ShipToDestroy[ShipIndex]);
			}
		}
	}

	// TODO company battles


	// AI. Play them in random order
	TArray<UFlareCompany*> CompaniesToSimulateAI = Companies;
	while(CompaniesToSimulateAI.Num())
	{
		int32 Index = FMath::RandRange(0, CompaniesToSimulateAI.Num() - 1);
		CompaniesToSimulateAI[Index]->SimulateAI();
		CompaniesToSimulateAI.RemoveAt(Index);
	}

	// Base revenue between company. 1 per 100000 is share between all companies
	uint32 SharingCompanyCount = 0;
	int64 SharedPool = 0;

	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company =Companies[CompanyIndex];
		if (Company != PlayerCompany)
		{
			int64 MoneyToTake = Company->GetMoney() / 1000;
			if (Company->TakeMoney(MoneyToTake))
			{
				SharedPool +=MoneyToTake;
			}
			SharingCompanyCount++;
		}
	}

	// Share poll
	int64 PoolPart = SharedPool / SharingCompanyCount;
	int64 PoolBonus = SharedPool % SharingCompanyCount; // The bonus is given to a random company

	int32 BonusIndex = FMath::RandRange(0, SharingCompanyCount - 1);

	FLOGV("Share part amount is : %d", PoolPart/100);
	int32 SharingCompanyIndex = 0;
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company =Companies[CompanyIndex];
		if (Company != PlayerCompany)
		{
			Company->GiveMoney(PoolPart);

			if(CompanyIndex == BonusIndex)
			{
				Company->GiveMoney(PoolBonus);
			}

			SharingCompanyIndex++;
		}
	}



	// Check money integrity
	if (! WorldMoneyReferenceInit)
	{
		WorldMoneyReference = GetWorldMoney();
		WorldMoneyReferenceInit = true;
	}
	else
	{
		uint64 WorldMoney = GetWorldMoney();

		if (WorldMoneyReference != WorldMoney)
		{
			FLOGV("WARNING : World money integrity failure : world contain %lld credits but reference is %lld", WorldMoney, WorldMoneyReference)
		}
	}


	/**
	 *  Begin day
	 */
	WorldData.Date++;


	// Factories
	FLOG("Factories");
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		Factories[FactoryIndex]->Simulate();
	}

	FLOG("Automatic transport");
	// Automatic transport
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SimulateTransport();
	}

	// Peoples
	FLOG("Peoples");
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->GetPeople()->Simulate();
	}



	FLOG("Trade routes");

	// Trade routes
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		TArray<UFlareTradeRoute*>& TradeRoutes = Companies[CompanyIndex]->GetCompanyTradeRoutes();

		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			TradeRoutes[RouteIndex]->Simulate();
		}
	}

	// Travels
	for (int TravelIndex = 0; TravelIndex < Travels.Num(); TravelIndex++)
	{
		Travels[TravelIndex]->Simulate();
	}

	// Reputation stabilization
	for (int CompanyIndex1 = 0; CompanyIndex1 < Companies.Num(); CompanyIndex1++)
	{
		UFlareCompany* Company1 =Companies[CompanyIndex1];

		for (int CompanyIndex2 = 0; CompanyIndex2 < Companies.Num(); CompanyIndex2++)
		{
			UFlareCompany* Company2 =Companies[CompanyIndex2];

			if(Company1 == Company2)
			{
				continue;
			}

			float Reputation = Company1->GetReputation(Company2);
			if(Reputation != 0.f)
			{
				Company1->GiveReputation(Company2, -0.01 * FMath::Sign(Reputation), false);
			}
		}
	}

	// Price variation.
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SimulatePriceVariation();
	}
	// Price homogenization
	SimulatePriceHomogenization();


	// Process events
}

void UFlareWorld::FastForward()
{
	Simulate();
	// TODO repair
	/*int64 FastForwardEnd = WorldData.Time + 86400;

	while(WorldData.Time < FastForwardEnd)
	{
		TArray<FFlareWorldEvent> NextEvents = GenerateEvents();

		if (NextEvents.Num() == 0)
		{
			// Nothing will append in futur
			return;
		}

		FFlareWorldEvent& NextEvent = NextEvents[0];

		if (NextEvent.Time < WorldData.Time)
		{
			FLOGV("Fast forward fail: next event is in the past. Current time is %ld but next event time %ld", WorldData.Time, NextEvent.Time);
			return;
		}

		int64 TimeJump = NextEvent.Time - WorldData.Time;

		if(NextEvent.Time > FastForwardEnd)
		{
			TimeJump = FastForwardEnd - WorldData.Time;
		}

		Simulate(TimeJump);

		if (NextEvent.Visibility == EFlareEventVisibility::Blocking)
		{
			// End fast forward
			break;
		}
	}*/
}

void UFlareWorld::ForceDate(int64 Date)
{
	while(WorldData.Date < Date)
	{
		Simulate();
	}
}

inline static bool EventDateComparator (const FFlareWorldEvent& ip1, const FFlareWorldEvent& ip2)
 {
	 return (ip1.Date < ip2.Date);
 }

TArray<FFlareWorldEvent> UFlareWorld::GenerateEvents(UFlareCompany* PointOfView)
{
	TArray<FFlareWorldEvent> NextEvents;

	// TODO Implements PointOfView

	// Generate travel events
	for (int TravelIndex = 0; TravelIndex < Travels.Num(); TravelIndex++)
	{
		FFlareWorldEvent TravelEvent;

		TravelEvent.Date = WorldData.Date + Travels[TravelIndex]->GetRemainingTravelDuration();
		TravelEvent.Visibility = EFlareEventVisibility::Blocking;
		NextEvents.Add(TravelEvent);
	}

	// Generate factory events
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		FFlareWorldEvent *FactoryEvent = Factories[FactoryIndex]->GenerateEvent();
		if (FactoryEvent)
		{
			NextEvents.Add(*FactoryEvent);
		}
	}

	NextEvents.Sort(&EventDateComparator);

	return NextEvents;
}

void UFlareWorld::ClearFactories(UFlareSimulatedSpacecraft *ParentSpacecraft)
{
	for (int FactoryIndex = Factories.Num() -1 ; FactoryIndex >= 0; FactoryIndex--)
	{
		UFlareFactory* Factory = Factories[FactoryIndex];
		if (Factory->GetParent() == ParentSpacecraft)
		{
			Factories.RemoveAt(FactoryIndex);
		}
	}
}

void UFlareWorld::AddFactory(UFlareFactory* Factory)
{
	Factories.Add(Factory);
}

UFlareTravel* UFlareWorld::StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector)
{
	if (!TravelingFleet->CanTravel())
	{
		return NULL;
	}

	if (TravelingFleet->IsTraveling())
	{
		TravelingFleet->GetCurrentTravel()->ChangeDestination(DestinationSector);
		return TravelingFleet->GetCurrentTravel();
	}
	else if (TravelingFleet->GetCurrentSector() == DestinationSector)
	{
		//Try to start a travel to current sector
		return NULL;
	}
	else
	{
		// Remove immobilized ships
		TravelingFleet->RemoveImmobilizedShips();

		// Make the fleet exit the sector
		UFlareSimulatedSector* OriginSector = TravelingFleet->GetCurrentSector();
		OriginSector->RetireFleet(TravelingFleet);

		// Create the travel
		FFlareTravelSave TravelData;
		TravelData.FleetIdentifier = TravelingFleet->GetIdentifier();
		TravelData.OriginSectorIdentifier = OriginSector->GetIdentifier();
		TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();
		TravelData.DepartureDate = GetDate();
		return LoadTravel(TravelData);
	}
}

void UFlareWorld::DeleteTravel(UFlareTravel* Travel)
{
	Travels.Remove(Travel);
}

void UFlareWorld::SimulatePriceHomogenization()
{
	float ContaminationFactor = 0.1f;
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		float PriceMin = 0.0;
		float PriceMax = 0.0;
		uint32 ResourceStocks = 0;
		uint32 ResourceDailyConsumption = 0;

		float MaxPriceToResumeProduction = 0;


		for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = Sectors[SectorIndex];
			float SectorPrice = Sector->GetPreciseResourcePrice(Resource);
			if(SectorIndex == 0)
			{
				PriceMin = SectorPrice;
				PriceMax = SectorPrice;
			}
			else
			{
				PriceMin = FMath::Min(SectorPrice, PriceMin);
				PriceMax = FMath::Max(SectorPrice, PriceMax);
			}

			for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
			{
				UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];

				for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
				{
					UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
					if (!Factory->IsActive() || !Factory->IsNeedProduction() || !Factory->HasOutputFreeSpace())
					{
						// No resources needed or full
						break;
					}

					// TODO Count only the real usage

					// Input flow
					for (int32 FactoryResourceIndex = 0; FactoryResourceIndex < Factory->GetInputResourcesCount(); FactoryResourceIndex++)
					{
						FFlareResourceDescription* FactoryResource = Factory->GetInputResource(FactoryResourceIndex);
						if(Resource == FactoryResource)
						{
							ResourceDailyConsumption += Factory->GetInputResourceQuantity(FactoryResourceIndex) / Factory->GetProductionDuration();
							break;
						}

					}

					// Output flow
					for (int32 FactoryResourceIndex = 0; FactoryResourceIndex < Factory->GetOutputResourcesCount(); FactoryResourceIndex++)
					{
						FFlareResourceDescription* FactoryResource = Factory->GetOutputResource(FactoryResourceIndex);
						if(Resource == FactoryResource)
						{
							float Margin = Factory->GetMarginRatio();

							if(Margin >= UFlareFactory::MinMargin)
							{
								continue;
							}

							float ProductionBalance = (float) Factory->GetProductionBalance();

							float SellPrice = ProductionBalance / Factory->GetMarginRatio();
							float CostPrice = SellPrice - ProductionBalance;

							float NeedInflation = CostPrice / (0.8 * SellPrice);


							float PriceToResumeProduction = (SellPrice * NeedInflation) / Factory->GetOutputResourceQuantity(FactoryResourceIndex);

							MaxPriceToResumeProduction = FMath::Max(MaxPriceToResumeProduction, PriceToResumeProduction);

							/*FLOGV("Margin: %f",  Margin);
							FLOGV("ProductionBalance: %f",  ProductionBalance);
							FLOGV("SellPrice: %f",  SellPrice);
							FLOGV("CostPrice: %f",  CostPrice);
							FLOGV("NeedInflation: %f",  NeedInflation);
							FLOGV("PriceToResumeProduction: %f",  PriceToResumeProduction);
*/




							break;
						}

					}
				}

				ResourceStocks += Station->GetCargoBay()->GetResourceQuantity(Resource);
			}

			ResourceDailyConsumption += Sector->GetPeople()->GetRessourceConsumption(Resource);
		}

		// Travelling ship stocks

		for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
		{
			UFlareCompany* Company =Companies[CompanyIndex];


			for (int32 ShipIndex = 0 ; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
			{
				UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];
				if (Ship->GetCurrentSector() == NULL)
				{
					ResourceStocks += Ship->GetCargoBay()->GetResourceQuantity(Resource);
				}
			}
		}


		/*FLOGV("PriceMax: %f",  PriceMax);
		FLOGV("PriceMin: %f",  PriceMin);*/

		float WorldPrice = (PriceMax + PriceMin) / 2;


		float StockLimit = 50;

		float StockRatio = StockLimit;
		float Variation = 0;
		// TODO Exclude maintenance resources

		if (ResourceDailyConsumption > 0)
		{
			StockRatio = (float) ResourceStocks/ (float) ResourceDailyConsumption;
		}

		if(StockRatio < StockLimit)
		{
			// Less than 10 day of stock
			Variation = 0.1 * (1 - StockRatio / StockLimit); //1.0% with no stocks. 0% with 100 day of stocks
		}
		else if (StockRatio < StockLimit * 4)
		{
			Variation = -0.05 *  (StockRatio - StockLimit) / (StockLimit * 3); // -0.5% with 400 day of stock. 0% with 100 day of stocks
		}
		else
		{
			Variation = -0.05;
		}

		WorldPrice *= (1 + Variation / 100.f);

		FLOGV("World price for %s : %f", *Resource->Name.ToString(), WorldPrice / 100);

		if(StockRatio < StockLimit && WorldPrice < MaxPriceToResumeProduction * 1.25)
		{
			WorldPrice = MaxPriceToResumeProduction * 1.25;
		}

		/*FLOGV("World price for %s : %f", *Resource->Name.ToString(), WorldPrice / 100);
		FLOGV("  - stocks: %d", ResourceStocks);
		FLOGV("  - daily consumption: %d", ResourceDailyConsumption);
		FLOGV("  - stock ratio: %f days", StockRatio);
		FLOGV("  - variation : %f percent", Variation);
		FLOGV("  - MaxPriceToResumeProduction : %f", MaxPriceToResumeProduction / 100);
*/

		WorldPrice = FMath::Min(UFlareSector::GetDefaultResourcePrice(Resource) * 20, WorldPrice);


		for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
		{
			float CurrentPrice = Sectors[SectorIndex]->GetPreciseResourcePrice(Resource);

			float NewPrice = CurrentPrice * (1 - ContaminationFactor) + WorldPrice * ContaminationFactor;
			 Sectors[SectorIndex]->SetPreciseResourcePrice(Resource, NewPrice);
			 //FLOGV(" - price in %s : %f", *Sectors[SectorIndex]->GetSectorName().ToString(), NewPrice / 100);

		}
	}
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

UFlareCompany* UFlareWorld::FindCompany(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetIdentifier() == Identifier)
		{
			return Company;
		}
	}
	return NULL;
}

UFlareCompany* UFlareWorld::FindCompanyByShortName(FName CompanyShortName) const
{
	// Find company
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetShortName() == CompanyShortName)
		{
			return Company;
		}
	}
	return NULL;
}

UFlareSimulatedSector* UFlareWorld::FindSector(FName Identifier) const
{
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		if (Sector->GetIdentifier() == Identifier)
		{
			return Sector;
		}
	}
	return NULL;
}

UFlareSimulatedSector* UFlareWorld::FindSectorBySpacecraft(FName SpacecraftIdentifier) const
{
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		for (int j = 0; j < Sector->GetSectorSpacecrafts().Num(); j++)
		{
			if (Sector->GetSectorSpacecrafts()[j]->GetDescription()->Identifier == SpacecraftIdentifier)
			{
				return Sector;
			}
		}
	}
	return NULL;
}

UFlareFleet* UFlareWorld::FindFleet(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		UFlareFleet* Fleet = Company->FindFleet(Identifier);
		if (Fleet)
		{
			return Fleet;
		}
	}
	return NULL;
}

UFlareTradeRoute* UFlareWorld::FindTradeRoute(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		UFlareTradeRoute* TradeRoute = Company->FindTradeRoute(Identifier);
		if (TradeRoute)
		{
			return TradeRoute;
		}
	}
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareWorld::FindSpacecraft(FName ShipImmatriculation)
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation);
		if (Spacecraft)
		{
			return Spacecraft;
		}
	}
	return NULL;
}


uint64 UFlareWorld::GetWorldMoney()
{
	// World money is the sum of company money + factory money + people money

	uint64 CompanyMoney = 0;
	uint64 FactoryMoney = 0;
	uint64 PeopleMoney = 0;

	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];

		CompanyMoney += Company->GetMoney();

		TArray<UFlareSimulatedSpacecraft*>& Spacecrafts = Company->GetCompanySpacecrafts();
		for (int ShipIndex = 0; ShipIndex < Spacecrafts.Num() ; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Spacecrafts[ShipIndex];

			for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];
				FactoryMoney += Factory->GetReservedMoney();
			}
		}
	}

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		PeopleMoney += Sectors[SectorIndex]->GetPeople()->GetMoney();
	}

	uint64 WorldMoney = CompanyMoney + FactoryMoney + PeopleMoney;

	//FLOGV("World money: %lld", WorldMoney);
	//FLOGV("  - company: %lld", CompanyMoney);
	//FLOGV("  - factory: %lld", FactoryMoney);
	//FLOGV("  - people : %lld", PeopleMoney);

	return WorldMoney;
}
