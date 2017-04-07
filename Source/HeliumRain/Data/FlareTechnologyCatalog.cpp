
#include "../Flare.h"
#include "FlareTechnologyCatalog.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTechnologyCatalog::UFlareTechnologyCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	Registry.GetAssetsByClass(UFlareTechnologyCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareTechnologyCatalog::UFlareTechnologyCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareTechnologyCatalogEntry* Technology = Cast<UFlareTechnologyCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Technology);
		TechnologyCatalog.Add(Technology);
	}
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareTechnologyDescription* UFlareTechnologyCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareTechnologyCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareTechnologyCatalogEntry* const* Entry = TechnologyCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

