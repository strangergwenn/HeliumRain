
#include "FlareWorldHelper.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareWorld.h"
#include "../Economy/FlareCargoBay.h"
#include "FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "FlareScenarioTools.h"


TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldHelper::ComputeWorldResourceStats(AFlareGame* Game)
{
	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;

	// Init
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats ResourceStats;
		ResourceStats.Production = 0;
		ResourceStats.Consumption = 0;
		ResourceStats.Balance = 0;
		ResourceStats.Stock = 0;
		ResourceStats.Capacity = 0;

		WorldStats.Add(Resource, ResourceStats);
	}

	for (int SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> SectorStats;

		SectorStats = SectorHelper::ComputeSectorResourceStats(Sector);


		for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

			WorldHelper::FlareResourceStats& SectorResourceStats = SectorStats[Resource];
			WorldHelper::FlareResourceStats& ResourceStats = WorldStats[Resource];
			ResourceStats.Production += SectorResourceStats.Production;
			ResourceStats.Consumption += SectorResourceStats.Consumption;
			ResourceStats.Balance += SectorResourceStats.Balance;
			ResourceStats.Stock += SectorResourceStats.Stock;
			ResourceStats.Capacity += SectorResourceStats.Capacity;

			WorldStats.Add(Resource, ResourceStats);
		}
	}


	// Balance
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

		ResourceStats->Balance = ResourceStats->Production - ResourceStats->Consumption;

		/*FLOGV("World stats for %s: Production=%f Consumption=%f Balance=%f Stock=%d",
			  *Resource->Name.ToString(),
			  ResourceStats->Production,
			  ResourceStats->Consumption,
			  ResourceStats->Balance,
			  ResourceStats->Stock);*/
	}

	return WorldStats;
}
