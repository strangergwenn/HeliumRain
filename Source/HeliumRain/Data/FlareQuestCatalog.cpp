
#include "../Flare.h"
#include "FlareQuestCatalog.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestCatalog::UFlareQuestCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareQuestCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareQuestCatalog::UFlareQuestCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareQuestCatalogEntry* Quest = Cast<UFlareQuestCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Quest);
		Quests.Add(Quest);
	}
}
