
#include "FlareSimulatedSpacecraftDamageSystem.h"
#include "../../Flare.h"

#include "../FlareSimulatedSpacecraft.h"
#include "../FlareSpacecraftComponent.h"

#include "../../Data/FlareSpacecraftComponentsCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlarePlanetarium.h"

#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGameTools.h"

#include "FlareSimulatedSpacecraftWeaponsSystem.h"


DECLARE_CYCLE_STAT(TEXT("FlareSimulatedDamageSystem UpdateSubsystemHealth"), STAT_FlareSimulatedDamageSystem_UpdateSubsystemHealth, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSimulatedDamageSystem GetWeaponGroupHealth"), STAT_FlareSimulatedDamageSystem_GetWeaponGroupHealth, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSimulatedDamageSystem ApplyDamage"), STAT_FlareSimulatedDamageSystem_ApplyDamage, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSimulatedDamageSystem IsPowered"), STAT_FlareSimulatedDamageSystem_IsPowered, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraftDamageSystem"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraftDamageSystem::UFlareSimulatedSpacecraftDamageSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSimulatedSpacecraftDamageSystem::Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Description = Spacecraft->GetDescription();
	Data = OwnerData;
	DamageDirty = true;
	AmmoDirty = true;
	IsPoweredCacheIndex = 0;

	for (int32 Index = EFlareSubsystem::SYS_None; Index <= EFlareSubsystem::SYS_WeaponAndAmmo; Index++)
	{
		SubsystemHealth.Add(1.0f);
	}

	for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];
		ComponentData->IsPoweredCacheIndex = -1;
	}

	WasControllable = !IsUncontrollable();
	WasAlive = IsAlive();
}


/*----------------------------------------------------
	System API
----------------------------------------------------*/

void UFlareSimulatedSpacecraftDamageSystem::TickSystem()
{
}

bool UFlareSimulatedSpacecraftDamageSystem::IsAlive() const
{
	if(Spacecraft->GetDescription()->IsSubstation)
	{
		return true;
	}
	return GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport) > 0;
}

bool UFlareSimulatedSpacecraftDamageSystem::HasPowerOutage() const
{
	return GetPowerOutageDuration() > 0.f;
}

float UFlareSimulatedSpacecraftDamageSystem::GetPowerOutageDuration() const
{
	return Data->PowerOutageDelay;
}

bool UFlareSimulatedSpacecraftDamageSystem::IsStranded() const
{
	if (!IsAlive()) {
		return false;
	}

	if(Spacecraft->IsStation())
	{
		return false;
	}
	return (GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) < 0.3f || IsUncontrollable());
}

bool UFlareSimulatedSpacecraftDamageSystem::IsUncontrollable() const
{
	if (Spacecraft->IsStation())
	{
		return false;
	}

	if (!IsAlive())
	{
		return true;
	}

	return (GetSubsystemHealth(EFlareSubsystem::SYS_RCS) == 0.0f);
}

bool UFlareSimulatedSpacecraftDamageSystem::IsDisarmed() const
{
	if(!Spacecraft->IsMilitary())
	{
		return true;
	}

	if(Spacecraft->IsStation())
	{
		return true;
	}

	if (Spacecraft->GetSize() == EFlarePartSize::S && IsUncontrollable())
	{
		return true;
	}

	if (!IsAlive())
	{
		return true;
	}

	return (GetSubsystemHealth(EFlareSubsystem::SYS_WeaponAndAmmo) == 0.0f);
}

bool UFlareSimulatedSpacecraftDamageSystem::IsCrewEndangered() const
{
	if (!IsAlive())
	{
		return true;
	}

	return (GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport) < BROKEN_RATIO);
}


float UFlareSimulatedSpacecraftDamageSystem::GetGlobalDamageRatio()
{
	float DamageRatioSum = 0;
	UFlareSpacecraftComponentsCatalog* Catalog = Spacecraft->GetGame()->GetShipPartsCatalog();

	for (FFlareSpacecraftComponentSave& ComponentData : Spacecraft->GetData().Components)
	{
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData.ComponentIdentifier);

		float DamageRatio = GetDamageRatio(ComponentDescription, &ComponentData);
		DamageRatioSum += DamageRatio;
	}

	return DamageRatioSum / Spacecraft->GetData().Components.Num();
}

