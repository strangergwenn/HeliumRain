
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "FlareFactory.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareFactory::UFlareFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareFactory::Load(UFlareSimulatedSpacecraft* ParentSpacecraft, const FFlareFactoryDescription* Description, const FFlareFactorySave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();

	FactoryData = Data;
	FactoryDescription = Description;
	Parent = ParentSpacecraft;
}


FFlareFactorySave* UFlareFactory::Save()
{
	return &FactoryData;
}

void UFlareFactory::Simulate(int64 Duration)
{

	if (!FactoryData.Active)
	{
		return;
	}

	int64 RemainingDuration = Duration;

	while (RemainingDuration >= 0)
	{
		// Check if production is running
		if(!IsNeedProduction())
		{
			// Don't produce if not needed
			return;
		}
		else if (HasCostReserved())
		{
			int64 ProducedTime = FMath::Min(RemainingDuration, FactoryDescription->ProductionTime - FactoryData.ProductedDuration);
			FactoryData.ProductedDuration += ProducedTime ;
			RemainingDuration -= ProducedTime;

			if(FactoryData.ProductedDuration < FactoryDescription->ProductionTime)
			{
				// In production
				return;
			}

			if (!HasOutputFreeSpace())
			{
				// TODO display warning to user
				// No free space wait.
				FLOGV("%s : Production Paused : no output free space", *FactoryDescription->Name.ToString())
				return;
			}

			DoProduction();
		}
		else if (HasInputResources() && HasInputMoney())
		{
			BeginProduction();
		}
		else
		{
			// Production can't start
			FLOGV("%s : Production can't start resources ? %d money ?%d", *FactoryDescription->Name.ToString(), HasInputResources(), HasInputMoney())
			return;
		}
	}
}

void UFlareFactory::Start()
{
	FactoryData.Active = true;

	// Stop other factories
	TArray<UFlareFactory*>& Factories = Parent->GetFactories();

	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		UFlareFactory* Factory = Factories[FactoryIndex];
		if(Factory == this)
		{
			continue;
		}
		else
		{
			Factory->Pause();
		}
	}
}

void UFlareFactory::Pause()
{
	FactoryData.Active = false;
}

void UFlareFactory::Stop()
{
	FactoryData.Active = false;
	CancelProduction();
}

void UFlareFactory::SetInfiniteCycle(bool Mode)
{
	FactoryData.InfiniteCycle = Mode;
}

void UFlareFactory::SetCycleCount(uint32 Count)
{
	FactoryData.CycleCount = Count;
}

bool UFlareFactory::HasCostReserved()
{
	if (FactoryData.CostReserved < FactoryDescription->ProductionCost)
	{
		return false;
	}

	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];

		bool ResourceFound = false;

		for (int ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier ==  Resource->Resource->Data.Identifier)
			{
				if (FactoryData.ResourceReserved[ReservedResourceIndex].Quantity < Resource->Quantity)
				{
					return false;
				}
				ResourceFound = true;
			}
		}

		if (!ResourceFound)
		{
			return false;
		}
	}

	return true;
}

bool UFlareFactory::HasInputMoney()
{
	return Parent->GetCompany()->GetMoney() >= FactoryDescription->ProductionCost;
}

bool UFlareFactory::HasInputResources()
{
	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];
		if (!Parent->HasResources(&Resource->Resource->Data, Resource->Quantity))
		{
			return false;
		}
	}
	return true;
}

bool UFlareFactory::HasOutputFreeSpace()
{
	TArray<FFlareCargo>& CargoBay = Parent->GetCargoBay();

	TArray<FFlareFactoryResource> OutputResources = FactoryDescription->OutputResources;

	// First pass, fill already existing slots
	for (int CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		for (int ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == CargoBay[CargoIndex].Resource)
			{
				// Same resource
				uint32 AvailableCapacity = CargoBay[CargoIndex].Capacity - CargoBay[CargoIndex].Quantity;
				if (AvailableCapacity > 0)
				{
					OutputResources[ResourceIndex].Quantity -= FMath::Min(AvailableCapacity, OutputResources[ResourceIndex].Quantity);

					if (OutputResources[ResourceIndex].Quantity == 0)
					{
						OutputResources.RemoveAt(ResourceIndex);
					}

					// Never 2 output with the same resource, so, break
					break;
				}
			}
		}
	}

	// Fill free cargo slots
	for (int CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		if (OutputResources.Num() == 0)
		{
			// No more resource to try to put
			break;
		}

		if (CargoBay[CargoIndex].Quantity == 0)
		{
			// Empty slot, fill it
			OutputResources[0].Quantity -= FMath::Min(CargoBay[CargoIndex].Capacity, OutputResources[0].Quantity);

			if (OutputResources[0].Quantity == 0)
			{
				OutputResources.RemoveAt(0);
			}
		}
	}

	return OutputResources.Num() == 0;
}

