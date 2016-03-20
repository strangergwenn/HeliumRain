
#include "../../Flare.h"

#include "../FlareSpacecraftInterface.h"
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
	// No power outage in simulation
	return false;
}

float UFlareSimulatedSpacecraftDamageSystem::GetPowerOutageDuration() const
{
	// No power outage in simulation
	return 0;
}

float UFlareSimulatedSpacecraftDamageSystem::GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor, bool WithAmmo) const
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
					Total += GetDamageRatio(ComponentDescription, ComponentData, WithArmor) * (IsPowered(ComponentData) ? 1 : 0);
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
					Total += GetDamageRatio(ComponentDescription, ComponentData, WithArmor) * (IsPowered(ComponentData) ? 1 : 0);
				}
			}
			Health = Total/EngineCount;
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
				if (ComponentDescription->GeneralCharacteristics.LifeSupport)
				{
					Health = GetDamageRatio(ComponentDescription, ComponentData, WithArmor);
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
					Total += GetDamageRatio(ComponentDescription, ComponentData, WithArmor);
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


					Total += GetDamageRatio(ComponentDescription, ComponentData, WithArmor)
							* ( IsPowered(ComponentData) ? 1 : 0)
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
					Total += GetDamageRatio(ComponentDescription, ComponentData, WithArmor);
				}
			}
			Health = Total/HeatSinkCount;
		}
		break;
	}

	return Health;

	return 1.0f;
}

float UFlareSimulatedSpacecraftDamageSystem::GetWeaponGroupHealth(int32 GroupIndex, bool WithArmor, bool WithAmmo) const
{
	return 1.0f;
}

float UFlareSimulatedSpacecraftDamageSystem::GetTemperature() const
{
	// TODO replace mock
	return 400;

}

float UFlareSimulatedSpacecraftDamageSystem::GetDamageRatio(FFlareSpacecraftComponentDescription* ComponentDescription,
															FFlareSpacecraftComponentSave* ComponentData,
															bool WithArmor) const
{
	if (ComponentDescription)
	{
		float RemainingHitPoints = ComponentDescription->ArmorHitPoints + ComponentDescription->HitPoints - ComponentData->Damage;
		return FMath::Clamp(RemainingHitPoints / (ComponentDescription->HitPoints + (WithArmor ? ComponentDescription->ArmorHitPoints : 0.f)), 0.f, 1.f);
	}
	else
	{
		return 1.f;
	}
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

		if (ComponentDescription->GeneralCharacteristics.ElectricSystem &&
				SlotDescription->PoweredComponents.Contains(ComponentToPowerData->ShipSlotIdentifier))
		{
			if (GetDamageRatio(ComponentDescription, ComponentData, false) > 0) {
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




#undef LOCTEXT_NAMESPACE
