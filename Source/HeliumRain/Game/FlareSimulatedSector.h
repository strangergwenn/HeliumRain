
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
USTRUCT()
struct FFlareSectorBattleState
{
	GENERATED_USTRUCT_BODY()

	/** If false, no battle: no ships are dangerous or at war */
	UPROPERTY(EditAnywhere, Category = Content)
	bool InBattle;

	/** If in battle, and not in fight, the battle can be wont or lost. */
	UPROPERTY(EditAnywhere, Category = Content)
	bool BattleWon;

	/** Can be true only if a battle. If true, some ship can fight in both camps */
	UPROPERTY(EditAnywhere, Category = Content)
	bool InFight;

	/** True if the active ships cann fight */
	UPROPERTY(EditAnywhere, Category = Content)
	bool InActiveFight;

	/** If in fight, and not in  active fight, the active fight can be wont or lost. */
	UPROPERTY(EditAnywhere, Category = Content)
	bool ActiveFightWon;


	/** Indicate if the company can travel. */
	UPROPERTY(EditAnywhere, Category = Content)
	bool RetreatPossible;

	/** Indicate if there is dangerous enemy ships */
	UPROPERTY(EditAnywhere, Category = Content)
	bool HasDanger;

	int32 FriendlyControllableShipCount;
	int32 FriendlyStationCount;
	int32 FriendlyStationInCaptureCount;

	FFlareSectorBattleState Init()
	{
		InBattle = false;
		BattleWon = false;
		InFight = false;
		InActiveFight = false;
		ActiveFightWon = false;
		RetreatPossible = false;
		HasDanger = false;
		FriendlyControllableShipCount = 0;
		FriendlyStationCount = 0;
		FriendlyStationInCaptureCount = 0;

		return *this;
	}

	bool WantFight()
	{
		return !(!InBattle
				||  (!InFight && !BattleWon)
				||  (InFight && !InActiveFight && !ActiveFightWon));
	}

	bool operator==(const FFlareSectorBattleState& lhs)
	{
		return lhs.InBattle == InBattle
				&& lhs.BattleWon == BattleWon
				&& lhs.InFight == InFight
				&& lhs.InActiveFight == InActiveFight
				&& lhs.ActiveFightWon == ActiveFightWon
				&& lhs.RetreatPossible == RetreatPossible;
	}

	bool operator!=(const FFlareSectorBattleState& lhs)
	{
		return !(*this == lhs);
	}
};

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

	/** Parent celestial body identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName CelestialBodyIdentifier;

	/** Orbit altitude */
	UPROPERTY(EditAnywhere, Category = Content)
	float Altitude;

	/** Orbit phase */
	UPROPERTY(EditAnywhere, Category = Content)
	float Phase;

	/** Is this sector ice-rich */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsIcy;

	/** Is this sector available for pumping stations */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsGeostationary;

	/** Is this sector poor in solar activity */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsSolarPoor;

	/** Is this sector impossible to discover */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsHiddenFromTelescopes;

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

	/** Minimal orbit */
	UPROPERTY(EditAnywhere, Category = Content)
	float MinimalOrbitAltitude;

	/** Maximal orbit */
	UPROPERTY(EditAnywhere, Category = Content)
	float MaximalOrbitAltitude;
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
	TArray<FFlareMeteoriteSave> MeteoriteData;

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

	UPROPERTY(VisibleAnywhere, Category = Save)
	FFlareFloatBuffer FleetSupplyConsumptionStats;

	UPROPERTY(VisibleAnywhere, Category = Save)
	int32 DailyFleetSupplyConsumption;
};


