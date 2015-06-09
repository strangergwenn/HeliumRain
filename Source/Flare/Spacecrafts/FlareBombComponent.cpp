
#include "../Flare.h"
#include "FlareInternalComponent.h"


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
	if(ComponentDescription)
	{
		FLOGV("UFlareBombComponent::GetMesh OK %s", *this->GetReadableName());
		return ComponentDescription->WeaponCharacteristics.BombCharacteristics.BombMesh;
	}
	FLOG("UFlareBombComponent::GetMesh KO");

	return Super::GetMesh(PresentationMode);
}


float UFlareBombComponent::GetDamageRatio(bool WithArmor) const
{
	return 1.0;
}
