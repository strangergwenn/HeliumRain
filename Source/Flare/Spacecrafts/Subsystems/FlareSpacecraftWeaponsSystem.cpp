
#include "../../Flare.h"

#include "FlareSpacecraftWeaponsSystem.h"
#include "../FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftWeaponsSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftWeaponsSystem::UFlareSpacecraftWeaponsSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}

UFlareSpacecraftWeaponsSystem::~UFlareSpacecraftWeaponsSystem()
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
}

/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftWeaponsSystem::TickSystem(float DeltaSeconds)
{
}

void UFlareSpacecraftWeaponsSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetDescription();
	Data = OwnerData;
}

inline static bool ConstPredicate (const FFlareWeaponGroup& ip1, const FFlareWeaponGroup& ip2)
 {
	 return (ip1.Description->WeaponCharacteristics.Order > ip2.Description->WeaponCharacteristics.Order);
 }

void UFlareSpacecraftWeaponsSystem::Start()
{
	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();
	WeaponGroupList.Empty();


	TArray<UActorComponent*> Weapons = Spacecraft->GetComponentsByClass(UFlareWeapon::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Weapons.Num(); ComponentIndex++)
	{
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Weapons[ComponentIndex]);
		WeaponList.Add(Weapon);
		WeaponDescriptionList.Add(Weapon->GetDescription());
		int32 GroupIndex = GetGroupByWeaponIdentifer(Weapon->GetDescription()->Identifier);
		if (GroupIndex < 0)
		{
			// No existing group yet
			FFlareWeaponGroup* WeaponGroup = new FFlareWeaponGroup();

			WeaponGroup->Description = Weapon->GetDescription();

			if (WeaponGroup->Description->WeaponCharacteristics.TurretCharacteristics.IsTurret)
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_TURRET;
			}
			else if (WeaponGroup->Description->WeaponCharacteristics.BombCharacteristics.IsBomb)
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_BOMB;
			}
			else
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_GUN;
			}
			WeaponGroup->LastFiredWeaponIndex = 0;
			WeaponGroup->Weapons.Add(Weapon);

			WeaponGroupList.Add(WeaponGroup);
		}
		else
		{
			FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];
			WeaponGroup->Weapons.Add(Weapon);
		}
	}

	// init last fired ammo with the one with the most ammo count
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{

		FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];

		FLOGV("Group %d : component=%s", GroupIndex, *WeaponGroup->Description->Identifier.ToString())
		FLOGV("Group %d : type=%d", GroupIndex,  WeaponGroup->Type.GetValue());
		FLOGV("Group %d : count=%d", GroupIndex, WeaponGroup->Weapons.Num());
		int MinAmmo = 0;
		int MinAmmoIndex = -1;

		for (int32 WeaponIndex = 0; WeaponIndex < WeaponGroup->Weapons.Num(); WeaponIndex++)
		{
			int32 CurrentAmmo = WeaponGroup->Weapons[WeaponIndex]->GetCurrentAmmo();
			if(MinAmmoIndex == -1 || CurrentAmmo < MinAmmo)
			{
				MinAmmo = CurrentAmmo;
				MinAmmoIndex = WeaponIndex;
			}
		}

		WeaponGroup->LastFiredWeaponIndex = MinAmmoIndex;
	}

	// Sort group
	WeaponGroupList.Sort(&ConstPredicate);

	// TODO save
	ActiveWeaponGroupIndex = -1;
	ActiveWeaponGroup = NULL;
	LastActiveWeaponGroupIndex = 0;
}



void UFlareSpacecraftWeaponsSystem::StartFire()
{
	// TODO new control modes
	//if (Spacecraft->GetDamageSystem()->IsAlive() && (IsPiloted || !ExternalCamera))
	//{
	if(ActiveWeaponGroup && ActiveWeaponGroup->Type != EFlareWeaponGroupType::WG_TURRET)
	{

		// TODO semi auto and alterned fire

		// Handle broken ammo

		for (int32 i = 0; i < ActiveWeaponGroup->Weapons.Num(); i++)
		{
			ActiveWeaponGroup->Weapons[i]->StartFire();
		}
	}
	//}
}

void UFlareSpacecraftWeaponsSystem::StopFire()
{
	// TODO new control modes
	//if (Spacecraft->GetDamageSystem()->IsAlive() && (IsPiloted || !ExternalCamera))
	//{
	if(ActiveWeaponGroup && ActiveWeaponGroup->Type != EFlareWeaponGroupType::WG_TURRET)
	{
		// TODO semi auto and alterned fire


		for (int32 i = 0; i < ActiveWeaponGroup->Weapons.Num(); i++)
		{
			ActiveWeaponGroup->Weapons[i]->StopFire();
		}
	}
	//}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeaponGroup(int32 Index)
{

	if(Index >= 0 && Index < WeaponGroupList.Num())
	{
		ActiveWeaponGroupIndex = Index;
		ActiveWeaponGroup = WeaponGroupList[Index];
	}
	else
	{
		ActiveWeaponGroup = NULL;
		ActiveWeaponGroupIndex = -1;
	}
	LastActiveWeaponGroupIndex = ActiveWeaponGroupIndex;
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons(bool Activate)
{
	if(Activate) {
		ActivateWeapons();
	}
	else
	{
		DesactivateWeapons();
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons()
{
	ActivateWeaponGroup(LastActiveWeaponGroupIndex);
}

void UFlareSpacecraftWeaponsSystem::DesactivateWeapons()
{
	StopFire();
	 ActiveWeaponGroup = NULL;
	 ActiveWeaponGroupIndex = -1;
}

void UFlareSpacecraftWeaponsSystem::ToogleWeaponActivation()
{
	if (ActiveWeaponGroupIndex < 0)
	{
		ActivateWeapons();
	}
	else
	{
		DesactivateWeapons();
	}
}

int32 UFlareSpacecraftWeaponsSystem::GetGroupByWeaponIdentifer(FName Identifier) const
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if(WeaponGroupList[GroupIndex]->Description->Identifier == Identifier)
		{
			return GroupIndex;
		}
	}
	return -1;
}


#undef LOCTEXT_NAMESPACE