/** Spawn settings for a station */
USTRUCT()
struct FFlareStationSpawnParameters
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content)
	FVector Location;

	UPROPERTY(EditAnywhere, Category = Content)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, Category = Content)
	FName AttachActorName;

	UPROPERTY(EditAnywhere, Category = Content)
	FName AttachComplexStationName;

	UPROPERTY(EditAnywhere, Category = Content)
	FName AttachComplexConnectorName;

	FFlareStationSpawnParameters()
	{
		Location = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
		AttachActorName = NAME_None;
		AttachComplexStationName = NAME_None;
		AttachComplexConnectorName = NAME_None;
	}
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
	UFlareSimulatedSpacecraft* CreateStation(FName StationClass, UFlareCompany* Company, bool UnderConstruction,
		FFlareStationSpawnParameters SpawnParameters = FFlareStationSpawnParameters());

    /** Create a ship in the level  for a specific company */
	UFlareSimulatedSpacecraft* CreateSpacecraft(FName ShipClass, UFlareCompany* Company, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company. No null parameter accepted */
	UFlareSimulatedSpacecraft* CreateSpacecraft(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetLocation, FRotator TargetRotation = FRotator::ZeroRotator,
		FFlareSpacecraftSave* CapturedSpacecraft = NULL, bool SafeSpawnAtLocation = false, bool UnderConstruction = false, FName AttachComplexStationName = NAME_None);

	/** Create an asteroid */
	void CreateAsteroid(int32 ID, FName Name, FVector Location);

	/** Add the fleet and its ships from the sector */
	void AddFleet(UFlareFleet* Fleet);

	/** Remove the fleet but let its ships in the sector */
	void DisbandFleet(UFlareFleet* Fleet);

	/** Retire the fleet and its ships from the sector */
	void RetireFleet(UFlareFleet* Fleet);

	int RemoveSpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	void SetSectorOrbitParameters(const FFlareSectorOrbitParameters& OrbitParameters);

	/** Check whether we can build a station, understand why if not */
	bool CanBuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company, TArray<FText>& OutReason,
		bool IgnoreCost = false, bool InComplex = false, bool InComplexSpecial = false);

	/** Build a station on this sector */
	UFlareSimulatedSpacecraft* BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company,
		FFlareStationSpawnParameters SpawnParameters = FFlareStationSpawnParameters());

	bool CanUpgrade(UFlareCompany* Company);

	bool CanUpgradeStation(UFlareSimulatedSpacecraft* Station, TArray<FText>& OutReason);

	bool UpgradeStation(UFlareSimulatedSpacecraft* Station);

	// Attaching
	void AttachStationToAsteroid(UFlareSimulatedSpacecraft* Spacecraft);
	void AttachStationToActor(UFlareSimulatedSpacecraft* Spacecraft, FName AttachActorName);
	void AttachStationToComplexStation(UFlareSimulatedSpacecraft* Spacecraft, FName AttachStationName, FName AttachConnectorName);

	void SimulatePriceVariation();

	void SimulatePriceVariation(FFlareResourceDescription* Resource);

	/** Can we load or buy this resource in this sector ? */
	bool WantSell(FFlareResourceDescription* Resource, UFlareCompany* Client);

	/** Can we unload or sell this resource in this sector ? */
	bool WantBuy(FFlareResourceDescription* Resource, UFlareCompany* Client);

	void ClearBombs();

	/** Get the balance of forces in the sector */
	void GetSectorBalance(UFlareCompany* Company, int32& PlayerShips, int32& EnemyShips, int32& NeutralShips, bool ActiveOnly);

	/** Get the balance of forces as a text */
	FText GetSectorBalanceText(bool ActiveOnly);

	void ProcessMeteorites();
	void GenerateMeteorites();
	void GenerateMeteoriteGroup(UFlareSimulatedSpacecraft* TargetStation, float PowerRatio);

	TMap<FFlareResourceDescription*, int32> DistributeResources(TMap<FFlareResourceDescription*, int32> Resources, UFlareSimulatedSpacecraft* Source, UFlareCompany* TargetCompany, bool DryRun);

protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
	FFlareSectorSave                        SectorData;
    TArray<UFlareSimulatedSpacecraft*>      SectorStations;
	TArray<UFlareSimulatedSpacecraft*>      SectorChildStations;
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
	//TMap<FFlareResourceDescription*, float> ResourcePrices;
	//TMap<FFlareResourceDescription*, FFlareFloatBuffer> LastResourcePrices;

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

	inline TArray<UFlareSimulatedSpacecraft*> const& GetSectorStations() const
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

	int32 GetMaxStationsPerCompany()
	{
		return 10;
	}

	int32 GetSectorCompanyStationCount(UFlareCompany* Company, bool IncludeCapture) const;

	bool IsTravelSector()
	{
		return SectorData.IsTravelSector;
	}

	int64 GetStationConstructionFee(int64 BasePrice, UFlareCompany* Company);

	uint32 GetResourceCount(UFlareCompany* Company, FFlareResourceDescription* Resource, bool IncludeShips = false, bool AllowTrade = false);

	float GetLightRatio()
	{
		return LightRatio;
	}

	//int64 GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext, int32 Age = 0);

	//float GetPreciseResourcePrice(FFlareResourceDescription* Resource, int32 Age = 0);

	void SwapPrices();

	void SetPreciseResourcePrice(FFlareResourceDescription* Resource, float NewPrice);

	void UpdateFleetSupplyConsumptionStats();

	void OnFleetSupplyConsumed(int32 Quantity);

	void UpdateReserveShips();

	static float GetDefaultResourcePrice(FFlareResourceDescription* Resource);

	uint32 GetTransfertResourcePrice(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource);

	const FFlareSectorOrbitParameters* GetOrbitParameters() const
	{
		return &SectorOrbitParameters;
	}

	FText GetSectorFriendlynessText(UFlareCompany* Company);

	/** Get the friendlyness status toward a company, as a color */
	FLinearColor GetSectorFriendlynessColor(UFlareCompany* Company);
	
	/** Get the friendlyness status toward a company */
	EFlareSectorFriendlyness::Type GetSectorFriendlyness(UFlareCompany* Company);

	/** Get the current battle status of a company */
	FFlareSectorBattleState GetSectorBattleState(UFlareCompany* Company);

	/** Get the current battle status text */
	FText GetSectorBattleStateText(UFlareCompany* Company);

	/** Return true if the company is in a battle where it can 	be hurt */
	bool IsInDangerousBattle(UFlareCompany* Company);

	/** Is a battle in progress with the player */
	bool IsPlayerBattleInProgress();

	int32 GetCompanyCapturePoints(UFlareCompany* Company) const;

	TArray<FFlareMeteoriteSave>& GetMeteorites()
	{
		return SectorData.MeteoriteData;
	}
};
