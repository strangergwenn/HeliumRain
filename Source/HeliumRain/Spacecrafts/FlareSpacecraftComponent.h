#pragma once

#include "Engine.h"
#include "FlareSpacecraftTypes.h"
#include "FlareTurretPilot.h"
#include "FlareSpacecraftComponent.generated.h"

class AFlareSpacecraftPawn;
class AFlareSpacecraft;
class UFlareCompany;


/** Part type values */
UENUM()
namespace EFlarePartType
{
	enum Type
	{
		None,
		OrbitalEngine,
		RCS,
		Weapon,
		Meta,
		InternalComponent,
		Num
	};
}

/** Light flickering status */
UENUM()
namespace EFlareLightStatus
{
	enum Type
	{
		Lit,
		Flickering,
		Dark,
		Num
	};
}

/** Shell fuze type */
UENUM()
namespace EFlareShellFuzeType
{
	enum Type
	{
		Contact, // The shell explode on contact
		Proximity, // The shell explode near the target
	};
}

/** Shell damage type */
UENUM()
namespace EFlareShellDamageType
{
	enum Type
	{
		HighExplosive, // Explosion send shell pieces around at hight velocity.
		ArmorPiercing, // Not explosive shell, The damage are done by kinetic energy. Classique bullets.
		HEAT,          // Heat Explosive Anti Tank. The explosion is focalized in a hot beam of metal melting armor.
		LightSalvage,  // No actual damage, enable retrieval of light ships
		HeavySalvage,  // No actual damage, enable retrieval of heavy ships
	};
}

/** Component general characteristic */
USTRUCT()
struct FFlareSpacecraftComponentGeneralCharacteristics
{
	GENERATED_USTRUCT_BODY()

	// Contain the ship crew.
	UPROPERTY(EditAnywhere, Category = Content) bool LifeSupport;

	// Is a electric system.
	UPROPERTY(EditAnywhere, Category = Content) bool ElectricSystem;

	// Heat radiation surface in m^2
	UPROPERTY(EditAnywhere, Category = Content) float HeatSink;

	// Heat production on usage in KiloWatt on max usage
	UPROPERTY(EditAnywhere, Category = Content) float HeatProduction;

	// Cargo volume un m^3
	UPROPERTY(EditAnywhere, Category = Content) float Cargo;
};

/** Engine characteristic */
USTRUCT()
struct FFlareSpacecraftComponentEngineCharacteristics
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content) bool IsEngine;

	// Engine thrust in KN
	UPROPERTY(EditAnywhere, Category = Content) float EnginePower;

	//  Value represents global rcs system initial capabilities. Angular acceleration un °/s^2
	UPROPERTY(EditAnywhere, Category = Content) float AngularAccelerationRate;

	/** Sound played when firing */
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* EngineSound;
};

/** Gun characteristic */
USTRUCT()
struct FFlareSpacecraftComponentBombCharacteristics
{
	GENERATED_USTRUCT_BODY()

	/** Is a gun  */
	UPROPERTY(EditAnywhere, Category = Content) bool IsBomb;

	/** Bomb mesh */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* BombMesh;

	/** Distance before armed */
	UPROPERTY(EditAnywhere, Category = Content) float ActivationDistance;

	/** Bomb forward velocity in m/s */
	UPROPERTY(EditAnywhere, Category = Content) float DropLinearVelocity;

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float DropAngularVelocity;
};

/** Gun characteristic */
USTRUCT()
struct FFlareSpacecraftComponentGunCharacteristics
{
	GENERATED_USTRUCT_BODY()

	/** Is a gun  */
	UPROPERTY(EditAnywhere, Category = Content) bool IsGun;

	/** Shell kinetic energy in KJ */
	UPROPERTY(EditAnywhere, Category = Content) float KineticEnergy;

	/** Weapon firerate in ammo/min */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoRate;

	/** Weapon ammo velocity in m/s */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoVelocity;

	/** Weapon ammo range before disappear in meter */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoRange;

	/** Weapon ammo precision in ° */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoPrecision;

	/** Weapon barrel count */
	UPROPERTY(EditAnywhere, Category = Content) int32 GunCount;

	/** Alterned fire. If true, the gun don't fire at same time */
	UPROPERTY(EditAnywhere, Category = Content) bool AlternedFire;

	/** If true, the gun fire one shell at each fire activation */
	UPROPERTY(EditAnywhere, Category = Content) bool SemiAutomaticFire;

	/** Effect used when firing */
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* FiringEffect;

	/** Effect used for tracer ammunitions */
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* TracerEffect;

	/** Decal used when an explosion hit a ship */
	UPROPERTY(EditAnywhere, Category = Content) UMaterialInterface* ExplosionMaterial;
};

/** Turret characteristic */
USTRUCT()
struct FFlareSpacecraftComponentTurretCharacteristics
{
	GENERATED_USTRUCT_BODY()

	/** Is a turret */
	UPROPERTY(EditAnywhere, Category = Content) bool IsTurret;

	/** Turret base angular velocity */
	UPROPERTY(EditAnywhere, Category = Content) float TurretAngularVelocity;

