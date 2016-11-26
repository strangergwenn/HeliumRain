#include "../Flare.h"
#include "FlareWorldHelper.h"
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

		WorldStats.Add(Resource, ResourceStats);
	}

	for (int SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		for (int SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

			// Stock
			TArray<FFlareCargo>& CargoBaySlots = Spacecraft->GetCargoBay()->GetSlots();
			for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
			{
				FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

				if (!Cargo.Resource)
				{
					continue;
				}

				WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Cargo.Resource];

				ResourceStats->Stock += Cargo.Quantity;
			}

			for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];
				if ((!Factory->IsActive() || !Factory->IsNeedProduction()))
				{
					// No resources needed
					break;
				}

				// Input flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
					WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

					float Flow = (float) Factory->GetInputResourceQuantity(ResourceIndex) / (float) Factory->GetProductionDuration();
					ResourceStats->Consumption += Flow;
				}

				// Ouput flow
				for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);
					WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

					float Flow = (float) Factory->GetOutputResourceQuantity(ResourceIndex) / (float) Factory->GetProductionDuration();
					ResourceStats->Production+= Flow;
				}
			}
		}

		// Customer flow
		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

			ResourceStats->Consumption += Sector->GetPeople()->GetRessourceConsumption(Resource, false);
		}
	}

	// FS
	FFlareResourceDescription* FleetSupply = Game->GetScenarioTools()->FleetSupply;
	WorldHelper::FlareResourceStats *FSResourceStats = &WorldStats[FleetSupply];
	FFlareFloatBuffer* Stats = &Game->GetGameWorld()->GetData()->FleetSupplyConsumptionStats;
	float MeanConsumption = Stats->GetMean(0, Stats->MaxSize-1);
	FSResourceStats->Consumption = MeanConsumption;

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
