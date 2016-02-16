
#include "../Flare.h"
#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../Player/FlarePlayerController.h"
#include "FlareFactory.h"


#define LOCTEXT_NAMESPACE "FlareFactoryInfo"


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

void UFlareFactory::Simulate()
{

	if (!FactoryData.Active)
	{
		return;
	}


	// Check if production is running
	if (!IsNeedProduction())
	{
		// Don't produce if not needed
		return;
	}

	if (HasCostReserved())
	{

		if (FactoryData.ProductedDuration < GetCycleData().ProductionTime)
		{
			FactoryData.ProductedDuration += 1;
		}

		if (FactoryData.ProductedDuration < GetCycleData().ProductionTime)
		{

			// Still In production
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
	TryBeginProduction();
}

void UFlareFactory::TryBeginProduction()
{
	if (IsNeedProduction() && !HasCostReserved() && HasInputResources() && HasInputMoney())
	{
		BeginProduction();
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
		if (Factory == this)
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
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			ExistingResource = true;
			FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity = MaxSlot;
			break;
		}
	}

	if (!ExistingResource)
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
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			FactoryData.OutputCargoLimit.RemoveAt(CargoLimitIndex);
			return;
		}
	}
}

void UFlareFactory::SetTargetShipClass(FName Identifier)
{
	if (FactoryData.TargetShipClass != Identifier)
	{
		FactoryData.TargetShipClass = Identifier;
		FactoryData.ProductedDuration = 0;
	}
}

bool UFlareFactory::HasCostReserved()
{
	if (FactoryData.CostReserved < GetProductionCost())
	{
		return false;
	}

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];

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
	return Parent->GetCompany()->GetMoney() >= GetProductionCost();
}

bool UFlareFactory::HasInputResources()
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		if (!Parent->GetCargoBay()->HasResources(&Resource->Resource->Data, Resource->Quantity))
		{
			return false;
		}
	}
	return true;
}

