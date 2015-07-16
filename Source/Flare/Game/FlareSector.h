#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;

UCLASS()
class FLARE_API UFlareActiveSector : public UObject
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


protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/
    TArray<AFlareSpacecraft*>      SectorStations;
    TArray<AFlareSpacecraft*>      SectorShips;
    UFlareSimulatedSector*                   Sector;

public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

    AFlareGame* GetGame() const
    {
        return Cast<AFlareGame>(GetOuter()->GetWorld()->GetAuthGameMode());
    }

};
