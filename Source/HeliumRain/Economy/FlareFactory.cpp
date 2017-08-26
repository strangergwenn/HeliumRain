
#include "FlareFactory.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"

#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareCompany.h"
#include "../Game/FlareSimulatedSector.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"


#define LOCTEXT_NAMESPACE "FlareFactoryInfo"

#define MAX_DAMAGE_MALUS 10


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
	CycleCostCacheLevel = -1;

	if (IsShipyard() && FactoryData.TargetShipClass == NAME_None && FactoryData.Active)
	{
		FLOG("WARNING: fix corrupted shipyard state");
		FactoryData.Active = false;
	}
}


FFlareFactorySave* UFlareFactory::Save()
{
	return &FactoryData;
}

void UFlareFactory::Simulate()
{
	FCHECK(Parent);
	FCHECK(Parent->GetCurrentSector());
	FCHECK(Parent->GetCurrentSector()->GetPeople());

	if (!FactoryData.Active)
	{
		goto post_prod;
	}


	// Check if production is running
	if (!IsNeedProduction())
	{
		// Don't produce if not needed
		goto post_prod;
	}

	if (HasCostReserved())
	{

		if (FactoryData.ProductedDuration < GetProductionTime(GetCycleData()))
		{
			FactoryData.ProductedDuration += 1;
		}

		if (FactoryData.ProductedDuration < GetProductionTime(GetCycleData()))
		{

			// Still In production
			goto post_prod;
		}

		if (!HasOutputFreeSpace())
		{
			// TODO display warning to user
			// No free space wait.
			goto post_prod;
		}

		DoProduction();

	}
	TryBeginProduction();

	if (GetProductionTime(GetCycleData()) == 0  && HasCostReserved())
	{
		DoProduction();
	}


post_prod:

	if (FactoryDescription->VisibleStates)
	{
		UpdateDynamicState();
	}
}

void UFlareFactory::TryBeginProduction()
{
	if (GetMarginRatio() < 0.f)
	{
		FLOGV("WARNING: Margin ratio for %s is : %f", *FactoryDescription->Name.ToString(), GetMarginRatio())
	}

	if (IsNeedProduction() && !HasCostReserved() && HasInputResources() && HasInputMoney())
	{
		BeginProduction();
	}

}

void UFlareFactory::UpdateDynamicState()
{
	if(FactoryData.TargetShipClass == NAME_None)
	{
		Parent->SetDynamicComponentState(TEXT("idle"));
	}
	else
	{
		Parent->SetDynamicComponentState(FactoryData.TargetShipClass,
				((float)GetProductedDuration() / (float)GetRemainingProductionDuration()));
	}
}

void UFlareFactory::Start()
{
	FactoryData.Active = true;
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
	bool AllowDepts = !IsShipyard()
			|| (GetTargetShipCompany() != NAME_None && GetTargetShipCompany() != Parent->GetCompany()->GetIdentifier());


	if(AllowDepts)
	{
		return true;
	}
	return Parent->GetCompany()->GetMoney() >= GetProductionCost();
}

