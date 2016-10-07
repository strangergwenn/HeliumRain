
#include "../Flare.h"
#include "FlareSpacecraftSubComponent.h"
#include "FlareSpacecraft.h"


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

float UFlareSpacecraftSubComponent::GetArmorAtLocation(FVector Location)
{
	return ParentComponent->GetArmorAtLocation(Location);
}

float UFlareSpacecraftSubComponent::ApplyDamage(float Energy, EFlareDamage::Type DamageType)
{
	return ParentComponent->ApplyDamage(Energy, DamageType);
}
