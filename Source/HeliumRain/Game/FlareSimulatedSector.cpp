
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
	if(Cast<UFlareWorld>(GetOuter()))
	{
		Game = Cast<UFlareWorld>(GetOuter())->GetGame();
	}
	else
	{
		Game = Cast<UFlareTravel>(GetOuter())->GetGame();
	}



	SectorData = Data;
	SectorDescription = Description;
	SectorOrbitParameters = OrbitParameters;
	SectorShips.Empty();
	SectorStations.Empty();
	SectorSpacecrafts.Empty();
	SectorFleets.Empty();

	FFlareCelestialBody* Body = Game->GetGameWorld()->GetPlanerarium()->FindCelestialBody(SectorOrbitParameters.CelestialBodyIdentifier);
	if (Body)
	{
		LightRatio = Game->GetGameWorld()->GetPlanerarium()->GetLightRatio(Body, SectorOrbitParameters.Altitude);
	}
	else
	{
		LightRatio = 1.0;
	}

	LoadPeople(SectorData.PeopleData);

	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);

		if (!Spacecraft)
		{
			FLOGV("UFlareSimulatedSector::Load : Missing spacecraft '%s'", *SectorData.SpacecraftIdentifiers[i].ToString());
			continue;
		}

		if (Spacecraft->IsStation())
		{
			SectorStations.Add(Spacecraft);
		}
		else
		{
			SectorShips.Add(Spacecraft);
		}
		SectorSpacecrafts.Add(Spacecraft);
		Spacecraft->SetCurrentSector(this);
	}


	for (int i = 0 ; i < SectorData.FleetIdentifiers.Num(); i++)
	{
		UFlareFleet* Fleet = Game->GetGameWorld()->FindFleet(SectorData.FleetIdentifiers[i]);
		if (Fleet)
		{
			SectorFleets.Add(Fleet);
			Fleet->SetCurrentSector(this);
		}
		else
		{
			FLOGV("UFlareSimulatedSector::Load : Missing fleet %s in sector %s", *SectorData.FleetIdentifiers[i].ToString(), *GetSectorName().ToString());
		}

	}

	LoadResourcePrices();
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

	SaveResourcePrices();

	if(Game->GetActiveSector() && Game->GetActiveSector()->GetSimulatedSector() == this)
	{
		Game->GetActiveSector()->Save();
	}

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
	ShipData.Level = 1;

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
		OrbitalEngineIdentifier = FName("engine-thresher");
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

		//Save optimization
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;
		ComponentData.Weapon.FiredAmmo = 0;

		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->OrbitalEngineCount; i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = OrbitalEngineIdentifier;
		ComponentData.ShipSlotIdentifier = FName(*("engine-" + FString::FromInt(i)));
		ComponentData.Damage = 0.f;

		//Save optimization
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;
		ComponentData.Weapon.FiredAmmo = 0;

		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->GunSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Game->GetDefaultWeaponIdentifier();
		ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
		ComponentData.Damage = 0.f;
		ComponentData.Weapon.FiredAmmo = 0;

		//Save optimization
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;

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

		//Save optimization
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;
		ComponentData.Weapon.FiredAmmo = 0;

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
	}
	else
	{
		SectorShips.Add(Spacecraft);
	}
	SectorSpacecrafts.Add(Spacecraft);

	Spacecraft->SetCurrentSector(this);

	FLOGV("UFlareSimulatedSector::CreateShip : Created ship '%s' at %s", *Spacecraft->GetImmatriculation().ToString(), *TargetPosition.ToString());

	if (!Spacecraft->IsStation())
	{
		UFlareFleet* NewFleet = Company->CreateAutomaticFleet(Spacecraft);


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
	float MinSize = 0.6;
	float MinMaxSize = 0.9;
	float MaxMaxSize = 1.1;
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
	SectorFleets.AddUnique(Fleet);

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		Fleet->GetShips()[ShipIndex]->SetCurrentSector(this);
		SectorShips.AddUnique(Fleet->GetShips()[ShipIndex]);
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
	//FLOGV("UFlareSimulatedSector::RetireFleet %s", *Fleet->GetFleetName().ToString());
	if (SectorFleets.Remove(Fleet) == 0)
	{
		FLOGV("UFlareSimulatedSector::RetireFleet : RetireFleet fail. Fleet '%s' is not in sector '%s'", *Fleet->GetFleetName().ToString(), *GetSectorName().ToString())
		return;
	}

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Fleet->GetShips()[ShipIndex];
		Spacecraft->ForceUndock();

		if (RemoveSpacecraft(Fleet->GetShips()[ShipIndex]) == 0)
		{
			FLOGV("UFlareSimulatedSector::RetireFleet : RetireFleet fail. Ship '%s' is not in sector '%s'", *Fleet->GetShips()[ShipIndex]->GetImmatriculation().ToString(), *GetSectorName().ToString())
		}
	}
}

int UFlareSimulatedSector::RemoveSpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	SectorSpacecrafts.Remove(Spacecraft);
	return SectorShips.Remove(Spacecraft);
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/


FText UFlareSimulatedSector::GetSectorDescription() const
{
	return SectorDescription->Description;
}

