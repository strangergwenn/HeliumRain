
#include "../Flare.h"
#include "FlareSimulatedSector.h"
#include "FlareGame.h"
#include "FlareWorld.h"
#include "FlareFleet.h"
#include "../Economy/FlareCargoBay.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSimulatedSector"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSector::UFlareSimulatedSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PersistentStationIndex = 0;
}

void UFlareSimulatedSector::Load(const FFlareSectorDescription* Description, const FFlareSectorSave& Data, const FFlareSectorOrbitParameters& OrbitParameters)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();


	SectorData = Data;
	SectorDescription = Description;
	SectorOrbitParameters = OrbitParameters;
	SectorShips.Empty();
	SectorShipInterfaces.Empty();
	SectorStations.Empty();
	SectorStationInterfaces.Empty();
	SectorSpacecrafts.Empty();
	SectorFleets.Empty();

	LoadPeople(SectorData.PeopleData);

	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);
		if (Spacecraft->IsStation())
		{
			SectorStations.Add(Spacecraft);
			SectorStationInterfaces.Add(Spacecraft);
		}
		else
		{
			SectorShips.Add(Spacecraft);
			SectorShipInterfaces.Add(Spacecraft);
		}
		SectorSpacecrafts.Add(Spacecraft);
		Spacecraft->SetCurrentSector(this);
	}


	for (int i = 0 ; i < SectorData.FleetIdentifiers.Num(); i++)
	{
		UFlareFleet* Fleet = Game->GetGameWorld()->FindFleet(SectorData.FleetIdentifiers[i]);
		SectorFleets.Add(Fleet);
		Fleet->SetCurrentSector(this);
	}
}

UFlarePeople* UFlareSimulatedSector::LoadPeople(const FFlarePeopleSave& PeopleData)
{
	// Create the new people
	People = NewObject<UFlarePeople>(this, UFlarePeople::StaticClass());
	People->Load(this, PeopleData);

	return People;
}

FFlareSectorSave* UFlareSimulatedSector::Save()
{
	SectorData.SpacecraftIdentifiers.Empty();
	SectorData.FleetIdentifiers.Empty();

	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		SectorData.SpacecraftIdentifiers.Add(SectorSpacecrafts[i]->GetImmatriculation());
	}

	for (int i = 0 ; i < SectorFleets.Num(); i++)
	{
		SectorData.FleetIdentifiers.Add(SectorFleets[i]->GetIdentifier());
	}

	SectorData.PeopleData = *People->Save();

	return &SectorData;
}


UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(StationClass);
	UFlareSimulatedSpacecraft* Station = NULL;

	// Invalid desc ? Get a new one
	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if (Desc)
	{
		Station = CreateShip(Desc, Company, TargetPosition, TargetRotation);

		// Needs an esteroid ? 
		if (Station && Desc->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid))
		{
			AttachStationToAsteroid(Station);
		}
	}

	return Station;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, Company, TargetPosition);
	}
	else
	{
		FLOGV("CreateShip failed: Unkwnon ship %s", *ShipClass.ToString());
	}

	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;

	// Default data
	FFlareSpacecraftSave ShipData;
	ShipData.Location = TargetPosition;
	ShipData.Rotation = TargetRotation;
	ShipData.LinearVelocity = FVector::ZeroVector;
	ShipData.AngularVelocity = FVector::ZeroVector;
	ShipData.SpawnMode = EFlareSpawnMode::Spawn;
	Game->Immatriculate(Company, ShipDescription->Identifier, &ShipData);
	ShipData.Identifier = ShipDescription->Identifier;
	ShipData.Heat = 600 * ShipDescription->HeatCapacity;
	ShipData.PowerOutageDelay = 0;
	ShipData.PowerOutageAcculumator = 0;
	ShipData.IsAssigned = false;
	ShipData.DynamicComponentStateIdentifier = NAME_None;
	ShipData.DynamicComponentStateProgress = 0.f;

	if (ShipDescription->DynamicComponentStates.Num() > 0)
	{
		ShipData.DynamicComponentStateIdentifier = ShipDescription->DynamicComponentStates[0].StateIdentifier;
	}

	FName RCSIdentifier;
	FName OrbitalEngineIdentifier;

	// Size selector
	if (ShipDescription->Size == EFlarePartSize::S)
	{
		RCSIdentifier = FName("rcs-coral");
		OrbitalEngineIdentifier = FName("engine-octopus");
	}
	else if (ShipDescription->Size == EFlarePartSize::L)
	{
		RCSIdentifier = FName("rcs-rift");
		OrbitalEngineIdentifier = FName("pod-surtsey");
	}
	else
	{
		// TODO
	}

	for (int32 i = 0; i < ShipDescription->RCSCount; i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = RCSIdentifier;
		ComponentData.ShipSlotIdentifier = FName(*("rcs-" + FString::FromInt(i)));
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->OrbitalEngineCount; i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = OrbitalEngineIdentifier;
		ComponentData.ShipSlotIdentifier = FName(*("engine-" + FString::FromInt(i)));
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->GunSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Game->GetDefaultWeaponIdentifier();
		ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
		ComponentData.Damage = 0.f;
		ComponentData.Weapon.FiredAmmo = 0;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->TurretSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Game->GetDefaultTurretIdentifier();
		ComponentData.ShipSlotIdentifier = ShipDescription->TurretSlots[i].SlotIdentifier;
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;
		ComponentData.Weapon.FiredAmmo = 0;
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->InternalComponentSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = ShipDescription->InternalComponentSlots[i].ComponentIdentifier;
		ComponentData.ShipSlotIdentifier = ShipDescription->InternalComponentSlots[i].SlotIdentifier;
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	// Init pilot
	ShipData.Pilot.Identifier = "chewie";
	ShipData.Pilot.Name = "Chewbacca";

	// Init company
	ShipData.CompanyIdentifier = Company->GetIdentifier();

	// Asteroid info
	ShipData.AsteroidData.Identifier = NAME_None;
	ShipData.AsteroidData.AsteroidMeshID = 0;
	ShipData.AsteroidData.Scale = FVector(1, 1, 1);

	// Create the ship
	Spacecraft = Company->LoadSpacecraft(ShipData);
	if (Spacecraft->IsStation())
	{
		SectorStations.Add(Spacecraft);
		SectorStationInterfaces.Add(Spacecraft);
	}
	else
	{
		SectorShips.Add(Spacecraft);
		SectorShipInterfaces.Add(Spacecraft);
	}
	SectorSpacecrafts.Add(Spacecraft);

	Spacecraft->SetCurrentSector(this);

	FLOGV("UFlareSimulatedSector::CreateShip : Created ship '%s' at %s", *Spacecraft->GetImmatriculation().ToString(), *TargetPosition.ToString());

	// TODO remove automatic fleet creation
	if (!Spacecraft->IsStation())
	{
		UFlareFleet* NewFleet = Company->CreateFleet(FText::FromString(Spacecraft->GetImmatriculation().ToString()), Spacecraft->GetCurrentSector());
		NewFleet->AddShip(Spacecraft);

		// If the ship is in the player company, select the new fleet
		if (Game->GetPC()->GetCompany() == Company)
		{
			Game->GetPC()->SelectFleet(NewFleet);
		}
	}

	return Spacecraft;
}

void UFlareSimulatedSector::CreateAsteroid(int32 ID, FName Name, FVector Location)
{
	if (ID < 0 || ID >= Game->GetAsteroidCatalog()->Asteroids.Num())
	{
		FLOGV("UFlareSimulatedSector::CreateAsteroid : Can't find ID %d", ID);
		return;
	}

	// Compute size
	float MinSize = 0.4;
	float MinMaxSize = 0.8;
	float MaxMaxSize = 1.6;
	float MaxSize = FMath::Lerp(MinMaxSize, MaxMaxSize, FMath::Clamp(Location.Size() / 100000.0f, 0.0f, 1.0f));
	float Size = FMath::FRandRange(MinSize, MaxSize);

	// Write data
	FFlareAsteroidSave Data;
	Data.AsteroidMeshID = ID;
	Data.Identifier = Name;
	Data.LinearVelocity = FVector::ZeroVector;
	Data.AngularVelocity = FMath::VRand() * FMath::FRandRange(-1.f,1.f);
	Data.Scale = FVector(1,1,1) * Size;
	Data.Rotation = FRotator(FMath::FRandRange(0,360), FMath::FRandRange(0,360), FMath::FRandRange(0,360));
	Data.Location = Location;

	SectorData.AsteroidData.Add(Data);
}

void UFlareSimulatedSector::AddFleet(UFlareFleet* Fleet)
{
	SectorFleets.Add(Fleet);

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		Fleet->GetShips()[ShipIndex]->SetCurrentSector(this);
		SectorShips.AddUnique(Fleet->GetShips()[ShipIndex]);
		SectorShipInterfaces.AddUnique(Fleet->GetShips()[ShipIndex]);
		SectorSpacecrafts.AddUnique(Fleet->GetShips()[ShipIndex]);
	}
}

