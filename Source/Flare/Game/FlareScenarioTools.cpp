
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

	// Initial setup : outpost
	UFlareSimulatedSector* Outpost = World->FindSector("outpost");
	Outpost->CreateShip("ship-dragon", PlayerCompany, FVector::ZeroVector);
	Outpost->CreateStation("station-hub", PlayerCompany, FVector::ZeroVector);
	Outpost->CreateStation("station-tokamak", PlayerCompany, FVector::ZeroVector);
	Outpost->CreateStation("station-solar-plant", PlayerCompany, FVector::ZeroVector);
	Outpost->CreateStation("station-solar-plant", World->FindCompanyByShortName("HFR"), FVector::ZeroVector);
	Outpost->CreateStation("station-solar-plant", World->FindCompanyByShortName("HFR"), FVector::ZeroVector);
	Outpost->CreateStation("station-tokamak", World->FindCompanyByShortName("SUN"), FVector::ZeroVector);
	Outpost->CreateStation("station-hub", World->FindCompanyByShortName("GWS"), FVector::ZeroVector);
}

void UFlareScenarioTools::GenerateDebugScenario()
{
	SetupWorld();


	UFlareSimulatedSector* Outpost = World->FindSector("outpost");
	FFlareResourceDescription* Water = Game->GetResourceCatalog()->Get("h2o");
	FFlareResourceDescription* Food = Game->GetResourceCatalog()->Get("food");
	FFlareResourceDescription* Fuel = Game->GetResourceCatalog()->Get("fuel");
	FFlareResourceDescription* Plastics = Game->GetResourceCatalog()->Get("plastics");
	FFlareResourceDescription* Hydrogen = Game->GetResourceCatalog()->Get("h2");
	FFlareResourceDescription* Helium = Game->GetResourceCatalog()->Get("he3");

	// Create player ship
	FLOG("UFlareScenarioTools::GenerateDebugScenario create initial ship");
	UFlareSimulatedSpacecraft* InitialShip = Outpost->CreateShip("ship-solen", PlayerCompany, FVector::ZeroVector);
	PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
	PlayerData->SelectedFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();




	// Initial setup : "Outpost"
	for(int Index = 0; Index < 5; Index ++)
	{
		Outpost->CreateShip("ship-omen", PlayerCompany, FVector::ZeroVector)->AssignToSector(true);
	}

	for(int Index = 0; Index < 3; Index ++)
	{
		Outpost->CreateStation("station-solar-plant", PlayerCompany, FVector::ZeroVector)->GiveResources(Water, 100);
	}

	Outpost->CreateStation("station-habitation", PlayerCompany, FVector::ZeroVector)->GiveResources(Food, 30);
	Outpost->CreateStation("station-farm", PlayerCompany, FVector::ZeroVector);
	UFlareSimulatedSpacecraft* Refinery = Outpost->CreateStation("station-refinery", PlayerCompany, FVector::ZeroVector);
	Refinery->GetFactories()[0]->SetOutputLimit(Plastics, 1);
	Refinery->GetFactories()[0]->SetOutputLimit(Hydrogen, 1);
	Refinery->GiveResources(Fuel, 50);

	UFlareSimulatedSpacecraft* PompingStation = Outpost->CreateStation("station-pomping", PlayerCompany, FVector::ZeroVector);
	PompingStation->GetFactories()[0]->SetOutputLimit(Hydrogen, 1);
	PompingStation->GetFactories()[0]->SetOutputLimit(Helium, 1);
	PompingStation->GiveResources(Fuel, 50);

	Outpost->GetPeople()->GiveBirth(1000);
	Outpost->GetPeople()->SetHappiness(1);


	World->FindSector("frozen-realm")->CreateShip("ship-omen", PlayerCompany, FVector::ZeroVector);

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
	// Create asteroids at "Outpost"
	for (int32 Index = 0; Index < 20; Index++)
	{
		FString AsteroidName = FString("asteroid") + FString::FromInt(Index);
		World->FindSector("outpost")->CreateAsteroid(FMath::RandRange(0, 5), FName(*AsteroidName) , 200000 * FMath::VRand());
	}

	// Create asteroids at "Frozen Realm"
	for (int32 Index = 0; Index < 50; Index++)
	{
		FString AsteroidName = FString("asteroid") + FString::FromInt(Index);
		World->FindSector("frozen-realm")->CreateAsteroid(FMath::RandRange(0, 5), FName(*AsteroidName), 200000 * FMath::VRand());
	}
}



#undef LOCTEXT_NAMESPACE
