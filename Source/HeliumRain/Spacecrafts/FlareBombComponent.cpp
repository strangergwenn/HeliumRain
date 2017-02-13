
#include "../Flare.h"
#include "FlareBombComponent.h"

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
		//FLOGV("UFlareBombComponent::GetMesh OK %s", *this->GetReadableName());
		return ComponentDescription->Mesh;
	}
	//FLOG("UFlareBombComponent::GetMesh KO");

	return Super::GetMesh(PresentationMode);
}


float UFlareBombComponent::GetDamageRatio() const
{
	return 1.0;
}
