#pragma once

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

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float ActivationTime; // in s

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float MaxAcceleration;// in cm.s-2

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float NominalVelocity; // In cm/s

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float MaxBurnDuration; // In s

	/** Bomb angular velocity in °/s */
	UPROPERTY(EditAnywhere, Category = Content) float AngularAcceleration; // degree.s-2
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

	/** Repair cost (in fleet supply) */
	UPROPERTY(EditAnywhere, Category = Content) int32 RefillCost;

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

	/** Scale for effect shown with a shell explode */
	UPROPERTY(EditAnywhere, Category = Content) float ExplosionEffectScale;

	/** Effect shown with a shell or a fragment impact a target */
	UPROPERTY(EditAnywhere, Category = Content) UParticleSystem* ImpactEffect;

	/** Scale for effect shown with a shell or a fragment impact a target */
	UPROPERTY(EditAnywhere, Category = Content) float ImpactEffectScale;

	/** Gun characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentGunCharacteristics GunCharacteristics;

	/** Turret characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentTurretCharacteristics TurretCharacteristics;

	/** Bomb characteristic structure */
	UPROPERTY(EditAnywhere, Category = Content) FFlareSpacecraftComponentBombCharacteristics BombCharacteristics;

	/** Weapon target affility */
	UPROPERTY(EditAnywhere, Category = Content) float AntiSmallShipValue;
	UPROPERTY(EditAnywhere, Category = Content) float AntiLargeShipValue;
	UPROPERTY(EditAnywhere, Category = Content) float AntiStationValue;
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

	/** Part combat points */
	UPROPERTY(EditAnywhere, Category = Content) int32 CombatPoints;

	/** Repair cost (in fleet supply) */
	UPROPERTY(EditAnywhere, Category = Content) int32 RepairCost;

	/** Component armor, in percent. 100 for full absorb */
	UPROPERTY(EditAnywhere, Category = Content) float Armor;

	/** Hit point for component fonctionnaly. Component not working when no more hit points */
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

	/** Customize this material */
	static void CustomizeMaterial(UMaterialInstanceDynamic* Mat, AFlareGame* Game, FLinearColor BasePaint, FLinearColor Paint, FLinearColor Overlay, FLinearColor Light, int32 Pattern, UTexture2D* Emblem);
	
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

	/** Return the amount of armor. If not destructible, return 1 */
	virtual float GetArmor();

	/** Return the amount of armor at the world location. If not destructible, return 1 */
	virtual float GetArmorAtLocation(FVector Location);

	/** Apply damage to this component. Return inflicted damage ratio */
	virtual float ApplyDamage(float Energy, EFlareDamage::Type DamageType, UFlareCompany* DamageSource);

	/** Return the remaining hit points ratio. 1 for no damage, 0 for destroyed */
	virtual float GetDamageRatio() const;

	/** Return true if the ship component is destroyed */
	virtual bool IsDestroyed() const;

	/** Return true if the ship component is powered. A destroyed component is not powered */
	virtual bool IsPowered() const;

	/** Return the current amount of generated power */
	virtual float GetGeneratedPower() const;

	/** Return the maximum amount of generated power */
	virtual float GetMaxGeneratedPower() const;

	/** Return true if is a generator (broken or not) */
	virtual bool IsGenerator() const;

	virtual bool IsBroken() const;

	virtual float GetUsableRatio() const;

	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const;

	/** Return the current amount of heat sink surface in m^2 */
	virtual float GetHeatSinkSurface() const;

	/** Return true if is a heatsink (broken or not) */
	virtual bool IsHeatSink() const;

	/** Return component total hit points */
	virtual float GetTotalHitPoints() const;

	virtual void OnRepaired();

	/** Create a damaged effect */
	virtual void StartDestroyedEffects();

	/** Spawn damage effects */
	virtual void StartDamagedEffect(FVector Location, FRotator Rotation, EFlarePartSize::Type WeaponSize);

	/** Should we start destruction effects ? */
	virtual bool IsDestroyedEffectRelevant();

	/** Is this component being drawn */
	virtual bool IsComponentVisible() const;

	/** Normalize a color */
	static FLinearColor NormalizeColor(FLinearColor Col);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraftPawn*                   SpacecraftPawn;

	AFlareSpacecraft*	                    Spacecraft;

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
	TArray<UFlareSpacecraftComponent*>      PowerSources;
	float                                   PreviousHealthRatio;

	// Heat state
	float                                   HeatSinkSurface; // Maximum heat surface in m^2
	float                                   HeatProduction; // Maxiumum heat production, in KW
	bool                                    LocalHeatEffect; // Is component temperature vary localy
	float                                   LocalTemperature;
	int32                                   PreviousTemperatureKelvin;

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
		return (ComponentDescription && ComponentDescription->Mesh ? ComponentDescription->Mesh : GetStaticMesh());
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