bool UFlareFactory::HasInputResources()
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		if (!Parent->GetCargoBay()->HasResources(&Resource->Resource->Data, Resource->Quantity, Parent->GetCompany()))
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
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		if(!CargoBay->CheckRestriction(CargoBay->GetSlot(CargoIndex), Parent->GetCompany()))
		{
			continue;
		}

		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == CargoBay->GetSlot(CargoIndex)->Resource)
			{
				// Same resource
				int32 AvailableCapacity = CargoBay->GetSlotCapacity() - CargoBay->GetSlot(CargoIndex)->Quantity;
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
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		if (OutputResources.Num() == 0)
		{
			// No more resource to try to put
			break;
		}

		if (CargoBay->GetSlot(CargoIndex)->Quantity == 0)
		{
			// Empty slot, fill it
			OutputResources[0].Quantity -= FMath::Min(CargoBay->GetSlotCapacity(), OutputResources[0].Quantity);

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
	bool AllowDepts = !IsShipyard()
			|| (GetTargetShipCompany() != NAME_None && GetTargetShipCompany() != Parent->GetCompany()->GetIdentifier());

	if(!Parent->GetCompany()->TakeMoney(GetProductionCost(), AllowDepts))
	{
		return;
	}


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

		if (Parent->GetCargoBay()->TakeResources(&Resource->Resource->Data, ResourceToTake, Parent->GetCompany()) < Resource->Quantity)
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

		int32 GivenQuantity = Parent->GetCargoBay()->GiveResources(Resource, FactoryData.ResourceReserved[ReservedResourceIndex].Quantity, Parent->GetCompany());

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
	FactoryData.TargetShipClass = NAME_None;
	FactoryData.TargetShipCompany = NAME_None;

	if (IsShipyard())
	{
		// Wait next ship order
		FactoryData.Active = false;
		Parent->UpdateShipyardProduction();
	}
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
		if (Parent->GetCargoBay()->GiveResources(&Resource->Resource->Data, Resource->Quantity, Parent->GetCompany()) < Resource->Quantity)
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
				PerformDiscoverSectorAction(Action);
				break;

			case EFlareFactoryAction::GainResearch:
				PerformGainResearchAction(Action);
				break;

			case EFlareFactoryAction::BuildStation:
				PerformBuildStationAction(Action);
				break;

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

		NextEvent.Date= GetGame()->GetGameWorld()->GetDate() + GetProductionTime(GetCycleData()) - FactoryData.ProductedDuration;
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

		if(FactoryData.TargetShipCompany != NAME_None)
		{
			Company = Game->GetGameWorld()->FindCompany(FactoryData.TargetShipCompany);
		}

		FVector SpawnPosition = Parent->GetSpawnLocation();
		for (uint32 Index = 0; Index < Action->Quantity; Index++)
		{
			// Get data
			UFlareSimulatedSpacecraft* Spacecraft = Parent->GetCurrentSector()->CreateSpacecraft(ShipDescription, Company, SpawnPosition);
			AFlarePlayerController* PC = Parent->GetGame()->GetPC();
			FFlareMenuParameterData Data;
			Data.Spacecraft = Spacecraft;

			// Notify PC
			if (PC && Spacecraft && Spacecraft->GetCompany() == PC->GetCompany())
			{
				PC->Notify(LOCTEXT("ShipBuilt", "Ship production complete"),
					FText::Format(LOCTEXT("ShipBuiltFormat", "Your ship {0} is ready to use !"), UFlareGameTools::DisplaySpacecraftName(Spacecraft)),
					FName("ship-production-complete"),
					EFlareNotification::NT_Economy,
					false,
					EFlareMenu::MENU_Ship,
					Data);

				Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("build-ship").PutInt32("size", Spacecraft->GetSize()));

				PC->SetAchievementProgression("ACHIEVEMENT_SHIP", 1);
			}
		}
	}

	FactoryData.TargetShipClass = NAME_None;
	FactoryData.TargetShipCompany = NAME_None;

	// No more ship to produce
	Stop();
}

// Compute a proximity score (lower is better)
// Hundreds are bodies, dozens are altitude, units are phase
static int ComputeProximityScore(const UFlareSimulatedSector& SectorA, const UFlareSimulatedSector& SectorB)
{
	int Score = 0;

	// Get parameters
	const FFlareSectorOrbitParameters* ParamsA = SectorA.GetOrbitParameters();
	const FFlareSectorOrbitParameters* ParamsB = SectorB.GetOrbitParameters();

	// Same planetary body as source
	if (ParamsA->CelestialBodyIdentifier != ParamsB->CelestialBodyIdentifier)
	{
		Score += 100;
	}

	// Altitude
	float MaxAltitude = 10000;
	float AltitudeDistance = FMath::Abs(ParamsA->Altitude - ParamsB->Altitude);
	AltitudeDistance = FMath::Clamp(AltitudeDistance, 0.0f, MaxAltitude);
	Score += 10 * (AltitudeDistance / MaxAltitude);

	// Phase
	float MaxPhase = 360;
	float PhaseDistance = FMath::Abs(ParamsA->Phase - ParamsB->Phase);
	PhaseDistance = FMath::Clamp(PhaseDistance, 0.0f, MaxPhase);
	Score += PhaseDistance / MaxPhase;

	return Score;
}

