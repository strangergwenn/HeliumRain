
#include "../Flare.h"

#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareSector.h"
#include "FlareTravel.h"
#include "FlareFleet.h"
#include "FlareBattle.h"

#include "../Data/FlareSectorCatalogEntry.h"
#include "../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareWorld"

#define FLEET_SUPPLY_CONSUMPTION_STATS 365

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
	TArray<UFlareSectorCatalogEntry*> SectorList = Game->GetSectorCatalog();
	for (int32 SectorIndex = 0; SectorIndex < SectorList.Num(); SectorIndex++)
	{
		const FFlareSectorDescription* SectorDescription = &SectorList[SectorIndex]->Data;

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
			NewSectorData.IsTravelSector = false;

			// Init population
			NewSectorData.PeopleData.Population = 0;
			NewSectorData.PeopleData.BirthPoint = 0;
			NewSectorData.PeopleData.DeathPoint = 0;
			NewSectorData.PeopleData.FoodStock = 0;
			NewSectorData.PeopleData.FuelStock = 0;
			NewSectorData.PeopleData.ToolStock = 0;
			NewSectorData.PeopleData.TechStock = 0;
			NewSectorData.PeopleData.FoodConsumption = 0;
			NewSectorData.PeopleData.FuelConsumption = 0;
			NewSectorData.PeopleData.ToolConsumption = 0;
			NewSectorData.PeopleData.TechConsumption = 0;
			NewSectorData.PeopleData.HappinessPoint = 0;
			NewSectorData.PeopleData.HungerPoint = 0;
			NewSectorData.PeopleData.Money = 0;
			NewSectorData.PeopleData.Dept = 0;


			SectorSave = &NewSectorData;
		}

		FFlareSectorOrbitParameters OrbitParameters;
		OrbitParameters.CelestialBodyIdentifier = SectorDescription->CelestialBodyIdentifier;
		OrbitParameters.Altitude = SectorDescription->Altitude;
		OrbitParameters.Phase = SectorDescription->Phase;

		LoadSector(SectorDescription, *SectorSave, OrbitParameters);
	}

	// Load all travels
	for (int32 i = 0; i < WorldData.TravelData.Num(); i++)
	{
		LoadTravel(WorldData.TravelData[i]);
	}

	WorldMoneyReferenceInit = false;


	if (WorldData.FleetSupplyConsumptionStats.MaxSize != FLEET_SUPPLY_CONSUMPTION_STATS)
	{
		WorldData.FleetSupplyConsumptionStats.Resize(FLEET_SUPPLY_CONSUMPTION_STATS);
	}
}

void UFlareWorld::PostLoad()
{
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

	//FLOGV("UFlareWorld::LoadCompany : loaded '%s'", *Company->GetCompanyName().ToString());

    return Company;
}


