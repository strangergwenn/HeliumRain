#include "../Flare.h"
#include "FlareResourceCatalog.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareResourceCatalog::UFlareResourceCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	Registry.GetAssetsByClass(UFlareResourceCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareResourceCatalogEntry* Resource = Cast<UFlareResourceCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Resource);
		
		Resources.Add(Resource);

		if (IsCustomerResource(&Resource->Data))
		{
			ConsumerResources.Add(Resource);
		}

		if (IsMaintenanceResource(&Resource->Data))
		{
			MaintenanceResources.Add(Resource);
		}
	}
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

UFlareResourceCatalogEntry* UFlareResourceCatalog::GetEntry(FFlareResourceDescription* Resource) const
{
	for (int32 ResourceIndex = 0; ResourceIndex < Resources.Num(); ResourceIndex++)
	{
		if(Resource == &Resources[ResourceIndex]->Data)
		{
			return Resources[ResourceIndex];
		}
	}
	return NULL;
}
