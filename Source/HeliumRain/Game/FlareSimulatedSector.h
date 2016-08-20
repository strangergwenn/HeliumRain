
#pragma once

#include "Object.h"
#include "FlareAsteroid.h"
#include "../Data/FlareAsteroidCatalog.h"
#include "../Spacecrafts/FlareBomb.h"
#include "../Economy/FlarePeople.h"
#include "../Player/FlareSoundManager.h"
#include "FlareSimulatedSector.generated.h"

class UFlareSimulatedSpacecraft;
struct FFlareSpacecraftDescription;
class UFlareFleet;
class AFlareGame;
struct FFlarePlayerSave;
struct FFlareResourceDescription;

/** Factory action type values */
UENUM()
namespace EFlareTransportLimitType
{
	enum Type
	{
		Production,
		CargoBay
	};
}

/** Sector friendlyness status */
UENUM()
namespace EFlareSectorFriendlyness
{
	enum Type
	{
		NotVisited, /** The company have not visited the sector yet */
		Neutral, /** The sector have neither friendly spacecraft nor hostile spacecraft */
		Friendly, /** The sector have only friendly spacecraft */
		Contested, /** The sector have both friendly and hostile spacecraft */
		Hostile /** The sector have only hostile spacecraft */
	};
}

/** Sector battle status */
UENUM()
namespace EFlareSectorBattleState
{
	enum Type
	{
		NoBattle, /** No battle. No ships are dangerous or at war */
		BattleWon, /** Battle ended. This is enemy ships but not dangerous */
		BattleLost, /** Battle ended. This is dangerons enemy ships but not owned dangerous ship */
		BattleLostNoRetreat, /** Battle ended. This is dangerons enemy ships but not owned dangerous ship. No ship can travel */
		Battle, /** A battle is in progress. Each ennemy company has at least one dangerous ship. One of ship can travel.*/
		BattleNoRetreat, /** A battle is in progress. Each ennemy company has at least one dangerous ship. No ship can travel */
	};
}

/** Debris field settings */
USTRUCT()
struct FFlareDebrisFieldInfo
{
	GENERATED_USTRUCT_BODY()

	/** Debris mesh catalog */
	UPROPERTY(EditAnywhere, Category = Content)
	class UFlareAsteroidCatalog* DebrisCatalog;

	/** Debris field intensity */
	UPROPERTY(EditAnywhere, Category = Content)
	float DebrisFieldDensity;

	/** Debris size */
	UPROPERTY(EditAnywhere, Category = Content)
	float MinDebrisSize;

	/** Debris size */
	UPROPERTY(EditAnywhere, Category = Content)
	float MaxDebrisSize;
};

/** Sector description */
USTRUCT()
struct FFlareSectorDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	/** Sector identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Orbit phase */
	UPROPERTY(EditAnywhere, Category = Content)
	float Phase;

	/** Peaceful sector */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsPeaceful;

	/** Is this sector ice-rich */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsIcy;

	/** Is this sector available for pumping stations */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsGeostationary;

	/** Is this sector poor in solar activity */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsSolarPoor;

	/** Level to load for this sector */
	UPROPERTY(EditAnywhere, Category = Content)
	FName LevelName;

	/** Track to play for this sector */
	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<EFlareMusicTrack::Type> LevelTrack;

	/** Debris field catalog */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareDebrisFieldInfo DebrisFieldInfo;

};


/** Sector orbit description */
USTRUCT()
struct FFlareSectorOrbitDescription
{
	GENERATED_USTRUCT_BODY()

	/** Orbit altitude */
	UPROPERTY(EditAnywhere, Category = Content)
	float Altitude;

	/** Orbit altitude */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSectorDescription> Sectors;
};


/** Sector celestial body description */
USTRUCT()
struct FFlareSectorCelestialBodyDescription
{
	GENERATED_USTRUCT_BODY()

	/** Parent celestial body identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName CelestialBodyIdentifier;

	/** Name of the celestial body */
	UPROPERTY(EditAnywhere, Category = Content)
	FText CelestialBodyName;

	/** Celestial body radius in pixels on the orbital map */
	UPROPERTY(EditAnywhere, Category = Content)
	int32 CelestialBodyRadiusOnMap;

	/** Celestial body image */
	UPROPERTY(EditAnywhere, Category = Content)
	FSlateBrush CelestialBodyPicture;

	/** Orbit altitude */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSectorOrbitDescription> Orbits;
};


/** Sector orbit parameters */
USTRUCT()
struct FFlareSectorOrbitParameters
{
	GENERATED_USTRUCT_BODY()
	/** Parent celestial body identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CelestialBodyIdentifier;

	/** Orbit altitude */
	UPROPERTY(EditAnywhere, Category = Save)
	double Altitude;

	/** Orbit phase */
	UPROPERTY(EditAnywhere, Category = Save)
	double Phase;
};

/** Sector resources prices*/
USTRUCT()
struct FFFlareResourcePrice
{
	GENERATED_USTRUCT_BODY()
	/** Resource */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ResourceIdentifier;

	/** Price */
	UPROPERTY(EditAnywhere, Category = Save)
	float Price;

	UPROPERTY(EditAnywhere, Category = Save)
	FFlareFloatBuffer Prices;
};

/** Sector save data */
USTRUCT()
struct FFlareSectorSave
{
	GENERATED_USTRUCT_BODY()