UFlareSimulatedSector* UFlareWorld::LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters)
{
	UFlareSimulatedSector* Sector = NULL;

	// Create the new sector
	Sector = NewObject<UFlareSimulatedSector>(this, UFlareSimulatedSector::StaticClass(), SectorData.Identifier);
	Sector->Load(Description, SectorData, OrbitParameters);
	Sectors.AddUnique(Sector);

	//FLOGV("UFlareWorld::LoadSector : loaded '%s'", *Sector->GetSectorName().ToString());

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


FFlareWorldSave* UFlareWorld::Save()
{
	WorldData.CompanyData.Empty();
	WorldData.SectorData.Empty();
	WorldData.TravelData.Empty();

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

		WorldData.SectorData.Add(*Sector->Save());
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


void UFlareWorld::CompanyMutualAssistance()
{
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	// Base revenue between company. 1 per 100000 is share between all companies
	uint32 SharingCompanyCount = 0;
	int64 SharedPool = 0;

	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company =Companies[CompanyIndex];
		if (Company != PlayerCompany)
		{
			int64 MoneyToTake = Company->GetMoney() / 1000;
			if (MoneyToTake > 0)
			{
				if (Company->TakeMoney(MoneyToTake))
				{
					SharedPool +=MoneyToTake;
				}
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
				FLOGV("WARNING : World integrity failure : station %s in %s is not a station", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());
				Integrity = false;
			}
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
		int64 WorldMoney = GetWorldMoney();

		if (WorldMoneyReference != WorldMoney)
		{
			FLOGV("WARNING : World integrity failure : world contain %lld credits but reference is %lld", WorldMoney, WorldMoneyReference)
			Integrity = false;
		}
	}

	//  Check companyintegrity
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetCompanySpacecrafts().Num() != Company->GetCompanyShips().Num() + Company->GetCompanyStations().Num())
		{
			FLOGV("WARNING : World integrity failure : %s have %d spacecraft but %d ships and %s stations", *Company->GetCompanyName().ToString(),
				  Company->GetCompanySpacecrafts().Num(),
				  Company->GetCompanyShips().Num(),
				  Company->GetCompanyStations().Num());
			Integrity = false;
		}

		// Ships
		for (int32 ShipIndex = 0 ; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];

			UFlareSimulatedSector* ShipSector = Ship->GetCurrentSector();

			if(ShipSector)
			{
				if (Ship->GetCurrentFleet() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s in %s is in no fleet",
						  *Ship->GetImmatriculation().ToString(),
						  *ShipSector->GetSectorName().ToString());
					Integrity = false;
				}

				if(!ShipSector->GetSectorShips().Contains(Ship))
				{
					FLOGV("WARNING : World integrity failure : %s in %s but not in sector ship list",
						  *Ship->GetImmatriculation().ToString(),
						  *ShipSector->GetSectorName().ToString());
					Integrity = false;
				}
			}
			else
			{
				if (Ship->GetCurrentFleet() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s not in sector but in no fleet",
						  *Ship->GetImmatriculation().ToString());
					Integrity = false;

				}
				else if(Ship->GetCurrentFleet()->GetCurrentTravel() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s in fleet %s but not in sector and not in travel",
						  *Ship->GetImmatriculation().ToString(),
						  *Ship->GetCurrentFleet()->GetFleetName().ToString());
					if(Ship->GetCurrentFleet()->GetCurrentSector() != NULL)
					{
						FLOGV("  - %s in %s",
							  *Ship->GetCurrentFleet()->GetFleetName().ToString(),
							  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						if (Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorSpacecrafts().Contains(Ship))
						{
							FLOGV("  - %s contains the ship in its list",
								  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						}
						else
						{
							FLOGV("  - %s don't contains the ship in its list",
								  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						}

						Ship->GetCurrentFleet()->GetCurrentSector()->AddFleet(Ship->GetCurrentFleet());
						FLOGV("Fix integrity : set %s to %s",
							   *Ship->GetImmatriculation().ToString(),
							  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
					}
					else
					{
						FLOGV("  - %s in no sector", *Ship->GetCurrentFleet()->GetFleetName().ToString());
						if (Ship->GetCompany()->GetKnownSectors().Num() > 0)
						{
							Ship->GetCompany()->GetKnownSectors()[0]->AddFleet(Ship->GetCurrentFleet());
							FLOGV("Fix integrity : set %s to %s",
							   *Ship->GetImmatriculation().ToString(),
							  *Ship->GetCurrentSector()->GetSectorName().ToString());
						}
					}
					Integrity = false;
				}
			}
		}

		// Fleets
		for (int32 FleetIndex = 0 ; FleetIndex < Company->GetCompanyFleets().Num(); FleetIndex++)
		{
			UFlareFleet* Fleet = Company->GetCompanyFleets()[FleetIndex];

			if(Fleet->GetShipCount() == 0)
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is empty",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
			}

			if(Fleet->GetCurrentSector() == NULL )
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is not in a sector",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
			}
			else if(Fleet->GetCurrentSector()->IsTravelSector() && Fleet->GetCurrentTravel() == NULL)
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is in a travel sector and not in travel",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
			}
		}
	}
	return Integrity;
}

