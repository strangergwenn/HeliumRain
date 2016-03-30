#pragma once

#include "FlareScenarioTools.generated.h"

class UFlareCompany;
struct FFlarePlayerSave;

UCLASS()
class HELIUMRAIN_API UFlareScenarioTools : public UObject
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Init(UFlareCompany* Company, FFlarePlayerSave* Player);


	void GenerateEmptyScenario();

	void GenerateFighterScenario();

	void GenerateFreighterScenario();

	void GenerateDebugScenario();
	
	/** Setup the common world */
	void FillWorld();


protected:

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Create the player ship */
	void CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class);

	/** Create asteroid, artefact and common things */
	void SetupWorld();

	/** Spawn a series of asteroids in this sector */
	void SetupAsteroids(UFlareSimulatedSector* Sector, int32 Count = 50, FVector DistributionShape = FVector(2, 50, 1));
	

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Main data
	UFlareCompany*                             PlayerCompany;
	FFlarePlayerSave*                          PlayerData;
	AFlareGame*                                Game;
	UFlareWorld*                               World;

	// Notable sectors (Nema)
	UFlareSimulatedSector*                     BlueHeart;
	UFlareSimulatedSector*                     MinerHome;
	UFlareSimulatedSector*                     Lighthouse;
	UFlareSimulatedSector*                     TheSpire;
	UFlareSimulatedSector*                     TheDepths;
	UFlareSimulatedSector*                     FirstLight;

	// Notable sectors (Anka)
	UFlareSimulatedSector*                     Outpost;
	UFlareSimulatedSector*                     Crossroads;
	UFlareSimulatedSector*                     TheDig;

	// Notable sectors (Hela)
	UFlareSimulatedSector*                     FrozenRealm;

	// Notable sectors (Asta)
	UFlareSimulatedSector*                     Decay;


	// Companies
	UFlareCompany*                             MiningSyndicate;
	UFlareCompany*                             HelixFoundries;
	UFlareCompany*                             Sunwatch;
	UFlareCompany*                             IonLane;
	UFlareCompany*                             UnitedFarmsChemicals;
	UFlareCompany*                             GhostWorksShipyards;

	// Resources
	FFlareResourceDescription*                 Water;
	FFlareResourceDescription*                 Food;
	FFlareResourceDescription*                 Fuel;
	FFlareResourceDescription*                 Plastics;
	FFlareResourceDescription*                 Hydrogen;
	FFlareResourceDescription*                 Helium;
	FFlareResourceDescription*                 Silica;
	FFlareResourceDescription*                 Steel;
	FFlareResourceDescription*                 Tools;
	FFlareResourceDescription*                 Tech;

};
