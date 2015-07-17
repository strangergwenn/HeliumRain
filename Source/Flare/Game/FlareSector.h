#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;

UCLASS()
class FLARE_API UFlareSector : public UObject
{
    GENERATED_UCLASS_BODY()

public:

    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

    /** Create a station in the level  for a specific company */
    AFlareSpacecraft* CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

    /** Create a ship in the level  for a specific company */
    AFlareSpacecraft* CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition);

    /** Create a ship or station in the level  for a specific company */
    AFlareSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

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

    UFlareSimulatedSector*                   Sector;
	AFlareGame*                   Game;

public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

    AFlareGame* GetGame() const
    {
        return Cast<AFlareGame>(GetOuter()->GetWorld()->GetAuthGameMode());
    }

};
