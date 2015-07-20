
#pragma once

#include "Object.h"
#include "FlareSimulatedSector.generated.h"

class UFlareSimulatedSpacecraft;
class UFlareGame;
struct FFlarePlayerSave;

/** Sector knowledge status */
UENUM()
namespace EFlareSectorKnowledge
{
    enum Type
    {
        Unknown, /** The exisitaece of this sector is unknown */
        Known, /** The sector is visible on the map but its content is unknown */
        Visited /** The sector is visited, all static structure are visible */
    };
}


/** Sector save data */
USTRUCT()
struct FFlareSectorSave
{
    GENERATED_USTRUCT_BODY()

    /** Name */
    UPROPERTY(EditAnywhere, Category = Save)
    FString Name;

    /** Save identifier */
    UPROPERTY(EditAnywhere, Category = Save)
    FName Identifier;

    UPROPERTY(VisibleAnywhere, Category = Save)
    TArray<FFlareSpacecraftSave> ShipData;

    UPROPERTY(VisibleAnywhere, Category = Save)
    TArray<FFlareSpacecraftSave> StationData;

    UPROPERTY(VisibleAnywhere, Category = Save)
    TArray<FFlareBombSave> BombData;

    UPROPERTY(VisibleAnywhere, Category = Save)
    TArray<FFlareAsteroidSave> AsteroidData;

};

UCLASS()
class FLARE_API UFlareSimulatedSector : public UObject
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

    /** Spawn a simulated spacecraft from save data */
    virtual UFlareSimulatedSpacecraft* LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData);

    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

    /** Create a station in the level  for a specific company */
    UFlareSimulatedSpacecraft* CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

    /** Create a ship in the level  for a specific company */
    UFlareSimulatedSpacecraft* CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition);

    /** Create a ship or station in the level  for a specific company */
    UFlareSimulatedSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);



    /** Init world with empty scenario */
	//virtual void InitEmptyScenario(FFlarePlayerSave* PlayerData);

    /** Init world with peaceful scenario */
	//virtual void InitPeacefulScenario(FFlarePlayerSave* PlayerData);

    /** Init world with threatened scenario */
	//virtual void InitThreatenedScenario(FFlarePlayerSave* PlayerData, UFlareCompany* PlayerCompany);

    /** Init world with aggresve scenario */
	//virtual void InitAggresiveScenario(FFlarePlayerSave* PlayerData, UFlareCompany* PlayerCompany);



protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
    FFlareSectorSave                       SectorData;
    TArray<UFlareSimulatedSpacecraft*>      SectorStations;
    TArray<UFlareSimulatedSpacecraft*>      SectorShips;

	AFlareGame*                   Game;

public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

    AFlareGame* GetGame() const
    {
		return Game;
    }

    inline FName GetIdentifier() const
    {
        return SectorData.Identifier;
    }

    inline FString GetSectorName() const
    {
        return SectorData.Name;
    }

    inline TArray<UFlareSimulatedSpacecraft*>& GetSectorStations()
    {
        return SectorStations;
    }

    inline TArray<UFlareSimulatedSpacecraft*>& GetSectorShips()
    {
        return SectorShips;
    }

};