float UFlareSimulatedSpacecraftDamageSystem::GetGlobalHealth()
{
	if(Spacecraft->IsStation())
	{
		return GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport);
	}
	else
	{
		float GlobalHealth = (GetSubsystemHealth(EFlareSubsystem::SYS_Temperature)
			+ GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion)
			+ GetSubsystemHealth(EFlareSubsystem::SYS_RCS)
			+ GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport)
			+ GetSubsystemHealth(EFlareSubsystem::SYS_Power));

		if (Spacecraft->IsMilitary())
		{
			GlobalHealth += GetSubsystemHealth(EFlareSubsystem::SYS_Weapon);
			return GlobalHealth / 6.0f;
		}
		else
		{
			return GlobalHealth / 5.0f;
		}
	}
}

float UFlareSimulatedSpacecraftDamageSystem::GetSubsystemHealth(EFlareSubsystem::Type Type) const
{
	if (DamageDirty || AmmoDirty)
	{
		UFlareSimulatedSpacecraftDamageSystem* UnprotectedThis = const_cast<UFlareSimulatedSpacecraftDamageSystem *>(this);
		UnprotectedThis->UpdateSubsystemsHealth();
	}

	return SubsystemHealth[Type];
}

float UFlareSimulatedSpacecraftDamageSystem::GetWeaponGroupHealth(int32 GroupIndex, bool WithAmmo) const
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSimulatedDamageSystem_GetWeaponGroupHealth);

	 FFlareSimulatedWeaponGroup* WeaponGroup = Spacecraft->GetWeaponsSystem()->GetWeaponGroup(GroupIndex);
	 float Health = 0.0;

	 float Total = 0.f;
	 for (int32 ComponentIndex = 0; ComponentIndex < WeaponGroup->Weapons.Num(); ComponentIndex++)
	 {
		 FFlareSpacecraftComponentSave* Weapon = WeaponGroup->Weapons[ComponentIndex];

		 int32 MaxAmmo = WeaponGroup->Description->WeaponCharacteristics.AmmoCapacity;
		 int32 CurrentAmmo = MaxAmmo - Weapon->Weapon.FiredAmmo;

		 Total += GetClampedUsableRatio(WeaponGroup->Description, Weapon)
				 *((CurrentAmmo > 0 || !WithAmmo) ? 1 : 0);

	 }
	 Health = Total/WeaponGroup->Weapons.Num();

	 return Health;
}

int32 UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(FFlareSpacecraftComponentDescription* ComponentDescription)
{
	return FMath::Max(1,ComponentDescription->RepairCost);
}

int32 UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(FFlareSpacecraftComponentDescription* ComponentDescription)
{
	return FMath::Max(1, ComponentDescription->WeaponCharacteristics.RefillCost);
}

float UFlareSimulatedSpacecraftDamageSystem::Repair(FFlareSpacecraftComponentDescription* ComponentDescription,
			 FFlareSpacecraftComponentSave* ComponentData,
			 float MaxRepairRatio,
			 float MaxFS)
{
	float RepairCost = 0.f;
	float MaxAffordableRatio = MaxFS / (float) GetRepairCost(ComponentDescription);
	float RepairNeedsRatio = 1.f - GetDamageRatio(ComponentDescription, ComponentData);

	float RepairRatio = FMath::Min(RepairNeedsRatio, MaxAffordableRatio);
	RepairRatio = FMath::Min(RepairRatio, MaxRepairRatio);


	if (RepairRatio > 0.f)
	{
		float MaxHitPoints = GetMaxHitPoints(ComponentDescription);
		if(ComponentData->Damage > MaxHitPoints)
		{
			ComponentData->Damage = MaxHitPoints;
		}

		ComponentData->Damage = FMath::Max(0.f, ComponentData->Damage - MaxHitPoints * RepairRatio);

		if(ComponentData->Damage < 0.001)
		{
			ComponentData->Damage = 0;
		}

		SetDamageDirty(ComponentDescription);
		RepairCost = RepairRatio * GetRepairCost(ComponentDescription);

		//FLOGV("%s %s repair %f for %f fs (damage ratio: %f)",  *Spacecraft->GetImmatriculation().ToString(),  *ComponentData->ShipSlotIdentifier.ToString(), RepairRatio, RepairCost, GetDamageRatio(ComponentDescription, ComponentData));

		Spacecraft->GetCompany()->InvalidateCompanyValueCache();

		if (Spacecraft->IsActive())
		{
			Spacecraft->GetActive()->OnRepaired();
		}
	}
	return RepairCost;
}