void UFlareSimulatedSector::DisbandFleet(UFlareFleet* Fleet)
{
	if (SectorFleets.Remove(Fleet) == 0)
	{
        FLOGV("UFlareSimulatedSector::DisbandFleet : Disband fail. Fleet '%s' is not in sector '%s'", *Fleet->GetFleetName().ToString(), *GetSectorName().ToString())
		return;
	}
}

void UFlareSimulatedSector::RetireFleet(UFlareFleet* Fleet)
{
	FLOGV("UFlareSimulatedSector::RetireFleet %s", *Fleet->GetFleetName().ToString());
	if (SectorFleets.Remove(Fleet) == 0)
	{
		FLOGV("UFlareSimulatedSector::RetireFleet : RetireFleet fail. Fleet '%s' is not in sector '%s'", *Fleet->GetFleetName().ToString(), *GetSectorName().ToString())
		return;
	}

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		Fleet->GetShips()[ShipIndex]->SetCurrentSector(NULL);
		if (RemoveSpacecraft(Fleet->GetShips()[ShipIndex]) == 0)
		{
			FLOGV("UFlareSimulatedSector::RetireFleet : RetireFleet fail. Ship '%s' is not in sector '%s'", *Fleet->GetShips()[ShipIndex]->GetImmatriculation().ToString(), *GetSectorName().ToString())
		}
	}
}

int UFlareSimulatedSector::RemoveSpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	SectorSpacecrafts.Remove(Spacecraft);
	SectorShipInterfaces.Remove(Spacecraft);
	return SectorShips.Remove(Spacecraft);
}

