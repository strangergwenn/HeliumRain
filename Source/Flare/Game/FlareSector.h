#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "FlareSimulatedSector.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;
class AFlareAsteroid;

UCLASS()
class FLARE_API UFlareSector : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(const FFlareSectorSave& Data, UFlareSimulatedSector* Sector);

	/** Save the sector to a save file */
	virtual FFlareSectorSave* Save(TArray<FFlareSpacecraftSave>& SpacecraftData);

	/** Destroy the sector */
	virtual void Destroy();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Create a station in the level  for a specific company */
	// AFlareSpacecraft* CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Create a ship in the level  for a specific company */
	// AFlareSpacecraft* CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company. No null parameter accepted */
	// AFlareSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Add an asteroid to the world at a specific location */
	// void CreateAsteroidAt(int32 ID, FVector Location);

	// virtual void EmptySector();

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareSpacecraft* LoadSpacecraft(const FFlareSpacecraftSave& ShipData);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	/** Destroy a ship or a station*/
	virtual void DestroySpacecraft(AFlareSpacecraft* Spacecraft, bool Destroying = false);

	virtual void SetPause(bool Pause);

	AActor* GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize = true, AActor* ActorToIgnore = NULL);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorStations;
	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorShips;
	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorSpacecrafts;
	UPROPERTY()
	TArray<AFlareAsteroid*>      SectorAsteroids;
	UPROPERTY()
	TArray<AFlareBomb*>      SectorBombs;
	UPROPERTY()
	TArray<AFlareShell*>      SectorShells;
	// TODO shell register


	FFlareSectorSave        SectorData;
	AFlareGame*                   Game;
	UFlareSimulatedSector*        SimulatedSector;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FFlareSectorSave* GetData()
	{
		return &SectorData;
	}

	TArray<AFlareSpacecraft*> GetCompanyShips(UFlareCompany* Company);

	TArray<AFlareSpacecraft*> GetCompanySpacecrafts(UFlareCompany* Company);

	AFlareSpacecraft* FindSpacecraft(FName Immatriculation);

	inline FName GetIdentifier() const
	{
		return SectorData.Identifier;
	}

	inline UFlareSimulatedSector* GetSimulatedSector()
	{
		return SimulatedSector;
	}

	inline TArray<AFlareSpacecraft*>& GetSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	inline TArray<AFlareSpacecraft*>& GetStations()
	{
		return SectorStations;
	}

	inline TArray<AFlareAsteroid*>& GetAsteroids()
	{
		return SectorAsteroids;
	}


};