bool UFlareSimulatedSector::CanBuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company, TArray<FText>& OutReasons, bool IgnoreCost)
{
	bool Result = true;

	// Too many stations
	if (SectorStations.Num() >= GetMaxStationsInSector())
	{
		OutReasons.Add(LOCTEXT("BuildTooManyStations", "There are too many stations in the sector"));
		Result = false;
	}

	// Does it needs sun
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure) && SectorDescription->IsSolarPoor)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresSun", "This station requires high solar exposure"));
		Result = false;
	}

	// Does it needs not icy sector
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::HideOnIce) &&SectorDescription->IsIcy)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresNoIcy", "This station can only be built in non-icy sectors"));
		Result = false;
	}

	// Does it needs icy sector
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::HideOnNoIce) && !SectorDescription->IsIcy)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresIcy", "This station can only be built in icy sectors"));
		Result = false;
	}

	// Does it needs an geostationary orbit ?
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::GeostationaryOrbit) && !SectorDescription->IsGeostationary)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresGeo", "This station can only be built in geostationary sectors"));
		Result = false;
	}

	// Does it needs an asteroid ?
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid) && SectorData.AsteroidData.Num() == 0)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresAsteroid", "This station can only be built on an asteroid"));
		Result = false;
	}

	if(IgnoreCost)
	{
		return Result;
	}

	// Check money cost
	if (Company->GetMoney() < GetStationConstructionFee(StationDescription->CycleCost.ProductionCost))
	{
		OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresMoney", "Not enough credits ({0} / {1})"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetMoney())),
			FText::AsNumber(UFlareGameTools::DisplayMoney(GetStationConstructionFee(StationDescription->CycleCost.ProductionCost)))));
		Result = false;
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
		OutReasons.Add(LOCTEXT("BuildRequiresCargo", "No cargo with free space"));
		Result = false;
	}
	
	// Compute total available resources
	TArray<FFlareCargo> AvailableResources;


	// TODO Use getCompanyResources

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
		uint32 AvailableQuantity = 0;

		for (int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
		{
			if (AvailableResources[AvailableResourceIndex].Resource == &(FactoryResource->Resource->Data))
			{
				AvailableQuantity = AvailableResources[AvailableResourceIndex].Quantity;
				if (AvailableQuantity >= FactoryResource->Quantity)
				{
					ResourceFound = true;
				}
				break;
			}
		}
		if (!ResourceFound)
		{
			OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresResources", "Not enough {0} ({1} / {2})"),
					FactoryResource->Resource->Data.Name,
					FText::AsNumber(AvailableQuantity),
					FText::AsNumber(FactoryResource->Quantity)));

			Result = false;
		}
	}

	return Result;
}

bool UFlareSimulatedSector::BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	TArray<FText> Reasons;
	if (!CanBuildStation(StationDescription, Company, Reasons))
	{
		FLOGV("UFlareSimulatedSector::BuildStation : Failed to build station '%s' for company '%s' (%s)",
			*StationDescription->Identifier.ToString(),
			*Company->GetCompanyName().ToString(),
			*Reasons[0].ToString());
		return false;
	}

	int64 ProductionCost = GetStationConstructionFee(StationDescription->CycleCost.ProductionCost);

	// Pay station cost
	if(!Company->TakeMoney(ProductionCost))
	{
		return false;
	}

	GetPeople()->Pay(ProductionCost);

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
			UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

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

bool UFlareSimulatedSector::CanUpgradeStation(UFlareSimulatedSpacecraft* Station, TArray<FText>& OutReasons)
{
	bool Result = true;

	UFlareCompany* Company = Station->GetCompany();

	// Check money cost
	if (Company->GetMoney() < Station->GetStationUpgradeFee())
	{
		OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresMoney", "Not enough credits ({0} / {1})"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetMoney())),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Station->GetStationUpgradeFee()))));
		Result = false;
	}

	// Compute total available resources
	TArray<FFlareCargo> AvailableResources;


	// TODO Use getCompanyResources

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
	for (int32 ResourceIndex = 0; ResourceIndex < Station->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &Station->GetDescription()->CycleCost.InputResources[ResourceIndex];
		bool ResourceFound = false;
		uint32 AvailableQuantity = 0;

		for (int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
		{
			if (AvailableResources[AvailableResourceIndex].Resource == &(FactoryResource->Resource->Data))
			{
				AvailableQuantity = AvailableResources[AvailableResourceIndex].Quantity;
				if (AvailableQuantity >= FactoryResource->Quantity)
				{
					ResourceFound = true;
				}
				break;
			}
		}
		if (!ResourceFound)
		{
			OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresResources", "Not enough {0} ({1} / {2})"),
					FactoryResource->Resource->Data.Name,
					FText::AsNumber(AvailableQuantity),
					FText::AsNumber(FactoryResource->Quantity)));

			Result = false;
		}
	}

	return Result;
}

bool UFlareSimulatedSector::UpgradeStation(UFlareSimulatedSpacecraft* Station)
{
	UFlareCompany* Company = Station->GetCompany();


	TArray<FText> Reasons;
	if (!CanUpgradeStation(Station, Reasons))
	{
		FLOGV("UFlareSimulatedSector::UpgradeStation : Failed to upgrade station '%s' for company '%s' (%s)",
			*Station->GetDescription()->Identifier.ToString(),
			*Company->GetCompanyName().ToString(),
			*Reasons[0].ToString());
		return false;
	}

	int64 ProductionCost = Station->GetStationUpgradeFee();

	// Pay station cost
	if(!Company->TakeMoney(ProductionCost))
	{
		return false;
	}

	GetPeople()->Pay(ProductionCost);

	// Take resource cost
	for (int ResourceIndex = 0; ResourceIndex < Station->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &Station->GetDescription()->CycleCost.InputResources[ResourceIndex];
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
			UFlareSimulatedSpacecraft* SourceStation = SectorStations[StationIndex];

			if (SourceStation->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= SourceStation->GetCargoBay()->TakeResources(Resource, ResourceToTake);
		}

		if (ResourceToTake > 0)
		{
			FLOG("UFlareSimulatedSector::BuildStation : Failed to take resource cost for build station a station but CanBuild test succeded");
		}
	}

	Station->Upgrade();

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

void UFlareSimulatedSector::SimulatePriceVariation()
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		SimulatePriceVariation(Resource);
	}
}

