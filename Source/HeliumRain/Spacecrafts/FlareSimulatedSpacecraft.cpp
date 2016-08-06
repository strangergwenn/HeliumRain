
#include "../Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareWorld.h"
#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"
#include "../Data/FlareFactoryCatalogEntry.h"
#include "FlareSimulatedSpacecraft.h"


#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraft"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraft::UFlareSimulatedSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveSpacecraft = NULL;
}


void UFlareSimulatedSpacecraft::Load(const FFlareSpacecraftSave& Data)
{
	Game = Cast<UFlareCompany>(GetOuter())->GetGame();
	SpacecraftData = Data;

	// Load spacecraft description
	SpacecraftDescription = Game->GetSpacecraftCatalog()->Get(Data.Identifier);

	// Initialize damage system
	DamageSystem = NewObject<UFlareSimulatedSpacecraftDamageSystem>(this, UFlareSimulatedSpacecraftDamageSystem::StaticClass());
	DamageSystem->Initialize(this, &SpacecraftData);

	// Initialize weapons system
	WeaponsSystem = NewObject<UFlareSimulatedSpacecraftWeaponsSystem>(this, UFlareSimulatedSpacecraftWeaponsSystem::StaticClass());
	WeaponsSystem->Initialize(this, &SpacecraftData);

	Game->GetGameWorld()->ClearFactories(this);
	Factories.Empty();

	// Load factories
	for (int FactoryIndex = 0; FactoryIndex < SpacecraftDescription->Factories.Num(); FactoryIndex++)
	{
		FFlareFactorySave FactoryData;
		FFlareFactoryDescription* FactoryDescription = &SpacecraftDescription->Factories[FactoryIndex]->Data;

		if (FactoryIndex < SpacecraftData.FactoryStates.Num())
		{
			FactoryData = SpacecraftData.FactoryStates[FactoryIndex];
		}
		else
		{
				FactoryData.Active = FactoryDescription->AutoStart;
				FactoryData.CostReserved = 0;
				FactoryData.ProductedDuration = 0;
				FactoryData.InfiniteCycle = true;
				FactoryData.CycleCount = 0;
				FactoryData.TargetShipClass = NAME_None;
				FactoryData.TargetShipCompany = NAME_None;
				FactoryData.OrderShipClass = NAME_None;
				FactoryData.OrderShipCompany = NAME_None;
				FactoryData.OrderShipAdvancePayment = 0;
		}

		UFlareFactory* Factory = NewObject<UFlareFactory>(GetGame()->GetGameWorld(), UFlareFactory::StaticClass());
		Factory->Load(this, FactoryDescription, FactoryData);
		Factories.Add(Factory);
		Game->GetGameWorld()->AddFactory(Factory);
	}

	CargoBay = NewObject<UFlareCargoBay>(this, UFlareCargoBay::StaticClass());
	CargoBay->Load(this, SpacecraftData.Cargo);


	// Lock resources
	LockResources();

	if(ActiveSpacecraft)
	{
		ActiveSpacecraft->Load(this);
	}
}

FFlareSpacecraftSave* UFlareSimulatedSpacecraft::Save()
{
	SpacecraftData.FactoryStates.Empty();
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		SpacecraftData.FactoryStates.Add(*Factories[FactoryIndex]->Save());
	}

	SpacecraftData.Cargo = *CargoBay->Save();

	if(IsActive())
	{
		GetActive()->Save();
	}

	return &SpacecraftData;
}


UFlareCompany* UFlareSimulatedSpacecraft::GetCompany() const
{
	// TODO Cache
	return Game->GetGameWorld()->FindCompany(SpacecraftData.CompanyIdentifier);
}


EFlarePartSize::Type UFlareSimulatedSpacecraft::GetSize()
{
	return SpacecraftDescription->Size;
}

bool UFlareSimulatedSpacecraft::IsMilitary() const
{
	return FFlareSpacecraftDescription::IsMilitary(SpacecraftDescription);
}

