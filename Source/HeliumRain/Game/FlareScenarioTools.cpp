
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Economy/FlareCargoBay.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "FlareScenarioTools.h"


#define LOCTEXT_NAMESPACE "FlareScenarioToolsInfo"


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareScenarioTools::UFlareScenarioTools(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareScenarioTools::Init(UFlareCompany* Company, FFlarePlayerSave* Player)
{
	Game = Cast<AFlareGame>(GetOuter());
	World = Game->GetGameWorld();
	PlayerCompany = Company;
	PlayerData = Player;

	Nema = World->GetPlanerarium()->FindCelestialBody("nema");
	Anka = World->GetPlanerarium()->FindCelestialBody("anka");
	Hela = World->GetPlanerarium()->FindCelestialBody("hela");
	Asta = World->GetPlanerarium()->FindCelestialBody("asta");
	Adena = World->GetPlanerarium()->FindCelestialBody("adena");

	// Notable sectors (Nema)
	TheDepths =   World->FindSector("the-depths");
	FirstLight =  World->FindSector("first-light");
	MinersHome =  World->FindSector("miners-home");
	Anomaly =     World->FindSector("anomaly");
	BlueHeart =   World->FindSector("blue-heart");
	Lighthouse =  World->FindSector("lighthouse");
	BlueShores =  World->FindSector("blue-shores");
	TheSpire =    World->FindSector("the-spire");
	Pendulum =    World->FindSector("pendulum");
	
	// Notable sectors (Anka)
	Outpost =     World->FindSector("outpost");
	Crossroads =  World->FindSector("crossroads");
	Colossus =    World->FindSector("colossus");
	TheDig =      World->FindSector("the-dig");
	TheForge =    World->FindSector("the-forge");

	// Notable sectors (Hela)
	FrozenRealm = World->FindSector("frozen-realm");
	NightsHome =  World->FindSector("nights-home");
	ShoreOfIce =  World->FindSector("shore-of-ice");
	Ruins =       World->FindSector("ruins");
	WinterJunction = World->FindSector("winter-junction");

	// Notable sectors (Asta)
	Decay =       World->FindSector("decay");
	Boneyard =    World->FindSector("boneyard");
	Daedalus =    World->FindSector("daedalus");

	// Notable sectors (Adena)
	Solitude =    World->FindSector("solitude");
	Serenity =    World->FindSector("serenity");
	Tranquility = World->FindSector("tranquility");


	// Companies
	MiningSyndicate =      World->FindCompanyByShortName("MSY");
	HelixFoundries =       World->FindCompanyByShortName("HFR");
	Sunwatch =             World->FindCompanyByShortName("SUN");
	IonLane =              World->FindCompanyByShortName("ION");
	UnitedFarmsChemicals = World->FindCompanyByShortName("UFC");
	GhostWorksShipyards =  World->FindCompanyByShortName("GWS");
	NemaHeavyWorks =       World->FindCompanyByShortName("NHW");
	Pirates =              World->FindCompanyByShortName("PIR");
	AxisSupplies =         World->FindCompanyByShortName("AXS");

	// Resources
	Water =    Game->GetResourceCatalog()->Get("h2o");
	Food =     Game->GetResourceCatalog()->Get("food");
	Fuel =     Game->GetResourceCatalog()->Get("fuel");
	Plastics = Game->GetResourceCatalog()->Get("plastics");
	Hydrogen = Game->GetResourceCatalog()->Get("h2");
	Helium =   Game->GetResourceCatalog()->Get("he3");
	Silica =   Game->GetResourceCatalog()->Get("sio2");
	IronOxyde =Game->GetResourceCatalog()->Get("feo");
	Steel =    Game->GetResourceCatalog()->Get("steel");
	Tools =    Game->GetResourceCatalog()->Get("tools");
	Tech =     Game->GetResourceCatalog()->Get("tech");
	Carbon =     Game->GetResourceCatalog()->Get("carbon");
	Methane =     Game->GetResourceCatalog()->Get("ch4");
	FleetSupply =     Game->GetResourceCatalog()->Get("fleet-supply");

	// Ships
	ShipSolen = "ship-solen";
	ShipOmen = "ship-omen";
	ShipGhoul = "ship-ghoul";

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
	StationOutpost = "station-outpost";
}

void UFlareScenarioTools::GenerateEmptyScenario()
{
	FLOG("UFlareScenarioTools::GenerateEmptyScenario");
	SetupWorld();
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

	CreatePlayerShip(FirstLight, "ship-solen");
}

void UFlareScenarioTools::GenerateDebugScenario()
{
	FLOG("UFlareScenarioTools::GenerateFreighterScenario");
	SetupWorld();

	// Discover all sectors
	if (!PlayerData->QuestData.PlayTutorial)
	{
		for (int SectorIndex = 0; SectorIndex < World->GetSectors().Num(); SectorIndex++)
		{
			PlayerCompany->DiscoverSector(World->GetSectors()[SectorIndex]);
		}
	}

	// Add more stuff
	CreatePlayerShip(MinersHome, "ship-omen");
	CreatePlayerShip(FrozenRealm, "ship-omen");
	CreateStations(StationIceMine, PlayerCompany, ShoreOfIce, 1);
	CreateStations(StationIceMine, PlayerCompany, MinersHome, 1);
}

UFlareSimulatedSpacecraft* UFlareScenarioTools::CreateRecoveryPlayerShip()
{
	return CreatePlayerShip(FirstLight, "ship-solen");
}


/*----------------------------------------------------
	Common world
----------------------------------------------------*/

void UFlareScenarioTools::SetupWorld()
{
	// Setup common stuff
	SetupAsteroids();

	// Setup player sector knwoleage
	PlayerCompany->DiscoverSector(TheDepths);
	PlayerCompany->DiscoverSector(BlueHeart);
	PlayerCompany->DiscoverSector(TheSpire);
	PlayerCompany->DiscoverSector(Outpost);
	PlayerCompany->DiscoverSector(NightsHome);

	// Discover public sectors
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(HelixFoundries);
	SetupKnownSectors(Sunwatch);
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(UnitedFarmsChemicals);
	SetupKnownSectors(IonLane);
	SetupKnownSectors(GhostWorksShipyards);
	SetupKnownSectors(NemaHeavyWorks);
	SetupKnownSectors(Pirates);
	
	// Company setup
	PlayerCompany->GiveMoney(500000);
	MiningSyndicate->GiveMoney(100000000);
	HelixFoundries->GiveMoney(100000000);
	Sunwatch->GiveMoney(100000000);
	UnitedFarmsChemicals->GiveMoney(100000000);
	IonLane->GiveMoney(100000000);
	GhostWorksShipyards->GiveMoney(100000000);
	NemaHeavyWorks->GiveMoney(100000000);
	Pirates->GiveMoney(0);

	// Population setup
	BlueHeart->GetPeople()->GiveBirth(3000);
	FrozenRealm->GetPeople()->GiveBirth(1000);
	TheForge->GetPeople()->GiveBirth(500);
	
	// Nema main economy
	CreateStations(StationIceMine, MiningSyndicate, TheDepths, 3);
	CreateStations(StationFarm, UnitedFarmsChemicals, Lighthouse, 2);
	CreateStations(StationSolarPlant, Sunwatch, Lighthouse, 2, 2);
	CreateStations(StationIronMine, MiningSyndicate, MinersHome, 2);
	CreateStations(StationSteelworks, NemaHeavyWorks, MinersHome, 1);
	CreateStations(StationToolFactory, NemaHeavyWorks, MinersHome, 1);
	CreateStations(StationMethanePump, UnitedFarmsChemicals, TheSpire, 2);
	CreateStations(StationHydrogenPump, NemaHeavyWorks, TheSpire, 1, 2);
	CreateStations(StationCarbonRefinery, UnitedFarmsChemicals, TheSpire, 1);
	CreateStations(StationPlasticsRefinery, UnitedFarmsChemicals, TheSpire, 2, 2);

	// Create Blue Heart capital station
	{
		float StationRadius = 50000;
		FVector UpVector(0, 0, 1);
		FVector BaseLocation = FVector(-200000.0, 0, 0);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("BlueHeartCore");

		// BH Shipyard
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0);
		StationParams.Rotation = FRotator::ZeroRotator;
		CreateStations("station-bh-shipyard",   NemaHeavyWorks, BlueHeart, 1, 1, StationParams);

		// BH Arsenal
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(30, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 30));
		CreateStations("station-bh-arsenal",    AxisSupplies,   BlueHeart, 1, 1, StationParams);

		// BH Hub
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(-30, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, -30));
		CreateStations("station-bh-hub",        IonLane, BlueHeart, 1, 1, StationParams);

		// BH Habitation 1
		StationParams.Location = BaseLocation + FVector(StationRadius + 600, 0, 7168);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(180, 0, 0));
		CreateStations("station-bh-habitation", NemaHeavyWorks, BlueHeart, 1, 1, StationParams);

		// BH Habitation 2
		StationParams.Location = BaseLocation + FVector(StationRadius + 600, 0, -7168);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 0));
		CreateStations("station-bh-habitation", NemaHeavyWorks, BlueHeart, 1, 1, StationParams);
	}
	
	// Anka HFR factory
	CreateStations(StationSteelworks, HelixFoundries, TheForge, 2);
	CreateStations(StationToolFactory, HelixFoundries, TheForge, 2);
	CreateStations(StationHabitation, Sunwatch, TheForge, 1);

	// Create Night's Home capital station
	{
		float StationRadius = 22480;
		FVector UpVector(0, 0, 1);
		FVector BaseLocation = FVector(-3740, 0, -153600);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("NightsHomeCore");

		// NH Arsenal
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(-135, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(90, 0, -135));
		CreateStations("station-nh-arsenal", AxisSupplies, NightsHome, 1, 2, StationParams);

		// NH Shipyard
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(180, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 180));
		CreateStations("station-nh-shipyard", GhostWorksShipyards, NightsHome, 1, 1, StationParams);

		// NH Habitation
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(135, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(-90, 0, 135));
		CreateStations("station-nh-habitation", GhostWorksShipyards, NightsHome, 1, 1, StationParams);
	}
	
	// Hela secondary economy
	CreateStations(StationArsenal, AxisSupplies, FrozenRealm, 1, 2);
	CreateStations(StationFarm, GhostWorksShipyards, FrozenRealm, 1);
	CreateStations(StationSolarPlant, GhostWorksShipyards, FrozenRealm, 1, 2);
	CreateStations(StationIceMine, GhostWorksShipyards, ShoreOfIce, 2);
	CreateStations(StationHub, GhostWorksShipyards, FrozenRealm, 1);

	// Asta pirate base
	CreateStations(StationShipyard, Pirates, Boneyard, 1);
	CreateStations(StationSolarPlant, Pirates, Boneyard, 1, 1);
	CreateStations(StationArsenal, Pirates, Boneyard, 1);
	CreateStations(StationOutpost, Pirates, Boneyard, 1);

	// Create hubs
	CreateStations(StationHub, IonLane, Crossroads, 2);
	CreateStations(StationHub, IonLane, Lighthouse, 1);
	CreateStations(StationHub, IonLane, BlueHeart, 1);
	CreateStations(StationHub, IonLane, MinersHome, 1);
	CreateStations(StationHub, IonLane, TheForge, 1);
	CreateStations(StationHub, IonLane, TheSpire, 1);
	
	// Create outposts
	CreateStations(StationOutpost, AxisSupplies, TheDepths, 1);
	CreateStations(StationOutpost, AxisSupplies, MinersHome, 1);
	CreateStations(StationOutpost, AxisSupplies, Lighthouse, 1);
	CreateStations(StationOutpost, AxisSupplies, BlueHeart, 1);
	CreateStations(StationOutpost, AxisSupplies, BlueShores, 1);
	CreateStations(StationOutpost, AxisSupplies, TheSpire, 1);
	CreateStations(StationOutpost, AxisSupplies, TheForge, 1);
	CreateStations(StationOutpost, AxisSupplies, Crossroads, 1);
	CreateStations(StationOutpost, AxisSupplies, TheDig, 1);
	CreateStations(StationOutpost, AxisSupplies, FrozenRealm, 1);
	CreateStations(StationOutpost, AxisSupplies, Ruins, 1);
	CreateStations(StationOutpost, AxisSupplies, WinterJunction, 1);
	CreateStations(StationOutpost, AxisSupplies, Tranquility, 1);

	// Create cargos
	CreateShips(ShipSolen, GhostWorksShipyards, FrozenRealm, 3);
	CreateShips(ShipSolen, IonLane, Lighthouse, 3);
	CreateShips(ShipOmen, IonLane, MinersHome, 2);
	CreateShips(ShipOmen, IonLane, FrozenRealm, 1);
	CreateShips(ShipSolen, MiningSyndicate, MinersHome, 2);
	CreateShips(ShipSolen, NemaHeavyWorks, MinersHome, 2);
	CreateShips(ShipSolen, UnitedFarmsChemicals, TheSpire, 3);
	CreateShips(ShipOmen, UnitedFarmsChemicals, TheSpire, 1);
	CreateShips(ShipSolen, Sunwatch, Lighthouse, 4);
	CreateShips(ShipSolen, HelixFoundries, TheForge, 3);
	CreateShips(ShipOmen, HelixFoundries, TheForge, 1);
	CreateShips(ShipSolen, Pirates, Boneyard, 1);

	// Create military ships
	CreateShips(ShipGhoul, Pirates, Boneyard, 1);
}

