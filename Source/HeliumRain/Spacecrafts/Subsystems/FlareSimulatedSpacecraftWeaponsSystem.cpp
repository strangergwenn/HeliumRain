
#include "FlareSimulatedSpacecraftWeaponsSystem.h"
#include "../../Flare.h"

#include "../FlareSpacecraftTypes.h"
#include "../FlareSpacecraftComponent.h"
#include "../FlareSimulatedSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraftWeaponsSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraftWeaponsSystem::UFlareSimulatedSpacecraftWeaponsSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}

UFlareSimulatedSpacecraftWeaponsSystem::~UFlareSimulatedSpacecraftWeaponsSystem()
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
}
/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSimulatedSpacecraftWeaponsSystem::Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

	// Clear previous data
	WeaponList.Empty();
	WeaponDescriptionList.Empty();
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		delete WeaponGroupList[GroupIndex];
	}
	WeaponGroupList.Empty();

	UFlareSpacecraftComponentsCatalog* Catalog = Spacecraft->GetGame()->GetShipPartsCatalog();

	for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if(ComponentDescription->Type != EFlarePartType::Weapon)
		{
			continue;
		}

		WeaponList.Add(ComponentData);
		WeaponDescriptionList.Add(ComponentDescription);

		int32 GroupIndex = GetGroupByWeaponIdentifer(ComponentDescription->Identifier);
		if (GroupIndex < 0)
		{
			// No existing group yet
			FFlareSimulatedWeaponGroup* WeaponGroup = new FFlareSimulatedWeaponGroup();

			WeaponGroup->Description = ComponentDescription;

			if (WeaponGroup->Description->WeaponCharacteristics.TurretCharacteristics.IsTurret)
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_TURRET;
			}
			else if (WeaponGroup->Description->WeaponCharacteristics.BombCharacteristics.IsBomb)
			{
				if(WeaponGroup->Description->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration > 0)
				{
					WeaponGroup->Type = EFlareWeaponGroupType::WG_MISSILE;
				}
				else
				{
					WeaponGroup->Type = EFlareWeaponGroupType::WG_BOMB;
				}
			}
			else
			{
				WeaponGroup->Type = EFlareWeaponGroupType::WG_GUN;
			}
			WeaponGroup->Weapons.Add(ComponentData);
			WeaponGroupList.Add(WeaponGroup);
		}
		else
		{
			FFlareSimulatedWeaponGroup* WeaponGroup = WeaponGroupList[GroupIndex];
			WeaponGroup->Weapons.Add(ComponentData);
		}
	}
}


int32 UFlareSimulatedSpacecraftWeaponsSystem::GetGroupByWeaponIdentifer(FName Identifier) const
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

bool UFlareSimulatedSpacecraftWeaponsSystem::HasAntiLargeShipWeapon()
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true) <= 0)
		{
			continue;
		}

		if(WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiLargeShipValue > 0.5)
		{
			return true;
		}
	}
	return false;
}

bool UFlareSimulatedSpacecraftWeaponsSystem::HasAntiSmallShipWeapon()
{
	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true) <= 0)
		{
			continue;
		}

		if(WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiSmallShipValue > 0.5)
		{
			return true;
		}
	}
	return false;
}


void UFlareSimulatedSpacecraftWeaponsSystem::GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableSmallMilitary, float* IsUncontrollableLargeMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned)
{
	float LargePool = 0;
	float SmallPool = 0;
	float StationPool = 0;
	float UncontrollableCivilPool = 0;
	float UncontrollableSmallMilitaryPool = 0;
	float UncontrollableLargeMilitaryPool = 0;
	float NotUncontrollablePool = 0;
	float HarpoonedPool = 0;


	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		EFlareShellDamageType::Type DamageType = WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType;

		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true) <= 0)
		{
			continue;
		}

		if(DamageType == EFlareShellDamageType::LightSalvage)
		{
			UncontrollableCivilPool += 1.0;
			UncontrollableSmallMilitaryPool += 1.0;
		}
		else if (DamageType == EFlareShellDamageType::HeavySalvage)
		{
			UncontrollableCivilPool += 1.0;
			UncontrollableLargeMilitaryPool += 1.0;
		}
		else
		{
			NotUncontrollablePool += 1.0;
			UncontrollableSmallMilitaryPool += 0.01;
			UncontrollableLargeMilitaryPool += 0.01;
			HarpoonedPool += 1.0;
		}

		SmallPool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiSmallShipValue;
		LargePool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiLargeShipValue;
		StationPool += WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiStationValue;
	}


	if(LargePool > 0 || SmallPool > 0)
	{
		FVector2D PoolVector = FVector2D(LargePool, SmallPool);
		PoolVector.Normalize();
		LargePool = PoolVector.X;
		SmallPool = PoolVector.Y;
	}

	StationPool = FMath::Clamp(StationPool, 0.f, 0.1f);
	HarpoonedPool = FMath::Clamp(HarpoonedPool, 0.f, 0.1f);
	NotUncontrollablePool = FMath::Clamp(NotUncontrollablePool, 0.f, 0.1f);
	UncontrollableCivilPool = FMath::Clamp(UncontrollableCivilPool, 0.f, 0.1f);
	UncontrollableSmallMilitaryPool = FMath::Clamp(UncontrollableSmallMilitaryPool, 0.f, 0.1f);
	UncontrollableLargeMilitaryPool = FMath::Clamp(UncontrollableLargeMilitaryPool, 0.f, 0.1f);

	*IsLarge  = LargePool;
	*IsSmall  = SmallPool;
	*IsNotUncontrollable  = NotUncontrollablePool;
	*IsUncontrollableCivil  = UncontrollableCivilPool;
	*IsUncontrollableSmallMilitary  = UncontrollableSmallMilitaryPool;
	*IsUncontrollableLargeMilitary  = UncontrollableLargeMilitaryPool;
	*IsStation = StationPool;
	*IsHarpooned = HarpoonedPool;
}