void UFlareFactory::BeginProduction()
{
	Parent->GetCompany()->TakeMoney(FactoryDescription->ProductionCost);


	// Consume input resources
	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];
		FFlareCargoSave* AlreadyReservedCargo = NULL;


		// Check in reserved resources
		for (int ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier ==  Resource->Resource->Data.Identifier)
			{
				AlreadyReservedCargo = &FactoryData.ResourceReserved[ReservedResourceIndex];
				break;
			}
		}

		uint32 ResourceToTake = Resource->Quantity;

		if (AlreadyReservedCargo)
		{
			ResourceToTake -= AlreadyReservedCargo->Quantity;
		}

		if (ResourceToTake <= 0)
		{
			continue;
		}

		if (Parent->TakeResources(&Resource->Resource->Data, ResourceToTake) < Resource->Quantity)
		{
			FLOGV("Fail to take %d resource '%s' to %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString());
		}

		if (AlreadyReservedCargo)
		{
			AlreadyReservedCargo->Quantity += ResourceToTake;
		}
		else
		{
			FFlareCargoSave NewReservedResource;
			NewReservedResource.Quantity = ResourceToTake;
			NewReservedResource.ResourceIdentifier = Resource->Resource->Data.Identifier;
			FactoryData.ResourceReserved.Add(NewReservedResource);
		}
	}

	FactoryData.CostReserved = FactoryDescription->ProductionCost;
}

void UFlareFactory::CancelProduction()
{
	Parent->GetCompany()->GiveMoney(FactoryData.CostReserved);
	FactoryData.CostReserved = 0;

	// Restore reserved resources
	for (int ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
	{
		FFlareResourceDescription*Resource = Game->GetResourceCatalog()->Get(FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier);

		uint32 GivenQuantity = Parent->GiveResources(Resource, FactoryData.ResourceReserved[ReservedResourceIndex].Quantity);

		if (GivenQuantity >= FactoryData.ResourceReserved[ReservedResourceIndex].Quantity)
		{
			FactoryData.ResourceReserved.RemoveAt(ReservedResourceIndex);
		}
		else
		{
			FactoryData.ResourceReserved[ReservedResourceIndex].Quantity -= GivenQuantity;
		}
	}

	FactoryData.ProductedDuration = 0;
}

void UFlareFactory::DoProduction()
{
	// Pay cost
	uint32 PaidCost = FMath::Min(FactoryDescription->ProductionCost, FactoryData.CostReserved);
	FactoryData.CostReserved -= PaidCost;
	// TODO Give lost money to people

	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];

		for (int ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier != Resource->Resource->Data.Identifier)
			{
				continue;
			}

			if (Resource->Quantity >= FactoryData.ResourceReserved[ReservedResourceIndex].Quantity)
			{
				FactoryData.ResourceReserved.RemoveAt(ReservedResourceIndex);
			}
			else
			{
				FactoryData.ResourceReserved[ReservedResourceIndex].Quantity -= Resource->Quantity;
			}
		}
	}

	// Generate output resources
	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->OutputResources[ResourceIndex];
		if (Parent->GiveResources(&Resource->Resource->Data, Resource->Quantity) < Resource->Quantity)
		{
			FLOGV("Fail to give %d resource '%s' to %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString());
		}
	}

	// Perform output actions
	for (int ActionIndex = 0 ; ActionIndex < FactoryDescription->OutputActions.Num() ; ActionIndex++)
	{
		const FFlareFactoryAction* Action = &FactoryDescription->OutputActions[ActionIndex];
		switch(Action->Action)
		{
			case EFlareFactoryAction::CreateShip:
				PerformCreateShipAction(Action);
				break;
			case EFlareFactoryAction::DiscoverSector:
			case EFlareFactoryAction::GainTechnology:
				// TODO
			default:
				FLOGV("Warning ! Not implemented factory action %d", (Action->Action+0));
		}
	}

	FactoryData.ProductedDuration = 0;
	if(!HasInfiniteCycle())
	{
		FactoryData.CycleCount--;
	}
}

FFlareWorldEvent *UFlareFactory::GenerateEvent()
{
	if (!FactoryData.Active || !IsNeedProduction())
	{
		return NULL;
	}

	// Check if production is running
	if (HasCostReserved())
	{
		if (!HasOutputFreeSpace())
		{
			return NULL;
		}

		NextEvent.Time = GetGame()->GetGameWorld()->GetTime() + FactoryDescription->ProductionTime - FactoryData.ProductedDuration;
		NextEvent.Visibility = EFlareEventVisibility::Silent;
		return &NextEvent;
	}
	return NULL;
}

void UFlareFactory::PerformCreateShipAction(const FFlareFactoryAction* Action)
{
	FFlareSpacecraftDescription* ShipDescription = GetGame()->GetSpacecraftCatalog()->Get(Action->Identifier);
	if (!ShipDescription)
	{
		return;
	}

	UFlareCompany* Company = Parent->GetCompany();
	FVector SpawnPosition = Parent->GetSpawnLocation();

	for (int Index = 0; Index < Action->Quantity; Index++)
	{
		Parent->GetCurrentSector()->CreateShip(ShipDescription, Company, SpawnPosition);
	}
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

int64 UFlareFactory::GetRemainingProductionDuration()
{
	return FactoryDescription->ProductionTime - FactoryData.ProductedDuration;
}