float UFlareSimulatedSpacecraftDamageSystem::Refill(FFlareSpacecraftComponentDescription* ComponentDescription,
			 FFlareSpacecraftComponentSave* ComponentData,
			 float MaxRefillRatio,
			 float MaxFS)
{
	float RefillCost = 0.f;
	float MaxAffordableRatio = MaxFS / (float) GetRefillCost(ComponentDescription);



	if(ComponentDescription->Type != EFlarePartType::Weapon)
	{
		return 0.f;
	}
	int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
	int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

	float FillRatio = FMath::Clamp((float) CurrentAmmo / (float) MaxAmmo, 0.f, 1.f);
	float RefillNeedsRatio = 1.f - FillRatio;

	float RefillRatio = FMath::Min(RefillNeedsRatio, MaxAffordableRatio);
	RefillRatio = FMath::Min(RefillRatio, MaxRefillRatio);


	if (RefillRatio > 0.f)
	{
		int32 RefillAmount = FMath::CeilToInt(FMath::Max(1.f, MaxAmmo * RefillRatio));

		int32 NewAmmoCount =  CurrentAmmo + RefillAmount;

		ComponentData->Weapon.FiredAmmo = FMath::Clamp(MaxAmmo - NewAmmoCount, 0, MaxAmmo);
		SetAmmoDirty();

		RefillCost = RefillRatio * GetRefillCost(ComponentDescription);

		/*FLOGV("%s %s refill %f for %f fs (fill ratio: %f)",
			  *Spacecraft->GetImmatriculation().ToString(),
			  *ComponentData->ShipSlotIdentifier.ToString(),
			  RefillRatio,
			  RefillCost,
			  ((float)(MaxAmmo - ComponentData->Weapon.FiredAmmo) / (float) MaxAmmo));*/

		/*FLOGV("RefillRatio %f,",RefillRatio);
		FLOGV("MaxAmmo %d,",MaxAmmo);
		FLOGV("CurrentAmmo %d,",CurrentAmmo);
		FLOGV("NewAmmoCount %d,",NewAmmoCount);
		FLOGV("ComponentData->Weapon.FiredAmmo %d,",ComponentData->Weapon.FiredAmmo);
*/
		Spacecraft->GetCompany()->InvalidateCompanyValueCache();

		if (Spacecraft->IsActive())
		{
			Spacecraft->GetActive()->OnRefilled();
		}
	}
	return RefillCost;
}

