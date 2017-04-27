
#include "FlareInternalComponent.h"
#include "../Flare.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareInternalComponent::UFlareInternalComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = false;
	HasFlickeringLights = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareInternalComponent::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);
	if (!ComponentDescription)
	{
		FLOGV("!!! Internal component %s is not correctly mapped :", *(this->GetReadableName()));
	}
}

FFlareSpacecraftComponentSave* UFlareInternalComponent::Save()
{
	return Super::Save();
}

void UFlareInternalComponent::StartDestroyedEffects()
{
	// Do nothing
}

void UFlareInternalComponent::GetBoundingSphere(FVector& Location, float& SphereRadius)
{
	SphereRadius = Radius * 100; // In cm
	Location = GetComponentLocation();
}
