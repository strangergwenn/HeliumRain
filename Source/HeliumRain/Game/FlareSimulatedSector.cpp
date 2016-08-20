
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
	ShipData.DynamicComponentStateIdentifier = NAME_None;
	ShipData.DynamicComponentStateProgress = 0.f;
	ShipData.IsTrading = false;
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
	
	// Compute total available resources in company ships
	TArray<FFlareCargo> AvailableResources;

	for (int SpacecraftIndex = 0; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorShips[SpacecraftIndex];


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

UFlareSimulatedSpacecraft* UFlareSimulatedSector::BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	TArray<FText> Reasons;
	if (!CanBuildStation(StationDescription, Company, Reasons))
	{
		FLOGV("UFlareSimulatedSector::BuildStation : Failed to build station '%s' for company '%s' (%s)",
			*StationDescription->Identifier.ToString(),
			*Company->GetCompanyName().ToString(),
			*Reasons[0].ToString());
		return NULL;
	}

	int64 ProductionCost = GetStationConstructionFee(StationDescription->CycleCost.ProductionCost);

	// Pay station cost
	if (!Company->TakeMoney(ProductionCost))
	{
		return NULL;
	}

	GetPeople()->Pay(ProductionCost);

	// Take resource cost
	for (int ResourceIndex = 0; ResourceIndex < StationDescription->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &StationDescription->CycleCost.InputResources[ResourceIndex];
		uint32 ResourceToTake = FactoryResource->Quantity;
		FFlareResourceDescription* Resource = &(FactoryResource->Resource->Data);


		// Take from ships
		for (int ShipIndex = 0; ShipIndex < SectorShips.Num() && ResourceToTake > 0; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Ship->GetCargoBay()->TakeResources(Resource, ResourceToTake);
		}

		if (ResourceToTake > 0)
		{
			FLOG("UFlareSimulatedSector::BuildStation : Failed to take resource cost for build station a station but CanBuild test succeded");
		}
	}

	return CreateStation(StationDescription->Identifier, Company, FVector::ZeroVector);
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

	for (int SpacecraftIndex = 0; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorShips[SpacecraftIndex];


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


		// Take from ships
		for (int ShipIndex = 0; ShipIndex < SectorShips.Num() && ResourceToTake > 0; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Ship->GetCargoBay()->TakeResources(Resource, ResourceToTake);
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

int64 UFlareSimulatedSector::GetStationConstructionFee(int64 BasePrice)
{
	return BasePrice + 1000000 * SectorStations.Num();
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