	/** Given Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FText GivenName;

	/** Sector identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareBombSave> BombData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareAsteroidSave> AsteroidData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> FleetIdentifiers;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> SpacecraftIdentifiers;

	/** Local sector time. */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 LocalTime;

	/** Sector peole. */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlarePeopleSave PeopleData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFFlareResourcePrice> ResourcePrices;

	UPROPERTY(VisibleAnywhere, Category = Save)
	bool IsTravelSector;
};


UCLASS()
class HELIUMRAIN_API UFlareSimulatedSector : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    /*----------------------------------------------------
        Save
    ----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(const FFlareSectorDescription* Description, const FFlareSectorSave& Data, const FFlareSectorOrbitParameters& OrbitParameters);

	/** Load sector people */
	virtual UFlarePeople* LoadPeople(const FFlarePeopleSave& PeopleData);

	/** Save the sector to a save file */
    virtual FFlareSectorSave* Save();

	void LoadResourcePrices();

	void SaveResourcePrices();


    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

    /** Create a station in the level  for a specific company */
	UFlareSimulatedSpacecraft* CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

    /** Create a ship in the level  for a specific company */
	UFlareSimulatedSpacecraft* CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company. No null parameter accepted */
	UFlareSimulatedSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetLocation, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Create an asteroid */
	void CreateAsteroid(int32 ID, FName Name, FVector Location);

	/** Add the fleet and its ships from the sector */
	void AddFleet(UFlareFleet* Fleet);

	/** Remove the fleet but let its ships in the sector */
	void DisbandFleet(UFlareFleet* Fleet);

	/** Retire the fleet and its ships from the sector */
	void RetireFleet(UFlareFleet* Fleet);

	int RemoveSpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	/** Check whether we can build a station, understand why if not */
	bool CanBuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company, TArray<FText>& OutReason, bool IgnoreCost = false);

	UFlareSimulatedSpacecraft* BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company);

	bool CanUpgrade(UFlareCompany* Company);

	bool CanUpgradeStation(UFlareSimulatedSpacecraft* Station, TArray<FText>& OutReason);

	bool UpgradeStation(UFlareSimulatedSpacecraft* Station);


	void AttachStationToAsteroid(UFlareSimulatedSpacecraft* Spacecraft);

	void SimulatePriceVariation();

	void SimulatePriceVariation(FFlareResourceDescription* Resource);

protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
	FFlareSectorSave                        SectorData;
    TArray<UFlareSimulatedSpacecraft*>      SectorStations;
	TArray<UFlareSimulatedSpacecraft*>      SectorShips;
	TArray<UFlareSimulatedSpacecraft*>      SectorSpacecrafts;

	TArray<UFlareFleet*>                    SectorFleets;

	UPROPERTY()
	UFlarePeople*							People;

	int32                                   PersistentStationIndex;
	float									LightRatio;

	AFlareGame*                             Game;

	UPROPERTY()
	FFlareSectorOrbitParameters             SectorOrbitParameters;
	const FFlareSectorDescription*          SectorDescription;
	TMap<FFlareResourceDescription*, float> ResourcePrices;
	TMap<FFlareResourceDescription*, FFlareFloatBuffer> LastResourcePrices;

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

	/** Get the name of this sector */
	FText GetSectorName();

	FString GetSectorCode();

	inline const FFlareSectorDescription* GetDescription() const
	{
		return SectorDescription;
	}

    inline FName GetIdentifier() const
    {
        return SectorData.Identifier;
    }

	/** Get the description of this sector */
	FText GetSectorDescription() const;

    inline TArray<UFlareSimulatedSpacecraft*>& GetSectorStations()
    {
        return SectorStations;
    }

    inline TArray<UFlareSimulatedSpacecraft*>& GetSectorShips()
    {
        return SectorShips;
    }

	inline TArray<UFlareSimulatedSpacecraft*>& GetSectorSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	inline TArray<UFlareFleet*>& GetSectorFleets()
	{
		return SectorFleets;
	}

	inline UFlarePeople* GetPeople()
	{
		return People;
	}

	int32 GetMaxStationsInSector()
	{
		return 30;
	}

	bool IsTravelSector()
	{
		return SectorData.IsTravelSector;
	}


	int64 GetStationConstructionFee(int64 BasePrice);

	uint32 GetResourceCount(UFlareCompany* Company, FFlareResourceDescription* Resource, bool IncludeShips = false, bool AllowTrade = false);

	float GetLightRatio()
	{
		return LightRatio;
	}

	int64 GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext, int32 Age = 0);

	float GetPreciseResourcePrice(FFlareResourceDescription* Resource, int32 Age = 0);

	void SwapPrices();

	void SetPreciseResourcePrice(FFlareResourceDescription* Resource, float NewPrice);


	static float GetDefaultResourcePrice(FFlareResourceDescription* Resource);

	uint32 GetTransfertResourcePrice(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource);

	inline FFlareSectorOrbitParameters* GetOrbitParameters()
	{
		return &SectorOrbitParameters;
	}

	FText GetSectorFriendlynessText(UFlareCompany* Company);

	/** Get the friendlyness status toward a company, as a color */
	FLinearColor GetSectorFriendlynessColor(UFlareCompany* Company);


	/** Get the friendlyness status toward a company */
	EFlareSectorFriendlyness::Type GetSectorFriendlyness(UFlareCompany* Company);

	/** Get the current battle status of a company */
	EFlareSectorBattleState::Type  GetSectorBattleState(UFlareCompany* Company);

};
