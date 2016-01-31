
#include "../../Flare.h"

#include "FlareSimulatedSpacecraftWeaponsSystem.h"

#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraftWeaponsSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraftWeaponsSystem::UFlareSimulatedSpacecraftWeaponsSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSimulatedSpacecraftWeaponsSystem::Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Description = Spacecraft->GetDescription();
	Data = OwnerData;
}

/*----------------------------------------------------
	System API
----------------------------------------------------*/

int32 UFlareSimulatedSpacecraftWeaponsSystem::GetWeaponGroupCount() const
{
	// TODO replace mock
	return 0;
}

EFlareWeaponGroupType::Type UFlareSimulatedSpacecraftWeaponsSystem::GetActiveWeaponType() const
{
	// No active weapon in simulation
	return EFlareWeaponGroupType::WG_NONE;
}

#undef LOCTEXT_NAMESPACE
