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
	if (!ComponentDescription && Spacecraft)
	{
		UFlareInternalComponent* Component = Spacecraft->GetInternalComponentAtLocation(Location);
		if (Component)
		{
			return Component->GetRemainingArmorAtLocation(Location);
		}
	}
	return Super::GetRemainingArmorAtLocation(Location);
}

float UFlareAirframe::GetAvailablePower() const
{
	if(Spacecraft)
	{
		UFlareSpacecraftComponent* Cockpit = Spacecraft->GetCockpit();

		if (Cockpit)
		{
			Cockpit->UpdatePower();
			return Cockpit->GetAvailablePower();
		}
	}

	return Super::GetAvailablePower();
}
