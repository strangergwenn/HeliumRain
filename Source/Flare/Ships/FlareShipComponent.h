#pragma once

#include "Engine.h"
#include "FlareShipComponent.generated.h"

class AFlareShipBase;
class UFlareCompany;


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
namespace EFlarePartSize
{
	inline FString ToString(EFlarePartSize::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlarePartSize"), true);
		return EnumPtr->GetEnumName(EnumValue);
	}
}


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

/** Part attribute types */
UENUM()
namespace EFlarePartCharacteristicType
{
	enum Type
	{
		AmmoPower, // Weapon ammo energy in KJ
		AmmoCapacity, // Weapon ammo max capacity
		AmmoRate, // Weapon firerate in ammo/min
		EnginePower, // Engine thrust in KN
		EngineTankDrain, // Old
		RCSAccelerationRating, // Value represents global rcs system initial capabilities. Angular acceleration un °/s^2
		LifeSupport, // Value represents crew. When available crew fall below 0.5 the crew is disable, at 0, the crew is dead.
		HeatSink, // Value represents radiation surface in m^2
		ElectricSystem, // Value is positive to indicate the component generate power
		Cargo, // Value represents cargo volume in m^2
		HeatProduction, // Value represents heat production on usage in KiloWatt
		AmmoVelocity, // Weapon ammo velocity in m/s
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

/** Component characteristic */
USTRUCT()
struct FFlareShipComponentCharacteristic
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content) TEnumAsByte<EFlarePartCharacteristicType::Type> CharacteristicType;

	UPROPERTY(EditAnywhere, Category = Content) float CharacteristicValue;
};


/** Ship component attribute save data */
USTRUCT()
struct FFlareShipComponentAttributeSave
{
	GENERATED_USTRUCT_BODY()

	/** Attribute name */
	UPROPERTY(EditAnywhere, Category = Save) FName AttributeIdentifier;

	/** Attribute value */
	UPROPERTY(EditAnywhere, Category = Save) float AttributeValue;
};

/** Ship component save data */
USTRUCT()
struct FFlareShipComponentSave
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

	/** Component attributes */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareShipComponentAttributeSave> Attributes;
};


/** Base description of a ship component */
USTRUCT()
struct FFlareShipComponentDescription
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

	/** Array of characteristics */
	UPROPERTY(EditAnywhere, Category = Content)	TArray< FFlareShipComponentCharacteristic > Characteristics;

	/** Part mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* Mesh;

	/** Special effect mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* EffectMesh;

	/** Part mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

};


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareShipComponent : public UStaticMeshComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/** Initialize this component and register the master ship object */
	virtual void Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu = false);

	/** Save the ship component to a save file */
	virtual FFlareShipComponentSave* Save();

	/** Get the meshg scale */
	float GetMeshScale();

	/** Check if we initialized this component */
	bool IsInitialized();

	/** Set the new temperature of this component */
	virtual void SetTemperature(int32 TemperatureKelvin);

	/** Set the health of this component */
	virtual void SetHealth(float HealthRatio);

	/** Set the light status of this component */
	virtual void SetLightStatus(EFlareLightStatus::Type Status);

	/** Apply all customizations to the component */
	virtual void SetupComponentMesh();

	/** Create a special effect mesh */
	virtual void SetupEffectMesh();

	/** Get the current customization from the ship */
	virtual void UpdateCustomization();

	/** Update light effect */
	virtual void UpdateLight();

	/** Return bounding sphere in out parameters */
	virtual void GetBoundingSphere(FVector& Location, float& Radius);

	/** Check if the parent actor was rendered recently */
	bool IsVisibleByPlayer();


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

	/** Apply damage to this component. */
	virtual void ApplyDamage(float Energy);

	/** Apply overheat damage to this component only it is used. Burn damage are always applied. */
	virtual void ApplyHeatDamage(float OverheatEnergy, float BurnEnergy);

	/** Return the remaining hit points ratio. 1 for no damage, 0 for destroyed */
	float GetDamageRatio(bool WithArmor = false) const;

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
	virtual void UpdatePowerSources(TArray<UFlareShipComponent*>* AvailablePowerSources);

	/** Return true if is a generator (broken or not) */
	virtual bool IsGenerator() const;

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


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareShipBase*                         Ship;

	UPROPERTY()
	UFlareCompany*                          PlayerCompany;

	UPROPERTY()
	UStaticMeshComponent*                   EffectMesh;

	UPROPERTY()
	UParticleSystemComponent*               DestroyedEffects;

	// Materials
	UMaterialInstanceDynamic*               ComponentMaterial;
	UMaterialInstanceDynamic*               EffectMaterial;

	// Component description and data
	const FFlareShipComponentDescription*   ComponentDescription;
	FFlareShipComponentSave                 ShipComponentData;

	// General state
	float                                   LifeSupport;
	float                                   Power; // Current available power
	float                                   GeneratedPower; // Maximum generated power
	TArray<UFlareShipComponent*>            PowerSources;

	// Heat state
	float                                   HeatSinkSurface; // Maximum heat surface in m^2
	float                                   HeatProduction; // Maxiumum heat production, in KW

	// Light flickering state
	TEnumAsByte<EFlareLightStatus::Type>    LightFlickeringStatus;
	float                                   TimeLeftUntilFlicker;
	float                                   TimeLeftInFlicker;
	float                                   FlickerMaxOnPeriod;
	float                                   FlickerMaxOffPeriod;
	float                                   CurrentFlickerMaxPeriod;

	// Frame limiter
	int32                                   FramesToCountBeforeTick;
	int32                                   FramesSinceLastUpdate;


	// TODO M3 : Move to characteristic (engine description)
	UPROPERTY()		UParticleSystem*                        DeathEffectTemplate; 

};
