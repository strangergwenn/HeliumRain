
#include "HeliumRain/Data/FlareSpacecraftCatalog.h"
#include "../Flare.h"
#include <AssetRegistryModule.h>


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

struct FSortByEntrySize
{
	FORCEINLINE bool operator()(const UFlareSpacecraftCatalogEntry& EntryA, const UFlareSpacecraftCatalogEntry& EntryB) const
	{
		const FFlareSpacecraftDescription* A = &EntryA.Data;
		const FFlareSpacecraftDescription* B = &EntryB.Data;

		if (A->IsStation() && B->IsStation())
		{
			if (A->IsSubstation && !B->IsSubstation)
			{
				return true;
			}
			else if (!A->IsSubstation && B->IsSubstation)
			{
				return false;
			}
		}
		else if (A->IsStation() && !B->IsStation())
		{
			return true;
		}
		else if (!A->IsStation() && B->IsStation())
		{
			return false;
		}
		else if (A->IsMilitary() && B->IsMilitary())
		{
			return A->CombatPoints > B->CombatPoints;
		}
		else if (A->IsMilitary() && !B->IsMilitary())
		{
			return true;
		}
		else if (!A->IsMilitary() && B->IsMilitary())
		{
			return false;
		}
		else if (A->Mass > B->Mass)
		{
			return true;
		}
		else if (A->Mass < B->Mass)
		{
			return false;
		}

		return false;
	}
};

UFlareSpacecraftCatalog::UFlareSpacecraftCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareSpacecraftCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareSpacecraftCatalog::UFlareSpacecraftCatalog : Found '%s'", *AssetList[Index].GetFullName());
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

	StationCatalog.Sort(FSortByEntrySize());
	ShipCatalog.Sort(FSortByEntrySize());
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

