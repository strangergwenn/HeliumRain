;
#include "../Flare.h"
#include "FlareSpacecraftInterface.h"
#include "FlareSpacecraftComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftInterface::UFlareSpacecraftInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

const FSlateBrush* IFlareSpacecraftInterface::GetIcon(FFlareShipDescription* Characteristic)
{
	if (Characteristic)
	{
		if (Characteristic->OrbitalEngineCount == 0) // Station
		{
			return FFlareStyleSet::GetIcon("SS");
		}
		else if ((Characteristic->GunSlots.Num() +  Characteristic->TurretSlots.Num()) > 0) // Military
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("MS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("MM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("ML");
			}
		}
		else
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("CS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("CM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("CL");
			}
		}
	}
	return NULL;
}