int32 UFlareSimulatedSpacecraftWeaponsSystem::FindBestWeaponGroup(UFlareSimulatedSpacecraft* Target)
{
	int32 BestWeaponGroup = -1;
	float BestScore = 0;

	if (!Target) {
		return -1;
	}

	bool LargeTarget = (Target->GetSize() == EFlarePartSize::L);
	bool SmallTarget = (Target->GetSize() == EFlarePartSize::S);
	bool UncontrollableTarget = Target->GetDamageSystem()->IsUncontrollable();

	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		float Score = Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true);


		EFlareShellDamageType::Type DamageType = WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType;



		if (SmallTarget)
		{
			Score *= WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiSmallShipValue;
		}

		if (LargeTarget)
		{
			Score *= WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.AntiLargeShipValue;
		}

		if(DamageType == EFlareShellDamageType::LightSalvage || DamageType == EFlareShellDamageType::HeavySalvage)
		{
			Score *= (UncontrollableTarget ? 1.f : 0.f);
			Score *= (Target->IsHarpooned() ? 0.f: 1.f);
		}
		else
		{
			if(Target->IsMilitary())
			{
				Score *= (UncontrollableTarget ? 0.01f : 1.f);
			}
			else
			{
				Score *= (UncontrollableTarget ? 0.f : 1.f);
			}
		}

		if (Score > 0 && Score > BestScore)
		{
			BestWeaponGroup = GroupIndex;
			BestScore = Score;
		}
	}

	return BestWeaponGroup;
}

FName UFlareSimulatedSpacecraftWeaponsSystem::GetSlotIdentifierFromWeaponGroupIndex(const FFlareSpacecraftDescription* ShipDesc, int32 WeaponGroupIndex)
{
	FName TargetSlotName = NAME_None;

	for (int32 WeaponIndex = 0; WeaponIndex < ShipDesc->GunSlots.Num(); WeaponIndex++)
	{
		if (ShipDesc->GunSlots[WeaponIndex].GroupIndex == WeaponGroupIndex)
		{
			TargetSlotName = ShipDesc->GunSlots[WeaponIndex].SlotIdentifier;
			break;
		}
	}

	for (int32 WeaponIndex = 0; WeaponIndex < ShipDesc->TurretSlots.Num(); WeaponIndex++)
	{
		if (ShipDesc->TurretSlots[WeaponIndex].GroupIndex == WeaponGroupIndex)
		{
			TargetSlotName = ShipDesc->TurretSlots[WeaponIndex].SlotIdentifier;
			break;
		}
	}

	return TargetSlotName;
}

int32 UFlareSimulatedSpacecraftWeaponsSystem::GetGroupIndexFromSlotIdentifier(const FFlareSpacecraftDescription* ShipDesc, FName SlotName)
{
	int32 TargetIndex = 0;

	for (int32 WeaponIndex = 0; WeaponIndex < ShipDesc->GunSlots.Num(); WeaponIndex++)
	{
		if (ShipDesc->GunSlots[WeaponIndex].SlotIdentifier == SlotName)
		{
			TargetIndex = ShipDesc->GunSlots[WeaponIndex].GroupIndex;
			break;
		}
	}

	for (int32 WeaponIndex = 0; WeaponIndex < ShipDesc->TurretSlots.Num(); WeaponIndex++)
	{
		if (ShipDesc->TurretSlots[WeaponIndex].SlotIdentifier == SlotName)
		{
			TargetIndex = ShipDesc->TurretSlots[WeaponIndex].GroupIndex;
			break;
		}
	}

	return TargetIndex;
}

/*----------------------------------------------------
	System API
----------------------------------------------------*/

int32 UFlareSimulatedSpacecraftWeaponsSystem::GetWeaponGroupCount() const
{
	return WeaponGroupList.Num();
}

EFlareWeaponGroupType::Type UFlareSimulatedSpacecraftWeaponsSystem::GetActiveWeaponType() const
{
	// No active weapon in simulation
	return EFlareWeaponGroupType::WG_NONE;
}

#undef LOCTEXT_NAMESPACE
