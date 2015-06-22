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


void UFlareSpacecraftSubComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHealth(ParentComponent->GetDamageRatio());
	SetTemperature(ParentComponent->GetSpacecraft()->IsPresentationMode() ? 290 : ParentComponent->GetLocalTemperature());
}

float UFlareSpacecraftSubComponent::GetRemainingArmorAtLocation(FVector Location)
{
	return ParentComponent->GetRemainingArmorAtLocation(Location);
}

void UFlareSpacecraftSubComponent::ApplyDamage(float Energy)
{
	ParentComponent->ApplyDamage(Energy);
}
