;
#include "../Flare.h"
#include "FlareStationInterface.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareStationInterface::UFlareStationInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

const FSlateBrush* IFlareStationInterface::GetIcon(FFlareStationDescription* Characteristic)
{
	return FFlareStyleSet::GetIcon("SS");
}
