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
		AmmoPower,
		AmmoCapacity,
		AmmoRate,
		EnginePower,
		EngineTankDrain,
		RCSAccelerationRating,
		LifeSupport,
		HeatSink,
		ElectricSystem,
		Cargo,
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

/** Part characteristic */
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

class UFlareInternalComponent;

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

	/** Perform physical ship tick. */
	virtual void ShipTickComponent(float DeltaTime);

	/** Get the current customization from the ship */
	virtual void UpdateCustomization();


public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Component slot identifier */
	UPROPERTY(EditAnywhere, Category = Content) FName SlotIdentifier;

	/*----------------------------------------------------
		Damages
	----------------------------------------------------*/
	
	/**
	 * Return the amount of armor hit points at the world location.
	 * If not destructible, return a negative value
	 */ 
	virtual float GetRemainingArmorAtLocation(FVector Location);
	
	/**
	 * Apply damage to this component or its subcomponent.
	 */
	virtual void ApplyDamage(float Energy);
	
	/**
	 * Return the remaining hit points ratio. 1 for no damage, 0 for destroyed
	 */
	float GetDamageRatio() const;

	/**
	 * Return true if the ship component is destroyed
	 */
	virtual bool IsDestroyed() const;

	/**
	 * Return true if any lifesupport system is available
	 */
	virtual bool IsAlive() const;

	/**
	 * Return true if the ship component is powered. A destroyed component is not powered
	 */
	virtual bool IsPowered() const;

	/**
	 * Compute the current available power
	 */
	virtual void UpdatePower();

	/**
	 * Return the current amount of generated power
	 */
	virtual float GetGeneratedPower() const;

	/**
	 * Find the closest sources
	 */
	virtual void UpdatePowerSources(TArray<UFlareInternalComponent*>* AvailablePowerSources);

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

	UMaterialInstanceDynamic*               ComponentMaterial;
	UMaterialInstanceDynamic*               EffectMaterial;

	const FFlareShipComponentDescription*   ComponentDescription;

	// Light flickering style
	TEnumAsByte<EFlareLightStatus::Type>    LightFlickeringStatus;
	float                                   TimeLeftUntilFlicker;
	float                                   TimeLeftInFlicker;
	float                                   FlickerMaxOnPeriod;
	float                                   FlickerMaxOffPeriod;
	float                                   CurrentFlickerMaxPeriod;

	FFlareShipComponentSave                ShipComponentData;
	const FFlareShipComponentDescription* ComponentDescription;

	float LifeSupport;
	float Power;
	float GeneratedPower;
	TArray<UFlareShipComponent*> PowerSources;

};