bool UFlareSimulatedSpacecraft::IsStation() const
{
	return FFlareSpacecraftDescription::IsStation(SpacecraftDescription);
}


bool UFlareSimulatedSpacecraft::CanFight() const
{
	return GetDamageSystem()->IsAlive() && IsMilitary() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
}

bool UFlareSimulatedSpacecraft::CanTravel() const
{
	return !IsTrading() && GetDamageSystem()->IsAlive() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) > 0;
}


FName UFlareSimulatedSpacecraft::GetImmatriculation() const
{
	return SpacecraftData.Immatriculation;
}

UFlareSimulatedSpacecraftDamageSystem* UFlareSimulatedSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}

UFlareSimulatedSpacecraftWeaponsSystem* UFlareSimulatedSpacecraft::GetWeaponsSystem() const
{
	return WeaponsSystem;
}

void UFlareSimulatedSpacecraft::SetSpawnMode(EFlareSpawnMode::Type SpawnMode)
{
	SpacecraftData.SpawnMode = SpawnMode;
}

bool UFlareSimulatedSpacecraft::CanBeFlown(FText& OutInfo) const
{
	if (IsStation())
	{
		return false;
	}
	else if (!IsActive())
	{
		OutInfo = LOCTEXT("CantFlyDistantFormat", "You can't fly a ship from another sector");
	}

	return true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareSimulatedSpacecraft::SetCurrentSector(UFlareSimulatedSector* Sector)
{
	CurrentSector = Sector;

	// Mark the sector as visited
	if (!Sector->IsTravelSector())
	{
		GetCompany()->VisitSector(Sector);
	}
}


/*----------------------------------------------------
	Resources
----------------------------------------------------*/


bool UFlareSimulatedSpacecraft::CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft)
{
	// Check if both spacecraft are in the same sector
	if(GetCurrentSector() != OtherSpacecraft->GetCurrentSector())
	{
		return false;
	}

	// Check if spacecraft are not both stations
	if(IsStation() && OtherSpacecraft->IsStation())
	{
		return false;
	}

	// Check if spacecraft are not both ships
	if(!IsStation() && !OtherSpacecraft->IsStation())
	{
		return false;
	}

	// Check if spacecraft are are not already trading
	if(IsTrading() || OtherSpacecraft->IsTrading())
	{
		return false;
	}

	// Check if both spacecraft are not at war
	if(GetCompany()->GetWarState(OtherSpacecraft->GetCompany()) == EFlareHostility::Hostile)
	{
		return false;
	}

	return true;
}

EFlareResourcePriceContext::Type UFlareSimulatedSpacecraft::GetResourceUseType(FFlareResourceDescription* Resource)
{
	// Check we're and station
	if (!IsStation())
	{
		return EFlareResourcePriceContext::Default;
	}

	// Parse factories
	for (int FactoryIndex = 0; FactoryIndex < SpacecraftDescription->Factories.Num(); FactoryIndex++)
	{
		FFlareFactoryDescription* FactoryDescription = &SpacecraftDescription->Factories[FactoryIndex]->Data;

		// Is input resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryInput;
			}
		}

		// Is output resource of a station ?
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* FactoryResource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
			if (&FactoryResource->Resource->Data == Resource)
			{
				return EFlareResourcePriceContext::FactoryOutput;
			}
		}
	}

	// Customer resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) && GetGame()->GetResourceCatalog()->IsCustomerResource(Resource))
	{
		return EFlareResourcePriceContext::ConsumerConsumption;
	}

	// Maintenance resource ?
	if (SpacecraftDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance) && GetGame()->GetResourceCatalog()->IsMaintenanceResource(Resource))
	{
		return EFlareResourcePriceContext::MaintenanceConsumption;
	}

	return EFlareResourcePriceContext::Default;
}

