#pragma once
#include "FlareSimulatedSpacecraftDamageSystem.generated.h"

const float BROKEN_RATIO = 0.5f;
const float UNCONTROLLABLE_RATIO = 0.5f;

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
	virtual float GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithAmmo = false) const;

	float Repair(FFlareSpacecraftComponentDescription* ComponentDescription,
				 FFlareSpacecraftComponentSave* ComponentData,
				 float MaxRepairRatio,
				 float MaxFS);

	float Refill(FFlareSpacecraftComponentDescription* ComponentDescription,
				 FFlareSpacecraftComponentSave* ComponentData,
				 float MaxRefillRatio,
				 float MaxFS);

protected:

	float GetUsableRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
																FFlareSpacecraftComponentSave* ComponentData) const;


	float GetClampedUsableRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
																FFlareSpacecraftComponentSave* ComponentData) const;

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

	float GetMaxHitPoints(FFlareSpacecraftComponentDescription* ComponentDescription) const;

	float GetDamageRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
						 FFlareSpacecraftComponentSave* ComponentData) const;

	static int32 GetRepairCost(FFlareSpacecraftComponentDescription* ComponentDescription);

	static int32 GetRefillCost(FFlareSpacecraftComponentDescription* ComponentDescription);

	/** Get a subsystem's name */
	static FText GetSubsystemName(EFlareSubsystem::Type SubsystemType);
};
