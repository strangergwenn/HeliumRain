
#include "../Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareWorld.h"
#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"
#include "FlareSimulatedSpacecraft.h"


#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraft"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraft::UFlareSimulatedSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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

	// Initialize navigation system
	NavigationSystem = NewObject<UFlareSimulatedSpacecraftNavigationSystem>(this, UFlareSimulatedSpacecraftNavigationSystem::StaticClass());
	NavigationSystem->Initialize(this, &SpacecraftData);

	// Initialize docking system
	DockingSystem = NewObject<UFlareSimulatedSpacecraftDockingSystem>(this, UFlareSimulatedSpacecraftDockingSystem::StaticClass());
	DockingSystem->Initialize(this, &SpacecraftData);

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
}

FFlareSpacecraftSave* UFlareSimulatedSpacecraft::Save()
{
	SpacecraftData.FactoryStates.Empty();
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		SpacecraftData.FactoryStates.Add(*Factories[FactoryIndex]->Save());
	}

	SpacecraftData.Cargo = *CargoBay->Save();

	return &SpacecraftData;
}


UFlareCompany* UFlareSimulatedSpacecraft::GetCompany()
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
	return IFlareSpacecraftInterface::IsMilitary(SpacecraftDescription);
}

bool UFlareSimulatedSpacecraft::IsStation() const
{
	return IFlareSpacecraftInterface::IsStation(SpacecraftDescription);
}


bool UFlareSimulatedSpacecraft::CanFight() const
{
	return GetDamageSystem()->IsAlive() && IsMilitary() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0;
}

bool UFlareSimulatedSpacecraft::CanTravel() const
{
	return GetDamageSystem()->IsAlive() && GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) > 0;
}


FName UFlareSimulatedSpacecraft::GetImmatriculation() const
{
	return SpacecraftData.Immatriculation;
}

UFlareSimulatedSpacecraftDamageSystem* UFlareSimulatedSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}

UFlareSimulatedSpacecraftNavigationSystem* UFlareSimulatedSpacecraft::GetNavigationSystem() const
{
	return NavigationSystem;
}

UFlareSimulatedSpacecraftDockingSystem* UFlareSimulatedSpacecraft::GetDockingSystem() const
{
	return DockingSystem;
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
	else if (CurrentSector == NULL)
	{
		OutInfo = FText::Format(LOCTEXT("CantTravelFormat", "Can't fly during travel ({0})"), FText::FromName(GetImmatriculation()));
		return false;
	}
	else if (IsAssignedToSector())
	{
		OutInfo = FText::Format(LOCTEXT("CantAssignedFormat", "Can't fly if assigned ({0})"), FText::FromName(GetImmatriculation()));
		return false;
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
	if (Sector)
	{
		GetCompany()->VisitSector(Sector);
	}
	else
	{
		// Un assign the ship
		AssignToSector(false);
	}
}

void UFlareSimulatedSpacecraft::AssignToSector(bool Assign)
{
	// TODO Extract the ship from the fleet !

	if (CurrentSector == NULL)
	{
		SpacecraftData.IsAssigned = false;
	}
	else
	{
		SpacecraftData.IsAssigned = Assign;
	}

	if (Assign && GetCurrentFleet()) {
		GetCurrentFleet()->RemoveShip(this);
	}

	if (!Assign && !GetCurrentFleet())
	{
		// Add in automatic fleet
		GetCompany()->CreateAutomaticFleet(this);
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

	// Check if both spacecraft are not at war
	if(GetCompany()->GetWarState(OtherSpacecraft->GetCompany()) == EFlareHostility::Hostile)
	{
		return false;
	}

	return true;
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


#undef LOCTEXT_NAMESPACE
