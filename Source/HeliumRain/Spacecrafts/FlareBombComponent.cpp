
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