	/** Turret barrels angular velocity */
	UPROPERTY(EditAnywhere, Category = Content) float BarrelsAngularVelocity;

	/** Turret max angle. If >= 180 full rotation is possible */
	UPROPERTY(EditAnywhere, Category = Content) float TurretMaxAngle;

	/** Turret min angle. If <= -180 full rotation is possible */
	UPROPERTY(EditAnywhere, Category = Content) float TurretMinAngle;

	/** Turret barrels max angle.  If >= 180 full rotation is possible */
	UPROPERTY(EditAnywhere, Category = Content) float BarrelsMaxAngle;

	/** Turret barrels min angle. If <= -180 full rotation is possible */
	UPROPERTY(EditAnywhere, Category = Content) float BarrelsMinAngle;

	/** Turret mesh */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* TurretMesh;

	/** Barrel mesh */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* BarrelsMesh;

	/** Sound played on turret rotation */
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* TurretRotationSound;

	/** Sound played on barrel rotation*/
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* BarrelRotationSound;

};

/** Weapon characteristics */
USTRUCT()
struct FFlareSpacecraftComponentWeaponCharacteristics
{
	GENERATED_USTRUCT_BODY()

	/** Is a weapon */
	UPROPERTY(EditAnywhere, Category = Content) bool IsWeapon;

	/** Weapon order in weapon groups */
	UPROPERTY(EditAnywhere, Category = Content) int32 Order;

	/** Alterned weapons. If true, all the weapons don't fire at same time */
	UPROPERTY(EditAnywhere, Category = Content) bool AlternedWeapon;

	/** Damage type */
	UPROPERTY(EditAnywhere, Category = Content) TEnumAsByte<EFlareShellDamageType::Type> DamageType;

	/** Explosion (or fragment) energy in KJ */
	UPROPERTY(EditAnywhere, Category = Content) float ExplosionPower;

	/** Shell fragment count (for HE shell) */
	UPROPERTY(EditAnywhere, Category = Content) int32 AmmoFragmentCount;

	/** Ammo damage radius */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoDamageRadius;

	/** Ammo explosion effective radius (for HE shell) */
	UPROPERTY(EditAnywhere, Category = Content) float AmmoExplosionRadius;

	/** Weapon ammo max capacity */
	UPROPERTY(EditAnywhere, Category = Content) int32 AmmoCapacity;

	/** Damage type */
	UPROPERTY(EditAnywhere, Category = Content) TEnumAsByte<EFlareShellFuzeType::Type> FuzeType;

	/** If proximity fuse, below this distance the shell will explode */
	UPROPERTY(EditAnywhere, Category = Content) float FuzeMinDistanceThresold;

	/** If proximity fuse, above this distance the shell will no trig*/
	UPROPERTY(EditAnywhere, Category = Content) float FuzeMaxDistanceThresold;

	/** Sound played on impact */
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* ImpactSound;

	/** Sound played on damage */
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* DamageSound;

	/** Sound played when firing */
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* FiringSound;

	/** Effect shown with a shell explode */
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* ExplosionEffect;

	/** Effect shown with a shell or a fragment impact a target */
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* ImpactEffect;

	/** Gun characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentGunCharacteristics GunCharacteristics;

	/** Turret characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentTurretCharacteristics TurretCharacteristics;

	/** Bomb characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentBombCharacteristics BombCharacteristics;

};

/** Base description of a ship component */
USTRUCT()
struct FFlareSpacecraftComponentDescription
{
	GENERATED_USTRUCT_BODY()

	/** Part internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;

	/** Part size */
	UPROPERTY(EditAnywhere, Category = Content)	TEnumAsByte<EFlarePartType::Type> Type;

	/** Part size */
	UPROPERTY(EditAnywhere, Category = Content)	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Part name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Part description */
	UPROPERTY(EditAnywhere, Category = Content) FText Description;

	/** Part cost */
	UPROPERTY(EditAnywhere, Category = Content) int32 Cost;

	/** Hit point for component armor. Absorb first damages */
	UPROPERTY(EditAnywhere, Category = Content) float ArmorHitPoints;

	/** Hit point for component fonctionnaly. Absorb when no more armor. Component not working when no more hit points */
	UPROPERTY(EditAnywhere, Category = Content) float HitPoints;

	/** Part mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* Mesh;

	/** Effect used when destroyed*/
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* DestroyedEffect;

	/** Part mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

	/** General characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentGeneralCharacteristics GeneralCharacteristics;

	/** Engine characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentEngineCharacteristics EngineCharacteristics;

	/** Weapon characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentWeaponCharacteristics WeaponCharacteristics;

};


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareSpacecraftComponent : public UStaticMeshComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/** Initialize this component and register the master ship object */
	virtual void Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu = false);

	/** Save the ship component to a save file */
	virtual FFlareSpacecraftComponentSave* Save();

	/** Get the meshg scale */
	float GetMeshScale();

	/** Check if we initialized this component */
	bool IsInitialized();

	/** Set the visibility of this part while upgrading */
	virtual void SetVisibleInUpgrade(bool Visible);

	/** Set the new temperature of this component */
	virtual void SetTemperature(int32 TemperatureKelvin);

