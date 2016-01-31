#pragma once

#include "Engine.h"
#include "FlareSpacecraftTypes.generated.h"

/** Part size values */
UENUM()
namespace EFlarePartSize
{
	enum Type
	{
		S,
		M,
		L,
		Num
	};
}

/** Part type values */
UENUM()
namespace EFlareSpawnMode
{
	enum Type
	{
		Safe,
		Spawn,
		Travel
	};
}

/** Ship component turret save data */
USTRUCT()
struct FFlareSpacecraftComponentTurretSave
{
	GENERATED_USTRUCT_BODY()

	/** Attribute name */
	UPROPERTY(EditAnywhere, Category = Save) float TurretAngle;

	/** Attribute value */
	UPROPERTY(EditAnywhere, Category = Save) float BarrelsAngle;
};

/** Ship component weapons save data */
USTRUCT()
struct FFlareSpacecraftComponentWeaponSave
{
	GENERATED_USTRUCT_BODY()

	/** Attribute name */
	UPROPERTY(EditAnywhere, Category = Save) int32 FiredAmmo;
};

/** Turret pilot save data */
USTRUCT()
struct FFlareTurretPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

/** Ship component save data */
USTRUCT()
struct FFlareSpacecraftComponentSave
{
	GENERATED_USTRUCT_BODY()

	/** Component catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ComponentIdentifier;

	/** Ship slot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ShipSlotIdentifier;

	/** Taken damages */
	UPROPERTY(EditAnywhere, Category = Save)
	float Damage;

	/** Component turret data*/
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSpacecraftComponentTurretSave Turret;

	/** Component turret data*/
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSpacecraftComponentWeaponSave Weapon;

	/** Pilot */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareTurretPilotSave Pilot;
};

/** Ship pilot save data */
USTRUCT()
struct FFlareShipPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

/** Asteroid save data */
USTRUCT()
struct FFlareAsteroidSave
{
	GENERATED_USTRUCT_BODY()

	/** Asteroid identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Asteroid location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Asteroid rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Asteroid linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Asteroid angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Asteroid scale */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Scale;

	/** Content identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 AsteroidMeshID;
};

/** Spacecraft cargo save data */
USTRUCT()
struct FFlareCargoSave
{
	GENERATED_USTRUCT_BODY()

	/** Cargo resource */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ResourceIdentifier;

	/** Cargo quantity */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Quantity;
};

/** Spacecraft factory save data */
USTRUCT()
struct FFlareFactorySave
{
	GENERATED_USTRUCT_BODY()

	/** Factory is active */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Active;

	/** Money locked by the factory */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 CostReserved;

	/** Resource Reserved by the factory */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ResourceReserved;

	/** Cumulated production duration for this production cycle */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 ProductedDuration;

	/** Factory production mode */
	UPROPERTY(EditAnywhere, Category = Save)
	bool InfiniteCycle;

	/** Factory cycle to produce */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 CycleCount;

	/** Max slot used for factory output */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> OutputCargoLimit;
};

/** Spacecraft save data */
USTRUCT()
struct FFlareSpacecraftSave
{
	GENERATED_USTRUCT_BODY()

	/** Ship location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** The spawn mode of the ship. */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareSpawnMode::Type> SpawnMode;

	/** Ship linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Ship angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Ship immatriculation. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Immatriculation;

	/** Ship nickname. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FName NickName;

	/** Ship catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Ship company identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	/** Components list */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareSpacecraftComponentSave> Components;

	/** We are docked at this station */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DockedTo;

	/** We are docked at this specific dock */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 DockedAt;

	/** Accululated heat in KJ */
	UPROPERTY(EditAnywhere, Category = Save)
	float Heat;

	/** Duration until the end of the power outage, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageDelay;

	/** Pending power outage downtime, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageAcculumator;

	/** If attached to an asteroid, identifier of this asteroid */
	UPROPERTY(EditAnywhere, Category = Save)
	FName AttachPoint;

	/** Pilot */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareShipPilotSave Pilot;

	/** Cargo bay content */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> Cargo;

	/** Factory states */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareFactorySave> FactoryStates;

	/** Is spacecraft assigned to the sector */
	UPROPERTY(EditAnywhere, Category = Save)
	bool IsAssigned;
	
	/** Asteroid we're stuck to */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareAsteroidSave AsteroidData;

	/** Factory states */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> SalesExcludedResources;

};


struct SpacecraftHelper
{
	static float GetIntersectionPosition(FVector TargetLocation,
										 FVector TargetVelocity,
										 FVector SourceLocation,
										 FVector SourceVelocity,
										 float ProjectileSpeed,
										 float PredictionDelay,
										 FVector* ResultPosition);
};


UCLASS()
class HELIUMRAIN_API UFlareSpacecraftTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:
};