void UFlareWorld::Simulate()
{
	double StartTs = FPlatformTime::Seconds();
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();
	Game->GetPC()->MarkAsSimulating();

	/**
	 *  End previous day
	 */
	FLOGV("** Simulate day %d", WorldData.Date);

	FLOG("* Simulate > Battles");
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		// Check if battle
		bool HasBattle = false;
		for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
		{
			UFlareCompany* Company = Companies[CompanyIndex];

			if (Company == PlayerCompany && Sector == GetGame()->GetPC()->GetPlayerShip()->GetCurrentSector())
			{
				// Local sector, don't check if the player want fight
				continue;
			}

			FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);

			if(!BattleState.WantFight())
			{
				// Don't want fight
				continue;
			}

			FLOGV("%s want fight in %s", *Company->GetCompanyName().ToString(),
				  *Sector->GetSectorName().ToString());

			HasBattle = true;
			break;
		}

		if (HasBattle)
		{
			UFlareBattle* Battle = NewObject<UFlareBattle>(this, UFlareBattle::StaticClass());
			Battle->Load(Sector);
			Battle->Simulate();
		}

		// Remove destroyed spacecraft
		TArray<UFlareSimulatedSpacecraft*> SpacecraftToRemove;

		for (int32 SpacecraftIndex = 0 ; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

			if(!Spacecraft->GetDamageSystem()->IsAlive() && !Spacecraft->GetDescription()->IsSubstation)
			{
				SpacecraftToRemove.Add(Spacecraft);
			}
		}

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftToRemove.Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = SpacecraftToRemove[SpacecraftIndex];
			Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
		}
	}

	FLOG("* Simulate > AI");
	// AI. Play them in random order
	TArray<UFlareCompany*> CompaniesToSimulateAI = Companies;
	while(CompaniesToSimulateAI.Num())
	{
		int32 Index = FMath::RandRange(0, CompaniesToSimulateAI.Num() - 1);
		CompaniesToSimulateAI[Index]->SimulateAI();
		CompaniesToSimulateAI.RemoveAt(Index);
	}

	// Clear bombs
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->ClearBombs();
	}

	CompanyMutualAssistance();
	CheckIntegrity();

	/**
	 *  Begin day
	 */
	FLOG("* Simulate > New day");

	WorldData.Date++;

	// Write FS consumption stats
	WorldData.FleetSupplyConsumptionStats.Append(WorldData.DailyFleetSupplyConsumption);
	WorldData.DailyFleetSupplyConsumption = 0;

	// End trade, intercept, repair and refill, operations
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Companies[CompanyIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Company->GetCompanySpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Company->GetCompanySpacecrafts()[SpacecraftIndex];
			if (!Spacecraft->IsStation())
			{
				Spacecraft->SetTrading(false);
				Spacecraft->SetIntercepted(false);
			}
			Spacecraft->SetRepairing(false);
			Spacecraft->SetRefilling(false);
		}
	}

	// Ship capture
	ProcessShipCapture();
	ProcessStationCapture();

	// Factories
	FLOG("* Simulate > Factories");
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		Factories[FactoryIndex]->Simulate();
	}

	// Peoples
	FLOG("* Simulate > Peoples");
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->GetPeople()->Simulate();
	}


	FLOG("* Simulate > Trade routes");

	// Trade routes
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		TArray<UFlareTradeRoute*>& TradeRoutes = Companies[CompanyIndex]->GetCompanyTradeRoutes();

		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			TradeRoutes[RouteIndex]->Simulate();
		}
	}
	FLOG("* Simulate > Travels");
	// Travels
	TArray<UFlareTravel*> TravelsToProcess = Travels;
	for (int TravelIndex = 0; TravelIndex < TravelsToProcess.Num(); TravelIndex++)
	{
		TravelsToProcess[TravelIndex]->Simulate();
	}

	FLOG("* Simulate > Reputation");
	// Reputation stabilization
	for (UFlareCompany* Company : Companies)
	{
		Company->GiveReputationToOthers(-Company->GetShame(), false);
	}

	FLOG("* Simulate > Prices");
	// Price variation.
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SimulatePriceVariation();
	}

	// People money migration
	SimulatePeopleMoneyMigration();

	// Process events

	// Swap Prices.
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SwapPrices();
	}
	
	// Update reserve ships
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->UpdateReserveShips();
	}

	// Player being attacked ?
	ProcessIncomingPlayerEnemy();

	// Lets AI check if in battle
	CheckAIBattleState();

	
	double EndTs = FPlatformTime::Seconds();
	FLOGV("** Simulate day %d done in %.6fs", WorldData.Date-1, EndTs- StartTs);

	Game->GetQuestManager()->OnNextDay();

	GameLog::DaySimulated(WorldData.Date);
}

