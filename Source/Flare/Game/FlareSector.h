#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "FlareSimulatedSector.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;

UCLASS()
class FLARE_API UFlareSector : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(const FFlareSectorSave& Data);

	/** Save the sector to a save file */
	virtual FFlareSectorSave* Save();

	/** Destroy the sector */
	virtual void Destroy();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Create a station in the level  for a specific company */
	AFlareSpacecraft* CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Create a ship in the level  for a specific company */
	AFlareSpacecraft* CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company. No null parameter accepted */
	AFlareSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Add an asteroid to the world at a specific location */
	void CreateAsteroidAt(int32 ID, FVector Location);

	virtual void EmptySector();

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareSpacecraft* LoadShip(const FFlareSpacecraftSave& ShipData);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	TArray<AFlareSpacecraft*>      SectorStations;
	TArray<AFlareSpacecraft*>      SectorShips;
	TArray<AFlareAsteroid*>      SectorAsteroids;
	TArray<AFlareBomb*>      SectorBombs;
	TArray<AFlareShell*>      SectorShells;


	FFlareSectorSave        SectorData;
	AFlareGame*                   Game;

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

	inline FName GetIdentifier() const
	{
		return SectorData.Identifier;
	}

};
