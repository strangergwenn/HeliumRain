#pragma once

#include "FlareShipModule.generated.h"

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


/** Part size values */
UENUM()
namespace EFlarePartType
{
	enum Type
	{
		None,
		OrbitalEngine,
		RCS,
		Weapon,
		Num
	};
}

/** Part attribute types */
UENUM()
namespace EFlarePartAttributeType
{
	enum Type
	{
		Armor,
		AmmoPower,
		AmmoRange,
		AmmoCapacity,
		AmmoRate,
		EnginePower,
		EngineTankDrain,
		RCSAccelerationRating,
		Num
	};
}

/** Part characteristic */
USTRUCT()
struct FFlarePartCharacteristic
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content) TEnumAsByte<EFlarePartAttributeType::Type> CharacteristicType;

	UPROPERTY(EditAnywhere, Category = Content) float CharacteristicValue;
};


/** Base description of a ship component */
USTRUCT()
struct FFlareShipModuleDescription
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

	/** Array of characteristics */
	UPROPERTY(EditAnywhere, Category = Content)	TArray< FFlarePartCharacteristic > Characteristics;

	/** Part mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* Mesh;

	/** Special effect mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UStaticMesh* EffectMesh;

	/** Part mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

};


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareShipModule : public UStaticMeshComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void OnRegister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	/** Initialize this component and register the master ship object */
	virtual void Initialize(const FFlareShipModuleDescription* Description, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu = false);

	/** Get the meshg scale */
	float GetMeshScale();

	/** Check if we initialized this component */
	bool IsInitialized();

	/** Set the new temperature of this component */
	virtual void SetTemperature(int32 TemperatureKelvin);

	/** Apply all customizations to the module */
	virtual void SetupModuleMesh();

	/** Create a special effect mesh */
	virtual void SetupEffectMesh();

	/** Get the current customization from the ship */
	virtual void UpdateCustomization();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareShipBase* Ship;
	
	UPROPERTY()
	UFlareCompany* PlayerCompany;

	UPROPERTY()
	UStaticMeshComponent* EffectMesh;

	UMaterialInstanceDynamic* ModuleMaterial;
	UMaterialInstanceDynamic* EffectMaterial;

	const FFlareShipModuleDescription* ModuleDescription;


};
