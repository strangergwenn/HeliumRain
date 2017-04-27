
#include "FlareBombComponent.h"
#include "../Flare.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareBombComponent::UFlareBombComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = false;
	HasFlickeringLights = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareBombComponent::UpdateEffects(float Alpha)
{
	// Apply the glow value
	if (EffectMaterial && IsComponentVisible())
	{
		EffectMaterial->SetScalarParameterValue(TEXT("Opacity"), Alpha);
	}
}


UStaticMesh* UFlareBombComponent::GetMesh(bool PresentationMode) const
{
	if (ComponentDescription)
	{
		return ComponentDescription->Mesh;
	}

	return Super::GetMesh(PresentationMode);
}


float UFlareBombComponent::GetDamageRatio() const
{
	return 1.0;
}
