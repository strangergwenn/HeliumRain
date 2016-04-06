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
	
	
protected:

	/*----------------------------------------------------
		Common world
	----------------------------------------------------*/

	/** Setup the common world */
	void SetupWorld();

	/** Setup asteroids */
	void SetupAsteroids();

	/** Setup artifacts */
	void SetupArtifacts();

	/** Discover all public sectors*/
	void SetupKnownSectors(UFlareCompany* Company);


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Create the player ship */
	void CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class);

	/** Spawn a series of asteroids in this sector */
	void CreateAsteroids(UFlareSimulatedSector* Sector, int32 Count = 50, FVector DistributionShape = FVector(2, 50, 1));
		
	/** Create a ship */
	void CreateShips(FName ShipClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count);

	/** Create a station and fill its input */
	void CreateStations(FName StationClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count);


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Main data
	UFlareCompany*                             PlayerCompany;
	FFlarePlayerSave*                          PlayerData;
	AFlareGame*                                Game;
	UFlareWorld*                               World;

	// Notable sectors (Nema)
	UFlareSimulatedSector*                     TheDepths;
	UFlareSimulatedSector*                     FirstLight;
	UFlareSimulatedSector*                     MinersHome;
	UFlareSimulatedSector*                     Anomaly;
	UFlareSimulatedSector*                     BlueHeart;
	UFlareSimulatedSector*                     Lighthouse;
	UFlareSimulatedSector*                     TheSpire;

	// Notable sectors (Anka)
	UFlareSimulatedSector*                     Outpost;
	UFlareSimulatedSector*                     Colossus;
	UFlareSimulatedSector*                     Crossroads;
	UFlareSimulatedSector*                     TheDig;

	// Notable sectors (Hela)
	UFlareSimulatedSector*                     FrozenRealm;
	UFlareSimulatedSector*                     ShoreOfIce;

	// Notable sectors (Asta)
	UFlareSimulatedSector*                     Decay;

	// Notable sectors (Adena)
	UFlareSimulatedSector*                     Solitude;
	
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

	// Ships
	FName                                      ShipSolen;
	FName                                      ShipOmen;
	FName                                      ShipAtlas;
	FName                                      ShipGhoul;
	FName                                      ShipOrca;
	FName                                      ShipDragon;
	FName                                      ShipInvader;
	FName                                      ShipLeviathan;

	// Stations
	FName                                      StationFarm;
	FName                                      StationSolarPlant;
	FName                                      StationHabitation;
	FName                                      StationIceMine;
	FName                                      StationIronMine;
	FName                                      StationSilicaMine;
	FName                                      StationSteelworks;
	FName                                      StationToolFactory;
	FName                                      StationHydrogenPump;
	FName                                      StationHeliumPump;
	FName                                      StationMethanePump;
	FName                                      StationCarbonRefinery;
	FName                                      StationPlasticsRefinery;
	FName                                      StationArsenal;
	FName                                      StationShipyard;
	FName                                      StationHub;

};
