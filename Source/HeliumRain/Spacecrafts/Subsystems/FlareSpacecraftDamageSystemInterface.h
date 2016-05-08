#pragma once

#include "FlareSpacecraftDamageSystemInterface.generated.h"

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


/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareSpacecraftDamageSystemInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareSpacecraftDamageSystemInterface
{
	GENERATED_IINTERFACE_BODY()

public:

		/*----------------------------------------------------
			System Interface
		----------------------------------------------------*/

		virtual bool IsAlive() const = 0;

		virtual bool HasPowerOutage() const = 0;

		virtual float GetPowerOutageDuration() const = 0;

		virtual float GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor = false, bool WithAmmo = false) const = 0;

		virtual float GetWeaponGroupHealth(int32 GroupIndex, bool WithArmor = false, bool WithAmmo = true) const = 0;

		virtual float GetTemperature() const = 0;

		virtual float GetOverheatTemperature() const = 0;


		/** Get a subsystem's name */
		static FText GetSubsystemName(EFlareSubsystem::Type SubsystemType);

};
