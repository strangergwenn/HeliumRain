
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

	Outpost = World->FindSector("outpost");
	MinerHome = World->FindSector("miners-home");
	FrozenRealm = World->FindSector("frozen-realm");
	BlueHeart = World->FindSector("blue-heart");
	TheSpire = World->FindSector("the-spire");
	TheDepths = World->FindSector("the-depths");

	MiningSyndicate = World->FindCompanyByShortName("MSY");
	HelixFoundries = World->FindCompanyByShortName("HFR");
	Sunwatch = World->FindCompanyByShortName("SUN");
	IonLane = World->FindCompanyByShortName("ION");
	UnitedFarmsChemicals = World->FindCompanyByShortName("UFC");
	GhostWorksShipyards = World->FindCompanyByShortName("GWS");


	Water = Game->GetResourceCatalog()->Get("h2o");
	Food = Game->GetResourceCatalog()->Get("food");
	Fuel = Game->GetResourceCatalog()->Get("fuel");
	Plastics = Game->GetResourceCatalog()->Get("plastics");
	Hydrogen = Game->GetResourceCatalog()->Get("h2");
	Helium = Game->GetResourceCatalog()->Get("he3");
	Silica = Game->GetResourceCatalog()->Get("sio2");
	Steel= Game->GetResourceCatalog()->Get("steel");
	Tools= Game->GetResourceCatalog()->Get("tools");
	Tech= Game->GetResourceCatalog()->Get("tech");
}

void UFlareScenarioTools::GenerateEmptyScenario()
{

}