float UFlareSimulatedSpacecraftDamageSystem::ApplyDamage(FFlareSpacecraftComponentDescription* ComponentDescription,
						  FFlareSpacecraftComponentSave* ComponentData,
						  float Energy, EFlareDamage::Type DamageType, UFlareCompany* DamageSource)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSimulatedDamageSystem_ApplyDamage);

	float InflictedDamageRatio = 0;

	// Apply damage
	float StateBeforeDamage = GetDamageRatio(ComponentDescription, ComponentData);

	if (StateBeforeDamage == 0)
	{
		return 0;
	}

	float EffectiveEnergy;

	if (DamageType == EFlareDamage::DAM_HEAT)
	{
		EffectiveEnergy = Energy;
	}
	else
	{
		EffectiveEnergy = Energy * (1.f - GetArmor(ComponentDescription));
	}

	if (ComponentDescription->GeneralCharacteristics.LifeSupport && (DamageType == EFlareDamage::DAM_Collision || Spacecraft->IsPlayerShip()))
	{
		// Limit to 50% in case of collision or last player ship
		float MaxDamage = ComponentDescription->HitPoints*0.5;
		float MaxMissingDamage =  FMath::Max(0.f, MaxDamage - ComponentData->Damage);
		EffectiveEnergy = FMath::Min(EffectiveEnergy, MaxMissingDamage);
	}


	ComponentData->Damage += EffectiveEnergy;
	float MaxHitPoints = GetMaxHitPoints(ComponentDescription);
	if (ComponentData->Damage > MaxHitPoints)
	{
		ComponentData->Damage = MaxHitPoints;
	}
	float StateAfterDamage = GetDamageRatio(ComponentDescription, ComponentData);
	InflictedDamageRatio = StateBeforeDamage - StateAfterDamage;
	SetDamageDirty(ComponentDescription);

	if (EffectiveEnergy > 0)
	{
		CombatLog::SpacecraftComponentDamaged(Spacecraft, ComponentData, ComponentDescription, Energy, EffectiveEnergy, DamageType, StateBeforeDamage, StateAfterDamage);

		// This ship has been damaged and someone is to blame
		if (DamageSource != NULL && DamageSource != Spacecraft->GetCompany())
		{
			float ReputationCost = 0.f;

			if (Spacecraft->IsStation())
			{
				if(DamageType != EFlareDamage::DAM_Collision)
				{
					ReputationCost = -InflictedDamageRatio * 100;
				}
			}
			else
			{
				ReputationCost = -InflictedDamageRatio * 2;
			}



			UFlareCompany* PlayerCompany = Spacecraft->GetGame()->GetPC()->GetCompany();

			if (ReputationCost != 0 && DamageSource == PlayerCompany && Spacecraft->GetCompany() != PlayerCompany)
			{
				// Being shot by enemies is pretty much expected
				if (Spacecraft->GetCompany()->GetWarState(DamageSource) != EFlareHostility::Hostile)
				{
					// If it's a betrayal, lower attacker's reputation on everyone, give rep to victim

					// Lower attacker's reputation on victim
					Spacecraft->GetCompany()->GivePlayerReputationToOthers(ReputationCost/2);
					Spacecraft->GetCompany()->GivePlayerReputation(ReputationCost);

					Spacecraft->GetGame()->GetPC()->Notify(LOCTEXT("NeutralAttack", "Neutrality violation"),
						   FText::Format(LOCTEXT("NeutralAttackDescription", "Attacking neutral properties ({0}) will have diplomatic consequences."), UFlareGameTools::DisplaySpacecraftName(Spacecraft)),
						   FName("neutrality-violation"),
						   EFlareNotification::NT_Military);


				}
				else if(Spacecraft->IsActive() && Spacecraft->GetActive()->GetTimeSinceUncontrollable() > 5.f)
				{
					// If an attack on a prisoner, lower attacker's reputation on everyone, give rep to victim

					// Lower attacker's reputation on victim
					Spacecraft->GetCompany()->GivePlayerReputationToOthers(ReputationCost/5);
					Spacecraft->GetCompany()->GivePlayerReputation(ReputationCost/5);

					Spacecraft->GetGame()->GetPC()->Notify(LOCTEXT("PrisonerAttack", "Attacking prisoners"),
						   FText::Format(LOCTEXT("PrisonerAttackDescription", "Attacking uncontrollable ships ({0}) will have diplomatic consequences."), UFlareGameTools::DisplaySpacecraftName(Spacecraft)),
						   FName("prisoner-attack"),
						   EFlareNotification::NT_Military);
				}



			}
		}

		Spacecraft->GetCompany()->InvalidateCompanyValueCache();
	}

	LastDamageCause = DamageCause(DamageSource, DamageType);

	return InflictedDamageRatio;
}

float UFlareSimulatedSpacecraftDamageSystem::GetTemperature() const
{
	return Data->Heat / Description->HeatCapacity;
}

float UFlareSimulatedSpacecraftDamageSystem::GetMaxHitPoints(FFlareSpacecraftComponentDescription* ComponentDescription) const
{
	return Spacecraft->GetLevel() * ComponentDescription->HitPoints;
}


float UFlareSimulatedSpacecraftDamageSystem::GetDamageRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
															FFlareSpacecraftComponentSave* ComponentData) const
{
	if (ComponentDescription)
	{
		float MaxHitPoints = GetMaxHitPoints(ComponentDescription);

		float RemainingHitPoints = MaxHitPoints - ComponentData->Damage;
		return FMath::Clamp(RemainingHitPoints / MaxHitPoints, 0.f, 1.f);
	}
	else
	{
		return 1.f;
	}
}

float UFlareSimulatedSpacecraftDamageSystem::GetUsableRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
															FFlareSpacecraftComponentSave* ComponentData) const
{
	float FullUsageRatio = (GetDamageRatio(ComponentDescription, ComponentData) * (IsPowered(ComponentData) ? 1 : 0));

	if (FullUsageRatio < BROKEN_RATIO)
	{
		return 0.0f;
	}

	return FullUsageRatio;
}


/*----------------------------------------------------
	Internals
----------------------------------------------------*/

float UFlareSimulatedSpacecraftDamageSystem::GetClampedUsableRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
	FFlareSpacecraftComponentSave* ComponentData) const
{
	float CorrectionFactor = 1 / (1 - BROKEN_RATIO);
	return FMath::Max(0.f, (GetUsableRatio(ComponentDescription, ComponentData) - BROKEN_RATIO) * CorrectionFactor);
}

