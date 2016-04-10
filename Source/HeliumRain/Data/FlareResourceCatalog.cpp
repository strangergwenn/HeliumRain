#include "../Flare.h"
#include "FlareResourceCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareResourceCatalog::UFlareResourceCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareResourceDescription* UFlareResourceCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareResourceCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareResourceCatalogEntry* const* Entry = Resources.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

bool UFlareResourceCatalog::IsCustomerResource(FFlareResourceDescription* Resource) const
{
	for (int32 ResourceIndex = 0; ResourceIndex < ConsumerResources.Num(); ResourceIndex++)
	{
		if(Resource == &ConsumerResources[ResourceIndex]->Data)
		{
			return true;
		}
	}
	return false;
}

bool UFlareResourceCatalog::IsMaintenanceResource(FFlareResourceDescription* Resource) const
{
	for (int32 ResourceIndex = 0; ResourceIndex < MaintenanceResources.Num(); ResourceIndex++)
	{
		if(Resource == &MaintenanceResources[ResourceIndex]->Data)
		{
			return true;
		}
	}
	return false;
}
