#pragma once
#include "FlareSpacecraftDamageSystemInterface.h"
#include "FlareSimulatedSpacecraftDamageSystem.generated.h"

/** Spacecraft damage system class */
UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraftDamageSystem : public UObject, public IFlareSpacecraftDamageSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Initialize this system */
	virtual void Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

public:

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/

	virtual bool IsAlive() const;

	virtual bool HasPowerOutage() const;

	virtual float GetPowerOutageDuration() const;

	virtual float GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor = false, bool WithAmmo = false) const;

	virtual float GetWeaponGroupHealth(int32 GroupIndex, bool WithArmor = false, bool WithAmmo = true) const;

	virtual float GetTemperature() const;

	virtual float GetOverheatTemperature() const { return 1000; }

protected:

	float GetDamageRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
						 FFlareSpacecraftComponentSave* ComponentData,
						 bool WithArmor) const;

	bool IsPowered(FFlareSpacecraftComponentSave* ComponentToPowerData) const;
	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

};