void UFlareScenarioTools::GenerateFighterScenario()
{
	SetupWorld();

	// Create player ship
	FLOG("UFlareScenarioTools::GenerateFighterScenario create initial ship");
	UFlareSimulatedSpacecraft* InitialShip = World->FindSector("first-light")->CreateShip("ship-ghoul", PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
	PlayerData->SelectedFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
	PlayerCompany->DiscoverSector(Outpost);
	PlayerCompany->DiscoverSector(MinerHome);
	PlayerCompany->DiscoverSector(BlueHeart);
	PlayerCompany->DiscoverSector(TheSpire);
<<<<<<< HEAD
=======
	PlayerCompany->DiscoverSector(TheDepths);
>>>>>>> #150 It's now possible to order ship to others companies
	FillWorld();
}

void UFlareScenarioTools::GenerateFreighterScenario()
{
	SetupWorld();

	// Create player ship
	FLOG("UFlareScenarioTools::GenerateFreighterScenario create initial ship");
	UFlareSimulatedSpacecraft* InitialShip = World->FindSector("first-light")->CreateShip("ship-omen", PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
	PlayerData->SelectedFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
	PlayerCompany->DiscoverSector(Outpost);
	PlayerCompany->DiscoverSector(MinerHome);
	PlayerCompany->DiscoverSector(BlueHeart);
	PlayerCompany->DiscoverSector(TheSpire);
<<<<<<< HEAD
=======
	PlayerCompany->DiscoverSector(TheDepths);
>>>>>>> #150 It's now possible to order ship to others companies
	PlayerCompany->GiveMoney(10000);

	FillWorld();
}

void UFlareScenarioTools::FillWorld()
{

	MiningSyndicate->GiveMoney(100000);
	HelixFoundries->GiveMoney(100000);
	Sunwatch->GiveMoney(100000);
	UnitedFarmsChemicals->GiveMoney(100000);
	IonLane->GiveMoney(100000);

	// Initial setup: miner home

	for (int Index = 0; Index < 5; Index++)
	{
		MinerHome->CreateStation("station-mine", MiningSyndicate, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	for(int Index = 0; Index < 5; Index ++)
	{
		MinerHome->CreateStation("station-solar-plant", Sunwatch, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);
	}

	for(int Index = 0; Index < 5; Index ++)
	{
		MinerHome->CreateShip("ship-omen", Sunwatch, FVector::ZeroVector);
	}

	// Initial setup: Outpost
	Outpost->CreateStation("station-steelworks", HelixFoundries, FVector::ZeroVector);
	Outpost->CreateStation("station-steelworks", HelixFoundries, FVector::ZeroVector);
	Outpost->CreateStation("station-steelworks", HelixFoundries, FVector::ZeroVector);
	Outpost->CreateStation("station-tool-factory", HelixFoundries, FVector::ZeroVector);
	Outpost->CreateStation("station-tool-factory", HelixFoundries, FVector::ZeroVector);

	// Todo remove
	Outpost->CreateStation("station-solar-plant", HelixFoundries, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);

	for(int Index = 0; Index < 6; Index ++)
	{
		Outpost->CreateStation("station-solar-plant", Sunwatch, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);
	}

	for(int Index = 0; Index < 5; Index ++)
	{
		Outpost->CreateShip("ship-omen", Sunwatch, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 3; Index ++)
	{
		Outpost->CreateShip("ship-omen", HelixFoundries, FVector::ZeroVector);
	}

	// Initial setup Blue Heart
	BlueHeart->GetPeople()->GiveBirth(3000);

	for(int Index = 0; Index < 5; Index ++)
	{
		BlueHeart->CreateStation("station-habitation", IonLane, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 5; Index ++)
	{
		BlueHeart->CreateStation("station-hub", IonLane, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 2; Index ++)
	{
		BlueHeart->CreateStation("station-hub", Sunwatch, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 20; Index ++)
	{
		BlueHeart->CreateStation("station-solar-plant", Sunwatch, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);
	}

	for(int Index = 0; Index < 2; Index ++)
	{
		BlueHeart->CreateShip("ship-omen", Sunwatch, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 5; Index ++)
	{
		BlueHeart->CreateStation("station-farm", UnitedFarmsChemicals, FVector::ZeroVector);
	}

	for(int Index = 0; Index < 10; Index ++)
	{
		BlueHeart->CreateShip("ship-omen", IonLane, FVector::ZeroVector);
	}

	// Initial setup for The Spire

	for(int Index = 0; Index < 5; Index ++)
	{
		TheSpire->CreateStation("station-pumping", UnitedFarmsChemicals, FVector::ZeroVector);
		TheSpire->CreateStation("station-refinery", UnitedFarmsChemicals, FVector::ZeroVector);
		TheSpire->CreateShip("ship-omen", UnitedFarmsChemicals, FVector::ZeroVector);
	}

	// Initial setup for The Depths
	TheDepths->CreateStation("station-shipyard", GhostWorksShipyards, FVector::ZeroVector);


}

void UFlareScenarioTools::GenerateDebugScenario()
{
	SetupWorld();

	FLOG("UFlareScenarioTools::GenerateDebugScenario");

	/*----------------------------------------------------
		Outpost
	----------------------------------------------------*/
	
	// Add solar plants
	for (int Index = 0; Index < 3; Index ++)
	{
		Outpost->CreateStation("station-solar-plant", UnitedFarmsChemicals, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);
	}

	// Various stations
	Outpost->CreateStation("station-hub", UnitedFarmsChemicals, FVector::ZeroVector);
	Outpost->CreateStation("station-habitation", UnitedFarmsChemicals, FVector::ZeroVector)->GetCargoBay()->GiveResources(Food, 30);
	Outpost->CreateStation("station-farm", UnitedFarmsChemicals, FVector::ZeroVector);

	// Refinery
	UFlareSimulatedSpacecraft* Refinery = Outpost->CreateStation("station-refinery", PlayerCompany, FVector::ZeroVector);
	Refinery->GetFactories()[0]->SetOutputLimit(Plastics, 1);
	Refinery->GetCargoBay()->GiveResources(Fuel, 50);

	// Pumping station
	UFlareSimulatedSpacecraft* PumpingStation = Outpost->CreateStation("station-pumping", PlayerCompany, FVector::ZeroVector);
	PumpingStation->GetFactories()[0]->SetOutputLimit(Hydrogen, 1);
	PumpingStation->GetFactories()[0]->SetOutputLimit(Helium, 1);
	PumpingStation->GetCargoBay()->GiveResources(Fuel, 50);

	// Final settings
	Outpost->GetPeople()->GiveBirth(1000);
	Outpost->GetPeople()->SetHappiness(1);

	// Add Omens
	for (int Index = 0; Index < 5; Index++)
	{
		Outpost->CreateShip("ship-omen", UnitedFarmsChemicals, FVector::ZeroVector)->AssignToSector(true);
	}

	//PlayerCompany->SetHostilityTo(UnitedFarmsChemicals, true);

	// Player ship
	UFlareSimulatedSpacecraft* InitialShip = Outpost->CreateShip("ship-ghoul", PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
	PlayerData->SelectedFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();


	/*----------------------------------------------------
		Miner's Home
	----------------------------------------------------*/

	// Add Omens
	for(int Index = 0; Index < 5; Index ++)
	{
		MinerHome->CreateShip("ship-omen", PlayerCompany, FVector::ZeroVector)->AssignToSector(true);
	}

	// Add solar plants
	for(int Index = 0; Index < 2; Index ++)
	{
		MinerHome->CreateStation("station-solar-plant", PlayerCompany, FVector::ZeroVector)->GetCargoBay()->GiveResources(Water, 100);
	}

	MinerHome->CreateStation("station-hub", PlayerCompany, FVector::ZeroVector);

	UFlareSimulatedSpacecraft* Tokamak = MinerHome->CreateStation("station-tokamak", PlayerCompany, FVector::ZeroVector);
	Tokamak->GetCargoBay()->GiveResources(Water, 200);

	UFlareSimulatedSpacecraft* Steelworks = MinerHome->CreateStation("station-steelworks", PlayerCompany, FVector::ZeroVector);
	Steelworks->GetFactories()[0]->SetOutputLimit(Steel, 1);

	UFlareSimulatedSpacecraft* Mine = MinerHome->CreateStation("station-ice-mine", PlayerCompany, FVector::ZeroVector, FRotator::ZeroRotator);
	Mine->GetFactories()[0]->SetOutputLimit(Silica, 1);

	UFlareSimulatedSpacecraft* ToolFactory = MinerHome->CreateStation("station-tool-factory", PlayerCompany, FVector::ZeroVector);
	ToolFactory->GetFactories()[0]->SetOutputLimit(Tools, 1);

	UFlareSimulatedSpacecraft* Foundry = MinerHome->CreateStation("station-foundry", PlayerCompany, FVector::ZeroVector);
	Foundry->GetFactories()[0]->SetOutputLimit(Tech, 1);


	/*----------------------------------------------------
		Frozen Realm
	----------------------------------------------------*/

	World->FindSector("frozen-realm")->CreateShip("ship-omen", PlayerCompany, FVector::ZeroVector);


	/*----------------------------------------------------
		Trade routes
	----------------------------------------------------*/

	UFlareTradeRoute* OutpostToMinerHome = PlayerCompany->CreateTradeRoute(FText::FromString(TEXT("Trade route 1")));
	OutpostToMinerHome->AddSector(Outpost);
	OutpostToMinerHome->AddSector(MinerHome);

	OutpostToMinerHome->SetSectorLoadOrder(0, Helium, 0);
	OutpostToMinerHome->SetSectorLoadOrder(0, Hydrogen, 0);
	OutpostToMinerHome->SetSectorLoadOrder(0, Plastics, 0);
	OutpostToMinerHome->SetSectorUnloadOrder(0, Water, 0);
	OutpostToMinerHome->SetSectorUnloadOrder(0, Tools, 0);
	OutpostToMinerHome->SetSectorUnloadOrder(0, Tech, 0);

	OutpostToMinerHome->SetSectorLoadOrder(1, Tools, 0);
	OutpostToMinerHome->SetSectorLoadOrder(1, Tech, 0);
	OutpostToMinerHome->SetSectorLoadOrder(1, Water, 250);
	OutpostToMinerHome->SetSectorUnloadOrder(1, Helium, 0);
	OutpostToMinerHome->SetSectorUnloadOrder(1, Hydrogen, 0);
	OutpostToMinerHome->SetSectorUnloadOrder(1, Plastics, 0);
	
	UFlareFleet* TradeFleet1 = PlayerCompany->CreateFleet(FText::FromString(TEXT("Trade fleet 1")), MinerHome);
	TradeFleet1->AddShip(MinerHome->CreateShip("ship-atlas", PlayerCompany, FVector::ZeroVector));

	OutpostToMinerHome->AddFleet(TradeFleet1);


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
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void UFlareScenarioTools::SetupWorld()
{
	// Spawn asteroids
	SetupAsteroids(Outpost, 20, FVector(2, 20, 1));
	SetupAsteroids(MinerHome, 400, FVector(3, 50, 1));
	SetupAsteroids(World->FindSector("frozen-realm"), 20, FVector(5, 50, 2));
}

void UFlareScenarioTools::SetupAsteroids(UFlareSimulatedSector* Sector, int32 Count, FVector DistributionShape)
{
	float MaxAsteroidDistance = 10000;

	for (int32 Index = 0; Index < Count; Index++)
	{
		FString AsteroidName = FString("asteroid") + FString::FromInt(Index);
		FVector AsteroidLocation = MaxAsteroidDistance * GetRandomAsteroidLocation(DistributionShape.X, DistributionShape.Y, DistributionShape.Z);

		Sector->CreateAsteroid(FMath::RandRange(0, 5), FName(*AsteroidName), AsteroidLocation);
	}
}

FVector UFlareScenarioTools::GetRandomAsteroidLocation(float X, float Y, float Z)
{
	FVector Result;
	
	Result.X = FMath::RandRange(-X, X);
	Result.Y = FMath::RandRange(-Y, Y);
	Result.Z = FMath::RandRange(-Z, Z);

	return Result;
}



#undef LOCTEXT_NAMESPACE
