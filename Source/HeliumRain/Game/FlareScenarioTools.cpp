
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "FlareScenarioTools.h"


#define LOCTEXT_NAMESPACE "FlareScenarioToolsInfo"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareScenarioTools::UFlareScenarioTools(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Public methods
----------------------------------------------------*/

void UFlareScenarioTools::Init(UFlareCompany* Company, FFlarePlayerSave* Player)
{
	Game = Cast<AFlareGame>(GetOuter());
	World = Game->GetGameWorld();
	PlayerCompany = Company;
	PlayerData = Player;

	// Notable sectors (Nema)
	TheDepths =   World->FindSector("the-depths");
	FirstLight =  World->FindSector("first-light");
	MinersHome =  World->FindSector("miners-home");
	Anomaly =     World->FindSector("anomaly");
	BlueHeart =   World->FindSector("blue-heart");
	Lighthouse =  World->FindSector("lighthouse");
	TheSpire =    World->FindSector("the-spire");
	
	// Notable sectors (Anka)
	Outpost =     World->FindSector("outpost");
	Crossroads =  World->FindSector("crossroads");
	Colossus =    World->FindSector("colossus");
	TheDig =      World->FindSector("the-dig");

	// Notable sectors (Hela)
	FrozenRealm = World->FindSector("frozen-realm");
	ShoreOfIce =  World->FindSector("shore-of-ice");
	Ruins =       World->FindSector("ruins");

	// Notable sectors (Asta)
	Decay =       World->FindSector("decay");

	// Notable sectors (Adena)
	Solitude =    World->FindSector("solitude");

	// Companies
	MiningSyndicate =      World->FindCompanyByShortName("MSY");
	HelixFoundries =       World->FindCompanyByShortName("HFR");
	Sunwatch =             World->FindCompanyByShortName("SUN");
	IonLane =              World->FindCompanyByShortName("ION");
	UnitedFarmsChemicals = World->FindCompanyByShortName("UFC");
	GhostWorksShipyards =  World->FindCompanyByShortName("GWS");

	// Resources
	Water =    Game->GetResourceCatalog()->Get("h2o");
	Food =     Game->GetResourceCatalog()->Get("food");
	Fuel =     Game->GetResourceCatalog()->Get("fuel");
	Plastics = Game->GetResourceCatalog()->Get("plastics");
	Hydrogen = Game->GetResourceCatalog()->Get("h2");
	Helium =   Game->GetResourceCatalog()->Get("he3");
	Silica =   Game->GetResourceCatalog()->Get("sio2");
	Steel =    Game->GetResourceCatalog()->Get("steel");
	Tools =    Game->GetResourceCatalog()->Get("tools");
	Tech =     Game->GetResourceCatalog()->Get("tech");

	// Ships
	ShipSolen = "ship-solen";
	ShipOmen = "ship-omen";
	ShipAtlas = "ship-atlas";
	ShipGhoul = "ship-ghoul";
	ShipOrca = "ship-orca";
	ShipDragon = "ship-dragon";
	ShipInvader = "ship-invader";
	ShipLeviathan = "ship-leviathan";

	// Stations
	StationFarm = "station-farm";
	StationSolarPlant = "station-solar-plant";
	StationHabitation = "station-habitation";
	StationIceMine = "station-ice-mine";
	StationIronMine = "station-iron-mine";
	StationSilicaMine = "station-silica-mine";
	StationSteelworks = "station-steelworks";
	StationToolFactory = "station-tool-factory";
	StationHydrogenPump = "station-h2-pump";
	StationMethanePump = "station-ch4-pump";
	StationHeliumPump = "station-he3-pump";
	StationCarbonRefinery = "station-carbon-refinery";
	StationPlasticsRefinery = "station-plastics-refinery";
	StationArsenal = "station-arsenal";
	StationShipyard = "station-shipyard";
	StationHub = "station-hub";
}

void UFlareScenarioTools::GenerateEmptyScenario()
{
}

void UFlareScenarioTools::GenerateFighterScenario()
{
	FLOG("UFlareScenarioTools::GenerateFighterScenario");

	SetupWorld();
	CreatePlayerShip(FirstLight, "ship-ghoul");
}

void UFlareScenarioTools::GenerateFreighterScenario()
{
	FLOG("UFlareScenarioTools::GenerateFreighterScenario");

	SetupWorld();
	CreatePlayerShip(FirstLight, "ship-omen");
}

