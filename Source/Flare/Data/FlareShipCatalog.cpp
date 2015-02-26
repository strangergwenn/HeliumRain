
#include "../Flare.h"
#include "FlareShipCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipCatalog::UFlareShipCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareShipDescription* UFlareShipCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareShipCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareShipCatalogEntry* const* Entry = ShipCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}
	else
	{
		return NULL;
	}
}

