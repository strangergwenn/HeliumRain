
#include "../Flare.h"
#include "FlareShipPartsCatalog.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPartsCatalog::UFlareShipPartsCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareShipModuleDescription* UFlareShipPartsCatalog::Get(FName Identifier) const
{
	FFlareShipModuleDescription* Part = NULL;

	auto FindByName = [=](const UFlareShipPartsCatalogEntry* Candidate)
	{
		return Candidate && Candidate->Data.Identifier == Identifier;
	};

	UFlareShipPartsCatalogEntry* const* Temp = EngineCatalog.FindByPredicate(FindByName);
	if (Temp)
	{
		Part = &(*Temp)->Data;
	}

	if (!Part)
	{
		Temp = RCSCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	if (!Part)
	{
		Temp = WeaponCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	if (!Part)
	{
		Temp = InternalModulesCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}
	
	if (!Part)
	{
		Temp = MetaCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	return Part;
}

const void UFlareShipPartsCatalog::GetEngineList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size)
{
	for (int32 i = 0; i < EngineCatalog.Num(); i++)
	{
		if (EngineCatalog[i]->Data.Size == Size)
		{
			OutData.Add(&(EngineCatalog[i])->Data);
		}
	}
}

const void UFlareShipPartsCatalog::GetRCSList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size)
{
	for (int32 i = 0; i < RCSCatalog.Num(); i++)
	{
		if (RCSCatalog[i]->Data.Size == Size)
		{
			OutData.Add(&(RCSCatalog[i])->Data);
		}
	}
}

const void UFlareShipPartsCatalog::GetWeaponList(TArray<FFlareShipModuleDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size)
{
	for (int32 i = 0; i < WeaponCatalog.Num(); i++)
	{
		if (WeaponCatalog[i]->Data.Size == Size)
		{
			OutData.Add(&(WeaponCatalog[i])->Data);
		}
	}
}