struct FSortByProximity
{
	UFlareSimulatedSector* SourceSector;
	FSortByProximity(UFlareSimulatedSector* Sector)
		: SourceSector(Sector)
	{}

	bool operator()(const UFlareSimulatedSector& SectorA, const UFlareSimulatedSector& SectorB) const
	{
		return (ComputeProximityScore(SectorA, *SourceSector) < ComputeProximityScore(SectorB, *SourceSector));
	}
};

void UFlareFactory::PerformDiscoverSectorAction(const FFlareFactoryAction* Action)
{
	UFlareCompany* Company = Parent->GetCompany();

	// List all unknown sectors
	TArray<UFlareSimulatedSector*> Candidates;
	for (auto CandidateSector : Parent->GetGame()->GetGameWorld()->GetSectors())
	{
		if (!Company->IsKnownSector(CandidateSector) && !CandidateSector->GetDescription()->IsHiddenFromTelescopes)
		{
			Candidates.Add(CandidateSector);
		}
	}

	// Sort by score
	UFlareSimulatedSector* CurrentSector = Parent->GetCurrentSector();
	Candidates.Sort(FSortByProximity(CurrentSector));

	// Discover sector
	if (Candidates.Num())
	{
		AFlarePlayerController* PC = Parent->GetGame()->GetPC();
		UFlareSimulatedSector* TargetSector = NULL;

		// Pick a sector
		int TelescopeRange = 2;
		int Index = FMath::RandHelper(TelescopeRange);
		TargetSector = Candidates[Index];

		// Player-owned telescope (should always be the case according to #99) or other company ?
		if (Company == PC->GetCompany())
		{
			PC->DiscoverSector(TargetSector, false, true);
		}
		else
		{
			Company->DiscoverSector(TargetSector);
		}
	}
	else
	{
		FLOG("UFlareFactory::PerformDiscoverSectorAction : could not find a sector !");
	}

	Stop();
}

void UFlareFactory::PerformGainResearchAction(const FFlareFactoryAction* Action)
{
	UFlareCompany* Company = Parent->GetCompany();

	Company->GiveResearch(Action->Quantity * Parent->GetLevel());
	AFlarePlayerController* PC = Parent->GetGame()->GetPC();

	// Notify PC
	if (Parent->GetCompany() == PC->GetCompany())
	{
		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("produce-research").PutInt32("amount", Action->Quantity));
	}
}


void UFlareFactory::PerformBuildStationAction(const FFlareFactoryAction* Action)
{
	Parent->FinishConstruction();

	AFlarePlayerController* PC = Parent->GetGame()->GetPC();

	// Notify PC
	if (Parent->GetCompany() == PC->GetCompany())
	{
		// Get data
		FFlareMenuParameterData Data;
		Data.Spacecraft = Parent;


		PC->Notify(LOCTEXT("StationBuild", "Station build"),
			FText::Format(LOCTEXT("StationBuiltFormat", "Your station {0} is ready to use !"), UFlareGameTools::DisplaySpacecraftName(Parent)),
			FName("station-production-complete"),
			EFlareNotification::NT_Economy,
			false,
			EFlareMenu::MENU_Station,
			Data);

		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("finish-station-construction")
										 .PutInt32("upgrade", int32(Parent->GetLevel() > 1))
										 .PutName("identifier", Parent->GetDescription()->Identifier)
										 .PutName("sector", Parent->GetCurrentSector()->GetIdentifier()));

		PC->SetAchievementProgression("ACHIEVEMENT_STATION", 1);
	}

}