void UFlareSimulatedSector::SetShipToFly(UFlareSimulatedSpacecraft* Ship)
{
	SectorData.LastFlownShip = Ship->GetImmatriculation();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


FText UFlareSimulatedSector::GetSectorDescription() const
{
	return SectorDescription->Description;
}

bool UFlareSimulatedSector::CanBuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	// Does it needs sun
	if(StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure) && SectorDescription->IsSolarPoor)
	{
		return false;
	}

	// Does it needs not icy sector
	if(StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::HideOnIce) &&SectorDescription->IsIcy)
	{
		return false;
	}

	// Does it needs icy sector
	if(StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::HideOnNoIce) && !SectorDescription->IsIcy)
	{
		return false;
	}

	// Does it needs an geostationary orbit ?
	if(StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::GeostationaryOrbit) && !SectorDescription->IsGeostationary)
	{
		return false;
	}

	// Does it needs an asteroid ?
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid) && SectorData.AsteroidData.Num() == 0)
	{
		return false;
	}

	// Check money cost
	if (Company->GetMoney() < StationDescription->CycleCost.ProductionCost)
	{
		return false;
	}

	// First, it need a free cargo
	bool HasFreeCargo = false;
	for (int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

		if (Ship->GetCompany() != Company)
		{
			continue;
		}

		if (Ship->GetDescription()->CargoBayCount == 0)
		{
			// Not a cargo
			continue;
		}

		HasFreeCargo = true;
		break;
	}

	if (!HasFreeCargo)
	{
		return false;
	}


	// Compute total available resources
	TArray<FFlareCargo> AvailableResources;
	for (int SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorSpacecrafts[SpacecraftIndex];

		if (Spacecraft->GetCompany() != Company)
		{
			continue;
		}

		UFlareCargoBay* CargoBay = Spacecraft->GetCargoBay();
		for (uint32 CargoIndex = 0; CargoIndex < CargoBay->GetSlotCount(); CargoIndex++)
		{
			FFlareCargo* Cargo = CargoBay->GetSlot(CargoIndex);

			if (!Cargo->Resource)
			{
				continue;
			}

			bool NewResource = true;
			for (int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
			{
				if (AvailableResources[AvailableResourceIndex].Resource == Cargo->Resource)
				{
					AvailableResources[AvailableResourceIndex].Quantity += Cargo->Quantity;
					NewResource = false;
					break;
				}
			}

			if (NewResource)
			{
				FFlareCargo NewResourceCargo;
				NewResourceCargo.Resource = Cargo->Resource;
				NewResourceCargo.Quantity = Cargo->Quantity;
				AvailableResources.Add(NewResourceCargo);
			}
		}
	}

	// Check resource cost
	for (int32 ResourceIndex = 0; ResourceIndex < StationDescription->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &StationDescription->CycleCost.InputResources[ResourceIndex];
		bool ResourceFound = false;

		for (int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
		{
			if (AvailableResources[AvailableResourceIndex].Resource == &(FactoryResource->Resource->Data))
			{
				ResourceFound = true;
				if (AvailableResources[AvailableResourceIndex].Quantity < FactoryResource->Quantity)
				{
					return false;
				}
				break;
			}
		}

		if (!ResourceFound)
		{
			return false;
		}
	}

	return true;
}

bool UFlareSimulatedSector::BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	if (!CanBuildStation(StationDescription, Company))
	{
		FLOGV("UFlareSimulatedSector::BuildStation : Failed to buid station '%s' for company '%s'",
			*StationDescription->Identifier.ToString(), *Company->GetIdentifier().ToString());
		return false;
	}

	// Pay station cost
	if(!Company->TakeMoney(StationDescription->CycleCost.ProductionCost))
	{
		return false;
	}

	// Take resource cost
	for (int ResourceIndex = 0; ResourceIndex < StationDescription->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &StationDescription->CycleCost.InputResources[ResourceIndex];
		uint32 ResourceToTake = FactoryResource->Quantity;
		FFlareResourceDescription* Resource = &(FactoryResource->Resource->Data);


		// First take from ships
		for (int ShipIndex = 0; ShipIndex < SectorShips.Num() && ResourceToTake > 0; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Ship->GetCargoBay()->TakeResources(Resource, ResourceToTake);
		}

		if (ResourceToTake == 0)
		{
			continue;
		}

		// Then take useless resources from station
		ResourceToTake -= TakeUselessResources(Company, Resource, ResourceToTake);

		if (ResourceToTake == 0)
		{
			continue;
		}

		// Finally take from all stations
		for (int StationIndex = 0; StationIndex < SectorStations.Num() && ResourceToTake > 0; StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorShips[StationIndex];

			if (Station->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Station->GetCargoBay()->TakeResources(Resource, ResourceToTake);
		}

		if (ResourceToTake > 0)
		{
			FLOG("UFlareSimulatedSector::BuildStation : Failed to take resource cost for build station a station but CanBuild test succeded");
		}
	}

	UFlareSimulatedSpacecraft* Spacecraft = CreateStation(StationDescription->Identifier, Company, FVector::ZeroVector);

	return true;
}

void UFlareSimulatedSector::AttachStationToAsteroid(UFlareSimulatedSpacecraft* Spacecraft)
{
	FFlareAsteroidSave* AsteroidSave = NULL;
	float AsteroidSaveDistance = 100000000;
	int32 AsteroidSaveIndex = -1;

	// Take the center-est available asteroid
	for (int AsteroidIndex = 0; AsteroidIndex < SectorData.AsteroidData.Num(); AsteroidIndex++)
	{
		FFlareAsteroidSave* AsteroidCandidate = &SectorData.AsteroidData[AsteroidIndex];
		if (AsteroidCandidate->Location.Size() < AsteroidSaveDistance)
		{
			AsteroidSaveDistance = AsteroidCandidate->Location.Size();
			AsteroidSaveIndex = AsteroidIndex;
			AsteroidSave = AsteroidCandidate;
		}
	}

	// Found it
	if (AsteroidSave)
	{
		FLOGV("UFlareSimulatedSector::AttachStationToAsteroid : Found asteroid we need to attach to ('%s')", *AsteroidSave->Identifier.ToString());
		Spacecraft->SetAsteroidData(AsteroidSave);
		SectorData.AsteroidData.RemoveAt(AsteroidSaveIndex);
	}
	else
	{
		FLOG("UFlareSimulatedSector::AttachStationToAsteroid : Failed to use asteroid !");
	}
}

void UFlareSimulatedSector::SimulateTransport()
{
	TArray<uint32> CompanyRemainingTransportCapacity;

	// Company transport
	for (int CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
		uint32 TransportCapacity = GetTransportCapacity(Company);

		uint32 UsedCapacity = SimulateTransport(Company, TransportCapacity);

		CompanyRemainingTransportCapacity.Add(TransportCapacity - UsedCapacity);
	}

	SimulateTrade(CompanyRemainingTransportCapacity);
}

uint32 UFlareSimulatedSector::SimulateTransport(UFlareCompany* Company, uint32 InitialTranportCapacity)
{

	uint32 TransportCapacity = InitialTranportCapacity;

	if (TransportCapacity == 0)
	{
		// No transport
		return 0;
	}

	// TODO Store ouput resource from station in overflow to storage

	//FLOGV("Initial TransportCapacity=%u", TransportCapacity);

	if (PersistentStationIndex >= SectorStations.Num())
	{
		PersistentStationIndex = 0;
	}

	//FLOGV("PersistentStationIndex=%d", PersistentStationIndex);

	// TODO 5 pass:
	// 1 - fill resources consumers
	// 2 - one with the exact quantity
	// 3 - the second with the double
	// 4 - third with 1 slot alignemnt
	// 5 - a 4th with inactive stations
	// 6 - empty full output for station with no output space // TODO

	//1 - fill resources consumers
	FillResourceConsumers(Company, TransportCapacity, true);

	// 2 - one with the exact quantity
	if (TransportCapacity)
	{
		AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::Production, 1, true);
	}

	// 3 - the second with the double
	if (TransportCapacity)
	{
		AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::Production, 2, true);
	}

	// 4 - third with slot alignemnt
	if (TransportCapacity)
	{
		AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::CargoBay, 1, true);
	}

	// 5 - a 4th with inactive stations
	if (TransportCapacity)
	{
		AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::CargoBay, 1, false);
	}

	//FLOGV("SimulateTransport end TransportCapacity=%u", TransportCapacity);
	return InitialTranportCapacity - TransportCapacity;
}

