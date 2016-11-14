#include "../../Flare.h"
#include "FlareSpacecraftWeaponsSystemInterface.h"


#define LOCTEXT_NAMESPACE "FlareSpacecraftWeaponsSystemInterface"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftWeaponsSystemInterface::UFlareSpacecraftWeaponsSystemInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Helper
----------------------------------------------------*/

FText IFlareSpacecraftWeaponsSystemInterface::GetWeaponModeInfo() const
{
	FText ModeText;

	EFlareWeaponGroupType::Type WeaponType = GetActiveWeaponType();

	switch (WeaponType)
	{
		case EFlareWeaponGroupType::WG_NONE:
			ModeText = LOCTEXT("NoWeaponMode", "Flying");
			break;

		case EFlareWeaponGroupType::WG_GUN:
			ModeText = LOCTEXT("WeaponFighterMode", "Fighting");
			break;

		case EFlareWeaponGroupType::WG_BOMB:
			ModeText = LOCTEXT("WeaponBomberMode", "Bombing");
			break;

		case EFlareWeaponGroupType::WG_TURRET:
		default:
			ModeText = LOCTEXT("WeaponCapitalShipMode", "Targeting");
			break;
	}

	return ModeText;
}

#undef LOCTEXT_NAMESPACE
