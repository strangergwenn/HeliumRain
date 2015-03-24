;
#include "../Flare.h"
#include "FlareAirframe.h"
#include "FlareInternalComponent.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareAirframe::UFlareAirframe(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFlareAirframe::OnRegister()
{
	Super::OnRegister();
	SetCollisionProfileName("BlockAllDynamic");
}

float UFlareAirframe::GetRemainingArmorAtLocation(FVector Location)
{
	if(!ComponentDescription)
	{
		UFlareInternalComponent* Component = Ship->GetInternalComponentAtLocation(Location);
		if(Component)
		{
			return Component->GetRemainingArmorAtLocation(Location);
		}
	}
	return Super::GetRemainingArmorAtLocation(Location);
}

float UFlareAirframe::GetAvailablePower() const
{
	UFlareShipComponent* Cockpit =  Ship->GetCockpit();

	if(Cockpit) {
		Cockpit->UpdatePower();
		return Cockpit->GetAvailablePower();
	}
	else
	{
		return Super::GetAvailablePower();
	}
}