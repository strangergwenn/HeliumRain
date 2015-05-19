
#include "../Flare.h"

#include "FlareSpacecraftDockingSystem.h"
#include "FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftDockingSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftDockingSystem::UFlareSpacecraftDockingSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftDockingSystem::TickSystem(float DeltaSeconds)
{
}

void UFlareSpacecraftDockingSystem::Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	Description = Spacecraft->GetDescription();
	Data = OwnerData;

}

void UFlareSpacecraftDockingSystem::Start()
{

}


#undef LOCTEXT_NAMESPACE