void UFlareWorld::CheckAIBattleState()
{
	for (UFlareCompany* Company : Companies)
	{
		Company->GetAI()->CheckBattleState();
	}
}

void UFlareWorld::ProcessShipCapture()
{
	TArray<UFlareSimulatedSpacecraft*> ShipToCapture;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorShips().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorShips()[SpacecraftIndex];

			if(Spacecraft->IsHarpooned()) {
				// Capture the ship if the following condition is ok :
				// - The harpoon owner must be at war this the ship owner
				// - The harpoon owner must in won state : military presence only for him

				UFlareCompany* HarpoonOwner = Spacecraft->GetHarpoonCompany();


				FFlareSectorBattleState  HarpoonOwnerBattleState = Sector->GetSectorBattleState(HarpoonOwner);
				FFlareSectorBattleState  SpacecraftOwnerBattleState = Sector->GetSectorBattleState(Spacecraft->GetCompany());


				if(HarpoonOwner
						&& HarpoonOwner->GetWarState(Spacecraft->GetCompany()) == EFlareHostility::Hostile
						&& !HarpoonOwnerBattleState.HasDanger)
				{
					// If battle won state, this mean the Harpoon owner has at least one dangerous ship
					// This also mean that no company at war with this company has a military ship

					ShipToCapture.Add(Spacecraft);
					// Need to keep the harpoon for capture process
				}
				else if(!SpacecraftOwnerBattleState.HasDanger)
				{
					Spacecraft->SetHarpooned(NULL);
				}
			}
		}
	}

	FLOGV("ShipToCapture %d", ShipToCapture.Num());


	// Capture consist on :
	// - Kill rotation and velocity
	// - respawn the ship at near location
	// - change its owner
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipToCapture.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ShipToCapture[SpacecraftIndex];

		UFlareCompany* HarpoonOwner = Spacecraft->GetHarpoonCompany();
		UFlareCompany* Owner = Spacecraft->GetCompany();
		UFlareSimulatedSector* Sector =  Spacecraft->GetCurrentSector();
		FVector SpawnLocation =  Spacecraft->GetData().Location;
		FRotator SpawnRotation =  Spacecraft->GetData().Rotation;
		FFlareSpacecraftSave Data = Spacecraft->GetData();
		FFlareSpacecraftDescription* ShipDescription = Spacecraft->GetDescription();

		Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
		UFlareSimulatedSpacecraft* NewShip = Sector->CreateSpacecraft(ShipDescription, HarpoonOwner, SpawnLocation, SpawnRotation, &Data);

		Owner->GiveReputationToOthers(5, false);
		Owner->GiveReputation(HarpoonOwner, -10, false);
		HarpoonOwner->GiveReputationToOthers(-5, false);

		if (GetGame()->GetPC()->GetCompany() == HarpoonOwner)
		{
			FFlareMenuParameterData MenuData;
			MenuData.Sector = Sector;

			GetGame()->GetPC()->Notify(LOCTEXT("ShipCaptured", "Ship captured"),
				FText::Format(LOCTEXT("ShipCapturedFormat", "You have captured a {0}-class ship in {1}. Its new name is {2}."),
							  NewShip->GetDescription()->Name,
							  FText::FromString(Sector->GetSectorName().ToString()),
							  FText::FromString(NewShip->GetImmatriculation().ToString())),
				FName("ship-captured"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Sector,
				MenuData);
		}
	}
}