void UFlareSimulatedSector::SimulatePriceVariation(FFlareResourceDescription* Resource)
{
	float OldPrice = GetPreciseResourcePrice(Resource);
	// Prices can increase because :

	//  - The input of a station is low (and less than half)
	//  - Consumer ressource is low
	//  - Maintenance ressource is low (and less than half)


	// Prices can decrease because :
	//  - Output of a station is full (and more than half)
	//  - Consumer ressource is full (and more than half)
	//  - Maintenance ressource is full (and more than half) (very slow decrease)


	float WantedVariation = 0;
	float WantedTotal = 0;


	// Prices never go below min production cost
	for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[CountIndex];

		float StockRatio = FMath::Clamp((float) Station->GetCargoBay()->GetResourceQuantity(Resource) / (float) Station->GetCargoBay()->GetSlotCapacity(), 0.f, 1.f);

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

			if(!Factory->IsActive())
			{
				continue;
			}


			if (Factory->HasInputResource(Resource))
			{
				if (StockRatio < 0.8f)
				{
					float Weight = Factory->GetInputResourceQuantity(Resource);
					WantedVariation += Weight * (1.f - (StockRatio / 0.8)); // Max 1
					WantedTotal += Weight;
				}
			}


			if (Factory->HasOutputResource(Resource))
			{
				if (StockRatio > 0.8f)
				{
					float Weight = Factory->GetOutputResourceQuantity(Resource);
					WantedVariation -= Weight * (StockRatio - 0.8) / 0.2; // Max 1
					WantedTotal += Weight;
				}
			}
		}

		if(Station->HasCapability(EFlareSpacecraftCapability::Consumer) && Game->GetResourceCatalog()->IsCustomerResource(Resource))
		{
			if (StockRatio < 0.8f)
			{
				float Weight = GetPeople()->GetRessourceConsumption(Resource);
				WantedVariation += Weight * (1.f - (StockRatio / 0.8)); // Max 1
				WantedTotal += Weight;
			}
		}

		if(Station->HasCapability(EFlareSpacecraftCapability::Maintenance) && Game->GetResourceCatalog()->IsMaintenanceResource(Resource))
		{
			if (StockRatio < 0.8f)
			{
				WantedVariation += 1.f - (StockRatio / 0.8); // Max 1
				WantedTotal += 1;
			}
		}
	}

	float MeanWantedVariation = (WantedTotal > 0 ? WantedVariation / WantedTotal : 0);


	if(MeanWantedVariation != 0.f)
	{

		float OldPriceRatio = (OldPrice - Resource->MinPrice) / (float) (Resource->MaxPrice - Resource->MinPrice);

		float MaxPriceVariation = 10;
		float OldPriceRatioToVariationDirection;

		if(MeanWantedVariation > 0)
		{
			OldPriceRatioToVariationDirection = OldPriceRatio;
		}
		else
		{
			OldPriceRatioToVariationDirection = 1 - OldPriceRatio;
		}

		float A = (MaxPriceVariation - 2) * (MaxPriceVariation - 2) / (MaxPriceVariation * (MaxPriceVariation - 1));
		float B = (MaxPriceVariation - 2) / (MaxPriceVariation * (MaxPriceVariation - 1));
		float C = MaxPriceVariation / (MaxPriceVariation - 2);

		float VariationScale = (1 / (A*OldPriceRatioToVariationDirection + B)) - C;



		float Variation = VariationScale * MeanWantedVariation;


		float NewPrice = FMath::Max(1.f, OldPrice * (1 + Variation / 100.f));


		//
		float MeanPrice = (float) (Resource->MaxPrice + Resource->MinPrice) / 2.f;

		SetPreciseResourcePrice(Resource, NewPrice);
		if(NewPrice > Resource->MaxPrice)
		{
			FLOGV("%s price at max in %s", *Resource->Name.ToString(), *GetSectorName().ToString());
		}
		else if(NewPrice < Resource->MinPrice)
		{
			FLOGV("%s price at min in %s", *Resource->Name.ToString(), *GetSectorName().ToString());
		}
		else
		{
			FLOGV("%s price in %s change from %f to %f (%f)", *Resource->Name.ToString(), *GetSectorName().ToString(), OldPrice / 100.f, NewPrice / 100.f, Variation);
		}
	}
}

void UFlareSimulatedSector::SimulateTransport()
{
	TArray<uint32> CompanyRemainingTransportCapacity;

	// Company transport
	for (int CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
		uint32 TransportCapacity = GetTransportCapacity(Company, false);

		uint32 UsedCapacity = SimulateTransport(Company, TransportCapacity);

		CompanyRemainingTransportCapacity.Add(TransportCapacity - UsedCapacity);
	}

	SimulateTrade(CompanyRemainingTransportCapacity);
}

int32 UFlareSimulatedSector::SimulateTransport(UFlareCompany* Company, int32 InitialTranportCapacity)
{

	int32 TransportCapacity = InitialTranportCapacity;

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

	//1.1 - fill resources maintenance
	FillResourceMaintenances(Company, TransportCapacity, true);

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

void UFlareSimulatedSector::FillResourceConsumers(UFlareCompany* Company, int32& TransportCapacity, bool AllowTrade)
{
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
		uint32 UnitSellPrice = GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
		//FLOGV("Distribute consumer ressource %s", *Resource->Name.ToString());


		// Transport consumer resources by priority
		for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorStations[CountIndex];

			if ((!AllowTrade && Station->GetCompany() != Company) || !Station->HasCapability(EFlareSpacecraftCapability::Consumer) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			bool IsTrade = (Station->GetCompany() != Company);
			//FLOGV("Check station %s needs:", *Station->GetImmatriculation().ToString());


			// Fill only one slot for each ressource
			if (Station->GetCargoBay()->GetResourceQuantity(Resource) > Station->GetCargoBay()->GetSlotCapacity())
			{
				FLOGV("Fill only one slot for each ressource. Has %d", Station->GetCargoBay()->GetResourceQuantity(Resource));

				continue;
			}


			int32 MaxQuantity = Station->GetCargoBay()->GetSlotCapacity() - Station->GetCargoBay()->GetResourceQuantity(Resource);
			int32 FreeSpace = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);
			int32 QuantityToTransfert = FMath::Min(MaxQuantity, FreeSpace);
			QuantityToTransfert = FMath::Min(TransportCapacity, QuantityToTransfert);

			// No need to limits, depth are allowed for habitations
			/*
			if(IsTrade)
			{
				// Compute max quantity the station can afford
				int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
				MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
				QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
				//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
				//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
			}

*/

			int32 TakenResources = TakeUselessResources(Company, Resource, QuantityToTransfert, AllowTrade, IsTrade);
			Station->GetCargoBay()->GiveResources(Resource, TakenResources);
			TransportCapacity -= TakenResources;

			if(TakenResources > 0 && IsTrade)
			{
				// Sell resources
				int64 TransactionAmount = TakenResources * UnitSellPrice;
				if (!Station->GetCompany()->TakeMoney(TransactionAmount, true))
					FLOGV("WARNING: %s	fail to sell %u inits of %s (as customer resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);
				Company->GiveMoney(TransactionAmount);
				//FLOGV("%s	%u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
			}


			//FLOGV("MaxQuantity %d", MaxQuantity);
			//FLOGV("FreeSpace %d", FreeSpace);
			//FLOGV("QuantityToTransfert %d", QuantityToTransfert);
			//FLOGV("TakenResources %d", TakenResources);
			//FLOGV("TransportCapacity %d", TransportCapacity);


			if (TransportCapacity == 0)
			{
				return;
			}

		}
	}
}

