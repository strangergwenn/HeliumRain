
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

#undef LOCTEXT_NAMESPACE
