
#include "../Flare.h"

#include "FlareSpacecraftWeaponsSystem.h"
#include "FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftWeaponsSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftWeaponsSystem::UFlareSpacecraftWeaponsSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
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

void UFlareSpacecraftWeaponsSystem::Start()
{

}

#undef LOCTEXT_NAMESPACE