void UFlareSimulatedSector::FillResourceMaintenances(UFlareCompany* Company, int32& TransportCapacity, bool AllowTrade)
{
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
		uint32 UnitSellPrice = GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
		//FLOGV("Distribute consumer ressource %s", *Resource->Name.ToString());


		// Transport consumer resources by priority
		for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorStations[CountIndex];

			if ((!AllowTrade && Station->GetCompany() != Company) || !Station->HasCapability(EFlareSpacecraftCapability::Maintenance) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
			{
				continue;
			}

			bool IsTrade = (Station->GetCompany() != Company);
			//FLOGV("Check station %s needs:", *Station->GetImmatriculation().ToString());


			// Fill only one slot for each ressource
			if (Station->GetCargoBay()->GetResourceQuantity(Resource) > Station->GetCargoBay()->GetSlotCapacity())
			{
				FLOGV("Fill only one slot for each ressource. Has %d", Station->GetCargoBay()->GetResourceQuantity(Resource));

				continue;
			}


			int32 MaxQuantity = Station->GetCargoBay()->GetSlotCapacity() - Station->GetCargoBay()->GetResourceQuantity(Resource);
			int32 FreeSpace = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);
			int32 QuantityToTransfert = FMath::Min(MaxQuantity, FreeSpace);
			QuantityToTransfert = FMath::Min(TransportCapacity, QuantityToTransfert);

			if(IsTrade)
			{
				// Compute max quantity the station can afford
				int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
				MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
				QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
				//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
				//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
			}



			int32 TakenResources = TakeUselessResources(Company, Resource, QuantityToTransfert, AllowTrade, IsTrade);
			Station->GetCargoBay()->GiveResources(Resource, TakenResources);
			TransportCapacity -= TakenResources;

			if(TakenResources > 0 && IsTrade)
			{
				// Sell resources
				int64 TransactionAmount = TakenResources * UnitSellPrice;
				if (!Station->GetCompany()->TakeMoney(TransactionAmount))
					FLOGV("WARNING: %s	fail to sell %u inits of %s (as maintenance resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);
				Company->GiveMoney(TransactionAmount);
				//FLOGV("%s	%u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
			}


			//FLOGV("MaxQuantity %d", MaxQuantity);
			//FLOGV("FreeSpace %d", FreeSpace);
			//FLOGV("QuantityToTransfert %d", QuantityToTransfert);
			//FLOGV("TakenResources %d", TakenResources);
			//FLOGV("TransportCapacity %d", TransportCapacity);


			if (TransportCapacity == 0)
			{
				return;
			}

		}
	}
}

