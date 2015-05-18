;
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "FlareAirframe.h"
#include "FlareInternalComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareAirframe::UFlareAirframe(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	HasFlickeringLights = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareAirframe::OnRegister()
{
	Super::OnRegister();
}

float UFlareAirframe::GetRemainingArmorAtLocation(FVector Location)
{
	if (!ComponentDescription)
	{
		UFlareInternalComponent* Component = Ship->GetInternalComponentAtLocation(Location);
		if (Component)
		{
			return Component->GetRemainingArmorAtLocation(Location);
		}
	}
	return Super::GetRemainingArmorAtLocation(Location);
}

float UFlareAirframe::GetAvailablePower() const
{
	UFlareSpacecraftComponent* Cockpit = Ship->GetCockpit();

	if (Cockpit)
	{
		Cockpit->UpdatePower();
		return Cockpit->GetAvailablePower();
	}
	else
	{
		return Super::GetAvailablePower();
	}
}