void UFlareSimulatedSpacecraft::LockResources()
{
	GetCargoBay()->UnlockAll();

	if (GetDescription()->Factories.Num() > 0)
	{
		FFlareFactoryDescription* Factory = &GetDescription()->Factories[0]->Data;

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->CycleCost.InputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->CycleCost.InputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Input, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}

		for (int32 ResourceIndex = 0 ; ResourceIndex < Factory->CycleCost.OutputResources.Num() ; ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &Factory->CycleCost.OutputResources[ResourceIndex];

			if (!GetCargoBay()->LockSlot(&Resource->Resource->Data, EFlareResourceLock::Output, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *(&Resource->Resource->Data)->Name.ToString(), *GetImmatriculation().ToString());
			}
		}
	}

	if (HasCapability(EFlareSpacecraftCapability::Consumer))
	{
		for (int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			if (!GetCargoBay()->LockSlot(Resource, EFlareResourceLock::Input, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *Resource->Name.ToString(), *GetImmatriculation().ToString());
			}
		}
	}

	if (HasCapability(EFlareSpacecraftCapability::Maintenance))
	{
		for (int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;

			if (!GetCargoBay()->LockSlot(Resource, EFlareResourceLock::Input, false))
			{
				FLOGV("Fail to lock a slot of %s in %s", *Resource->Name.ToString(), *GetImmatriculation().ToString());
			}
		}
	}
}

void UFlareSimulatedSpacecraft::SetAsteroidData(FFlareAsteroidSave* Data)
{
	SpacecraftData.AsteroidData.Identifier = Data->Identifier;
	SpacecraftData.AsteroidData.AsteroidMeshID = Data->AsteroidMeshID;
	SpacecraftData.AsteroidData.Scale = Data->Scale;
	SpacecraftData.Location = Data->Location;
	SpacecraftData.Rotation = Data->Rotation;
}

void UFlareSimulatedSpacecraft::SetDynamicComponentState(FName Identifier, float Progress)
{
	SpacecraftData.DynamicComponentStateIdentifier = Identifier;
	SpacecraftData.DynamicComponentStateProgress = Progress;
}

void UFlareSimulatedSpacecraft::ForceUndock()
{
	SpacecraftData.DockedTo = NAME_None;
	SpacecraftData.DockedAt = -1;
}

void UFlareSimulatedSpacecraft::SetTrading(bool Trading)
{
	if(IsStation())
	{
		FLOGV("Fail to set trading state to %s : station are never locked in trading state", *GetImmatriculation().ToString());
		return;
	}
	SpacecraftData.IsTrading = Trading;
}

EFlareHostility::Type UFlareSimulatedSpacecraft::GetPlayerWarState() const
{
	return GetCompany()->GetPlayerWarState();
}

const FSlateBrush* FFlareSpacecraftDescription::GetIcon(FFlareSpacecraftDescription* Characteristic)
{
	if (Characteristic)
	{
		if (FFlareSpacecraftDescription::IsStation(Characteristic))
		{
			return FFlareStyleSet::GetIcon("SS");
		}
		else if (FFlareSpacecraftDescription::IsMilitary(Characteristic))
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("MS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("MM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("ML");
			}
		}
		else
		{
			if (Characteristic->Size == EFlarePartSize::S)
			{
				return FFlareStyleSet::GetIcon("CS");
			}
			else if (Characteristic->Size == EFlarePartSize::M)
			{
				return FFlareStyleSet::GetIcon("CM");
			}
			else if (Characteristic->Size == EFlarePartSize::L)
			{
				return FFlareStyleSet::GetIcon("CL");
			}
		}
	}
	return NULL;
}

bool FFlareSpacecraftDescription::IsStation(FFlareSpacecraftDescription* SpacecraftDesc)
{
	return SpacecraftDesc->OrbitalEngineCount == 0;
}

bool FFlareSpacecraftDescription::IsMilitary(FFlareSpacecraftDescription* SpacecraftDesc)
{
	return SpacecraftDesc->GunSlots.Num() > 0 || SpacecraftDesc->TurretSlots.Num() > 0;
}

#undef LOCTEXT_NAMESPACE