void UFlareScenarioTools::GenerateDebugScenario()
{

	/*----------------------------------------------------
		Player setup
	----------------------------------------------------*/

	// Discover known sectors
	if (!PlayerData->QuestData.PlayTutorial)
	{
		for (int SectorIndex = 0; SectorIndex < World->GetSectors().Num(); SectorIndex++)
		{
			PlayerCompany->DiscoverSector(World->GetSectors()[SectorIndex]);
		}
	}

	CreatePlayerShip(MinersHome, "ship-omen");
	CreatePlayerShip(FrozenRealm, "ship-omen");
	PlayerCompany->GiveMoney(10000000);
	CreateStations(StationIceMine, PlayerCompany, FrozenRealm, 1);
	CreateStations(StationIceMine, PlayerCompany, MinersHome, 1);
}


/*----------------------------------------------------
	Common world
----------------------------------------------------*/

void UFlareScenarioTools::SetupWorld()
{
	// Setup common stuff
	SetupAsteroids();
	SetupArtifacts();

	// Discover public sectors
	SetupKnownSectors(PlayerCompany);
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(HelixFoundries);
	SetupKnownSectors(Sunwatch);
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(UnitedFarmsChemicals);
	SetupKnownSectors(IonLane);
	SetupKnownSectors(GhostWorksShipyards);

	// Player-exclusive sectors
	PlayerCompany->DiscoverSector(TheDig);
	PlayerCompany->DiscoverSector(Decay);
	PlayerCompany->DiscoverSector(Anomaly);
	PlayerCompany->DiscoverSector(Decay);
	PlayerCompany->DiscoverSector(ShoreOfIce);
	PlayerCompany->DiscoverSector(Solitude);

	// Company setup
	PlayerCompany->GiveMoney(5000000);
	MiningSyndicate->GiveMoney(100000000);
	HelixFoundries->GiveMoney(100000000);
	Sunwatch->GiveMoney(100000000);
	UnitedFarmsChemicals->GiveMoney(100000000);
	IonLane->GiveMoney(100000000);
	GhostWorksShipyards->GiveMoney(100000000);

	// Population setup
	BlueHeart->GetPeople()->GiveBirth(3000);

	// Create initial stations
	CreateStations(StationFarm, UnitedFarmsChemicals, Lighthouse, 4);
	CreateStations(StationSolarPlant, Sunwatch, Lighthouse, 8);
	CreateStations(StationHabitation, IonLane, BlueHeart, 4);
	CreateStations(StationIronMine, MiningSyndicate, MinersHome, 6);

	CreateStations(StationIceMine, MiningSyndicate, FrozenRealm, 3);

	CreateStations(StationSteelworks, HelixFoundries, Outpost, 3);
	CreateStations(StationToolFactory, HelixFoundries, Outpost, 1);
	CreateStations(StationMethanePump, UnitedFarmsChemicals, TheSpire, 4);
	CreateStations(StationHydrogenPump, HelixFoundries, TheSpire, 3);


	CreateStations(StationCarbonRefinery, UnitedFarmsChemicals, TheSpire, 4);
	CreateStations(StationPlasticsRefinery, UnitedFarmsChemicals, TheSpire, 4);
	CreateStations(StationShipyard, GhostWorksShipyards, FrozenRealm, 1);
	CreateStations(StationArsenal, GhostWorksShipyards, FrozenRealm, 1);

	// Create hubs
	CreateStations(StationHub, IonLane, Crossroads, 4);
	CreateStations(StationHub, IonLane, Lighthouse, 2);
	CreateStations(StationHub, IonLane, BlueHeart, 1);
	CreateStations(StationHub, IonLane, MinersHome, 1);
	CreateStations(StationHub, IonLane, Outpost, 1);
	CreateStations(StationHub, IonLane, TheSpire, 1);
	CreateStations(StationHub, IonLane, FrozenRealm, 1);

	// Create cargos
	CreateShips(ShipOmen, IonLane, Crossroads, 60);
	CreateShips(ShipAtlas, IonLane, Crossroads, 2);
	CreateShips(ShipOmen, MiningSyndicate, MinersHome, 10);
	CreateShips(ShipOmen, HelixFoundries, Outpost, 10);
	CreateShips(ShipOmen, UnitedFarmsChemicals, TheSpire, 10);
	CreateShips(ShipOmen, Sunwatch, Lighthouse, 10);

}

void UFlareScenarioTools::SetupAsteroids()
{
	CreateAsteroids(FirstLight, 8, FVector(5, 10, 5));
	CreateAsteroids(MinersHome, 35, FVector(13, 50, 7));
	CreateAsteroids(TheDepths, 31, FVector(5, 25, 10));

	CreateAsteroids(Outpost, 12, FVector(8, 20, 5));
	CreateAsteroids(TheDig, 22, FVector(15, 50, 7));

	CreateAsteroids(FrozenRealm, 19, FVector(9, 50, 5));
	CreateAsteroids(ShoreOfIce, 22, FVector(17, 25, 9));
	CreateAsteroids(Ruins, 15, FVector(7, 18, 9));
}