void UFlareScenarioTools::SetupAsteroids()
{
	CreateAsteroids(FirstLight, 31, FVector(30, 7, 9));
	CreateAsteroids(MinersHome, 60, FVector(75, 15, 13));
	CreateAsteroids(TheDepths, 39, FVector(35, 5, 10));

	CreateAsteroids(Outpost, 47, FVector(49, 16, 8));
	CreateAsteroids(TheForge, 32, FVector(51, 12, 9));
	CreateAsteroids(TheDig, 42, FVector(47, 13, 20));

	CreateAsteroids(ShoreOfIce, 42, FVector(55, 9, 17));
	CreateAsteroids(Ruins, 45, FVector(38, 7, 9));

	CreateAsteroids(Boneyard, 38, FVector(37, 18, 7));

	CreateAsteroids(Serenity, 27, FVector(42, 9, 7));
}
void UFlareScenarioTools::SetupKnownSectors(UFlareCompany* Company)
{
	// Nema
	Company->DiscoverSector(TheDepths);
	Company->DiscoverSector(MinersHome);
	Company->DiscoverSector(BlueHeart);
	Company->DiscoverSector(Lighthouse);
	Company->DiscoverSector(BlueShores);
	Company->DiscoverSector(TheSpire);
	// Unknown : FirstLight, Anomaly, Pendulum

	// Anka
	Company->DiscoverSector(Crossroads);
	Company->DiscoverSector(TheDig);
	Company->DiscoverSector(TheForge);
	// Unknown : Colossus, Outpost

	// Hela
	Company->DiscoverSector(NightsHome);
	Company->DiscoverSector(FrozenRealm);
	Company->DiscoverSector(WinterJunction);
	// Unknown : Ruins, ShoreOfIce

	// Asta
	// Unknown : Decay, Boneyard, Daedalus

	// Adena
	Company->DiscoverSector(Tranquility);
	// Unknown : Serenity, Solitude
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareScenarioTools::CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class)
{
	UFlareSimulatedSpacecraft* InitialShip = Sector->CreateSpacecraft(Class, PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
	PlayerData->PlayerFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
	return InitialShip;
}

void UFlareScenarioTools::CreateAsteroids(UFlareSimulatedSector* Sector, int32 Count, FVector DistributionShape)
{
	FLOGV("UFlareScenarioTools::CreateAsteroids : Trying to spawn %d asteroids", Count);
	FCHECK(Sector);

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
							int32 AsteroidCatalogCount = Game->GetAsteroidCatalog() ? Game->GetAsteroidCatalog()->Asteroids.Num() : 0;
							Sector->CreateAsteroid(FMath::RandRange(0, AsteroidCatalogCount - 1), FName(*AsteroidName), AsteroidLocation);
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
		Sector->CreateSpacecraft(ShipClass, Company, FVector::ZeroVector);
	}
}

