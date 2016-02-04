
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

	FLOGV("UFlareWorld::LoadTravel : loaded travel for fleet '%s'", *Travel->GetFleet()->GetFleetName().ToString());

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

		FLOGV("UFlareWorld::Save : saving company ('%s')", *Company->GetName());
		FFlareCompanySave* TempData = Company->Save();
		WorldData.CompanyData.Add(*TempData);
	}

	// Sectors
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		FLOGV("UFlareWorld::Save : saving sector ('%s')", *Sector->GetName());

		FFlareSectorSave* TempData;
		if (ActiveSector && Sector->GetIdentifier() == ActiveSector->GetIdentifier())
		{
			FLOG("  sector saved as active sector");
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

		FLOGV("UFlareWorld::Save : saving travel for ('%s')", *Travel->GetFleet()->GetFleetName().ToString());
		FFlareTravelSave* TempData = Travel->Save();
		WorldData.TravelData.Add(*TempData);
	}

	return &WorldData;
}


void UFlareWorld::Simulate()
{
	WorldData.Date++;

	// Peoples
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->GetPeople()->Simulate();
	}

	// Automatic transport
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SimulateTransport();
	}

	// Factories
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		Factories[FactoryIndex]->Simulate();
	}

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

	// AI
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		Companies[CompanyIndex]->SimulateAI();
	}
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
	if (TravelingFleet->IsTraveling())
	{
		TravelingFleet->GetCurrentTravel()->ChangeDestination(DestinationSector);
		return TravelingFleet->GetCurrentTravel();
	}
	else
	{
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
	FLOGV("UFlareWorld::DeleteTravel : remove travel for fleet '%s'", *Travel->GetFleet()->GetFleetName().ToString());

	Travels.Remove(Travel);
}

bool UFlareWorld::TransfertResources(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource, uint32 Quantity)
{
	// TODO Check docking capabilities
	bool TransfertOK = true;

	uint32 ResourcePrice = SourceSpacecraft->GetCurrentSectorInterface()->GetResourcePrice(Resource);
	uint32 QuantityToTake = Quantity;

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Limit transaction bay available money
		uint32 MaxAffordableQuantity = DestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice;
		QuantityToTake = FMath::Min(QuantityToTake, MaxAffordableQuantity);
	}

	uint32 TakenResources = SourceSpacecraft->GetCargoBay()->TakeResources(Resource, QuantityToTake);
	uint32 GivenResources = DestinationSpacecraft->GetCargoBay()->GiveResources(Resource, TakenResources);
	uint32 PaybackResources = TakenResources - GivenResources;
	if (PaybackResources > 0)
	{
		SourceSpacecraft->GetCargoBay()->GiveResources(Resource, PaybackResources);
	}

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Pay
		uint32 Price = SourceSpacecraft->GetCurrentSectorInterface()->GetResourcePrice(Resource) * GivenResources;
		DestinationSpacecraft->GetCompany()->TakeMoney(Price);
		SourceSpacecraft->GetCompany()->GiveMoney(Price);
	}

	return TransfertOK;
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
