
#include "../Flare.h"
#include "FlareSpacecraftCatalog.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftCatalog::UFlareSpacecraftCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	Registry.GetAssetsByClass(UFlareSpacecraftCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		FLOGV("UFlareSpacecraftCatalog::UFlareSpacecraftCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareSpacecraftCatalogEntry* Spacecraft = Cast<UFlareSpacecraftCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Spacecraft);

		if (Spacecraft->Data.IsStation())
		{
			StationCatalog.Add(Spacecraft);
		}
		else
		{
			ShipCatalog.Add(Spacecraft);
		}
	}
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

