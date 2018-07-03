
#include "FlareScannableCatalog.h"
#include "../Flare.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareScannableCatalog::UFlareScannableCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareScannableCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareScannableCatalog::UFlareScannableCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareScannableCatalogEntry* Scannable = Cast<UFlareScannableCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Scannable);
		ScannableCatalog.Add(Scannable);
	}
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareScannableDescription* UFlareScannableCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareScannableCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareScannableCatalogEntry* const* Entry = ScannableCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