void UFlareSimulatedSector::FillResourceConsumers(UFlareCompany* Company, uint32& TransportCapacity, bool AllowTrade)
{
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
		uint32 UnitSellPrice = 1.01 * GetResourcePrice(Resource);
		FLOGV("Distribute consumer ressource %s", *Resource->Name.ToString());


		// Transport consumer resources by priority
		for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorStations[CountIndex];

			if ((!AllowTrade && Station->GetCompany() != Company) || !Station->HasCapability(EFlareSpacecraftCapability::Consumer) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			FLOGV("Check station %s needs:", *Station->GetImmatriculation().ToString());


			// Fill only one slot for each ressource
			if (Station->GetCargoBay()->GetResourceQuantity(Resource) > Station->GetDescription()->CargoBayCapacity)
			{
				FLOGV("Fill only one slot for each ressource. Has %d", Station->GetCargoBay()->GetResourceQuantity(Resource));

				continue;
			}


			uint32 MaxQuantity = Station->GetDescription()->CargoBayCapacity - Station->GetCargoBay()->GetResourceQuantity(Resource);
			uint32 FreeSpace = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);
			uint32 QuantityToTransfert = FMath::Min(MaxQuantity, FreeSpace);
			QuantityToTransfert = FMath::Min(TransportCapacity, QuantityToTransfert);

			if(Station->GetCompany() != Company)
			{
				// Compute max quantity the station can afford
				uint32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
				QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
				FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
				FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
			}



			uint32 TakenResources = TakeUselessResources(Company, Resource, QuantityToTransfert, AllowTrade);
			Station->GetCargoBay()->GiveResources(Resource, TakenResources);
			TransportCapacity -= TakenResources;

			if(TakenResources > 0 && Station->GetCompany() != Company)
			{
				// Sell resources
				int32 TransactionAmount = TakenResources * UnitSellPrice;
				Station->GetCompany()->TakeMoney(TransactionAmount);
				Company->GiveMoney(TransactionAmount);
				FLOGV("%s	%u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
			}


			FLOGV("MaxQuantity %d", MaxQuantity);
			FLOGV("FreeSpace %d", FreeSpace);
			FLOGV("QuantityToTransfert %d", QuantityToTransfert);
			FLOGV("TakenResources %d", TakenResources);
			FLOGV("TransportCapacity %d", TransportCapacity);


			if (TransportCapacity == 0)
			{
				return;
			}

		}
	}
}

