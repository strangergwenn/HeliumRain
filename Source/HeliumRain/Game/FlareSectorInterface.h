#pragma once


#include "../Economy/FlarePeople.h"
#include "../Player/FlareSoundManager.h"
#include "FlareSectorInterface.generated.h"


struct FFlareBombSave;
class UFlareSimulatedSector;
class IFlareSpacecraftInterface;

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

/** Resource price context */
UENUM()
namespace EFlareResourcePriceContext
{
	enum Type
	{
		Default, /** Default price */
		FactoryInput, /** Price selling to a factory needing the resource */
		FactoryOutput, /** Price buying the resource to a factory */
		ConsumerConsumption, /** Price selling to a the people */
		MaintenanceConsumption, /** Price selling to company using maintenance */
	};
}

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

	/*UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> StationData;*/

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareBombSave> BombData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareAsteroidSave> AsteroidData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> FleetIdentifiers;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> SpacecraftIdentifiers;

	/** Last flown identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName LastFlownShip;

	/** Local sector time. */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 LocalTime;

	/** Sector peole. */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlarePeopleSave PeopleData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFFlareResourcePrice> ResourcePrices;
};

UCLASS(abstract)
class UFlareSectorInterface  : public UObject
{
	GENERATED_UCLASS_BODY()

public:


protected:
	void LoadResourcePrices();

	void SaveResourcePrices();


	/*----------------------------------------------------
	  Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareSectorSave                        SectorData;

	AFlareGame*                             Game;

	UPROPERTY()
	FFlareSectorOrbitParameters             SectorOrbitParameters;
	const FFlareSectorDescription*          SectorDescription;
	TMap<FFlareResourceDescription*, float> ResourcePrices;

public:
	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FFlareSectorSave* GetData()
	{
		return &SectorData;
	}

	inline const FFlareSectorDescription* GetDescription() const
	{
		return SectorDescription;
	}
	/** Get the name of this sector */
	FText GetSectorName();

	FString GetSectorCode();

	inline FFlareSectorOrbitParameters* GetOrbitParameters()
	{
		return &SectorOrbitParameters;
	}

	/** Get the friendlyness status toward a company */
	EFlareSectorFriendlyness::Type GetSectorFriendlyness(UFlareCompany* Company);

	/** Get the current battle status of a company */
	EFlareSectorBattleState::Type  GetSectorBattleState(UFlareCompany* Company);

	/** Get the friendlyness status toward a company, as a text */
	FText GetSectorFriendlynessText(UFlareCompany* Company);

	/** Get the friendlyness status toward a company, as a color */
	FLinearColor GetSectorFriendlynessColor(UFlareCompany* Company);

	virtual UFlareSimulatedSector* GetSimulatedSector() PURE_VIRTUAL(UFlareSectorInterface::GetSimulatedSector, return NULL;)

	float GetPreciseResourcePrice(FFlareResourceDescription* Resource);

	void SetPreciseResourcePrice(FFlareResourceDescription* Resource, float NewPrice);

	virtual int64 GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext);

	static float GetDefaultResourcePrice(FFlareResourceDescription* Resource);

	virtual TArray<IFlareSpacecraftInterface*>& GetSectorStationInterfaces() PURE_VIRTUAL(UFlareSectorInterface::GetSectorStationInterfaces, static TArray<IFlareSpacecraftInterface*> Dummy; return Dummy;)

	virtual TArray<IFlareSpacecraftInterface*>& GetSectorShipInterfaces() PURE_VIRTUAL(UFlareSectorInterface::GetSectorShipInterfaces, static TArray<IFlareSpacecraftInterface*> Dummy; return Dummy;)

	uint32 GetTransfertResourcePrice(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource);

	// TODO Check docking capabilities
	/** Transfert resource from one spacecraft to another spacecraft */
	uint32 TransfertResources(IFlareSpacecraftInterface* SourceSpacecraft, IFlareSpacecraftInterface* DestinationSpacecraft, FFlareResourceDescription* Resource, uint32 Quantity);


	bool CanUpgrade(UFlareCompany* Company);

};
