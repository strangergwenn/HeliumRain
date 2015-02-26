
#include "../Flare.h"
#include "FlareStationCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareStationCatalog::UFlareStationCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareStationDescription* UFlareStationCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareStationCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareStationCatalogEntry* const* Entry = StationCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}
	else
	{
		return NULL;
	}
}

