
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
	if (!ActiveWeaponGroup)
	{
		return;
	}

	switch (ActiveWeaponGroup->Type)
	{
		case EFlareWeaponGroupType::WG_GUN:
			for (int32 i = 0; i < ActiveWeaponGroup->Weapons.Num(); i++)
			{
				if (WantFire)
				{
					ActiveWeaponGroup->Weapons[i]->StartFire();
				}
				else
				{
					ActiveWeaponGroup->Weapons[i]->StopFire();
				}
			}

			break;
		case EFlareWeaponGroupType::WG_BOMB:
			if (Armed && WantFire)
			{
				Armed = false;
				int32 FireWeaponIndex = (ActiveWeaponGroup->LastFiredWeaponIndex+1) % ActiveWeaponGroup->Weapons.Num();
				ActiveWeaponGroup->Weapons[FireWeaponIndex]->StartFire();
				ActiveWeaponGroup->LastFiredWeaponIndex = FireWeaponIndex;
			}
			else if (!WantFire)
			{
				Armed = true;
			}

		case EFlareWeaponGroupType::WG_NONE:
		case EFlareWeaponGroupType::WG_TURRET:
		default:
			break;
	}


}

void UFlareSpacecraftWeaponsSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetParent()->GetDescription();
	Data = OwnerData;
}

inline static bool ConstPredicate (const FFlareWeaponGroup& ip1, const FFlareWeaponGroup& ip2)
 {
	 return (ip1.Description->WeaponCharacteristics.Order < ip2.Description->WeaponCharacteristics.Order);
 }

void UFlareSpacecraftWeaponsSystem::Start()
{
	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
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
			WeaponGroup->Target = NULL;

			Weapon->SetWeaponGroup(WeaponGroup);
			WeaponGroupList.Add(WeaponGroup);
		}
		else
		{
			FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];
			WeaponGroup->Weapons.Add(Weapon);
			Weapon->SetWeaponGroup(WeaponGroup);
		}
	}

	// init last fired ammo with the one with the most ammo count
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{

		FFlareWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];

		FLOGV("Group %d : component=%s", GroupIndex, *WeaponGroup->Description->Identifier.ToString())
		FLOGV("Group %d : type=%d", GroupIndex, (int32)WeaponGroup->Type.GetValue());
		FLOGV("Group %d : count=%d", GroupIndex, WeaponGroup->Weapons.Num());
		int MinAmmo = 0;
		int MinAmmoIndex = -1;

		for (int32 WeaponIndex = 0; WeaponIndex < WeaponGroup->Weapons.Num(); WeaponIndex++)
		{
			int32 CurrentAmmo = WeaponGroup->Weapons[WeaponIndex]->GetCurrentAmmo();
			if (MinAmmoIndex == -1 || CurrentAmmo < MinAmmo)
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
	WantFire = false;
	StopAllWeapons();
}

void UFlareSpacecraftWeaponsSystem::SetActiveWeaponTarget(AFlareSpacecraft* Target)
{
	if(ActiveWeaponGroup)
	{
		ActiveWeaponGroup->Target = Target;
	}
}

AFlareSpacecraft* UFlareSpacecraftWeaponsSystem::GetActiveWeaponTarget()
{
	if(ActiveWeaponGroup)
	{
		return ActiveWeaponGroup->Target;
	}
	return NULL;
}

void UFlareSpacecraftWeaponsSystem::StopAllWeapons()
{
	for (int32 WeaponIndex = 0; WeaponIndex < WeaponList.Num(); WeaponIndex++)
	{
		WeaponList[WeaponIndex]->StopFire();
	}
}



void UFlareSpacecraftWeaponsSystem::StartFire()
{
		WantFire = true;
}

void UFlareSpacecraftWeaponsSystem::StopFire()
{
		WantFire = false;
}

void UFlareSpacecraftWeaponsSystem::ActivateWeaponGroup(int32 Index)
{

	if (Index >= 0 && Index < WeaponGroupList.Num() && ActiveWeaponGroupIndex != Index)
	{
		StopAllWeapons();
		ActiveWeaponGroupIndex = Index;
		ActiveWeaponGroup = WeaponGroupList[Index];
		LastActiveWeaponGroupIndex = ActiveWeaponGroupIndex;
		Armed = false;
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons(bool Activate)
{
	if (Activate)
	{
		ActivateWeapons();
	}
	else
	{
		DeactivateWeapons();
	}
}

void UFlareSpacecraftWeaponsSystem::ActivateWeapons()
{
	ActivateWeaponGroup(LastActiveWeaponGroupIndex);
}

void UFlareSpacecraftWeaponsSystem::DeactivateWeapons()
{
	StopFire();
	StopAllWeapons();
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
		DeactivateWeapons();
	}
}

int32 UFlareSpacecraftWeaponsSystem::GetGroupByWeaponIdentifer(FName Identifier) const
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (WeaponGroupList[GroupIndex]->Description->Identifier == Identifier)
		{
			return GroupIndex;
		}
	}
	return -1;
}

EFlareWeaponGroupType::Type UFlareSpacecraftWeaponsSystem::GetActiveWeaponType() const
{
	if (ActiveWeaponGroup)
	{
		return ActiveWeaponGroup->Type;
	}
	return EFlareWeaponGroupType::WG_NONE;
}

bool UFlareSpacecraftWeaponsSystem::HasUsableWeaponType(EFlareWeaponGroupType::Type Type) const
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (WeaponGroupList[GroupIndex]->Type == Type && Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, false, true))
		{
			return true;
		}
	}
	return false;
}

void UFlareSpacecraftWeaponsSystem::GetTargetSizePreference(float* IsSmall, float* IsLarge)
{
	float LargePool = 0;
	float SmallPool = 0;

	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, false, true) <= 0)
		{
			continue;
		}

		if (WeaponGroupList[GroupIndex]->Type == EFlareWeaponGroupType::WG_BOMB)
		{
			LargePool += 1.0;
		}
		else if (WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
		{
			LargePool += 1.0;
			SmallPool += 0.1;
		}
		else
		{
			LargePool += 0.1;
			SmallPool += 1.0;
		}
	}

	if(LargePool > 0 || SmallPool > 0)
	{
		FVector2D PoolVector = FVector2D(LargePool, SmallPool);
		PoolVector.Normalize();
		LargePool = PoolVector.X;
		SmallPool = PoolVector.Y;
	}

	*IsLarge  = LargePool;
	*IsSmall  = SmallPool;
}

#undef LOCTEXT_NAMESPACE
