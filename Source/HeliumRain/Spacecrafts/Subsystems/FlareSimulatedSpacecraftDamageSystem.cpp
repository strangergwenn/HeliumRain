
#include "../../Flare.h"

#include "../FlareSimulatedSpacecraft.h"
#include "../FlareSpacecraftComponent.h"
#include "../../Game/FlareGame.h"
#include "FlareSimulatedSpacecraftDamageSystem.h"

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
}


/*----------------------------------------------------
	System API
----------------------------------------------------*/

bool UFlareSimulatedSpacecraftDamageSystem::IsAlive() const
{
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
	return (GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion, false) < 0.3f || IsUncontrollable());
}

bool UFlareSimulatedSpacecraftDamageSystem::IsUncontrollable() const
{
	if(Spacecraft->IsStation())
	{
		return false;
	}

	if (!IsAlive()) {
		return true;
	}

	return (GetSubsystemHealth(EFlareSubsystem::SYS_RCS, false) == 0.0f);
}

bool UFlareSimulatedSpacecraftDamageSystem::IsDisarmed() const
{
	if(Spacecraft->IsStation())
	{
		return true;
	}
	if(Spacecraft->GetSize() == EFlarePartSize::S && IsUncontrollable())
	{
		return true;
	}

	if (!IsAlive()) {
		return true;
	}

	return (GetSubsystemHealth(EFlareSubsystem::SYS_Weapon, true) == 0.0f);
}

float UFlareSimulatedSpacecraftDamageSystem::GetGlobalHealth()
{
	float GlobalHealth = (GetSubsystemHealth(EFlareSubsystem::SYS_Temperature, false)
		+ GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion, false)
		+ GetSubsystemHealth(EFlareSubsystem::SYS_RCS, false)
		+ GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport, false)
		+ GetSubsystemHealth(EFlareSubsystem::SYS_Power, false));

	if (Spacecraft->IsMilitary())
	{
		GlobalHealth += GetSubsystemHealth(EFlareSubsystem::SYS_Weapon, false);
		return GlobalHealth / 6.0f;
	}
	else
	{
		return GlobalHealth / 5.0f;
	}
}

float UFlareSimulatedSpacecraftDamageSystem::GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithAmmo) const
{
	UFlareSpacecraftComponentsCatalog* Catalog = Spacecraft->GetGame()->GetShipPartsCatalog();

	// TODO cache
	
	float Health = 0.f;

	switch(Type)
	{
		case EFlareSubsystem::SYS_Propulsion:
		{
			float Total = 0.f;
			float EngineCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if(ComponentDescription->Type == EFlarePartType::OrbitalEngine)
				{


					EngineCount+=1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total/EngineCount;
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

				if(ComponentDescription->Type == EFlarePartType::RCS)
				{
					EngineCount+=1.f;
					Total += (GetUsableRatio(ComponentDescription, ComponentData) == 0 ? 0 : 1);
				}
			}
			float CorrectionFactor = 1/(1-UNCONTROLLABLE_RATIO);
			Health = FMath::Max(0.f, (Total/EngineCount - UNCONTROLLABLE_RATIO) * CorrectionFactor);
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

				if(ComponentDescription->GeneralCharacteristics.ElectricSystem)
				{
					GeneratorCount+=1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total/GeneratorCount;
		}
		break;
		case EFlareSubsystem::SYS_Weapon:
		{
			float Total = 0.f;
			float WeaponCount = 0;
			for (int32 ComponentIndex = 0; ComponentIndex < Data->Components.Num(); ComponentIndex++)
			{
				FFlareSpacecraftComponentSave* ComponentData = &Data->Components[ComponentIndex];

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

				if(ComponentDescription->Type == EFlarePartType::Weapon)
				{
					WeaponCount+=1.f;
					int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
					int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

					Total += GetClampedUsableRatio(ComponentDescription, ComponentData)
							*((CurrentAmmo > 0 || !WithAmmo) ? 1 : 0);
				}
			}
			if(WeaponCount == 0)
			{
				Health = 0;
			}
			else
			{
				Health = Total/WeaponCount;
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

				if(ComponentDescription->GeneralCharacteristics.HeatSink)
				{
					HeatSinkCount+=1.f;
					Total += GetClampedUsableRatio(ComponentDescription, ComponentData);
				}
			}
			Health = Total/HeatSinkCount;
		}
		break;
	}

	return Health;

	return 1.0f;
}



float UFlareSimulatedSpacecraftDamageSystem::GetTemperature() const
{
	return Data->Heat / Description->HeatCapacity;
}

float UFlareSimulatedSpacecraftDamageSystem::GetDamageRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
															FFlareSpacecraftComponentSave* ComponentData)
{
	if (ComponentDescription)
	{
		float RemainingHitPoints = ComponentDescription->HitPoints - ComponentData->Damage;
		return FMath::Clamp(RemainingHitPoints / ComponentDescription->HitPoints, 0.f, 1.f);
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

	if(FullUsageRatio < BROKEN_RATIO)
	{
		return 0.0f;
	}

	return FullUsageRatio;
}


float UFlareSimulatedSpacecraftDamageSystem::GetClampedUsableRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
															FFlareSpacecraftComponentSave* ComponentData) const
{
	float CorrectionFactor = 1/(1-BROKEN_RATIO);
	return FMath::Max(0.f, (GetUsableRatio(ComponentDescription, ComponentData) - BROKEN_RATIO) * CorrectionFactor);
}
bool UFlareSimulatedSpacecraftDamageSystem::IsPowered(FFlareSpacecraftComponentSave* ComponentToPowerData) const
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

		if(ComponentToPowerData == ComponentData  && ComponentDescription->GeneralCharacteristics.ElectricSystem)
		{
			return true;
		}

		if (ComponentDescription->GeneralCharacteristics.ElectricSystem &&
				SlotDescription->PoweredComponents.Contains(ComponentToPowerData->ShipSlotIdentifier) )
		{
			if (GetUsableRatio(ComponentDescription, ComponentData) > 0) {
				return true;
			}
			HasPowerSource = true;
		}
	}

	if (!HasPowerSource)
	{
		FLOGV("Warning: %s : %s has no power source", *Spacecraft->GetImmatriculation().ToString(),
			  *ComponentToPowerData->ShipSlotIdentifier.ToString());
	}

	return false;
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


#undef LOCTEXT_NAMESPACE