void UFlareWorld::ProcessStationCapture()
{
	TArray<UFlareSimulatedSpacecraft*> StationToCapture;
	TArray<UFlareCompany*> StationCapturer;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorStations()[SpacecraftIndex];

			FFlareSectorBattleState StationOwnerBattleState = Sector->GetSectorBattleState(Spacecraft->GetCompany());

			if (!StationOwnerBattleState.HasDanger)
			{
				// The station is not being captured
				Spacecraft->ResetCapture();
				continue;
			}

			// Find capturing companies
			for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
			{
				UFlareCompany* Company = Companies[CompanyIndex];

				if ((Company->GetWarState(Spacecraft->GetCompany()) != EFlareHostility::Hostile)
					|| Sector->GetSectorBattleState(Company).HasDanger)
				{
					// Friend don't capture and not winner don't capture
					continue;
				}

				// Capture
				int32 CompanyCapturePoint = Sector->GetCompanyCapturePoints(Company);
				if(Spacecraft->TryCapture(Company, CompanyCapturePoint))
				{
					StationToCapture.Add(Spacecraft);
					StationCapturer.Add(Company);
					break;
				}
			}
		}
	}

	FLOGV("Station to capture %d", StationToCapture.Num());

	// Capture consist on :
	// - Kill rotation and velocity
	// - respawn the ship at near location
	// - change its owner
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < StationToCapture.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = StationToCapture[SpacecraftIndex];
		UFlareCompany* Capturer = StationCapturer[SpacecraftIndex];
		UFlareCompany* Owner = Spacecraft->GetCompany();

		UFlareSimulatedSector* Sector =  Spacecraft->GetCurrentSector();
		FVector SpawnLocation =  Spacecraft->GetData().Location;
		FRotator SpawnRotation =  Spacecraft->GetData().Rotation;
		FFlareSpacecraftSave Data = Spacecraft->GetData();
		FFlareSpacecraftDescription* ShipDescription = Spacecraft->GetDescription();



		Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
		UFlareSimulatedSpacecraft* NewShip = Sector->CreateSpacecraft(ShipDescription, Capturer, SpawnLocation, SpawnRotation, &Data);
		Owner->GiveReputationToOthers(30, false);
		Owner->GiveReputation(Capturer, -40, false);
		Capturer->GiveReputationToOthers(-50, false);
		// Shame
		float Shame = 0.1 * NewShip->GetLevel();
		Capturer->GiveShame(Shame);
		Owner->GiveShame(-Shame);

		if (GetGame()->GetPC()->GetCompany() == Capturer)
		{
			FFlareMenuParameterData MenuData;
			MenuData.Sector = Sector;

			GetGame()->GetPC()->Notify(LOCTEXT("StationCaptured", "Station captured"),
				FText::Format(LOCTEXT("StationCapturedFormat", "You have captured a {0} in {1}. Its new name is {2}."),
							NewShip->GetDescription()->Name,
							FText::FromString(Sector->GetSectorName().ToString()),
							FText::FromString(NewShip->GetImmatriculation().ToString())),
				FName("station-captured"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Sector,
				MenuData);
		}
	}

}

