
#include "../Flare.h"

#include "FlareStationDock.h"
#include "FlareStation.h"

#include "../Ships/FlareShip.h"
#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareStation::AFlareStation(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Create static mesh component
	HullRoot = PCIP.CreateDefaultSubobject<UFlareShipModule>(this, TEXT("HullRoot"));
	HullRoot->SetSimulatePhysics(false);
	RootComponent = HullRoot;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareStation::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}

void AFlareStation::Initialize()
{
	if (DockingSlots.Num() == 0)
	{
		// Dock data
		int32 Count = 0;
		TArray<UActorComponent*> ActorComponents;
		GetComponents(ActorComponents);

		// Fill all dock slots
		for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
		{
			UFlareStationDock* Module = Cast<UFlareStationDock>(*ComponentIt);
			if (Module)
			{
				// Get data
				FVector DockLocation;
				FRotator DockRotation;
				Module->GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

				// Fill info
				FFlareDockingInfo Info;
				Info.Rotation = DockRotation;
				Info.EndPoint = DockLocation;
				Info.StartPoint = Info.EndPoint;
				Info.DockId = Count;
				Info.Station = this;
				Info.Granted = false;
				Info.Occupied = false;

				// Push this slot
				DockingSlots.Add(Info);
				Count++;
			}
		}
	}
}

void AFlareStation::Destroyed()
{
	Company->Unregister(this);
}


/*----------------------------------------------------
	Station interface
----------------------------------------------------*/

void AFlareStation::Load(const FFlareStationSave& Data)
{
	// Data
	StationData = Data;
	StationData.Name = FName(*GetName());
	StationDescription = GetGame()->GetStationCatalog()->Get(Data.Identifier);

	// Look for parent company
	SetOwnerCompany(GetGame()->FindCompany(Data.CompanyIdentifier));

	// Customization
	UpdateCustomization();
}

FFlareStationSave* AFlareStation::Save()
{
	StationData.Location = GetActorLocation();
	StationData.Rotation = GetActorRotation();

	return &StationData;
}

void AFlareStation::SetOwnerCompany(UFlareCompany* NewCompany)
{
	SetCompany(NewCompany);
	StationData.CompanyIdentifier = NewCompany->GetIdentifier();
	NewCompany->Register(this);
}

UFlareCompany* AFlareStation::GetCompany()
{
	return Company;
}

FFlareDockingInfo AFlareStation::RequestDock(IFlareShipInterface* Ship)
{
	FLOGV("AFlareStation::RequestDock ('%s')", *Ship->_getUObject()->GetName());

	// Looking for slot
	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (!DockingSlots[i].Granted)
		{
			FLOGV("AFlareStation::RequestDock : found valid dock %d", i);
			DockingSlots[i].Granted = true;
			DockingSlots[i].Ship = Ship;
			return DockingSlots[i];
		}
	}

	// Default values
	FFlareDockingInfo Info;
	Info.Granted = false;
	Info.Station = this;
	return Info;
}

void AFlareStation::ReleaseDock(IFlareShipInterface* Ship, int32 DockId)
{
	FLOGV("AFlareStation::ReleaseDock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	DockingSlots[DockId].Granted = false;
	DockingSlots[DockId].Occupied = false;
	DockingSlots[DockId].Ship = NULL;
}

void AFlareStation::Dock(IFlareShipInterface* Ship, int32 DockId)
{
	FLOGV("AFlareStation::Dock %d ('%s')", DockId, *Ship->_getUObject()->GetName());
	DockingSlots[DockId].Granted = true;
	DockingSlots[DockId].Occupied = true;
	DockingSlots[DockId].Ship = Ship;
}

TArray<IFlareShipInterface*> AFlareStation::GetDockedShips()
{
	TArray<IFlareShipInterface*> Result;

	for (int32 i = 0; i < DockingSlots.Num(); i++)
	{
		if (DockingSlots[i].Granted)
		{
			FLOGV("AFlareStation::GetDockedShips : found valid dock %d", i);
			Result.AddUnique(DockingSlots[i].Ship);
		}
	}

	return Result;
}