void UFlareSimulatedSector::AdaptativeTransportResources(UFlareCompany* Company, int32& TransportCapacity, EFlareTransportLimitType::Type TransportLimitType, uint32 TransportLimit, bool ActiveOnly, bool AllowTrade)
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

			bool IsTrade = (Station->GetCompany() != Company);


			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				int32 StoredQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				int32 ConsumedQuantity = Factory->GetInputResourceQuantity(ResourceIndex);
				int32 StorageCapacity = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);
				int32 UnitSellPrice = GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
						// TODO Cargo stations

				int32 NeededQuantity = 0;
				switch(TransportLimitType)
				{
					case EFlareTransportLimitType::Production:
						NeededQuantity = ConsumedQuantity * TransportLimit;
						break;
					case EFlareTransportLimitType::CargoBay:
						NeededQuantity = Station->GetCargoBay()->GetSlotCapacity() * TransportLimit;
						break;
				}

				//FLOGV("    Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *Resource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);


				if (StoredQuantity < NeededQuantity)
				{
					// Do transfert
					int32 QuantityToTransfert = FMath::Min(TransportCapacity, NeededQuantity - StoredQuantity);
					QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
					if(IsTrade)
					{
						// Compute max quantity the station can afford
						int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
						MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
						QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
						//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
						//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
					}

					int32 TakenResources = TakeUselessResources(Company, Resource, QuantityToTransfert, AllowTrade, IsTrade);
					Station->GetCargoBay()->GiveResources(Resource, TakenResources);
					if(TakenResources > 0 && IsTrade)
					{
						// Sell resources
						int64 TransactionAmount = TakenResources * UnitSellPrice;
						if (!Station->GetCompany()->TakeMoney(TransactionAmount))
							FLOGV("WARNING: %s	fail to sell %u inits of %s (as standard resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);
						Company->GiveMoney(TransactionAmount);
						Company->GiveReputation(Station->GetCompany(), 0.5f, true);
						Station->GetCompany()->GiveReputation(Company, 0.5f, true);
						//FLOGV("%s sell %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenResources, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
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

	while(true)
	{
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
			int32 RemainingTransportCapacity = CompanyRemainingTransportCapacity[CompanyIndex];
			int32 Quota = TotalTransportNeeds * RemainingTransportCapacity / TotalRemainingTransportCapacity;

			int32 InitialTransportCapacity = FMath::Min(Quota, RemainingTransportCapacity);
			int32 TransportCapacity = InitialTransportCapacity;

			//FLOGV("Company %s trade", *Company->GetCompanyName().ToString());

			//FLOGV("RemainingTransportCapacity=%u", RemainingTransportCapacity);
			//FLOGV("Quota=%u", Quota);
			//FLOGV("InitialTransportCapacity=%u", InitialTransportCapacity);


			//1 - fill resources consumers
			FillResourceConsumers(Company, TransportCapacity, true);

			//1.1 - fill resources maintenances
			FillResourceMaintenances(Company, TransportCapacity, true);

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

uint32 UFlareSimulatedSector::TakeUselessResources(UFlareCompany* Company, FFlareResourceDescription* Resource, int32 QuantityToTake, bool AllowTrade, bool AllowDepts)
{


	// Compute the max quantity the company can buy
	int32 MaxUnitBuyPrice = GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
	int32 MaxBuyableQuantity = (AllowDepts ? QuantityToTake : Company->GetMoney() / MaxUnitBuyPrice);
	MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
	QuantityToTake = FMath::Min(QuantityToTake, MaxBuyableQuantity);
	int32 RemainingQuantityToTake = QuantityToTake;
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
				int32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
				RemainingQuantityToTake -= TakenQuantity;
				if(TakenQuantity > 0 && Station->GetCompany() != Company)
				{
					//Buy
					int64 TransactionAmount = TakenQuantity * GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryOutput);
					Station->GetCompany()->GiveMoney(TransactionAmount);
					if (!Company->TakeMoney(TransactionAmount, AllowDepts))
						FLOGV("WARNING: %s	fail to buy %u inits of %s (as useless resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);

					Company->GiveReputation(Station->GetCompany(), 0.5f, true);
					Station->GetCompany()->GiveReputation(Company, 0.5f, true);
					//FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
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
			int32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuantity;
			if(TakenQuantity > 0 && Station->GetCompany() != Company)
			{
				//Buy
				int64 TransactionAmount = TakenQuantity * GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
				Station->GetCompany()->GiveMoney(TransactionAmount);
				if (!Company->TakeMoney(TransactionAmount, AllowDepts))
					FLOGV("WARNING: %s	fail to buy %u inits of %s (as useless resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);

				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
				//FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
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
			int32 TakenQuantity = Station->GetCargoBay()->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuantity;
			if(TakenQuantity > 0 && Station->GetCompany() != Company)
			{
				//Buy
				int64 TransactionAmount = TakenQuantity * GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
				Station->GetCompany()->GiveMoney(TransactionAmount);
				if (!Company->TakeMoney(TransactionAmount, AllowDepts))
					FLOGV("WARNING: %s	fail to buy %u inits of %s (as useless resource) to %s for %f", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount/100);

				//FLOGV("%s buy %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), TakenQuantity, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
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

uint32 UFlareSimulatedSector::GiveResources(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToGive, bool AllowTrade)
{
	uint32 RemainingQuantityToGive = QuantityToGive;

	RemainingQuantityToGive -= DoGiveResources(Company, Resource, RemainingQuantityToGive, false);

	if(RemainingQuantityToGive && AllowTrade)
	{
		RemainingQuantityToGive -= DoGiveResources(Company, Resource, RemainingQuantityToGive, true);
	}

	return QuantityToGive - RemainingQuantityToGive;
}

uint32 UFlareSimulatedSector::DoGiveResources(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToGive, bool AllowTrade)
{
	uint32 RemainingQuantityToGive = QuantityToGive;

	// Fill one production slot to active stations
	RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 1, true, false, AllowTrade);

	// Fill two production slot to active stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 2, true, false, AllowTrade);
	}

	// Fill 1 slot to active stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::CargoBay, 1, true, false, AllowTrade);
	}

	// Fill 1 slot to customer stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveCustomerResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::CargoBay, 1, AllowTrade);
	}

	// Give to inactive station
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::CargoBay, 1, false, false, AllowTrade);
	}

	// Give to storage stations
	if (RemainingQuantityToGive)
	{
		RemainingQuantityToGive -= AdaptativeGiveResources(Company, Resource, RemainingQuantityToGive, EFlareTransportLimitType::Production, 0, true, true, AllowTrade);
	}

    return QuantityToGive - RemainingQuantityToGive;
}

uint32 UFlareSimulatedSector::AdaptativeGiveResources(UFlareCompany* Company, FFlareResourceDescription* GivenResource, uint32 QuantityToGive, EFlareTransportLimitType::Type TransportLimitType, uint32 TransportLimit, bool ActiveOnly, bool StorageOnly, bool AllowTrade)
{

	int32 RemainingQuantityToGive = QuantityToGive;
	//uint32 UnitSellPrice = 1.01 * GetResourcePrice(GivenResource);

	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToGive > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if ((!AllowTrade && Station->GetCompany() != Company) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		if (StorageOnly)
		{
			if (Station->HasCapability(EFlareSpacecraftCapability::Storage))
			{
				int32 QuantityToTransfert = RemainingQuantityToGive;

				if(Station->GetCompany() != Company)
				{
					// Compute max quantity the station can afford
					int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / GetResourcePrice(GivenResource, EFlareResourcePriceContext::Default);
					MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
					QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
					//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
					//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
				}

				int32 GivenQuantity = Station->GetCargoBay()->GiveResources(GivenResource, QuantityToTransfert);
				RemainingQuantityToGive -= GivenQuantity;

				if(GivenQuantity > 0 && Station->GetCompany() != Company)
				{
					// Sell resources
					int64 TransactionAmount = GivenQuantity * GetResourcePrice(GivenResource, EFlareResourcePriceContext::Default);
					Station->GetCompany()->TakeMoney(TransactionAmount);
					Company->GiveMoney(TransactionAmount);
					Company->GiveReputation(Station->GetCompany(), 0.5f, true);
					Station->GetCompany()->GiveReputation(Company, 0.5f, true);
					//FLOGV("%s sell %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), QuantityToTransfert, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
				}

			}
			continue;
		}

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

				if (Resource != GivenResource)
				{
					continue;
				}

				int32 StoredQuantity = Station->GetCargoBay()->GetResourceQuantity(Resource);
				int32 ConsumedQuantity = Factory->GetInputResourceQuantity(ResourceIndex);
				int32 StorageCapacity = Station->GetCargoBay()->GetFreeSpaceForResource(Resource);

				int32 NeededQuantity = 0;
				switch(TransportLimitType)
				{
					case EFlareTransportLimitType::Production:
						NeededQuantity = ConsumedQuantity * TransportLimit;
						break;
					case EFlareTransportLimitType::CargoBay:
						NeededQuantity = Station->GetCargoBay()->GetSlotCapacity() * TransportLimit;
						break;
				}

				//FLOGV("    Give Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *Resource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);


				if (StoredQuantity < NeededQuantity)
				{
					// Do transfert
					int32 QuantityToTransfert = FMath::Min(RemainingQuantityToGive, NeededQuantity - StoredQuantity);

					if(Station->GetCompany() != Company)
					{
						// Compute max quantity the station can afford
						int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
						MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
						QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
						//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
						//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
					}

					QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
					Station->GetCargoBay()->GiveResources(Resource, QuantityToTransfert);

					RemainingQuantityToGive -= QuantityToTransfert;

					//FLOGV("      Give: QuantityToTransfert=%u RemainingQuantityToGive=%u", QuantityToTransfert, RemainingQuantityToGive);

					if(QuantityToTransfert > 0 && Station->GetCompany() != Company)
					{
						// Sell resources
						int64 TransactionAmount = QuantityToTransfert * GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
						Station->GetCompany()->TakeMoney(TransactionAmount);
						Company->GiveMoney(TransactionAmount);
						Company->GiveReputation(Station->GetCompany(), 0.5f, true);
						Station->GetCompany()->GiveReputation(Company, 0.5f, true);
						//FLOGV("%s sell %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), QuantityToTransfert, *Resource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
					}


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


uint32 UFlareSimulatedSector::AdaptativeGiveCustomerResources(UFlareCompany* Company, FFlareResourceDescription* GivenResource, uint32 QuantityToGive, EFlareTransportLimitType::Type TransportLimitType, uint32 TransportLimit, bool AllowTrade)
{
	if (!Game->GetResourceCatalog()->IsCustomerResource(GivenResource))
	{
		return 0;
	}

	int32 RemainingQuantityToGive = QuantityToGive;
	int32 UnitSellPrice = GetResourcePrice(GivenResource, EFlareResourcePriceContext::FactoryInput);

	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToGive > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if ((!AllowTrade && Station->GetCompany() != Company) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile || !Station->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			continue;
		}

		int32 StoredQuantity = Station->GetCargoBay()->GetResourceQuantity(GivenResource);
		int32 StorageCapacity = Station->GetCargoBay()->GetFreeSpaceForResource(GivenResource);

		int32 NeededQuantity = 0;
		switch(TransportLimitType)
		{
			case EFlareTransportLimitType::Production:
			{
				uint32 ConsumedQuantity = GetPeople()->GetRessourceConsumption(GivenResource);
				NeededQuantity = ConsumedQuantity * TransportLimit;
			}
				break;
			case EFlareTransportLimitType::CargoBay:
				NeededQuantity = Station->GetCargoBay()->GetSlotCapacity() * TransportLimit;
				break;
		}

		FLOGV("    Give Customer Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *GivenResource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);


		if (StoredQuantity < NeededQuantity)
		{
			// Do transfert
			int32 QuantityToTransfert = FMath::Min(RemainingQuantityToGive, NeededQuantity - StoredQuantity);

			// Customer station can generate detps
			/*if(Station->GetCompany() != Company)
			{
				// Compute max quantity the station can afford
				int32 MaxBuyableQuantity = Station->GetCompany()->GetMoney() / UnitSellPrice;
				MaxBuyableQuantity = FMath::Max(0, MaxBuyableQuantity);
				QuantityToTransfert = FMath::Min(MaxBuyableQuantity, QuantityToTransfert);
				//FLOGV("MaxBuyableQuantity  %u",  MaxBuyableQuantity);
				//FLOGV("QuantityToTransfert  %u",  QuantityToTransfert);
			}*/

			QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
			Station->GetCargoBay()->GiveResources(GivenResource, QuantityToTransfert);

			RemainingQuantityToGive -= QuantityToTransfert;

			//FLOGV("      Give: QuantityToTransfert=%u RemainingQuantityToGive=%u", QuantityToTransfert, RemainingQuantityToGive);

			if(QuantityToTransfert > 0 && Station->GetCompany() != Company)
			{
				// Sell resources
				int32 TransactionAmount = QuantityToTransfert * UnitSellPrice;
				Station->GetCompany()->TakeMoney(TransactionAmount, true);
				Company->GiveMoney(TransactionAmount);
				Company->GiveReputation(Station->GetCompany(), 0.5f, true);
				Station->GetCompany()->GiveReputation(Company, 0.5f, true);
				//FLOGV("%s sell %u inits of %s to %s for %d", *Company->GetCompanyName().ToString(), QuantityToTransfert, *GivenResource->Name.ToString(), *Station->GetCompany()->GetCompanyName().ToString(), TransactionAmount);
			}
		}
	}

	return QuantityToGive - RemainingQuantityToGive;
}

int64 UFlareSimulatedSector::GetStationConstructionFee(int64 BasePrice)
{
	return BasePrice + 1000000 * SectorStations.Num();
}

uint32 UFlareSimulatedSector::GetTransportCapacity(UFlareCompany* Company, bool AllCompanies)
{
	uint32 TransportCapacity = 0;

	for (int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];
		if ((AllCompanies || Ship->GetCompany() == Company) && Ship->IsAssignedToSector())
		{
			TransportCapacity += Ship->GetCargoBay()->GetCapacity();
		}
	}
	return TransportCapacity;
}

uint32 UFlareSimulatedSector::GetResourceCount(UFlareCompany* Company, FFlareResourceDescription* Resource, bool IncludeShips, bool AllowTrade)
{
	uint32 ResourceCount = 0;

	TArray<UFlareSimulatedSpacecraft*>* SpacecraftList = &SectorStations;

	if(IncludeShips)
	{
		SpacecraftList = &SectorSpacecrafts;
	}

	for (int32 StationIndex = 0 ; StationIndex < SpacecraftList->Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = (*SpacecraftList)[StationIndex];

		if ((!AllowTrade && Station->GetCompany() != Company) || Station->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		ResourceCount += Station->GetCargoBay()->GetResourceQuantity(Resource);
	}

	return ResourceCount;
}

int32 UFlareSimulatedSector::GetTransportCapacityBalance(UFlareCompany* Company, bool AllowTrade)
{
	return GetTransportCapacity(Company, AllowTrade) -  GetTransportCapacityNeeds(Company, AllowTrade);
}

int32 UFlareSimulatedSector::GetTransportCapacityNeeds(UFlareCompany* Company, bool AllowTrade)
{

	int32 TransportNeeds = 0;
	// For each ressource, find the required resources and available resources
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		int32 Input = 0;
		int32 Stock = 0;
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

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

			if(Station->HasCapability(EFlareSpacecraftCapability::Consumer) && Game->GetResourceCatalog()->IsCustomerResource(Resource))
			{
				// 1 slot as input
				Input += FMath::Max(0, (int32) Station->GetCargoBay()->GetSlotCapacity() - (int32)  Station->GetCargoBay()->GetResourceQuantity(Resource));
				NeedResource = true;
			}

			if(Station->HasCapability(EFlareSpacecraftCapability::Maintenance) && Game->GetResourceCatalog()->IsMaintenanceResource(Resource))
			{
				// 1 slot as input
				Input += FMath::Max(0, (int32) Station->GetCargoBay()->GetSlotCapacity() - (int32)  Station->GetCargoBay()->GetResourceQuantity(Resource));
				NeedResource = true;
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

void UFlareSimulatedSector::LoadResourcePrices()
{
	ResourcePrices.Empty();
	LastResourcePrices.Empty();
	for (int PriceIndex = 0; PriceIndex < SectorData.ResourcePrices.Num(); PriceIndex++)
	{
		FFFlareResourcePrice* ResourcePrice = &SectorData.ResourcePrices[PriceIndex];
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourcePrice->ResourceIdentifier);
		float Price = ResourcePrice->Price;
		ResourcePrices.Add(Resource, Price);
		FFlareFloatBuffer* Prices = &ResourcePrice->Prices;
		Prices->Resize(50);
		LastResourcePrices.Add(Resource, *Prices);
	}
}

void UFlareSimulatedSector::SaveResourcePrices()
{
	SectorData.ResourcePrices.Empty();

	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		if (ResourcePrices.Contains(Resource))
		{
			FFFlareResourcePrice Price;
			Price.ResourceIdentifier = Resource->Identifier;
			Price.Price = ResourcePrices[Resource];
			if (LastResourcePrices.Contains(Resource))
			{
				Price.Prices = LastResourcePrices[Resource];
				SectorData.ResourcePrices.Add(Price);
			}
		}
	}
}

FText UFlareSimulatedSector::GetSectorName()
{
	if (SectorData.GivenName.ToString().Len())
	{
		return SectorData.GivenName;
	}
	else if (SectorDescription->Name.ToString().Len())
	{
		return SectorDescription->Name;
	}
	else
	{
		return FText::FromString(GetSectorCode());
	}
}

FString UFlareSimulatedSector::GetSectorCode()
{
	// TODO cache ?
	return SectorOrbitParameters.CelestialBodyIdentifier.ToString() + "-" + FString::FromInt(SectorOrbitParameters.Altitude) + "-" + FString::FromInt(SectorOrbitParameters.Phase);
}


EFlareSectorFriendlyness::Type UFlareSimulatedSector::GetSectorFriendlyness(UFlareCompany* Company)
{
	if (!Company->HasVisitedSector(this))
	{
		return EFlareSectorFriendlyness::NotVisited;
	}

	if (GetSectorSpacecrafts().Num() == 0)
	{
		return EFlareSectorFriendlyness::Neutral;
	}

	int HostileSpacecraftCount = 0;
	int NeutralSpacecraftCount = 0;
	int FriendlySpacecraftCount = 0;

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareCompany* OtherCompany = GetSectorSpacecrafts()[SpacecraftIndex]->GetCompany();

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
		}
		else
		{
			NeutralSpacecraftCount++;
		}
	}

	if (FriendlySpacecraftCount > 0 && HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Contested;
	}

	if (FriendlySpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Friendly;
	}
	else if (HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Hostile;
	}
	else
	{
		return EFlareSectorFriendlyness::Neutral;
	}
}

EFlareSectorBattleState::Type UFlareSimulatedSector::GetSectorBattleState(UFlareCompany* Company)
{

	if (GetSectorShips().Num() == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	int HostileSpacecraftCount = 0;
	int DangerousHostileSpacecraftCount = 0;


	int FriendlySpacecraftCount = 0;
	int DangerousFriendlySpacecraftCount = 0;
	int CrippledFriendlySpacecraftCount = 0;

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < GetSectorShips().Num(); SpacecraftIndex++)
	{

		UFlareSimulatedSpacecraft* Spacecraft = GetSectorShips()[SpacecraftIndex];

		UFlareCompany* OtherCompany = Spacecraft->GetCompany();

		if (!Spacecraft->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
			{
				DangerousFriendlySpacecraftCount++;
			}

			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion) == 0)
			{
				CrippledFriendlySpacecraftCount++;
			}
		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
			if (Spacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
			{
				DangerousHostileSpacecraftCount++;
			}
		}
	}

	// No friendly or no hostile ship
	if (FriendlySpacecraftCount == 0 || HostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	// No friendly and hostile ship are not dangerous
	if (DangerousFriendlySpacecraftCount == 0 && DangerousHostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::NoBattle;
	}

	// No friendly dangerous ship so the enemy have one. Battle is lost
	if (DangerousFriendlySpacecraftCount == 0)
	{
		if (CrippledFriendlySpacecraftCount == FriendlySpacecraftCount)
		{
			return EFlareSectorBattleState::BattleLostNoRetreat;
		}
		else
		{
			return EFlareSectorBattleState::BattleLost;
		}
	}

	if (DangerousHostileSpacecraftCount == 0)
	{
		return EFlareSectorBattleState::BattleWon;
	}

	if (CrippledFriendlySpacecraftCount == FriendlySpacecraftCount)
	{
		return EFlareSectorBattleState::BattleNoRetreat;
	}
	else
	{
		return EFlareSectorBattleState::Battle;
	}
}

FText UFlareSimulatedSector::GetSectorFriendlynessText(UFlareCompany* Company)
{
	FText Status;

	switch (GetSectorFriendlyness(Company))
	{
		case EFlareSectorFriendlyness::NotVisited:
			Status = LOCTEXT("Unknown", "");
			break;
		case EFlareSectorFriendlyness::Neutral:
			Status = LOCTEXT("Neutral", "Neutral");
			break;
		case EFlareSectorFriendlyness::Friendly:
			Status = LOCTEXT("Friendly", "Friendly");
			break;
		case EFlareSectorFriendlyness::Contested:
			Status = LOCTEXT("Contested", "Contested");
			break;
		case EFlareSectorFriendlyness::Hostile:
			Status = LOCTEXT("Hostile", "Hostile");
			break;
	}

	return Status;
}

FLinearColor UFlareSimulatedSector::GetSectorFriendlynessColor(UFlareCompany* Company)
{
	FLinearColor Color;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	switch (GetSectorFriendlyness(Company))
	{
		case EFlareSectorFriendlyness::NotVisited:
			Color = Theme.UnknownColor;
			break;
		case EFlareSectorFriendlyness::Neutral:
			Color = Theme.NeutralColor;
			break;
		case EFlareSectorFriendlyness::Friendly:
			Color = Theme.FriendlyColor;
			break;
		case EFlareSectorFriendlyness::Contested:
			Color = Theme.DisputedColor;
			break;
		case EFlareSectorFriendlyness::Hostile:
			Color = Theme.EnemyColor;
			break;
	}

	return Color;
}


float UFlareSimulatedSector::GetPreciseResourcePrice(FFlareResourceDescription* Resource, int32 Age)
{
	if(Age == 0)
	{

		if (!ResourcePrices.Contains(Resource))
		{
			ResourcePrices.Add(Resource, GetDefaultResourcePrice(Resource));
		}

		return ResourcePrices[Resource];
	}
	else
	{
		if (!LastResourcePrices.Contains(Resource))
		{
			FFlareFloatBuffer Prices;
			Prices.Init(50);
			Prices.Append(GetPreciseResourcePrice(Resource, 0));
			LastResourcePrices.Add(Resource, Prices);
		}

		return LastResourcePrices[Resource].GetValue(Age);
	}

}

void UFlareSimulatedSector::SwapPrices()
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

		if (!LastResourcePrices.Contains(Resource))
		{
			FFlareFloatBuffer Prices;
			Prices.Init(50);
			LastResourcePrices.Add(Resource, Prices);
		}

		LastResourcePrices[Resource].Append(GetPreciseResourcePrice(Resource, 0));
	}
}

void UFlareSimulatedSector::SetPreciseResourcePrice(FFlareResourceDescription* Resource, float NewPrice)
{
	ResourcePrices[Resource] = FMath::Clamp(NewPrice, (float) Resource->MinPrice, (float) Resource->MaxPrice);
}

int64 UFlareSimulatedSector::GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext, int32 Age)
{
	int64 DefaultPrice = FMath::RoundToInt(GetPreciseResourcePrice(Resource, Age));

	switch (PriceContext)
	{
		case EFlareResourcePriceContext::Default:
			return DefaultPrice;
		break;
		case EFlareResourcePriceContext::FactoryOutput:
			return DefaultPrice - Resource->TransportFee;
		break;
		case EFlareResourcePriceContext::FactoryInput:
			return DefaultPrice + Resource->TransportFee;
		break;
		case EFlareResourcePriceContext::ConsumerConsumption:
			return DefaultPrice * 1.2; // TODO dynamic
		break;
		case EFlareResourcePriceContext::MaintenanceConsumption:
			return DefaultPrice * 1.5;
		break;
		default:
			return 0;
			break;
	}
}

float UFlareSimulatedSector::GetDefaultResourcePrice(FFlareResourceDescription* Resource)
{
	return (Resource->MinPrice + Resource->MaxPrice)/2;
}

uint32 UFlareSimulatedSector::GetTransfertResourcePrice(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource)
{
	UFlareSimulatedSpacecraft* Station = NULL;

	// Which one is any is a station ?
	if (SourceSpacecraft->IsStation())
	{
		Station = SourceSpacecraft;
	}
	else if (DestinationSpacecraft->IsStation())
	{
		Station = DestinationSpacecraft;
	}
	else
	{
		return GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
	}

	// Get context
	EFlareResourcePriceContext::Type ResourceUsage = Station->GetResourceUseType(Resource);
	if (ResourceUsage == EFlareResourcePriceContext::ConsumerConsumption ||
			ResourceUsage == EFlareResourcePriceContext::MaintenanceConsumption)
	{
		ResourceUsage = EFlareResourcePriceContext::FactoryInput;
	}

	// Get the usage and price
	return GetResourcePrice(Resource, ResourceUsage);
}


uint32 UFlareSimulatedSector::TransfertResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, uint32 Quantity)
{
	// TODO Check docking capabilities

	if(SourceSpacecraft->GetCurrentSector() != DestinationSpacecraft->GetCurrentSector())
	{
		FLOG("Warning cannot transfert resource because both ship are not in the same sector");
		return 0;
	}

	if(SourceSpacecraft->IsStation() && DestinationSpacecraft->IsStation())
	{
		FLOG("Warning cannot transfert resource between 2 stations");
		return 0;
	}

	int32 ResourcePrice = GetTransfertResourcePrice(SourceSpacecraft, DestinationSpacecraft, Resource);
	int32 QuantityToTake = Quantity;

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Limit transaction bay available money
		int32 MaxAffordableQuantity = DestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice;
		QuantityToTake = FMath::Min(QuantityToTake, MaxAffordableQuantity);
	}
	int32 ResourceCapacity = DestinationSpacecraft->GetCargoBay()->GetFreeSpaceForResource(Resource);

	QuantityToTake = FMath::Min(QuantityToTake, ResourceCapacity);


	int32 TakenResources = SourceSpacecraft->GetCargoBay()->TakeResources(Resource, QuantityToTake);
	int32 GivenResources = DestinationSpacecraft->GetCargoBay()->GiveResources(Resource, TakenResources);

	if (GivenResources > 0 && SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		// Pay
		int64 Price = ResourcePrice * GivenResources;
		DestinationSpacecraft->GetCompany()->TakeMoney(Price);
		SourceSpacecraft->GetCompany()->GiveMoney(Price);

		SourceSpacecraft->GetCompany()->GiveReputation(DestinationSpacecraft->GetCompany(), 0.5f, true);
		DestinationSpacecraft->GetCompany()->GiveReputation(SourceSpacecraft->GetCompany(), 0.5f, true);
	}

	return GivenResources;
}

bool UFlareSimulatedSector::CanUpgrade(UFlareCompany* Company)
{
	EFlareSectorBattleState::Type BattleState = GetSectorBattleState(Company);
	if(BattleState != EFlareSectorBattleState::NoBattle
			&& BattleState != EFlareSectorBattleState::BattleWon)
	{
		return false;
	}

	for(int StationIndex = 0 ; StationIndex < GetSectorStations().Num(); StationIndex ++ )
	{
		UFlareSimulatedSpacecraft* StationInterface = GetSectorStations()[StationIndex];
		if (StationInterface->GetCompany()->GetWarState(Company) != EFlareHostility::Hostile)
		{
			return true;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