void UFlareWorld::SimulatePeopleMoneyMigration()
{
	for (int SectorIndexA = 0; SectorIndexA < Sectors.Num(); SectorIndexA++)
	{
		UFlareSimulatedSector* SectorA = Sectors[SectorIndexA];

		for (int SectorIndexB = SectorIndexA + 1; SectorIndexB < Sectors.Num(); SectorIndexB++)
		{
			UFlareSimulatedSector* SectorB = Sectors[SectorIndexB];

			// Money and people migration
			float PopulationA = SectorA->GetPeople()->GetPopulation();
			float PopulationB = SectorB->GetPeople()->GetPopulation();


			if(PopulationA == 0 && PopulationB == 0)
			{
				// 2 sector without population. Do nothing
				continue;
			}
			else if (PopulationA  == 0)
			{
				// Origin sector has no population so it leak it's money
				uint32 TransfertA = SectorA->GetPeople()->GetMoney() / 1000;
				SectorA->GetPeople()->TakeMoney(TransfertA);
				SectorB->GetPeople()->Pay(TransfertA);
			}
			else if (PopulationB  == 0)
			{
				// Destination sector has no population so it leak it's money
				uint32 TransfertB = SectorB->GetPeople()->GetMoney() / 1000;
				SectorB->GetPeople()->TakeMoney(TransfertB);
				SectorA->GetPeople()->Pay(TransfertB);
			}
			else
			{
				// Both have population. The wealthier leak.
				float WealthA = SectorA->GetPeople()->GetWealth();
				float WealthB = SectorB->GetPeople()->GetWealth();
				float TotalWealth = WealthA + WealthB;

				float PercentRatio = 0.05f; // 5% at max
				float TravelDuration = FMath::Max(1.f, (float) UFlareTravel::ComputeTravelDuration(this, SectorA, SectorB));

				if(TotalWealth > 0)
				{
					if(WealthA > WealthB)
					{
						float LeakRatio = PercentRatio * 2 * ((WealthA / TotalWealth) - 0.5f) / TravelDuration;
						uint32 TransfertA = LeakRatio * SectorA->GetPeople()->GetMoney();
						SectorA->GetPeople()->TakeMoney(TransfertA);
						SectorB->GetPeople()->Pay(TransfertA);
					}
					else
					{
						float LeakRatio = 0.05f * 2 * ((WealthB / TotalWealth) - 0.5f) / TravelDuration; // 5% at max
						uint32 TransfertB = LeakRatio * SectorB->GetPeople()->GetMoney();
						SectorB->GetPeople()->TakeMoney(TransfertB);
						SectorA->GetPeople()->Pay(TransfertB);
					}
				}
			}
		}
	}
}

void UFlareWorld::FastForward()
{
	Simulate();
}

