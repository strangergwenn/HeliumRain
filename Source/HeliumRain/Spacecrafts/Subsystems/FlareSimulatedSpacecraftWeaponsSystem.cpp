
#include "../../Flare.h"

#include "FlareSimulatedSpacecraftWeaponsSystem.h"
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
				WeaponGroup->Type = EFlareWeaponGroupType::WG_BOMB;
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


void UFlareSimulatedSpacecraftWeaponsSystem::GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned)
{
	float LargePool = 0;
	float SmallPool = 0;
	float StationPool = 0;
	float UncontrollableCivilPool = 0;
	float UncontrollableMilitaryPool = 0;
	float NotUncontrollablePool = 0;
	float HarpoonedPool = 0;


	for (int32 GroupIndex = 0; GroupIndex < WeaponGroupList.Num(); GroupIndex++)
	{
		EFlareShellDamageType::Type DamageType = WeaponGroupList[GroupIndex]->Description->WeaponCharacteristics.DamageType;

		if (Spacecraft->GetDamageSystem()->GetWeaponGroupHealth(GroupIndex, true) <= 0)
		{
			continue;
		}

		if (WeaponGroupList[GroupIndex]->Type == EFlareWeaponGroupType::WG_BOMB)
		{
			if(DamageType == EFlareShellDamageType::HEAT)
			{
				LargePool += 1.0;
				StationPool += 0.1;
				NotUncontrollablePool += 1.0;
				UncontrollableMilitaryPool += 0.01;
			}
			else if(DamageType == EFlareShellDamageType::LightSalvage)
			{
				SmallPool += 1.0;
				UncontrollableCivilPool += 1.0;
				UncontrollableMilitaryPool += 1.0;

			}
			else if(DamageType == EFlareShellDamageType::HeavySalvage)
			{
				LargePool += 1.0;
				UncontrollableCivilPool += 1.0;
				UncontrollableMilitaryPool += 1.0;
			}
		}
		else
		{
			if (DamageType == EFlareShellDamageType::HEAT)
			{
				LargePool += 1.0;
				SmallPool += 0.1;
				StationPool = 0.1;
				NotUncontrollablePool += 1.0;
				UncontrollableMilitaryPool += 0.01;
				HarpoonedPool += 1.0;
			}
			else
			{
				LargePool += 0.01;
				SmallPool += 1.0;
				NotUncontrollablePool += 1.0;
				UncontrollableMilitaryPool += 0.01;
				HarpoonedPool += 1.0;
			}
		}
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
	UncontrollableMilitaryPool = FMath::Clamp(UncontrollableMilitaryPool, 0.f, 0.1f);

	*IsLarge  = LargePool;
	*IsSmall  = SmallPool;
	*IsNotUncontrollable  = NotUncontrollablePool;
	*IsUncontrollableCivil  = UncontrollableCivilPool;
	*IsUncontrollableMilitary  = UncontrollableMilitaryPool;
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


		if (WeaponGroupList[GroupIndex]->Type == EFlareWeaponGroupType::WG_BOMB)
		{
			if(DamageType == EFlareShellDamageType::HEAT)
			{
				Score *= (LargeTarget ? 1.f : 0.f);
				if(Target->IsMilitary())
				{
					Score *= (UncontrollableTarget ? 0.01f : 1.f);
				}
				else
				{
					Score *= (UncontrollableTarget ? 0.f : 1.f);
				}
			}
			else if(DamageType == EFlareShellDamageType::LightSalvage)
			{
				Score *= (SmallTarget ? 1.f : 0.f);
				Score *= (UncontrollableTarget ? 1.f : 0.f);
				Score *= (Target->IsHarpooned() ? 0.f: 1.f);
			}
			else if(DamageType == EFlareShellDamageType::HeavySalvage)
			{
				Score *= (LargeTarget ? 1.f : 0.f);
				Score *= (UncontrollableTarget ? 1.f : 0.f);
				Score *= (Target->IsHarpooned() ? 0.f: 1.f);
			}
		}
		else
		{
			if (DamageType == EFlareShellDamageType::HEAT)
			{
				Score *= (LargeTarget ? 1.f : 0.1f);
				if(Target->IsMilitary())
				{
					Score *= (UncontrollableTarget ? 0.01f : 1.f);
				}
				else
				{
					Score *= (UncontrollableTarget ? 0.f : 1.f);
				}
			}
			else
			{
				Score *= (SmallTarget ? 1.f : 0.01f);
				if(Target->IsMilitary())
				{
					Score *= (UncontrollableTarget ? 0.01f : 1.f);
				}
				else
				{
					Score *= (UncontrollableTarget ? 0.f : 1.f);
				}
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
