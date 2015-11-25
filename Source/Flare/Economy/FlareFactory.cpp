
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

	for (int32 FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
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

void UFlareFactory::SetOutputLimit(FFlareResourceDescription* Resource, uint32 MaxSlot)
{
	bool ExistingResource = false;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			ExistingResource = true;
			FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity = MaxSlot;
			break;
		}
	}

	if(!ExistingResource)
	{
		FFlareCargoSave NewCargoLimit;
		NewCargoLimit.ResourceIdentifier = Resource->Identifier;
		NewCargoLimit.Quantity = MaxSlot;
		FactoryData.OutputCargoLimit.Add(NewCargoLimit);
	}
}

void UFlareFactory::ClearOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			FactoryData.OutputCargoLimit.RemoveAt(CargoLimitIndex);
			return;
		}
	}
}

bool UFlareFactory::HasCostReserved()
{
	if (FactoryData.CostReserved < FactoryDescription->ProductionCost)
	{
		return false;
	}

	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];

		bool ResourceFound = false;

		for (int32 ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
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
	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
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

	TArray<FFlareFactoryResource> OutputResources = GetLimitedOutputResources();

	// First, fill already existing slots
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
	{
		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
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
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
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
	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];
		FFlareCargoSave* AlreadyReservedCargo = NULL;


		// Check in reserved resources
		for (int32 ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
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
	for (int32 ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
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

	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];

		for (int32 ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
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
	TArray<FFlareFactoryResource> OutputResources = GetLimitedOutputResources();
	for (int32 ResourceIndex = 0 ; ResourceIndex < OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &OutputResources[ResourceIndex];
		if (Parent->GiveResources(&Resource->Resource->Data, Resource->Quantity) < Resource->Quantity)
		{
			FLOGV("Fail to give %d resource '%s' to %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString());
		}
	}

	// Perform output actions
	for (int32 ActionIndex = 0 ; ActionIndex < FactoryDescription->OutputActions.Num() ; ActionIndex++)
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

	for (uint32 Index = 0; Index < Action->Quantity; Index++)
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

TArray<FFlareFactoryResource> UFlareFactory::GetLimitedOutputResources()
{
	TArray<FFlareCargo>& CargoBay = Parent->GetCargoBay();
	TArray<FFlareFactoryResource> OutputResources = FactoryDescription->OutputResources;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		uint32 MaxCapacity = Parent->GetDescription()->CargoBayCapacity * FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier);
		uint32 CurrentQuantity = 0;
		for (int32 CargoIndex = 0 ; CargoIndex < CargoBay.Num() ; CargoIndex++)
		{
			if(CargoBay[CargoIndex].Resource == Resource)
			{
				CurrentQuantity += CargoBay[CargoIndex].Quantity;
			}
		}

		uint32 MaxAddition;

		if(CurrentQuantity > MaxCapacity)
		{
			MaxAddition = 0;
		}
		else
		{
			MaxAddition = MaxCapacity - CurrentQuantity;
		}

		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == Resource)
			{
				OutputResources[ResourceIndex].Quantity = FMath::Min(MaxAddition, OutputResources[ResourceIndex].Quantity);

				if (OutputResources[ResourceIndex].Quantity == 0)
				{
					OutputResources.RemoveAt(ResourceIndex);
				}
				break;
			}
		}
	}
	return OutputResources;
}

uint32 UFlareFactory::GetOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			return FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		}
	}
	FLOGV("No output limit for %s", *Resource->Identifier.ToString());
	return 0;
}

bool UFlareFactory::HasOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			return true;
		}
	}
	return false;
}


int32 UFlareFactory::GetInputResourcesCount()
{
	return FactoryDescription->InputResources.Num();
}

FFlareResourceDescription* UFlareFactory::GetInputResource(int32 Index)
{
	return &FactoryDescription->InputResources[Index].Resource->Data;
}

uint32 UFlareFactory::GetInputResourceQuantity(int32 Index)
{
	return FactoryDescription->InputResources[Index].Quantity;
}

bool UFlareFactory::HasOutputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->OutputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &FactoryDescription->OutputResources[ResourceIndex].Resource->Data;
		if(ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

bool UFlareFactory::HasInputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &FactoryDescription->InputResources[ResourceIndex].Resource->Data;
		if(ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

