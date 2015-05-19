
#include "../Flare.h"

#include "FlareSpacecraftNavigationSystem.h"
#include "FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftNavigationSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftNavigationSystem::UFlareSpacecraftNavigationSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftNavigationSystem::TickSystem(float DeltaSeconds)
{
}

void UFlareSpacecraftNavigationSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

}

void UFlareSpacecraftNavigationSystem::Start()
{

}

#undef LOCTEXT_NAMESPACE