void UFlareScenarioTools::SetupArtifacts()
{
}

void UFlareScenarioTools::SetupKnownSectors(UFlareCompany* Company)
{
	// Notable sectors (Nema)
	Company->DiscoverSector(TheDepths);
	Company->DiscoverSector(FirstLight);
	Company->DiscoverSector(MinersHome);
	Company->DiscoverSector(BlueHeart);
	Company->DiscoverSector(Lighthouse);
	Company->DiscoverSector(TheSpire); // TODO GWENN

	// Notable sectors (Anka)
	Company->DiscoverSector(Outpost);
	//Company->DiscoverSector(Colossus); // TODO GWENN
	Company->DiscoverSector(Crossroads);

	// Notable sectors (Hela)
	Company->DiscoverSector(FrozenRealm);
	Company->DiscoverSector(ShoreOfIce);
	Company->DiscoverSector(Ruins);
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void UFlareScenarioTools::CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class)
{
	UFlareSimulatedSpacecraft* InitialShip = Sector->CreateShip(Class, PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
}

void UFlareScenarioTools::CreateAsteroids(UFlareSimulatedSector* Sector, int32 Count, FVector DistributionShape)
{
	FLOGV("UFlareScenarioTools::CreateAsteroids : Trying to spawn %d asteroids", Count);

	// Compute parameters
	float MaxAsteroidDistance = 15000;
	int32 AsteroidCount = 0;
	int32 CellCount = DistributionShape.X * DistributionShape.Y * DistributionShape.Z * 4;
	int32 FailCount = 0;

	while (AsteroidCount < Count && FailCount < 5000)
	{
		for (int32 X = -DistributionShape.X; X <= DistributionShape.X; X++)
		{
			for (int32 Y = -DistributionShape.Y; Y <= DistributionShape.Y; Y++)
			{
				for (int32 Z = -DistributionShape.Z; Z <= DistributionShape.Z; Z++)
				{
					if (FMath::RandHelper(CellCount) <= Count)
					{
						bool CanSpawn = true;
						FVector AsteroidLocation = MaxAsteroidDistance * FVector(X, Y, Z);

						// Check for collision
						TArray<FFlareAsteroidSave> Asteroids = Sector->Save()->AsteroidData;
						for (int32 Index = 0; Index < Asteroids.Num(); Index++)
						{
							if ((Asteroids[Index].Location - AsteroidLocation).Size() < MaxAsteroidDistance)
							{
								CanSpawn = false;
								break;
							}
						}

						// Spawn the asteroid
						if (CanSpawn)
						{
							FString AsteroidName = FString("asteroid") + FString::FromInt(AsteroidCount);
							Sector->CreateAsteroid(FMath::RandRange(0, 5), FName(*AsteroidName), AsteroidLocation);
							AsteroidCount++;
						}
						else
						{
							FailCount++;
						}
					}
				}
			}
		}
	}

	FLOGV("UFlareScenarioTools::CreateAsteroids : Spawned %d asteroids", AsteroidCount);
}

void UFlareScenarioTools::CreateShips(FName ShipClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count)
{
	for (uint32 Index = 0; Index < Count; Index++)
	{
		Sector->CreateShip(ShipClass, Company, FVector::ZeroVector);
	}
}

void UFlareScenarioTools::CreateStations(FName StationClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count)
{
	for (uint32 Index = 0; Index < Count; Index++)
	{
		UFlareSimulatedSpacecraft* Station = Sector->CreateStation(StationClass, Company, FVector::ZeroVector, FRotator::ZeroRotator);

		if (!Station)
		{
			continue;
		}

		// Give input resources
		if (Station->GetFactories().Num() > 0)
		{
			UFlareFactory* ActiveFactory = Station->GetFactories()[0];

			for (int32 ResourceIndex = 0; ResourceIndex < ActiveFactory->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* Resource = &ActiveFactory->GetDescription()->CycleCost.InputResources[ResourceIndex];

				Station->GetCargoBay()->GiveResources(&Resource->Resource->Data, Station->GetCargoBay()->GetSlotCapacity());
			}
		}
		
		// Give customer resources
		if (Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
				Station->GetCargoBay()->GiveResources(Resource, Station->GetCargoBay()->GetSlotCapacity());
			}
		}

		// Give customer resources
		if (Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
				Station->GetCargoBay()->GiveResources(Resource, Station->GetCargoBay()->GetSlotCapacity() / 2);
			}
		}


	}
}

#undef LOCTEXT_NAMESPACE