bool UFlareFactory::HasOutputFreeSpace()
{
	//TArray<FFlareCargo>& CargoBay = Parent->GetCargoBay();
	UFlareCargoBay* CargoBay = Parent->GetCargoBay();


	TArray<FFlareFactoryResource> OutputResources = GetLimitedOutputResources();

	// First, fill already existing slots
	for (uint32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == CargoBay->GetSlot(CargoIndex)->Resource)
			{
				// Same resource
				uint32 AvailableCapacity = CargoBay->GetSlot(CargoIndex)->Capacity - CargoBay->GetSlot(CargoIndex)->Quantity;
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
	for (uint32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		if (OutputResources.Num() == 0)
		{
			// No more resource to try to put
			break;
		}

		if (CargoBay->GetSlot(CargoIndex)->Quantity == 0)
		{
			// Empty slot, fill it
			OutputResources[0].Quantity -= FMath::Min(CargoBay->GetSlot(CargoIndex)->Capacity, OutputResources[0].Quantity);

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
	Parent->GetCompany()->TakeMoney(GetProductionCost());


	// Consume input resources
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
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

		if (Parent->GetCargoBay()->TakeResources(&Resource->Resource->Data, ResourceToTake) < Resource->Quantity)
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

	FactoryData.CostReserved = GetProductionCost();
}

void UFlareFactory::CancelProduction()
{
	Parent->GetCompany()->GiveMoney(FactoryData.CostReserved);
	FactoryData.CostReserved = 0;

	// Restore reserved resources
	for (int32 ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
	{
		FFlareResourceDescription*Resource = Game->GetResourceCatalog()->Get(FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier);

		uint32 GivenQuantity = Parent->GetCargoBay()->GiveResources(Resource, FactoryData.ResourceReserved[ReservedResourceIndex].Quantity);

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
	uint32 PaidCost = FMath::Min(GetProductionCost(), FactoryData.CostReserved);
	FactoryData.CostReserved -= PaidCost;
	Parent->GetCurrentSector()->GetPeople()->Pay(PaidCost);

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];

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
		if (Parent->GetCargoBay()->GiveResources(&Resource->Resource->Data, Resource->Quantity) < Resource->Quantity)
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
	if (!HasInfiniteCycle())
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

		NextEvent.Date= GetGame()->GetGameWorld()->GetDate() + GetCycleData().ProductionTime - FactoryData.ProductedDuration;
		NextEvent.Visibility = EFlareEventVisibility::Silent;
		return &NextEvent;
	}
	return NULL;
}

void UFlareFactory::PerformCreateShipAction(const FFlareFactoryAction* Action)
{
	FFlareSpacecraftDescription* ShipDescription = GetGame()->GetSpacecraftCatalog()->Get(FactoryData.TargetShipClass);

	if (ShipDescription)
	{
		UFlareCompany* Company = Parent->GetCompany();
		FVector SpawnPosition = Parent->GetSpawnLocation();
		for (uint32 Index = 0; Index < Action->Quantity; Index++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Parent->GetCurrentSector()->CreateShip(ShipDescription, Company, SpawnPosition);
			AFlarePlayerController* PC = Parent->GetGame()->GetPC();

			// Notify PC
			if (PC && Spacecraft)
			{
				PC->Notify(LOCTEXT("ShipBuilt", "Ship production complete"),
					FText::Format(LOCTEXT("ShipBuiltFormat", "Your ship {0} is ready to use !"), FText::FromString(Spacecraft->GetImmatriculation().ToString())),
					FName("ship-killed"),
					EFlareNotification::NT_Economy);
			}
		}
	}

	FactoryData.TargetShipClass = NAME_None;
	Stop();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareProductionData& UFlareFactory::GetCycleData()
{
	if (HasCreateShipAction() && FactoryData.TargetShipClass != NAME_None)
	{
		return GetCycleDataForShipClass(FactoryData.TargetShipClass);
	}
	else
	{
		return FactoryDescription->CycleCost;
	}
}

const FFlareProductionData& UFlareFactory::GetCycleDataForShipClass(FName Class)
{
	return GetGame()->GetSpacecraftCatalog()->Get(Class)->CycleCost;
}

bool UFlareFactory::HasCreateShipAction() const
{
	for (int32 Index = 0; Index < GetDescription()->OutputActions.Num(); Index++)
	{
		if (GetDescription()->OutputActions[Index].Action == EFlareFactoryAction::CreateShip)
		{
			return true;
		}
	}
	return false;
}

uint32 UFlareFactory::GetProductionCost(const FFlareProductionData* Data)
{
	const FFlareProductionData* CycleData = Data ? Data : &GetCycleData();
	check(CycleData);

	ScaledProductionCost = CycleData->ProductionCost;

	if (FactoryDescription->NeedSun)
	{
		FFlareCelestialBody* Body = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(Parent->GetCurrentSector()->GetOrbitParameters()->CelestialBodyIdentifier);

		if (Body)
		{
			float LightRatio = Game->GetGameWorld()->GetPlanerarium()->GetLightRatio(Body, Parent->GetCurrentSector()->GetOrbitParameters()->Altitude);
			ScaledProductionCost = CycleData->ProductionCost / LightRatio;
		}
	}

	return ScaledProductionCost;
}

int64 UFlareFactory::GetRemainingProductionDuration()
{
	return GetCycleData().ProductionTime - FactoryData.ProductedDuration;
}

TArray<FFlareFactoryResource> UFlareFactory::GetLimitedOutputResources()
{
	UFlareCargoBay* CargoBay = Parent->GetCargoBay();
	TArray<FFlareFactoryResource> OutputResources = GetCycleData().OutputResources;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		uint32 MaxCapacity = CargoBay->GetCapacity() * FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier);
		uint32 CurrentQuantity = 0;
		for (uint32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
		{
			if (CargoBay->GetSlot(CargoIndex)->Resource == Resource)
			{
				CurrentQuantity += CargoBay->GetSlot(CargoIndex)->Quantity;
			}
		}

		uint32 MaxAddition;

		if (CurrentQuantity > MaxCapacity)
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
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
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
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			return true;
		}
	}
	return false;
}

int32 UFlareFactory::GetInputResourcesCount()
{
	return GetCycleData().InputResources.Num();
}

FFlareResourceDescription* UFlareFactory::GetInputResource(int32 Index)
{
	return &GetCycleData().InputResources[Index].Resource->Data;
}

uint32 UFlareFactory::GetInputResourceQuantity(int32 Index)
{
	return GetCycleData().InputResources[Index].Quantity;
}

bool UFlareFactory::HasOutputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().OutputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

bool UFlareFactory::HasInputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

FText UFlareFactory::GetFactoryCycleCost(const FFlareProductionData* Data)
{
	FText ProductionCostText;
	FText CommaTextReference = LOCTEXT("Comma", " + ");

	// Cycle cost in credits
	uint32 CycleProductionCost = GetProductionCost(Data);
	if (CycleProductionCost > 0)
	{
		ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(CycleProductionCost));
	}

	// Cycle cost in resources
	for (int ResourceIndex = 0; ResourceIndex < Data->InputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &Data->InputResources[ResourceIndex];
		check(FactoryResource);

		ProductionCostText = FText::Format(LOCTEXT("ProductionResourcesFormat", "{0}{1} {2} {3}"),
			ProductionCostText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return ProductionCostText;
}

FText UFlareFactory::GetFactoryCycleInfo()
{
	FText CommaTextReference = LOCTEXT("Comma", " + ");
	FText ProductionOutputText;

	// No ship class selected
	if (HasCreateShipAction() && FactoryData.TargetShipClass == NAME_None)
	{
		return LOCTEXT("SelectShipClass", "Please select a ship class to build");
	}

	FText ProductionCostText = GetFactoryCycleCost(&GetCycleData());

	// Cycle output in factory actions
	for (int ActionIndex = 0; ActionIndex < GetDescription()->OutputActions.Num(); ActionIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryAction* FactoryAction = &GetDescription()->OutputActions[ActionIndex];
		check(FactoryAction);

		switch (FactoryAction->Action)
		{
			// Ship production
		case EFlareFactoryAction::CreateShip:
			ProductionOutputText = FText::Format(LOCTEXT("ProductionActionsFormat", "{0}{1} {2} {3}"),
				ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity),
				GetGame()->GetSpacecraftCatalog()->Get(FactoryData.TargetShipClass)->Name);
			break;

			// TODO
		case EFlareFactoryAction::DiscoverSector:
		case EFlareFactoryAction::GainTechnology:
		default:
			FLOGV("SFlareShipMenu::UpdateFactoryLimitsList : Unimplemented factory action %d", (FactoryAction->Action + 0));
		}
	}

	// Cycle output in resources
	for (int ResourceIndex = 0; ResourceIndex < GetCycleData().OutputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &GetCycleData().OutputResources[ResourceIndex];
		check(FactoryResource);

		ProductionOutputText = FText::Format(LOCTEXT("ProductionOutputFormat", "{0}{1} {2} {3}"),
			ProductionOutputText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return FText::Format(LOCTEXT("FactoryCycleInfoFormat", "Production cycle : {0} \u2192 {1} each {2}"),
		ProductionCostText, ProductionOutputText,
		FText::FromString(*UFlareGameTools::FormatDate(GetCycleData().ProductionTime, 2))); // FString needed here
}

FText UFlareFactory::GetFactoryStatus()
{
	FText ProductionStatusText;

	if (IsActive())
	{
		if (!IsNeedProduction())
		{
			ProductionStatusText = LOCTEXT("ProductionNotNeeded", "Production not needed");
		}
		else if (HasCostReserved())
		{
			ProductionStatusText = FText::Format(LOCTEXT("ProductionInProgressFormat", "Producing ({0}{1})"),
				FText::FromString(*UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2)), // FString needed here
				HasOutputFreeSpace() ? FText() : LOCTEXT("ProductionNoSpace", ", not enough space"));
		}
		else if (HasInputMoney() && HasInputResources())
		{
			ProductionStatusText = LOCTEXT("ProductionWillStart", "Starting");
		}
		else
		{
			if (!HasInputMoney())
			{
				ProductionStatusText = LOCTEXT("ProductionNotEnoughMoney", "Waiting for credits");
			}

			if (!HasInputResources())
			{
				ProductionStatusText = LOCTEXT("ProductionNotEnoughResources", "Waiting for resources");
			}
		}
	}
	else if (IsPaused())
	{
		ProductionStatusText = FText::Format(LOCTEXT("ProductionPaused", "Paused ({0} to completion)"),
			FText::FromString(*UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2))); // FString needed here
	}
	else
	{
		ProductionStatusText = LOCTEXT("ProductionStopped", "Stopped");
	}

	return ProductionStatusText;
}


#undef LOCTEXT_NAMESPACE