void UFlareSimulatedSector::AdaptativeTransportResources(UFlareCompany* Company, uint32& TransportCapacity, EFlareTransportLimitType::Type TransportLimitType, uint32 TransportLimit, bool ActiveOnly, bool AllowTrade)
{
	for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[PersistentStationIndex];
		PersistentStationIndex++;
		if (PersistentStationIndex >= SectorStations.Num())
		{
			PersistentStationIndex = 0;
		}

		if ((!AllowTrade && Station->GetCompany() != Company) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		//FLOGV("Check station %s needs:", *Station->GetImmatriculation().ToString());


		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

			//FLOGV("  Factory %s : IsActive=%d IsNeedProduction=%d", *Factory->GetDescription()->Name.ToString(), Factory->IsActive(),Factory->IsNeedProduction());

			if (ActiveOnly && (!Factory->IsActive() || !Factory->IsNeedProduction()))
			{
				//FLOG("    No resources needed");
				// No resources needed
				break;
			}

			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				uint32 StoredQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				uint32 ConsumedQuantity = Factory->GetInputResourceQuantity(ResourceIndex);
				uint32 StorageCapacity = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);
				uint32 UnitSellPrice = 1.01 * GetResourcePrice(Resource);
						// TODO Cargo stations

				uint32 NeededQuantity = 0;
				switch(TransportLimitType)
				{
					case EFlareTransportLimitType::Production:
						NeededQuantity = ConsumedQuantity * TransportLimit;
						break;
					case EFlareTransportLimitType::CargoBay:
						NeededQuantity = Station->GetDescription()->CargoBayCapacity * TransportLimit;
						break;
				}

				//FLOGV("    Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *Resource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);


				if (StoredQuantity < NeededQuantity)
				{
					// Do transfert
					uint32 QuantityToTransfert = FMath::Min(TransportCapacity, NeededQuantity - StoredQuantity);
					QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
					if(Station->GetCompany() != Company)
					{
						// Compute max quantity the station can afford
						uint32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
						QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
						//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
						//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
					}

					uint32 TakenResources = TakeUselessResources(Company, Resource, QuantityToTransfert, AllowTrade);
					Station->GetCargoBay()->GiveResources(Resource, TakenResources);
					if(TakenResources > 0 && Station->GetCompany() != Company)
					{
						// Sell resources
						int32 TransactionAmount = TakenResources * UnitSellPrice;
						Station->GetCompany()->TakeMoney(TransactionAmount);
						Company->GiveMoney(TransactionAmount);
						Company->GiveReputation(Station->GetCompany(), 0.5f, true);
						Station->GetCompany()->GiveReputation(Company, 0.5f, true);
						FLOGV("%s sell %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
					}

					TransportCapacity -= TakenResources;
					if (TakenResources > 0)
					{
						//FLOGV("      Do transfet : QuantityToTransfert=%u TakenResources=%u TransportCapacity=%u", QuantityToTransfert, TakenResources, TransportCapacity);
					}

					if (TransportCapacity == 0)
					{
						break;
					}
				}
			}

			if (TransportCapacity == 0)
			{
				break;
			}
		}

		if (TransportCapacity == 0)
		{
			break;
		}
	}
}

void UFlareSimulatedSector::SimulateTrade(TArray<uint32> CompanyRemainingTransportCapacity)
{
	// Trade
	// The sum of all resources remaining to transport is compute.
	// The remaining transport capacity of each company is used to buy and sell, the more transport need you have,
	// the more resource you have the right to transport.
	// it loop until it remain resources to transport, or all company skip

	FLOG("SimulateTrade");

	while(true)
	{
		TArray<int32> CompanyTransportNeeds;
		int32 TotalTransportNeeds = 0;
		int32 TotalRemainingTransportCapacity = 0;

		// Company transport
		for (int CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			TotalTransportNeeds += GetTransportCapacityNeeds(GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex], true);
		}

		// Company transport
		for (int CompanyIndex = 0; CompanyIndex< CompanyRemainingTransportCapacity.Num(); CompanyIndex++)
		{
			TotalRemainingTransportCapacity += CompanyRemainingTransportCapacity[CompanyIndex];
		}

		//FLOGV("TotalTransportNeeds=%d", TotalTransportNeeds);
		//FLOGV("TotalRemainingTransportCapacity=%d", TotalRemainingTransportCapacity);

		if(TotalTransportNeeds == 0 || TotalRemainingTransportCapacity == 0)
		{
			// Nothing to do
			return;
		}


		for (int CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			UFlareCompany* Company = GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
			uint32 RemainingTransportCapacity = CompanyRemainingTransportCapacity[CompanyIndex];
			uint32 Quota = TotalTransportNeeds * RemainingTransportCapacity / TotalRemainingTransportCapacity;

			uint32 InitialTransportCapacity = FMath::Min(Quota, RemainingTransportCapacity);
			uint32 TransportCapacity = InitialTransportCapacity;

			FLOGV("Company %s trade", *Company->GetCompanyName().ToString());

			//FLOGV("RemainingTransportCapacity=%u", RemainingTransportCapacity);
			//FLOGV("Quota=%u", Quota);
			//FLOGV("InitialTransportCapacity=%u", InitialTransportCapacity);


			//1 - fill resources consumers
			FillResourceConsumers(Company, TransportCapacity, true);

			// 2 - one with the exact quantity
			if (TransportCapacity)
			{
				AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::Production, 1, true, true);
			}

			// 3 - the second with the double
			if (TransportCapacity)
			{
				AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::Production, 2, true, true);
			}

			// 4 - third with slot alignemnt
			if (TransportCapacity)
			{
				AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::CargoBay, 1, true, true);
			}

			// 5 - a 4th with inactive stations
			if (TransportCapacity)
			{
				AdaptativeTransportResources(Company, TransportCapacity, EFlareTransportLimitType::CargoBay, 1, false, true);
			}

			if(InitialTransportCapacity == TransportCapacity)
			{
				// Nothing transported, abord
				CompanyRemainingTransportCapacity[CompanyIndex] = 0;
			}
			else
			{
				CompanyRemainingTransportCapacity[CompanyIndex] -= InitialTransportCapacity - TransportCapacity;
			}
			//FLOGV("final TransportCapacity=%u", TransportCapacity);
		}
	}
}

