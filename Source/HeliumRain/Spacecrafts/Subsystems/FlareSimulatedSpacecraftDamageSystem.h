#pragma once
#include "FlareSimulatedSpacecraftDamageSystem.generated.h"

/** Possible subsystems targets */
UENUM()
namespace EFlareSubsystem
{
	enum Type
	{
		SYS_None,
		SYS_Temperature,
		SYS_Propulsion,
		SYS_RCS,
		SYS_LifeSupport,
		SYS_Power,
		SYS_Weapon,
	};
}

/** Damage Type */
UENUM()
namespace EFlareDamage
{
	enum Type
	{
		DAM_None,
		DAM_Collision,
		DAM_Overheat,
		DAM_HighExplosive,
		DAM_ArmorPiercing,
		DAM_HEAT,
	};
}

/** Spacecraft damage system class */
UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraftDamageSystem : public UObject
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


	/** Get a subsystem's name */
	static FText GetSubsystemName(EFlareSubsystem::Type SubsystemType);
};