	/** Set the health of this component */
	virtual void SetHealth(float HealthRatio);

	/** Set the light status of this component */
	virtual void SetLightStatus(EFlareLightStatus::Type Status);

	/** Apply all customizations to the component */
	virtual void SetupComponentMesh();

	/** Get the current customization from the ship */
	virtual void UpdateCustomization();

	/** Update light effect */
	virtual void UpdateLight();

	/** Return bounding sphere in out parameters */
	virtual void GetBoundingSphere(FVector& Location, float& Radius);
	

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Component slot identifier */
	UPROPERTY(EditAnywhere, Category = Content) FName SlotIdentifier;


	/*----------------------------------------------------
		Damages
	----------------------------------------------------*/

	/** Return the amount of armor hit points at the world location. If not destructible, return a negative value */
	virtual float GetRemainingArmorAtLocation(FVector Location);

	/** Apply damage to this component. Return inflicted damage ratio */
	virtual float ApplyDamage(float Energy);

	/** Return the remaining hit points ratio. 1 for no damage, 0 for destroyed */
	virtual float GetDamageRatio(bool WithArmor = false) const;

	/** Return true if the ship component is destroyed */
	virtual bool IsDestroyed() const;

	/** Return true if the ship component is powered. A destroyed component is not powered */
	virtual bool IsPowered() const;

	/** Compute the current available power from power sources */
	virtual void UpdatePower();

	/** Return the current amount of generated power */
	virtual float GetGeneratedPower() const;

	/** Return the maximum amount of generated power */
	virtual float GetMaxGeneratedPower() const;

	/** Return the current amount of available power */
	virtual float GetAvailablePower() const;

	/** Find the closest power sources form all ship power sources */
	virtual void UpdatePowerSources(TArray<UFlareSpacecraftComponent*>* AvailablePowerSources);

	/** Return true if is a generator (broken or not) */
	virtual bool IsGenerator() const;

	virtual float GetUsableRatio() const;

	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const;

	/** Return the current amount of heat sink surface in m^2 */
	virtual float GetHeatSinkSurface() const;

	/** Return true if is a heatsink (broken or not) */
	virtual bool IsHeatSink() const;

	/** Return component total hit points, with armor and hitpoints */
	virtual float GetTotalHitPoints() const;

	/** Reset the taken damage to zero.	*/
	virtual void Repair();

	/** Create a damaged effect */
	virtual void StartDestroyedEffects();

	/** Spawn damage effects */
	virtual void StartDamagedEffect(FVector Location, FRotator Rotation, EFlarePartSize::Type WeaponSize);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraftPawn*                         SpacecraftPawn;

	AFlareSpacecraft*	                          Spacecraft;

	UPROPERTY()
	UFlareCompany*                          PlayerCompany;

	// Materials
	UMaterialInstanceDynamic*               ComponentMaterial;
	UMaterialInstanceDynamic*               EffectMaterial;

	// Component description and data
	FFlareSpacecraftComponentDescription*   ComponentDescription;
	FFlareSpacecraftComponentSave*          ShipComponentData;

	// General state
	bool                                    LifeSupport;
	float                                   Power; // Current available power
	float                                   GeneratedPower; // Maximum generated power
	TArray<UFlareSpacecraftComponent*>            PowerSources;

	// Heat state
	float                                   HeatSinkSurface; // Maximum heat surface in m^2
	float                                   HeatProduction; // Maxiumum heat production, in KW
	bool                                    LocalHeatEffect; // Is component temperature vary localy
	float                                   LocalTemperature;

	// Light flickering state
	bool                                    HasFlickeringLights;
	TEnumAsByte<EFlareLightStatus::Type>    LightFlickeringStatus;
	float                                   TimeLeftUntilFlicker;
	float                                   TimeLeftInFlicker;
	float                                   FlickerMaxOnPeriod;
	float                                   FlickerMaxOffPeriod;
	float                                   CurrentFlickerMaxPeriod;

	// Effects
	UPROPERTY()
	UParticleSystem*                        DestroyedEffectTemplate;

	UPROPERTY()
	UParticleSystemComponent*               DestroyedEffects;


	/*----------------------------------------------------
		Effects
	----------------------------------------------------*/

	UPROPERTY()
	UParticleSystem*                        ImpactEffectTemplateS;

	UPROPERTY()
	UParticleSystem*                        ImpactEffectTemplateL;

	int32                                   ImpactCount;

	int32                                   MaxImpactCount;

	float                                   ImpactEffectChance;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	virtual AFlareSpacecraft* GetSpacecraft() const
	{
		return Spacecraft;
	}

	virtual FFlareSpacecraftComponentDescription* GetDescription() const
	{
		return ComponentDescription;
	}

	virtual UStaticMesh* GetMesh(bool PresentationMode) const
	{
		return (ComponentDescription && ComponentDescription->Mesh ? ComponentDescription->Mesh : StaticMesh);
	}

	virtual bool HasLocalHeatEffect() const
	{
		return LocalHeatEffect;
	}

	virtual float GetLocalTemperature() const
	{
		return LocalTemperature;
	}
};
