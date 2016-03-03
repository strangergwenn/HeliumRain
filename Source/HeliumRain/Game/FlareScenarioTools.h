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
		Helpers
	----------------------------------------------------*/

	/** Create asteroid, artefact and common things */
	void SetupWorld();

	/** Setup the common world */
	void FillWorld();

	/** Spawn a series of asteroids in this sector */
	void SetupAsteroids(UFlareSimulatedSector* Sector, int32 Count = 50, FVector DistributionShape = FVector(2, 50, 1));

	/** Get a random vector in a spheroid shape */
	FVector GetRandomAsteroidLocation(float X, float Y, float Z);


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareCompany*             PlayerCompany;

	FFlarePlayerSave*          PlayerData;

	AFlareGame*                                Game;
	UFlareWorld*                               World;

	UFlareSimulatedSector* Outpost;
	UFlareSimulatedSector* MinerHome;
	UFlareSimulatedSector* FrozenRealm;
	UFlareSimulatedSector* BlueHeart;
	UFlareSimulatedSector* TheSpire;

	UFlareCompany* MiningSyndicate;
	UFlareCompany* HelixFoundries;
	UFlareCompany* Sunwatch;
	UFlareCompany* IonLane;
	UFlareCompany* UnitedFarmsChemicals;

	FFlareResourceDescription* Water;
	FFlareResourceDescription* Food;
	FFlareResourceDescription* Fuel;
	FFlareResourceDescription* Plastics;
	FFlareResourceDescription* Hydrogen;
	FFlareResourceDescription* Helium;
	FFlareResourceDescription* Silica;
	FFlareResourceDescription* Steel;
	FFlareResourceDescription* Tools;
	FFlareResourceDescription* Tech;


public:
	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	/*AFlarePlayerController* GetPC() const;

	AFlareGame* GetGame() const;

	UFlareWorld* GetGameWorld() const;*/
};
