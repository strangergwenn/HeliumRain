#pragma once

#include "../../Flare.h"
#include "FlareSpacecraftWeaponsSystemInterface.generated.h"


/** Status of the ship */
UENUM()
namespace EFlareWeaponGroupType
{
	enum Type
	{
		WG_NONE,
		WG_GUN,
		WG_BOMB,
		WG_TURRET,
		WG_MISSILE
	};
}


/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareSpacecraftWeaponsSystemInterface  : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareSpacecraftWeaponsSystemInterface
{
	GENERATED_IINTERFACE_BODY()

public:

		/*----------------------------------------------------
			System Interface
		----------------------------------------------------*/

		virtual int32 GetWeaponGroupCount() const = 0;

		virtual EFlareWeaponGroupType::Type GetActiveWeaponType() const = 0;

		FText GetWeaponModeInfo() const;


};