void UFlareSimulatedSpacecraftDamageSystem::UpdateSubsystemsHealth()
{
	if(DamageDirty)
	{
		for (int32 Index = EFlareSubsystem::SYS_None + 1; Index < EFlareSubsystem::SYS_WeaponAndAmmo; Index++)
		{
			SubsystemHealth[static_cast<EFlareSubsystem::Type>(Index)] = GetSubsystemHealthInternal(static_cast<EFlareSubsystem::Type>(Index));
		}
	}

	if(AmmoDirty || DamageDirty)
	{
		SubsystemHealth[EFlareSubsystem::SYS_WeaponAndAmmo] = GetSubsystemHealthInternal(EFlareSubsystem::SYS_WeaponAndAmmo);
	}

	DamageDirty = false;
	AmmoDirty = false;
}

float UFlareSimulatedSpacecraftDamageSystem::GetSubsystemHealthInternal(EFlareSubsystem::Type Type) const
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSimulatedDamageSystem_UpdateSubsystemHealth);

	UFlareSpacecraftComponentsCatalog* Catalog = Spacecraft->GetGame()->GetShipPartsCatalog();
	float Health = 0.f;

	switch (Type)
	{
		case EFlareSubsystem::SYS_Propulsion:
		{
			float Total = 0.f;
			float EngineCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
				{


					EngineCount += 1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total / EngineCount;
		}
		break;
		case EFlareSubsystem::SYS_RCS:
		{
			float Total = 0.f;
			float EngineCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if (ComponentDescription->Type == EFlarePartType::RCS)
				{
					EngineCount += 1.f;
					Total += (GetUsableRatio(ComponentDescription, ComponentData) == 0 ? 0 : 1);
				}
			}
			float CorrectionFactor = 1 / (1 - UNCONTROLLABLE_RATIO);
			Health = FMath::Max(0.f, (Total / EngineCount - UNCONTROLLABLE_RATIO) * CorrectionFactor);
		}
		break;
		case EFlareSubsystem::SYS_LifeSupport:
		{

			// No cockpit mean no destructible
			Health = 1.0f;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);
				if (ComponentDescription && ComponentDescription->GeneralCharacteristics.LifeSupport)
				{
					Health = GetDamageRatio(ComponentDescription, ComponentData);
					break;
				}
			}
		}
		break;
		case EFlareSubsystem::SYS_Power:
		{
			float Total = 0.f;
			float GeneratorCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if (ComponentDescription->GeneralCharacteristics.ElectricSystem)
				{
					GeneratorCount += 1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total / GeneratorCount;
		}
		break;
		case EFlareSubsystem::SYS_Weapon:
		case EFlareSubsystem::SYS_WeaponAndAmmo:
		{
			float Total = 0.f;
			float WeaponCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if (ComponentDescription->Type == EFlarePartType::Weapon)
				{
					WeaponCount += 1.f;
					int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
					int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

					bool WithAmmo = (Type == EFlareSubsystem::SYS_WeaponAndAmmo);
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData)	* ((CurrentAmmo > 0 || !WithAmmo) ? 1 : 0);
				}
			}
			if (WeaponCount == 0)
			{
				Health = 0;
			}
			else
			{
				Health = Total / WeaponCount;
			}
		}
		break;
		case EFlareSubsystem::SYS_Temperature:
		{
			float Total = 0.f;
			float HeatSinkCount = 0;

			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if (ComponentDescription->GeneralCharacteristics.HeatSink)
				{
					HeatSinkCount += 1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total / HeatSinkCount;
		}
		break;
	}

	return Health;
}

void UFlareSimulatedSpacecraftDamageSystem::SetPowerDirty()
{
	IsPoweredCacheIndex++;
}

void UFlareSimulatedSpacecraftDamageSystem::SetDamageDirty(FFlareSpacecraftComponentDescription* ComponentDescription)
{
	DamageDirty = true;
	if(ComponentDescription->GeneralCharacteristics.ElectricSystem)
	{
		SetPowerDirty();
	}
}

void UFlareSimulatedSpacecraftDamageSystem::SetAmmoDirty()
{
	AmmoDirty = true;
}