uint32 UFlareSimulatedSector::TakeUselessResources(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToTake, bool AllowTrade)
{


	// Compute the max quantity the company can buy
	uint32 UnitBuyPrice = 0.99 * GetResourcePrice(Resource);
	uint32 MaxBuyableQuantity = Company->GetMoney() / UnitBuyPrice;
	QuantityToTake = FMath::Min(QuantityToTake, MaxBuyableQuantity);
	uint32 RemainingQuantityToTake = QuantityToTake;
	// TODO storage station


	// First pass: take from station with factory that output the resource
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if ( (!AllowTrade && Station->GetCompany() != Company) || Station->HasCapability(EFlareSpacecraftCapability::Consumer) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if (Factory->HasOutputResource(Resource))
			{
				uint32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
				RemainingQuantityToTake -= TakenQuantity;
				if(TakenQuantity > 0 && Station->GetCompany() != Company)
				{
					//Buy
					int32 TransactionAmount = TakenQuantity * UnitBuyPrice;
					Station->GetCompany()->GiveMoney(TransactionAmount);
					Company->TakeMoney(TransactionAmount);
					Company->GiveReputation(Station->GetCompany(), 0.5f, true);
					Station->GetCompany()->GiveReputation(Company, 0.5f, true);
					FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
				}

				break;
			}
		}
	}

	// Second pass: take from storage station
	// TODO

	// Third pass: take from station with factory that don't input the resources
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
		bool NeedResource = false;

		if ( (!AllowTrade && Station->GetCompany() != Company) || Station->HasCapability(EFlareSpacecraftCapability::Consumer) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if (Factory->HasInputResource(Resource))
			{
				NeedResource =true;
				break;
			}
		}

		if (!NeedResource)
		{
			uint32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuantity;
			if(TakenQuantity > 0 && Station->GetCompany() != Company)
			{
				//Buy
				int32 TransactionAmount = TakenQuantity * UnitBuyPrice;
				Station->GetCompany()->GiveMoney(TransactionAmount);
				Company->TakeMoney(TransactionAmount);
				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
				FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
			}
		}
	}

	// 4th pass: take from station inactive station
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
		bool NeedResource = false;

		if ( (!AllowTrade && Station->GetCompany() != Company) || Station->IsConsumeResource(Resource) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if (Factory->IsActive() && Factory->IsNeedProduction() && Factory->HasInputResource(Resource))
			{
				NeedResource =true;
				break;
			}
		}

		if (!NeedResource)
		{
			uint32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuantity;
			if(TakenQuantity > 0 && Station->GetCompany() != Company)
			{
				//Buy
				int32 TransactionAmount = TakenQuantity * UnitBuyPrice;
				Station->GetCompany()->GiveMoney(TransactionAmount);
				Company->TakeMoney(TransactionAmount);
				FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
			}
		}

	}

	return QuantityToTake - RemainingQuantityToTake;
}

uint32 UFlareSimulatedSector::TakeResources(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToTake)
{
	uint32 RemainingQuantityToTake = QuantityToTake;

	{
		uint32 TakenQuantity = TakeUselessResources(Company, Resource, RemainingQuantityToTake);
		RemainingQuantityToTake -= TakenQuantity;
	}

	if (RemainingQuantityToTake > 0)
	{
		for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

			if ( Station->GetCompany() != Company)
			{
				continue;
			}


			uint32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuantity;
		}
	}

	return QuantityToTake - RemainingQuantityToTake;
}

uint32 UFlareSimulatedSector::GiveResources(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToGive)
{
	uint32 RemainingQuantityToGive = QuantityToGive;

	// Fill one production slot to active stations
	RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 1, true, false);

	// Fill two production slot to active stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 2, true, false);
	}

	// Fill 1 slot to active stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::CargoBay, 1, true, false);
	}

	// Give to inactive station
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::CargoBay, 1, false, false);
	}

	// Give to storage stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 0, true, true);
	}

    return QuantityToGive - RemainingQuantityToGive;
}