/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareProductionData& UFlareFactory::GetCycleData()
{
	if (IsShipyard() && FactoryData.TargetShipClass != NAME_None)
	{
		return GetCycleDataForShipClass(FactoryData.TargetShipClass);
	}
	else if (Parent->GetLevel() == CycleCostCacheLevel)
	{
		return CycleCostCache;
	}
	else
	{

		CycleCostCacheLevel = Parent->IsUnderConstruction() ? 1 : Parent->GetLevel();
		CycleCostCache.ProductionTime = FactoryDescription->CycleCost.ProductionTime;
		CycleCostCache.ProductionCost = FactoryDescription->CycleCost.ProductionCost * CycleCostCacheLevel;
		CycleCostCache.InputResources = FactoryDescription->CycleCost.InputResources;
		for (int32 ResourceIndex = 0 ; ResourceIndex < CycleCostCache.InputResources.Num() ; ResourceIndex++)
		{
			FFlareFactoryResource* Resource = &CycleCostCache.InputResources[ResourceIndex];
			Resource->Quantity *= CycleCostCacheLevel;
		}

		CycleCostCache.OutputResources = FactoryDescription->CycleCost.OutputResources;
		for (int32 ResourceIndex = 0 ; ResourceIndex < CycleCostCache.OutputResources.Num() ; ResourceIndex++)
		{
			FFlareFactoryResource* Resource = &CycleCostCache.OutputResources[ResourceIndex];
			Resource->Quantity *= CycleCostCacheLevel;
		}
		return CycleCostCache;
	}
}

const FFlareProductionData& UFlareFactory::GetCycleDataForShipClass(FName Class)
{
	return GetGame()->GetSpacecraftCatalog()->Get(Class)->CycleCost;
}

bool UFlareFactory::IsShipyard() const
{
	return GetDescription()->IsShipyard();
}

bool UFlareFactory::IsSmallShipyard() const
{
	return IsShipyard() && !GetDescription()->Identifier.ToString().Contains("large");
}

bool UFlareFactory::IsLargeShipyard() const
{
	return IsShipyard() && GetDescription()->Identifier.ToString().Contains("large");
}

uint32 UFlareFactory::GetProductionCost(const FFlareProductionData* Data)
{
	const FFlareProductionData* CycleData = Data ? Data : &GetCycleData();
	FCHECK(CycleData);

	ScaledProductionCost = CycleData->ProductionCost;
	
	return ScaledProductionCost;
}

int64 UFlareFactory::GetRemainingProductionDuration()
{
	return GetProductionTime(GetCycleData()) - FactoryData.ProductedDuration;
}

TArray<FFlareFactoryResource> UFlareFactory::GetLimitedOutputResources()
{
	UFlareCargoBay* CargoBay = Parent->GetCargoBay();
	TArray<FFlareFactoryResource> OutputResources = GetCycleData().OutputResources;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		int32 MaxCapacity = CargoBay->GetSlotCapacity() * FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier);
		int32 MaxAddition;
		int32 CurrentQuantity = CargoBay->GetResourceQuantity(Resource, GetParent()->GetCompany());

		if (CurrentQuantity >= MaxCapacity)
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

int32 UFlareFactory::GetOutputResourcesCount()
{
	return GetCycleData().OutputResources.Num();
}

FFlareResourceDescription* UFlareFactory::GetInputResource(int32 Index)
{
	return &GetCycleData().InputResources[Index].Resource->Data;
}

FFlareResourceDescription* UFlareFactory::GetOutputResource(int32 Index)
{
	return &GetCycleData().OutputResources[Index].Resource->Data;
}

uint32 UFlareFactory::GetInputResourceQuantity(int32 Index)
{
	return GetCycleData().InputResources[Index].Quantity;
}

