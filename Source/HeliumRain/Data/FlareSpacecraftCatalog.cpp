
#include "../Flare.h"
#include "FlareSpacecraftCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftCatalog::UFlareSpacecraftCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareSpacecraftDescription* UFlareSpacecraftCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareSpacecraftCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareSpacecraftCatalogEntry* const* Entry = ShipCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	Entry = StationCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