uint32 UFlareSimulatedSector::AdaptativeGiveResources(UFlareCompany* Company, FFlareResourceDescription* GivenResource, uint32 QuantityToGive, EFlareTransportLimitType::Type TransportLimitType, uint32 TransportLimit, bool ActiveOnly, bool StorageOnly)
{

	uint32 RemainingQuantityToGive = QuantityToGive;

	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToGive > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if ( Station->GetCompany() != Company)
		{
			continue;
		}

		if (StorageOnly)
		{
			if (Station->HasCapability(EFlareSpacecraftCapability::Storage))
			{
				uint32 GivenQuantity = Station->GetCargoBay()->GiveResources(GivenResource, RemainingQuantityToGive);
				RemainingQuantityToGive -= GivenQuantity;
			}
			continue;
		}

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

			FLOGV("  Factory %s : IsActive=%d IsNeedProduction=%d", *Factory->GetDescription()->Name.ToString(), Factory->IsActive(),Factory->IsNeedProduction());

			if (ActiveOnly && (!Factory->IsActive() || !Factory->IsNeedProduction()))
			{
				FLOG("    No resources needed");
				// No resources needed
				break;
			}

			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);

				if (Resource != GivenResource)
				{
					continue;
				}

				uint32 StoredQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				uint32 ConsumedQuantity = Factory->GetInputResourceQuantity(ResourceIndex);
				uint32 StorageCapacity = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);

				uint32 NeededQuantity = 0;
				switch(TransportLimitType)
				{
					case EFlareTransportLimitType::Production:
						NeededQuantity = ConsumedQuantity * TransportLimit;
						break;
					case EFlareTransportLimitType::CargoBay:
						NeededQuantity = Station->GetDescription()->CargoBayCapacity * TransportLimit;
						break;
				}

				FLOGV("    Give Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *Resource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);


				if (StoredQuantity < NeededQuantity)
				{
					// Do transfert
					uint32 QuantityToTransfert = FMath::Min(RemainingQuantityToGive, NeededQuantity - StoredQuantity);
					QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
					Station->GetCargoBay()->GiveResources(Resource, QuantityToTransfert);

					RemainingQuantityToGive -= QuantityToTransfert;

					FLOGV("      Give: QuantityToTransfert=%u RemainingQuantityToGive=%u", QuantityToTransfert, RemainingQuantityToGive);

					if (RemainingQuantityToGive == 0)
					{
						break;
					}
				}
			}

			if (RemainingQuantityToGive == 0)
			{
				break;
			}
		}
	}

	return QuantityToGive - RemainingQuantityToGive;
}

uint32 UFlareSimulatedSector::GetTransportCapacity(UFlareCompany* Company)
{
	uint32 TransportCapacity = 0;

	for (int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];
		if (Ship->GetCompany() == Company && Ship->IsAssignedToSector())
		{
			TransportCapacity += Ship->GetCargoBay()->GetCapacity();
		}
	}
	return TransportCapacity;
}

uint32 UFlareSimulatedSector::GetResourceCount(UFlareCompany* Company, FFlareResourceDescription* Resource)
{
	uint32 ResourceCount = 0;

	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if ( Station->GetCompany() != Company)
		{
			continue;
		}

		ResourceCount += Station->GetCargoBay()->GetResourceQuantity(Resource);
	}

	return ResourceCount;
}

int32 UFlareSimulatedSector::GetTransportCapacityBalance(UFlareCompany* Company, bool AllowTrade)
{
	return GetTransportCapacity(Company) -  GetTransportCapacityNeeds(Company, AllowTrade);
}

int32 UFlareSimulatedSector::GetTransportCapacityNeeds(UFlareCompany* Company, bool AllowTrade)
{

	int32 TransportNeeds = 0;
	// For each ressource, find the required resources and available resources
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
	{
		int32 Input = 0;
		int32 Stock = 0;
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;

		// For each station, check if consume resource or if has the ressources.
		for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
			bool NeedResource = false;

			if ((!AllowTrade && Station->GetCompany() != Company) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

				if (!Factory->IsActive())
				{
					continue;
				}

				if (Factory->HasInputResource(Resource))
				{
					// 1 slot as input
					Input += FMath::Max(0, (int32) Station->GetCargoBay()->GetSlotCapacity() - (int32)  Station->GetCargoBay()->GetResourceQuantity(Resource));
					NeedResource = true;
					break;
				}
			}

			if (!NeedResource)
			{
				Stock += Station->GetCargoBay()->GetResourceQuantity(Resource);
			}

		}

		TransportNeeds += FMath::Min(Input, Stock);
	}

	return TransportNeeds;
}

#undef LOCTEXT_NAMESPACE
