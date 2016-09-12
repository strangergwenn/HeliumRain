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

	/** Is this ship alive and well ? */
	virtual bool IsAlive() const;

	/** Is this ship temporarily unpowered ? */
	virtual bool HasPowerOutage() const;

	/** For how long ? */
	virtual float GetPowerOutageDuration() const;

	/** Get the current temperature */
	virtual float GetTemperature() const;

	/** Get the max temperature*/
	virtual float GetOverheatTemperature() const { return 1000; }


	/** Is this ship unable to use orbital engines for inter-sector navigation ? */
	virtual bool IsStranded() const;

	/** Is this ship unable to manoeuver at all ? */
	virtual bool IsUncontrollable() const;

	/** Is this ship unable to fight ? */
	virtual bool IsDisarmed() const;

	/** Get the health */
	virtual float GetGlobalHealth();

	/** Get the detailed health for this subsystem */
	virtual float GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor = false, bool WithAmmo = false) const;

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