void UFlareWorld::ProcessIncomingPlayerEnemy()
{
	if (GetGame()->GetPC()->GetPlayerShip())
	{
		UFlareSimulatedSector* PlayerSector = GetGame()->GetPC()->GetPlayerShip()->GetCurrentSector();

		// Notification data
		FText SingleShip = LOCTEXT("ShipSingle", "ship");
		FText MultipleShips = LOCTEXT("ShipPlural", "ships");
		int32 LightShipCount = 0;
		int32 HeavyShipCount = 0;
		FText CompanyName;

		// Count incoming ships
		for (int32 TravelIndex = 0; TravelIndex < GetTravels().Num(); TravelIndex++)
		{
			UFlareTravel* Travel = GetTravels()[TravelIndex];
			
			if (Travel->GetDestinationSector() == PlayerSector
				&& Travel->GetRemainingTravelDuration() == 1
				&& Travel->GetFleet()->GetFleetCompany()->GetPlayerWarState() == EFlareHostility::Hostile)
			{
				CompanyName = Travel->GetFleet()->GetFleetCompany()->GetCompanyName();
				LightShipCount += Travel->GetFleet()->GetMilitaryShipCountBySize(EFlarePartSize::S);
				HeavyShipCount += Travel->GetFleet()->GetMilitaryShipCountBySize(EFlarePartSize::L);
			}
		}

		// Warn player
		if (LightShipCount > 0 || HeavyShipCount > 0)
		{
			// Fighters
			FText LightShipText;
			if (LightShipCount > 0)
			{
				LightShipText = FText::Format(LOCTEXT("PlayerAttackedLightsFormat", "{0} light {1}"),
					FText::AsNumber(LightShipCount),
					(LightShipCount > 1) ? MultipleShips : SingleShip);
			}

			// Heavies
			FText HeavyShipText;
			if (HeavyShipCount > 0)
			{
				HeavyShipText = FText::Format(LOCTEXT("PlayerAttackedHeaviesFormat", "{0} heavy {1}"),
					FText::AsNumber(HeavyShipCount),
					(HeavyShipCount > 1) ? MultipleShips : SingleShip);

				if (LightShipCount > 0)
				{
					HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
				}
			}

			// Notify
			FFlareMenuParameterData Data;
			Data.Sector = PlayerSector;
			GetGame()->GetPC()->Notify(LOCTEXT("PlayerAttackedSoon", "Incoming attack"),
				FText::Format(LOCTEXT("PlayerAttackedSoonFormat", "Your current sector {0} will be attacked tomorrow by {1} with {2}{3}. Prepare for battle."),
					PlayerSector->GetSectorName(),
					CompanyName,
					LightShipText,
					HeavyShipText),
				FName("travel-raid-soon"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Sector,
				Data);
		}
	}
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

void UFlareWorld::OnFleetSupplyConsumed(int32 Quantity)
{
	WorldData.DailyFleetSupplyConsumption += Quantity;
}

UFlareTravel* UFlareWorld::	StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector)
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
		UFlareSimulatedSector* OriginSector = TravelingFleet->GetCurrentSector();


		// Remove intercepted ships
		if(OriginSector->GetSectorBattleState(TravelingFleet->GetFleetCompany()).HasDanger)
		{
			int32 InterceptedShips = TravelingFleet->InterceptShips();

			if (InterceptedShips > 0)
			{
				if(TravelingFleet->GetFleetCompany() == Game->GetPC()->GetCompany())
				{
					FText SingleShip = LOCTEXT("ShipSingle", "ship");
					FText MultipleShips = LOCTEXT("ShipPlural", "ships");
					FText ShipText = (InterceptedShips > 1) ? MultipleShips : SingleShip;

					FFlareMenuParameterData Data;
					Data.Sector = OriginSector;
					GetGame()->GetPC()->Notify(LOCTEXT("ShipsIntercepted", "Ships intercepted"),
						FText::Format(LOCTEXT("ShipCapturedFormat", "{0} {1} have been intercepted trying to escape from {2}."),
									  FText::AsNumber(InterceptedShips),
									  ShipText,
									  FText::FromString(OriginSector->GetSectorName().ToString())),
						FName("ships-intersepted"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Sector,
						Data);
				}

				if(InterceptedShips == TravelingFleet->GetShipCount())
				{
					// All ships intercepted
					return NULL;
				}
			}
		}

		// Remove immobilized ships
		TravelingFleet->RemoveImmobilizedShips();


		// Make the fleet exit the sector

		OriginSector->RetireFleet(TravelingFleet);

		// Create the travel
		FFlareTravelSave TravelData;
		TravelData.FleetIdentifier = TravelingFleet->GetIdentifier();
		TravelData.OriginSectorIdentifier = OriginSector->GetIdentifier();
		TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();
		TravelData.DepartureDate = GetDate();
		UFlareTravel::InitTravelSector(TravelData.SectorData);
		UFlareTravel* Travel = LoadTravel(TravelData);

		return Travel;
	}
}

void UFlareWorld::DeleteTravel(UFlareTravel* Travel)
{
	Travels.Remove(Travel);
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

	FCHECK(false);
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

	FCHECK(false);
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
	for (UFlareCompany* Company : Companies)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation);
		if (Spacecraft)
		{
			return Spacecraft;
		}
	}

	// Now check destroyed ships
	for (UFlareCompany* Company : Companies)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation, true);
		if (Spacecraft)
		{
			return Spacecraft;
		}
	}

	return NULL;
}


int64 UFlareWorld::GetWorldMoney()
{
	// World money is the sum of company money + factory money + people money

	int64 CompanyMoney = 0;
	int64 FactoryMoney = 0;
	int64 PeopleMoney = 0;

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
		PeopleMoney -= Sectors[SectorIndex]->GetPeople()->GetDept();
	}

	int64 WorldMoney = CompanyMoney + FactoryMoney + PeopleMoney;

	//FLOGV("World money: %lld", WorldMoney);
	//FLOGV("  - company: %lld", CompanyMoney);
	//FLOGV("  - factory: %lld", FactoryMoney);
	//FLOGV("  - people : %lld", PeopleMoney);

	return WorldMoney;
}

uint32 UFlareWorld::GetWorldPopulation()
{
	uint32 WorldPopulation = 0;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		WorldPopulation += Sectors[SectorIndex]->GetPeople()->GetPopulation();
	}

	return WorldPopulation;
}
#undef LOCTEXT_NAMESPACE
