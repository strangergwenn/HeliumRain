
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

void UFlareFactory::Simulate(long Duration)
{

	if (!FactoryData.Active)
	{
		return;
	}

	long RemainingDuration = Duration;

	while (RemainingDuration >= 0)
	{
		// Check if production is running
		if (FactoryData.CostReserved == FactoryDescription->ProductionCost)
		{
			if (!HasInputResources())
			{
				CancelProduction();
				FLOGV("%s : Production cancel, missing input resources", *FactoryDescription->Name.ToString())
				return;
			}

			if (FactoryData.ProductionBeginTime + FactoryDescription->ProductionTime > GetGame()->GetGameWorld()->GetTime())
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

			RemainingDuration = GetGame()->GetGameWorld()->GetTime() - (FactoryData.ProductionBeginTime + FactoryDescription->ProductionTime);
		}
		else if (HasInputResources() && HasInputMoney())
		{
			BeginProduction(GetGame()->GetGameWorld()->GetTime() - RemainingDuration);
		}
		else
		{
			// Production can't start
			FLOGV("%s : Production can't start resources ? %d money ?%d", *FactoryDescription->Name.ToString(), HasInputResources(), HasInputMoney())
			return;
		}
	}
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
	TArray<FFlareCargo>* CargoBay = Parent->GetCargoBay();

	TArray<FFlareFactoryResource> OutputResources = FactoryDescription->OutputResources;

	// First pass, fill already existing slots
	for (int CargoIndex = 0 ; CargoIndex < CargoBay->Num() ; CargoIndex++)
	{
		for (int ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == (*CargoBay)[CargoIndex].Resource)
			{
				// Same resource
				uint32 AvailableCapacity = (*CargoBay)[CargoIndex].Capacity - (*CargoBay)[CargoIndex].Quantity;
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
	for (int CargoIndex = 0 ; CargoIndex < CargoBay->Num() ; CargoIndex++)
	{
		if (OutputResources.Num() == 0)
		{
			// No more resource to try to put
			break;
		}

		if ((*CargoBay)[CargoIndex].Quantity == 0)
		{
			// Empty slot, fill it
			OutputResources[0].Quantity -= FMath::Min((*CargoBay)[CargoIndex].Capacity, OutputResources[0].Quantity);

			if (OutputResources[0].Quantity == 0)
			{
				OutputResources.RemoveAt(0);
			}
		}
	}

	return OutputResources.Num() == 0;
}

void UFlareFactory::BeginProduction(int64 SimulatedTime)
{
	Parent->GetCompany()->TakeMoney(FactoryDescription->ProductionCost);
	FactoryData.CostReserved = FactoryDescription->ProductionCost;
	FactoryData.ProductionBeginTime = SimulatedTime;
}

void UFlareFactory::CancelProduction()
{
	Parent->GetCompany()->GiveMoney(FactoryData.CostReserved);
	FactoryData.CostReserved = 0;
}

void UFlareFactory::DoProduction()
{
	FactoryData.CostReserved = 0;
	// TODO Give lost money to people

	// Consume input resource
	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->InputResources[ResourceIndex];
		if (Parent->TakeResources(&Resource->Resource->Data, Resource->Quantity) < Resource->Quantity)
		{
			FLOGV("Fail to take %d resource '%s' to %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString());
		}
	}

	// Generate output resource
	for (int ResourceIndex = 0 ; ResourceIndex < FactoryDescription->OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &FactoryDescription->OutputResources[ResourceIndex];
		if (Parent->GiveResources(&Resource->Resource->Data, Resource->Quantity) < Resource->Quantity)
		{
			FLOGV("Fail to give %d resource '%s' to %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString());
		}
	}
}

FFlareWorldEvent *UFlareFactory::GenerateEvent()
{
	if (!FactoryData.Active)
	{
		return NULL;
	}

	// Check if production is running
	if (FactoryData.CostReserved == FactoryDescription->ProductionCost)
	{
		if (!HasOutputFreeSpace())
		{
			return NULL;
		}

		NextEvent.Time = FMath::Max(FactoryData.ProductionBeginTime + FactoryDescription->ProductionTime, GetGame()->GetGameWorld()->GetTime());
		NextEvent.Visibility = EFlareEventVisibility::Silent;
		return &NextEvent;
	}
	return NULL;
}