uint32 UFlareFactory::GetOutputResourceQuantity(int32 Index)
{
	return GetCycleData().OutputResources[Index].Quantity;
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

uint32 UFlareFactory::GetInputResourceQuantity(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return GetCycleData().InputResources[ResourceIndex].Quantity;
		}
	}

	return 0;
}

uint32 UFlareFactory::GetOutputResourceQuantity(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().OutputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return GetCycleData().OutputResources[ResourceIndex].Quantity;
		}
	}

	return 0;
}


FText UFlareFactory::GetFactoryCycleCost(const FFlareProductionData* Data)
{
	FText ProductionCostText;
	FText CommaTextReference = LOCTEXT("Comma", " +");

	// Cycle cost in credits
	uint32 CycleProductionCost = GetProductionCost(Data);
	if (CycleProductionCost > 0)
	{
		ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
	}

	// Cycle cost in resources
	for (int ResourceIndex = 0; ResourceIndex < Data->InputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &Data->InputResources[ResourceIndex];
		FCHECK(FactoryResource);

		ProductionCostText = FText::Format(LOCTEXT("ProductionResourcesFormat", "{0}{1} {2} {3}"),
			ProductionCostText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return ProductionCostText;
}

FText UFlareFactory::GetFactoryCycleInfo()
{
	FText CommaTextReference = LOCTEXT("Comma", " +");
	FText ProductionOutputText;

	// No ship class selected
	if (IsShipyard() && FactoryData.TargetShipClass == NAME_None)
	{
		return LOCTEXT("SelectShipClass", "No ship in construction.");
	}

	FText ProductionCostText = GetFactoryCycleCost(&GetCycleData());

	// Cycle output in factory actions
	for (int ActionIndex = 0; ActionIndex < GetDescription()->OutputActions.Num(); ActionIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryAction* FactoryAction = &GetDescription()->OutputActions[ActionIndex];
		FCHECK(FactoryAction);

		switch (FactoryAction->Action)
		{
			// Ship production
			case EFlareFactoryAction::CreateShip:
				ProductionOutputText = FText::Format(LOCTEXT("CreateShipActionFormat", "{0}{1} {2} {3}"),
					ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity),
					GetGame()->GetSpacecraftCatalog()->Get(FactoryData.TargetShipClass)->Name);
				break;

			// Sector discovery
			case EFlareFactoryAction::DiscoverSector:
				ProductionOutputText = FText::Format(LOCTEXT("DiscoverSectorActionFormat", "{0}{1} sector"),
					ProductionOutputText, CommaText);
				break;

			// Research gain
			case EFlareFactoryAction::GainResearch:
				ProductionOutputText = FText::Format(LOCTEXT("GainResearchActionFormat", "{0}{1}{2} research"),
					ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity * Parent->GetLevel()));
				break;

			// Build station
			case EFlareFactoryAction::BuildStation:
				ProductionOutputText = LOCTEXT("BuildStationActionFormat", "finish station construction");
				break;

			default:
				FLOGV("SFlareShipMenu::UpdateFactoryLimitsList : Unimplemented factory action %d", (FactoryAction->Action + 0));
		}
	}

	// Cycle output in resources
	for (int ResourceIndex = 0; ResourceIndex < GetCycleData().OutputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &GetCycleData().OutputResources[ResourceIndex];
		FCHECK(FactoryResource);

		ProductionOutputText = FText::Format(LOCTEXT("ProductionOutputFormat", "{0}{1} {2} {3}"),
			ProductionOutputText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return FText::Format(LOCTEXT("FactoryCycleInfoFormat", "Production cycle : {0} \u2192 {1} in {2}"),
		ProductionCostText, ProductionOutputText,
		UFlareGameTools::FormatDate(GetProductionTime(GetCycleData()), 2));
}

FText UFlareFactory::GetFactoryStatus()
{
	FText ProductionStatusText;
	UFlareWorld* GameWorld = Game->GetGameWorld();


	if (IsActive())
	{
		if (!IsNeedProduction())
		{
			ProductionStatusText = LOCTEXT("ProductionNotNeeded", "Production not needed");
		}
		else if (HasCostReserved())
		{
			FText HasFreeSpace = HasOutputFreeSpace() ? FText() : LOCTEXT("ProductionNoSpace", ", not enough space");

			// Shipyards are a special case
			if (IsShipyard() && GetTargetShipClass() != NAME_None)
			{
				ProductionStatusText = FText::Format(LOCTEXT("ShipProductionInProgressFormat", "Building {0} for {1} ({2}{3})"),
					Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
					GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName(),
					UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2),
					HasFreeSpace);
			}
			else
			{
				ProductionStatusText = FText::Format(LOCTEXT("RegularProductionInProgressFormat", "Producing ({0}{1})"),
					UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2),
					HasFreeSpace);
			}
		}
		else if (HasInputMoney() && HasInputResources())
		{
			if (IsShipyard() && GetTargetShipClass() != NAME_None)
			{
				ProductionStatusText = FText::Format(LOCTEXT("ShipProductionWillStartFormat", "Starting building {0} for {1}"),
					Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
					GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
			}
			else
			{
				ProductionStatusText = LOCTEXT("ProductionWillStart", "Starting");
			}
		}
		else
		{
			if (!HasInputMoney())
			{
				if (IsShipyard() && GetTargetShipClass() != NAME_None)
				{
					ProductionStatusText = FText::Format(LOCTEXT("ShipProductionNotEnoughMoneyFormat", "Waiting for credits to build {0} for {1}"),
						Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
						GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
				}
				else
				{
					ProductionStatusText = LOCTEXT("ProductionNotEnoughMoney", "Waiting for credits");
				}
			}

			if (!HasInputResources())
			{
				if (IsShipyard() && GetTargetShipClass() != NAME_None)
				{
					ProductionStatusText = FText::Format(LOCTEXT("ShipProductionNotEnoughResourcesFormat", "Waiting for resources to build {0} for {1}"),
						Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
						GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
				}
				else
				{
					ProductionStatusText = LOCTEXT("ProductionNotEnoughResources", "Waiting for resources");
				}

			}


		}
	}
	else if (IsPaused())
	{
		ProductionStatusText = FText::Format(LOCTEXT("ProductionPaused", "Paused ({0} to completion)"),
			UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2));
	}
	else
	{
		ProductionStatusText = LOCTEXT("ProductionStopped", "Stopped");
	}

	return ProductionStatusText;
}

bool UFlareFactory::IsProducing()
{
	if (IsActive())
	{
		if (!IsNeedProduction())
		{
			return false;
		}
		else if (HasCostReserved())
		{
			if(HasOutputFreeSpace())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (HasInputMoney() && HasInputResources())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}


float UFlareFactory::GetMarginRatio()
{
	int64 SellPrice = 0;

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().OutputResources[ResourceIndex];

		SellPrice += Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;
	}

	if(SellPrice == 0)
	{
		return 0;
	}

	float Margin = (float) GetProductionBalance() / (float) SellPrice;

	return Margin;
}

int64 UFlareFactory::GetProductionBalance()
{
	// Factory rentability

	int64 Balance = 0;
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];

		Balance -= Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;
	}

	Balance -= GetProductionCost();

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().OutputResources[ResourceIndex];

		Balance += Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;
	}

	return Balance;
}

float UFlareFactory::GetProductionMalus(float Efficiency)
{
	return (1.f + (1.f - Efficiency) * (MAX_DAMAGE_MALUS - 1.f));
}

int64 UFlareFactory::GetProductionTime(const struct FFlareProductionData& Cycle)
{
	float Malus = GetProductionMalus(Parent->GetStationEfficiency());
	int64 ProductionTime = FMath::FloorToInt(Cycle.ProductionTime * Malus);

	if(FactoryDescription->IsTelescope())
	{
		return FMath::Max(int64(1), ProductionTime / Parent->GetLevel());
	}
	else
	{
		return ProductionTime;
	}
}


#undef LOCTEXT_NAMESPACE