bool UFlareSimulatedSpacecraftDamageSystem::IsPowered(FFlareSpacecraftComponentSave* ComponentToPowerData) const
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSimulatedDamageSystem_IsPowered);

	if(ComponentToPowerData->IsPoweredCacheIndex < IsPoweredCacheIndex)
	{
		UFlareSimulatedSpacecraftDamageSystem* UnprotectedThis = const_cast<UFlareSimulatedSpacecraftDamageSystem *>(this);
		UnprotectedThis->UpdatePower(ComponentToPowerData);
	}

	return ComponentToPowerData->IsPoweredCache;
}

void UFlareSimulatedSpacecraftDamageSystem::UpdatePower(FFlareSpacecraftComponentSave* ComponentToPowerData)
{
	bool AvailablePower = false;

	if (Spacecraft->IsStation())
	{
		AvailablePower = true;
	}
	else
	{
		UFlareSpacecraftComponentsCatalog* Catalog = Spacecraft->GetGame()->GetShipPartsCatalog();
		bool HasPowerSource = false;

		for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			FFlareSpacecraftSlotDescription* SlotDescription = NULL;

			// Find InternalComponentSlot
			for (int32 SlotIndex = 0; SlotIndex < Spacecraft->GetDescription()->InternalComponentSlots.Num(); SlotIndex ++)
			{
				if (Spacecraft->GetDescription()->InternalComponentSlots[SlotIndex].SlotIdentifier == ComponentData->ShipSlotIdentifier)
				{
					SlotDescription = &Spacecraft->GetDescription()->InternalComponentSlots[SlotIndex];
					break;
				}
			}

			if (!SlotDescription)
			{
				continue;
			}

			if(ComponentToPowerData == ComponentData  &&
					(ComponentDescription->GeneralCharacteristics.ElectricSystem || ComponentDescription->GeneralCharacteristics.LifeSupport))
			{
				HasPowerSource = true;
				AvailablePower = true;
				break;
			}

			if (ComponentDescription->GeneralCharacteristics.ElectricSystem &&
					SlotDescription->PoweredComponents.Contains(ComponentToPowerData->ShipSlotIdentifier) )
			{
				HasPowerSource = true;
				if (GetUsableRatio(ComponentDescription, ComponentData) > 0) {
					AvailablePower = true;
					break;
				}
			}
		}

		if (!HasPowerSource)
		{
			FLOGV("Warning: %s : %s has no power source", *Spacecraft->GetImmatriculation().ToString(),
				  *ComponentToPowerData->ShipSlotIdentifier.ToString());
		}
	}

	ComponentToPowerData->IsPoweredCacheIndex = IsPoweredCacheIndex;
	ComponentToPowerData->IsPoweredCache = AvailablePower;
}

FText UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName(EFlareSubsystem::Type SubsystemType)
{
	FText Text;

	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   Text = LOCTEXT("SYS_Temperature", "Cooling");      break;
		case EFlareSubsystem::SYS_Propulsion:    Text = LOCTEXT("SYS_Propulsion", "Engines");       break;
		case EFlareSubsystem::SYS_RCS:           Text = LOCTEXT("SYS_RCS", "RCS");                  break;
		case EFlareSubsystem::SYS_LifeSupport:   Text = LOCTEXT("SYS_LifeSupport", "Crew");         break;
		case EFlareSubsystem::SYS_Power:         Text = LOCTEXT("SYS_Power", "Power");              break;
		case EFlareSubsystem::SYS_Weapon:        Text = LOCTEXT("SYS_Weapon", "Weapons");           break;
	}

	return Text;
}

float UFlareSimulatedSpacecraftDamageSystem::GetArmor(FFlareSpacecraftComponentDescription* ComponentDescription)
{
	if (ComponentDescription)
	{
		return ComponentDescription->Armor / 100.f;
	}

	return 1;
}

void UFlareSimulatedSpacecraftDamageSystem::NotifyDamage()
{
	// Update uncontrollable status
	if (WasControllable && IsUncontrollable())
	{

		WasControllable = false;

		Spacecraft->GetGame()->GetQuestManager()->OnSpacecraftDestroyed(Spacecraft, true,
																			LastDamageCause);

	}


	// Update alive status
	if (WasAlive && !IsAlive())
	{

		WasAlive = false;

		Spacecraft->GetGame()->GetQuestManager()->OnSpacecraftDestroyed(Spacecraft, true,
																			LastDamageCause);
	}

}

#undef LOCTEXT_NAMESPACE
