;
#include "../Flare.h"
#include "FlareSpacecraftSubComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftSubComponent::UFlareSpacecraftSubComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareSpacecraftSubComponent::SetParentSpacecraftComponent(UFlareSpacecraftComponent* Component)
{
	ParentComponent = Component;
}



float UFlareSpacecraftSubComponent::GetRemainingArmorAtLocation(FVector Location)
{
	return ParentComponent->GetRemainingArmorAtLocation(Location);
}

void UFlareSpacecraftSubComponent::ApplyDamage(float Energy)
{
	ParentComponent->ApplyDamage(Energy);
}