void UFlareScenarioTools::CreateStations(FName StationClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count, int32 Level, FFlareStationSpawnParameters SpawnParameters)
{
	for (uint32 Index = 0; Index < Count; Index++)
	{
		UFlareSimulatedSpacecraft* Station = Sector->CreateStation(StationClass, Company, SpawnParameters);

		if (!Station)
		{
			continue;
		}

		Station->GetData().Level = Level;

		if (Station->GetFactories().Num() > 0)
		{
			UFlareFactory* ActiveFactory = Station->GetFactories()[0];

			// Give input resources
			for (int32 ResourceIndex = 0; ResourceIndex < ActiveFactory->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* Resource = &ActiveFactory->GetDescription()->CycleCost.InputResources[ResourceIndex];
				float StartRatio = FMath::FRandRange(0.25,0.75);
				Station->GetCargoBay()->GiveResources(&Resource->Resource->Data, Station->GetCargoBay()->GetSlotCapacity() * StartRatio, Company);
			}

			// Give output resources
			for (int32 ResourceIndex = 0; ResourceIndex < ActiveFactory->GetDescription()->CycleCost.OutputResources.Num(); ResourceIndex++)
			{
				const FFlareFactoryResource* Resource = &ActiveFactory->GetDescription()->CycleCost.OutputResources[ResourceIndex];
				float StartRatio = FMath::FRandRange(0.25,0.75);
				Station->GetCargoBay()->GiveResources(&Resource->Resource->Data, Station->GetCargoBay()->GetSlotCapacity() * StartRatio, Company);
			}
		}
		
		// Give customer resources
		if (Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
				float StartRatio = FMath::FRandRange(0.25,0.75);
				Station->GetCargoBay()->GiveResources(Resource, Station->GetCargoBay()->GetSlotCapacity() * StartRatio, Company);
			}
		}

		// Give customer resources
		if (Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
		{
			for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
				float StartRatio = FMath::FRandRange(0.25,0.75);
				Station->GetCargoBay()->GiveResources(Resource, Station->GetCargoBay()->GetSlotCapacity() * StartRatio, Company);
			}
		}


	}
}

#undef LOCTEXT_NAMESPACE
