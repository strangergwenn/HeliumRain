
#include "FlareSimulatedSector.h"
#include "../Flare.h"
#include "FlareWorld.h"
#include "FlareFleet.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareGameUserSettings.h"
#include <random>

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareMeteoriteCatalog.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../Quests/FlareQuestGenerator.h"

#include <ctime>


DECLARE_CYCLE_STAT(TEXT("FlareSector SimulatePriceVariation"), STAT_FlareSector_SimulatePriceVariation, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSector GetSectorFriendlyness"), STAT_FlareSector_GetSectorFriendlyness, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSector GetSectorBattleState"), STAT_FlareSector_GetSectorBattleState, STATGROUP_Flare);

#define FLEET_SUPPLY_CONSUMPTION_STATS 50
#define AI_MAX_STATION_PER_SECTOR 30

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
			Spacecraft->UpdateShipyardProduction();
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

	if (SectorData.FleetSupplyConsumptionStats.MaxSize != FLEET_SUPPLY_CONSUMPTION_STATS)
	{
		SectorData.FleetSupplyConsumptionStats.Resize(FLEET_SUPPLY_CONSUMPTION_STATS);
		SectorData.DailyFleetSupplyConsumption = 0;
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


UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateStation(FName StationClass, UFlareCompany* Company, bool UnderConstruction, FFlareStationSpawnParameters SpawnParameters)
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
		bool SafeSpawn = (SpawnParameters.AttachActorName != NAME_None);
		Station = CreateSpacecraft(Desc, Company, SpawnParameters.Location, SpawnParameters.Rotation, NULL, SafeSpawn, UnderConstruction);

		// Attach to asteroid
		if (Station && Desc->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid))
		{
			AttachStationToAsteroid(Station);
		}

		// Attach to world as substation
		if (Station && Desc->IsSubstation)
		{
			FCHECK(SpawnParameters.AttachActorName != NAME_None);
			AttachStationToActor(Station, SpawnParameters.AttachActorName);
		}

		// Attach to station complex as station element
		if (Station && SpawnParameters.AttachComplexStationName != NAME_None)
		{
			FCHECK(SpawnParameters.AttachComplexConnectorName != NAME_None);
			AttachStationToComplexStation(Station, SpawnParameters.AttachComplexStationName, SpawnParameters.AttachComplexConnectorName);
		}

		// Under construction
		if (UnderConstruction && Company == GetGame()->GetPC()->GetCompany())
		{
			Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("start-station-construction").PutInt32("upgrade", 0));
		}
	}

	return Station;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateSpacecraft(FName ShipClass, UFlareCompany* Company, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateSpacecraft(Desc, Company, TargetPosition);
	}
	else
	{
		FLOGV("CreateShip failed: Unkwnon ship %s", *ShipClass.ToString());
	}

	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateSpacecraft(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation,
	FFlareSpacecraftSave* CapturedSpacecraft, bool SafeSpawnAtLocation, bool UnderConstruction)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;

	// Default data
	FFlareSpacecraftSave ShipData;
	ShipData.IsDestroyed = false;
	ShipData.IsUnderConstruction = UnderConstruction;
	ShipData.Location = TargetPosition;
	ShipData.Rotation = TargetRotation;
	ShipData.LinearVelocity = FVector::ZeroVector;
	ShipData.AngularVelocity = FVector::ZeroVector;
	ShipData.SpawnMode = SafeSpawnAtLocation ? EFlareSpawnMode::Safe : EFlareSpawnMode::Spawn;
	Game->Immatriculate(Company, ShipDescription->Identifier, &ShipData);
	ShipData.Identifier = ShipDescription->Identifier;
	ShipData.Heat = 600 * ShipDescription->HeatCapacity;
	ShipData.PowerOutageDelay = 0;
	ShipData.PowerOutageAcculumator = 0;
	ShipData.DynamicComponentStateIdentifier = NAME_None;
	ShipData.DynamicComponentStateProgress = 0.f;
	ShipData.IsTrading = false;
	ShipData.IsIntercepted = false;
	ShipData.RepairStock = 0;
	ShipData.RefillStock = 0;
	ShipData.IsReserve = false;
	ShipData.AllowExternalOrder = true;
	ShipData.Level = 1;
	ShipData.HarpoonCompany = NAME_None;
	ShipData.DynamicComponentStateIdentifier = FName("idle");

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
		OrbitalEngineIdentifier = FName("pod-thera");
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
	ShipData.AsteroidData.Location = FVector::ZeroVector;
	ShipData.AsteroidData.LinearVelocity = FVector::ZeroVector;
	ShipData.AsteroidData.AngularVelocity = FVector::ZeroVector;
	ShipData.AsteroidData.Rotation = FRotator::ZeroRotator;

	// If captured, copy data from captured ship
	if (CapturedSpacecraft)
	{
		ShipData.Location = CapturedSpacecraft->Location;
		ShipData.Rotation = CapturedSpacecraft->Rotation;
		ShipData.Components = CapturedSpacecraft->Components;
		ShipData.Cargo = CapturedSpacecraft->Cargo;
		ShipData.FactoryStates = CapturedSpacecraft->FactoryStates;
		ShipData.AsteroidData = CapturedSpacecraft->AsteroidData;
		ShipData.DynamicComponentStateIdentifier = CapturedSpacecraft->DynamicComponentStateIdentifier;
		ShipData.DynamicComponentStateProgress = CapturedSpacecraft->DynamicComponentStateProgress;
		ShipData.Level = CapturedSpacecraft->Level;
		ShipData.SpawnMode = CapturedSpacecraft->SpawnMode;
		ShipData.AttachActorName = CapturedSpacecraft->AttachActorName;
		ShipData.IsUnderConstruction = CapturedSpacecraft->IsUnderConstruction;
		ShipData.CargoBackup = CapturedSpacecraft->CargoBackup;
	}

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
		// Add to player fleet if possible
		UFlareFleet* PlayerFleet = Game->GetPC()->GetPlayerFleet();

		if (Company == Game->GetPC()->GetCompany() && PlayerFleet &&  PlayerFleet->GetCurrentSector() == this)
		{
			PlayerFleet->AddShip(Spacecraft);
		}
		else
		{
			UFlareFleet* NewFleet = Company->CreateAutomaticFleet(Spacecraft);
		}
	}

	return Spacecraft;
}

void UFlareSimulatedSector::CreateAsteroid(int32 ID, FName Name, FVector Location)
{
	// Compute size
	float MinSize = 0.5;
	float MinMaxSize = 0.75;
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
	SectorStations.Remove(Spacecraft);
	SectorShips.Remove(Spacecraft);
	return SectorSpacecrafts.Remove(Spacecraft);
}


void UFlareSimulatedSector::SetSectorOrbitParameters(const FFlareSectorOrbitParameters& OrbitParameters)
{
	SectorOrbitParameters = OrbitParameters;
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

	// The sector must be known
	if (!Company->IsVisitedSector(this))
	{
		OutReasons.Add(LOCTEXT("StationVisitRequired", "You need to visit the sector first"));
		Result = false;
	}

	// Station technology
	if (!Company->IsTechnologyUnlockedStation(StationDescription))
	{
		OutReasons.Add(LOCTEXT("StationTechnologyRequired", "You need to unlock station technology first"));
		Result = false;
	}

	// The sector has danger
	if (GetSectorBattleState(Company).HasDanger)
	{
		OutReasons.Add(LOCTEXT("StationHasDanger", "There are enemies in this sector"));
		Result = false;
	}

	// Too many stations
	int32 StationCount = GetSectorCompanyStationCount(Company, true);

	if (StationCount >= GetMaxStationsPerCompany()/2 && !Company->IsTechnologyUnlocked("dense-sectors"))
	{
		OutReasons.Add(LOCTEXT("BuildNeedDenseSectors", "You have too many stations. Unlock 'dense sectors' technology to build more stations"));
		Result = false;
	}

	// Too many stations
	if (StationCount >= GetMaxStationsPerCompany())
	{
		OutReasons.Add(LOCTEXT("BuildTooManyStations", "You have too many stations"));
		Result = false;
	}

	// Too many stations
	if (!Company->IsPlayerCompany() && GetSectorStations().Num() > AI_MAX_STATION_PER_SECTOR)
	{
		OutReasons.Add(LOCTEXT("AIBuildTooManyStations", "AI can not build too much stations"));
		Result = false;
	}

	// Does it needs sun
	if (StationDescription->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure) && SectorDescription->IsSolarPoor)
	{
		OutReasons.Add(LOCTEXT("BuildRequiresSun", "This station can't be built near debris or dust"));
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
	if (Company->GetMoney() < GetStationConstructionFee(StationDescription->CycleCost.ProductionCost, Company))
	{
		OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresMoney", "Not enough credits ({0} / {1})"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetMoney())),
			FText::AsNumber(UFlareGameTools::DisplayMoney(GetStationConstructionFee(StationDescription->CycleCost.ProductionCost, Company)))));
		Result = false;
	}

	return Result;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company,	FFlareStationSpawnParameters SpawnParameters)
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

	int64 ProductionCost = GetStationConstructionFee(StationDescription->CycleCost.ProductionCost, Company);

	// Pay station cost
	if (!Company->TakeMoney(ProductionCost))
	{
		return NULL;
	}

	GetPeople()->Pay(ProductionCost);

	return CreateStation(StationDescription->Identifier, Company, true, SpawnParameters);
}

bool UFlareSimulatedSector::CanUpgradeStation(UFlareSimulatedSpacecraft* Station, TArray<FText>& OutReasons)
{
	bool Result = true;

	UFlareCompany* Company = Station->GetCompany();

	// Is under construction	
	if (Station->IsUnderConstruction())
	{
		OutReasons.Add(LOCTEXT("BuildImpossibleCOnstruction", "Can't upgrade stations in construction"));
		Result = false;
	}

	// Check money cost
	if (Company->GetMoney() < Station->GetStationUpgradeFee())
	{
		OutReasons.Add(FText::Format(LOCTEXT("BuildRequiresMoney", "Not enough credits ({0} / {1})"),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Company->GetMoney())),
			FText::AsNumber(UFlareGameTools::DisplayMoney(Station->GetStationUpgradeFee()))));
		Result = false;
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

void UFlareSimulatedSector::AttachStationToComplexStation(UFlareSimulatedSpacecraft* Spacecraft, FName AttachStationName, FName AttachConnectorName)
{
	UFlareSimulatedSpacecraft* Complex = GetGame()->GetGameWorld()->FindSpacecraft(AttachStationName);
	FCHECK(AttachConnectorName != NAME_None);
	FCHECK(Complex != NULL);

	// Setup connection with the parent
	FFlareConnectionSave ConnectionData;
	ConnectionData.ConnectorName = AttachConnectorName;
	ConnectionData.StationIdentifier = Spacecraft->GetImmatriculation();
	Complex->RegisterComplexElement(ConnectionData);
	
	// Setup connection with the guest
	Spacecraft->SetComplexStationAttachment(AttachStationName, AttachConnectorName);
}

void UFlareSimulatedSector::AttachStationToActor(UFlareSimulatedSpacecraft* Spacecraft, FName AttachActorName)
{
	Spacecraft->SetActorAttachment(AttachActorName);
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
	SCOPE_CYCLE_COUNTER(STAT_FlareSector_SimulatePriceVariation);

	float OldPrice = GetPreciseResourcePrice(Resource);
	// Prices can increase because :

	//  - The input of a station is low (and less than half)
	//  - Consumer ressource is low
	//  - Maintenance ressource is low (and less than half)


	// Prices can decrease because :
	//  - Output of a station is full (and more than half)
	//  - Consumer ressource is full (and more than half)
	//  - Maintenance ressource is full (and more than half) (very slow decrease)


	float WantedPriceSum = 0;
	float WantedWeightSum = 0;


	// Prices never go below min production cost
	for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[CountIndex];

		if(Station->GetCargoBay()->HasRestrictions())
		{
			// Not allow station with slot restriction to impact the price
			continue;
		}

		float StockRatio = FMath::Clamp((float) Station->GetCargoBay()->GetResourceQuantity(Resource, NULL) / (float) Station->GetCargoBay()->GetSlotCapacity(), 0.f, 1.f);

		for (int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

			if(!Factory->IsActive())
			{
				continue;
			}


			if (Factory->HasInputResource(Resource))
			{
				float Weight = Factory->GetInputResourceQuantity(Resource);
				WantedPriceSum += Weight * (1.f - StockRatio);
				WantedWeightSum += Weight;
			}

			if (Factory->HasOutputResource(Resource))
			{
				float Weight = Factory->GetOutputResourceQuantity(Resource);
				WantedPriceSum += Weight * (1.f - StockRatio);
				WantedWeightSum += Weight;
			}
		}

		if(Station->HasCapability(EFlareSpacecraftCapability::Consumer) && Resource->IsConsumerResource)
		{
			float Weight = GetPeople()->GetRessourceConsumption(Resource, false);
			WantedPriceSum += Weight * (1.f - StockRatio);
			WantedWeightSum += Weight;
		}

		if(Station->HasCapability(EFlareSpacecraftCapability::Maintenance) && Resource->IsMaintenanceResource)
		{
			WantedPriceSum += 1.f - StockRatio;
			WantedWeightSum += 1;
		}
	}

	if(WantedWeightSum > 0)
	{
		float MeanWantedPriceRatio = WantedPriceSum / WantedWeightSum;
		float OldPriceRatio = (OldPrice - Resource->MinPrice) / (float) (Resource->MaxPrice - Resource->MinPrice);


		float WantedVariation = MeanWantedPriceRatio - OldPriceRatio;


		/*FLOGV(">>> %s price in %s", *Resource->Name.ToString(), *GetSectorName().ToString());
		FLOGV("   WantedPriceSum %f", WantedPriceSum);
		FLOGV("   WantedWeightSum %f", WantedWeightSum);
		FLOGV("   MeanWantedPriceRatio %f", MeanWantedPriceRatio);
		FLOGV("   OldPrice %f", OldPrice);
		FLOGV("   OldPriceRatio %f", OldPriceRatio);
		FLOGV("   WantedVariation %f", WantedVariation);*/




		if(WantedVariation != 0.f)
		{



			float MaxPriceVariation = 10;
			float OldPriceRatioToVariationDirection;

			if(WantedVariation > 0)
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



			float Variation = VariationScale * WantedVariation;


			float NewPrice = FMath::Max(1.f, OldPrice * (1 + Variation / 100.f));


			/*FLOGV("   VariationScale %f", VariationScale);
			FLOGV("   Variation %f", Variation);
			FLOGV("   NewPrice %f", NewPrice);*/

			SetPreciseResourcePrice(Resource, NewPrice);
			/*if(NewPrice > Resource->MaxPrice)
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
			}*/
		}
	}
	else
	{

		// Find nearest sectors

		int64 MinTravelDuration = -1;
		int32 NumSector = 0;
		float PriceSum = 0;

		for (int SectorIndex = 0; SectorIndex < GetGame()->GetGameWorld()->GetSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* SectorCandidate = GetGame()->GetGameWorld()->GetSectors()[SectorIndex];

			if(SectorCandidate == this)
			{
				continue;
			}

			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), this, SectorCandidate, NULL);


			if (MinTravelDuration == -1 || MinTravelDuration > TravelDuration)
			{
				MinTravelDuration = TravelDuration;
				NumSector = 0;
				PriceSum = 0;
			}

			if (MinTravelDuration == TravelDuration)
			{
				PriceSum += GetPreciseResourcePrice(Resource);
				NumSector++;
			}
		}

		float MeanNearPrice = PriceSum / NumSector;
		SetPreciseResourcePrice(Resource, MeanNearPrice);
	}
}

bool UFlareSimulatedSector::WantSell(FFlareResourceDescription* Resource, UFlareCompany* Client)
{
	for (UFlareSimulatedSpacecraft* Station : GetSectorStations())
	{
		if (Station->GetCargoBay()->WantSell(Resource, Client, true))
		{
			return true;
		}
	}
	return false;
}

bool UFlareSimulatedSector::WantBuy(FFlareResourceDescription* Resource, UFlareCompany* Client)
{
	for (UFlareSimulatedSpacecraft* Station : GetSectorStations())
	{
		if (Station->GetCargoBay()->WantBuy(Resource, Client))
		{
			return true;
		}
	}
	return false;
}

void UFlareSimulatedSector::ClearBombs()
{
	for (int i = 0 ; i < SectorData.BombData.Num(); i++)
	{
		CombatLog::BombDestroyed(SectorData.BombData[i].Identifier);
	}

	SectorData.BombData.Empty();
}

void UFlareSimulatedSector::GetSectorBalance(UFlareCompany* Company, int32& PlayerShips, int32& EnemyShips, int32& NeutralShips, bool ActiveOnly)
{
	PlayerShips = 0;
	EnemyShips = 0;
	NeutralShips = 0;

	for (int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		if (ActiveOnly && !SectorShips[ShipIndex]->IsActive())
		{
			continue;
		}

		if (SectorShips[ShipIndex]->GetCompany()->GetWarState(Company) == EFlareHostility::Hostile
		 && SectorShips[ShipIndex]->IsMilitary() && !SectorShips[ShipIndex]->GetDamageSystem()->IsDisarmed())
		{
			EnemyShips++;
		}
		else if (SectorShips[ShipIndex]->GetCompany()->GetHostility(Company) == EFlareHostility::Owned
		 && SectorShips[ShipIndex]->IsMilitary() && !SectorShips[ShipIndex]->GetDamageSystem()->IsDisarmed())
		{
			PlayerShips++;
		}
		else
		{
			NeutralShips++;
		}
	}
}

FText UFlareSimulatedSector::GetSectorBalanceText(bool ActiveOnly)
{
	int32 PlayerShips, EnemyShips, NeutralShips;
	GetSectorBalance(GetGame()->GetPC()->GetCompany(), PlayerShips, EnemyShips, NeutralShips, ActiveOnly);

	FText PlayerShipsText = FText::Format(LOCTEXT("PlayerShipsFormat", "Ships : {0} friendly, "),
		FText::AsNumber(PlayerShips));

	FText HostileShipsText = FText::Format(LOCTEXT("HostileShipsFormat", "{0} {1}, "),
		FText::AsNumber(EnemyShips),
		EnemyShips > 1 ? LOCTEXT("HostileShips", "hostiles") : LOCTEXT("HostileShip", "hostile"));

	FText NeutralShipsText = FText::Format(LOCTEXT("NeutralShipsFormat", "{0} {1}"),
		FText::AsNumber(NeutralShips),
		NeutralShips > 1 ? LOCTEXT("NeutralShips", "neutrals") : LOCTEXT("NeutralShip", "neutral"));

	return FText::FromString(PlayerShipsText.ToString() + HostileShipsText.ToString() + NeutralShipsText.ToString());
}

int32 UFlareSimulatedSector::GetSectorCompanyStationCount(UFlareCompany* Company, bool IncludeCapture) const
{
	int32 CompanyStationCountInSector = 0;

	for(UFlareSimulatedSpacecraft* Station : SectorStations)
	{
		if(Station->GetCompany() == Company)
		{
			++CompanyStationCountInSector;
		}
	}

	CompanyStationCountInSector += Company->GetCaptureOrderCountInSector(this);

	return CompanyStationCountInSector;

}

int64 UFlareSimulatedSector::GetStationConstructionFee(int64 BasePrice, UFlareCompany* Company)
{
	return BasePrice;
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

		ResourceCount += Station->GetCargoBay()->GetResourceQuantity(Resource, NULL);
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
	SCOPE_CYCLE_COUNTER(STAT_FlareSector_GetSectorFriendlyness);

	if (!Company->HasVisitedSector(this))
	{
		return EFlareSectorFriendlyness::NotVisited;
	}

	if (GetSectorSpacecrafts().Num() == 0)
	{
		return EFlareSectorFriendlyness::Neutral;
	}

	int FriendlySpacecraftCount, HostileSpacecraftCount, NeutralSpacecraftCount;
	GetSectorBalance(Company, FriendlySpacecraftCount, HostileSpacecraftCount, NeutralSpacecraftCount, false);

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

FFlareSectorBattleState UFlareSimulatedSector::GetSectorBattleState(UFlareCompany* Company)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSector_GetSectorBattleState);

	FFlareSectorBattleState BattleState;
	BattleState.Init();

	if (GetSectorShips().Num() == 0)
	{
		return BattleState;
	}

	int HostileSpacecraftCount = 0;
	int DangerousHostileSpacecraftCount = 0;
	int DangerousHostileActiveSpacecraftCount = 0;


	int FriendlySpacecraftCount = 0;
	int DangerousFriendlySpacecraftCount = 0;
	int DangerousFriendlyActiveSpacecraftCount = 0;
	int CrippledFriendlySpacecraftCount = 0;

	int FriendlyStationCount = 0;
	int FriendlyStationInCaptureCount = 0;
	int FriendlyControllableShipCount = 0;


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
			if (!Spacecraft->GetDamageSystem()->IsDisarmed())
			{
				DangerousFriendlySpacecraftCount++;
				if(!Spacecraft->IsReserve())
				{
					DangerousFriendlyActiveSpacecraftCount++;
				}
			}

			if (Spacecraft->GetDamageSystem()->IsStranded())
			{
				CrippledFriendlySpacecraftCount++;
			}

			if(!Spacecraft->GetDamageSystem()->IsUncontrollable())
			{
				FriendlyControllableShipCount++;
			}

		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
			if (!Spacecraft->GetDamageSystem()->IsDisarmed())
			{
				DangerousHostileSpacecraftCount++;
				if(!Spacecraft->IsReserve())
				{
					DangerousHostileActiveSpacecraftCount++;
				}
			}
		}
	}

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < GetSectorStations().Num(); SpacecraftIndex++)
	{

		UFlareSimulatedSpacecraft* Spacecraft = GetSectorStations()[SpacecraftIndex];
		UFlareCompany* OtherCompany = Spacecraft->GetCompany();

		if (!Spacecraft->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
			CrippledFriendlySpacecraftCount++;

			FriendlyStationCount++;

			if (Spacecraft->IsBeingCaptured())
			{
				FriendlyStationInCaptureCount++;
			}
		}
		else if (OtherCompany->GetWarState(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
		}
	}

	BattleState.InBattle = true;
	BattleState.FriendlyStationCount = FriendlyStationCount;
	BattleState.FriendlyStationInCaptureCount = FriendlyStationInCaptureCount;
	BattleState.FriendlyControllableShipCount = FriendlyControllableShipCount;



	if (DangerousHostileSpacecraftCount > 0)
	{
		BattleState.HasDanger = true;
	}

	// No friendly or no hostile ship
	if (FriendlySpacecraftCount == 0 || HostileSpacecraftCount == 0)
	{
		BattleState.InBattle = false;
	}

	// No friendly and hostile ship are not dangerous
	if (DangerousFriendlySpacecraftCount == 0 && DangerousHostileSpacecraftCount == 0)
	{
		BattleState.InBattle = false;
	}

	if (CrippledFriendlySpacecraftCount != FriendlySpacecraftCount)
	{
		BattleState.RetreatPossible = false;
	}

	if(BattleState.InBattle)
	{
		// No friendly dangerous ship so the enemy have one. Battle is lost
		if (DangerousFriendlySpacecraftCount == 0)
		{
			BattleState.BattleWon = false;
		}
		else if (DangerousHostileSpacecraftCount == 0)
		{
			BattleState.BattleWon = true;
		}
		else
		{
			BattleState.InFight = true;

			if (DangerousFriendlyActiveSpacecraftCount == 0)
			{
				BattleState.ActiveFightWon = false;
			}
			else if (DangerousHostileActiveSpacecraftCount == 0)
			{
				BattleState.ActiveFightWon = true;
			}
			else
			{
				BattleState.InActiveFight = true;
			}
		}
	}

	return BattleState;
}


FText UFlareSimulatedSector::GetSectorBattleStateText(UFlareCompany* Company)
{
	FText BattleStatusText;

	FFlareSectorBattleState BattleState = GetSectorBattleState(Company);


	if(BattleState.InBattle)
	{
		if(BattleState.InFight)
		{
			if(BattleState.InActiveFight)
			{
				BattleStatusText = LOCTEXT("SectorBattleBattleFighing", "Battle in progress, fighting");
			}
			else if(BattleState.ActiveFightWon)
			{
				BattleStatusText = LOCTEXT("SectorBattleBattleWining", "Battle in progress, winning");
			}
			else
			{
				BattleStatusText = LOCTEXT("SectorBattleBattleLoosing", "Battle in progress, losing");
			}
		}
		else if(BattleState.BattleWon)
		{
			BattleStatusText = LOCTEXT("SectorBattleWon", "Battle won");
		}
		else
		{
			BattleStatusText = LOCTEXT("SectorBattleLostNoRetreat", "Battle lost");
		}

		if(BattleState.RetreatPossible)
		{
			BattleStatusText = FText::Format(LOCTEXT("BattleStatusWithRetreat", "{0}, retreat possible"), BattleStatusText);
		}
	}

	return BattleStatusText;
}

bool UFlareSimulatedSector::IsInDangerousBattle(UFlareCompany* Company)
{
	return GetSectorBattleState(Company).HasDanger;
}

FText UFlareSimulatedSector::GetSectorFriendlynessText(UFlareCompany* Company)
{
	FText Status;

	switch (GetSectorFriendlyness(Company))
	{
		case EFlareSectorFriendlyness::NotVisited:
			Status = LOCTEXT("Unknown", "Unknown");
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


static bool ReserveShipComparator (UFlareSimulatedSpacecraft& Ship1, UFlareSimulatedSpacecraft& Ship2)
{
	bool SELECT_SHIP1 = true;
	bool SELECT_SHIP2 = false;

	UFlareSimulatedSpacecraft* PlayerShip = Ship1.GetGame()->GetPC()->GetPlayerShip();
	UFlareFleet* PlayerFleet = Ship1.GetGame()->GetPC()->GetPlayerFleet();

	// Player ship is always the first in list
	if (&Ship1 == PlayerShip)
	{
		return SELECT_SHIP1;
	}

	if (&Ship2 == PlayerShip)
	{
		return SELECT_SHIP2;
	}

	// Ships in player fleet are prioritary
	if (Ship1.GetCurrentFleet() == PlayerFleet && Ship2.GetCurrentFleet() != PlayerFleet)
	{
		return SELECT_SHIP1;
	}

	if (Ship2.GetCurrentFleet() == PlayerFleet && Ship1.GetCurrentFleet() != PlayerFleet)
	{
		return SELECT_SHIP2;
	}

	// Priority to armed ships
	if (Ship1.GetDamageSystem()->IsDisarmed() && !Ship2.GetDamageSystem()->IsDisarmed())
	{
		return SELECT_SHIP2;
	}

	if (Ship2.GetDamageSystem()->IsDisarmed() && !Ship1.GetDamageSystem()->IsDisarmed())
	{
		return SELECT_SHIP1;
	}

	// Priority to controllable ships
	if (Ship1.GetDamageSystem()->IsUncontrollable() && !Ship2.GetDamageSystem()->IsUncontrollable())
	{
		return SELECT_SHIP2;
	}

	if (Ship2.GetDamageSystem()->IsUncontrollable() && !Ship1.GetDamageSystem()->IsUncontrollable())
	{
		return SELECT_SHIP1;
	}


	// TODO sort by fleet order


	// Priority to full ships
	if(Ship1.GetCargoBay()->GetUsedCargoSpace() != Ship2.GetCargoBay()->GetUsedCargoSpace())
	{
		return Ship1.GetCargoBay()->GetUsedCargoSpace() > Ship2.GetCargoBay()->GetUsedCargoSpace();
	}

	return FMath::RandBool();
}

void UFlareSimulatedSector::ProcessMeteorites()
{
	TArray<FFlareMeteoriteSave> MeteoriteToKeep;

	for(FFlareMeteoriteSave& Meteorite: SectorData.MeteoriteData)
	{
		if(Meteorite.DaysBeforeImpact > 0)
		{
			--Meteorite.DaysBeforeImpact;
			MeteoriteToKeep.Add(Meteorite);

			UFlareSimulatedSpacecraft* Station =  Game->GetGameWorld()->FindSpacecraft(Meteorite.TargetStation);

			bool PlayerTarget = false;

			if(Station && Station->GetCompany() == Game->GetPC()->GetCompany())
			{
				PlayerTarget = true;
			}


			if(GetGame()->GetQuestManager()->IsInterestingMeteorite(Meteorite))
			{
				if(Meteorite.DaysBeforeImpact == 0)
				{
					GetGame()->GetPC()->Notify(FText::Format(LOCTEXT("MeteoriteHere", "Meteorites in {0}"), GetSectorName()),
										FText::Format(LOCTEXT("MeteoriteHereFormat", "A meteorite group has entered {0} and threatens stations"), GetSectorName()),
										FName("meteorite-in-sector"),
										EFlareNotification::NT_Military,
										false);

				}
				else if(Meteorite.DaysBeforeImpact == 1 && !GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("early-warning") && PlayerTarget)
				{
					GetGame()->GetPC()->Notify(LOCTEXT("ImminentMeteoriteDetected", "Meteorites detected"),
										FText::Format(LOCTEXT("ImminentMeteoriteDetectedFormat", "A meteorite group has been detected as potential danger at {0}"), GetSectorName()),
										FName("meteorite-detected"),
										EFlareNotification::NT_Military,
										false);
				}

			}
		}
		else
		{
			if(Meteorite.Damage >= Meteorite.BrokenDamage || Meteorite.HasMissed)
			{
				continue;
			}

			UFlareSimulatedSpacecraft* TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(Meteorite.TargetStation);
			if(TargetSpacecraft)
			{
				UFlareSpacecraftComponentsCatalog* Catalog = Game->GetShipPartsCatalog();

				float Energy = Meteorite.BrokenDamage * Meteorite.LinearVelocity.SizeSquared() * 0.1;

				for (int32 ComponentIndex = 0; ComponentIndex < TargetSpacecraft->GetData().Components.Num(); ComponentIndex++)
				{
					FFlareSpacecraftComponentSave* TargetComponent = &TargetSpacecraft->GetData().Components[ComponentIndex];

					FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetComponent->ComponentIdentifier);

					float UsageRatio = TargetSpacecraft->GetDamageSystem()->GetUsableRatio(ComponentDescription, TargetComponent);
					float DamageRatio = TargetSpacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, TargetComponent);


					if (ComponentDescription && UsageRatio > 0)
					{
						if (ComponentDescription->Type == EFlarePartType::InternalComponent)
						{
							TargetSpacecraft->GetDamageSystem()->ApplyDamage(ComponentDescription, TargetComponent, Energy, EFlareDamage::DAM_Collision, NULL);
						}
					}
				}

				// Notify PC
				if(GetGame()->GetQuestManager()->IsInterestingMeteorite(Meteorite))
				{
					GetGame()->GetPC()->Notify(LOCTEXT("MeteoriteCrash", "Meteorite crashed"),
										FText::Format(LOCTEXT("MeteoriteCrashFormat", "A meteorite crashed on {0}"), UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft)),
										FName("meteorite-crash"),
										EFlareNotification::NT_Military,
										false);
				}

				GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("meteorite-hit-station").PutName("sector", GetIdentifier()));
			}
		}

	}

	SectorData.MeteoriteData.Empty();
	for(FFlareMeteoriteSave& Meteorite: MeteoriteToKeep)
	{
		SectorData.MeteoriteData.Add(Meteorite);
	}
}

void UFlareSimulatedSector::GenerateMeteorites()
{
	for(UFlareSimulatedSpacecraft* Station : SectorStations)
	{
		float Probability = 0.0004;
		if(FMath::FRand() >  Probability)
		{
			continue;
		}

		float PowerRatio = 1 + .002f * GetGame()->GetGameWorld()->GetDate();
		GenerateMeteoriteGroup(Station, PowerRatio);
	}

}

void UFlareSimulatedSector::GenerateMeteoriteGroup(UFlareSimulatedSpacecraft* TargetStation, float PowerRatio)
{
	std::mt19937 e2(time(0));

	// Velocity is pick with a standard deviation and a mean increasing with the powerRatio

	float BaseVelocity = 4000.f; //40 m/s
	float VelocityMean = BaseVelocity * (1 + (PowerRatio-1) *0.1);
	float VelocitySD = VelocityMean / 4;
	std::normal_distribution<> VelocityGen(VelocityMean, VelocitySD);

	float Velocity = FMath::Abs(VelocityGen(e2))+ 1.f;
	float VelocityRatio = Velocity / VelocityMean;

	bool IsMetal = Velocity > VelocityMean;


	float BaseResistance = 15000;
	float ResistanceMean = BaseResistance * PowerRatio / VelocityRatio; // Power use for velocity is not use for resistance
	float ResistanceSD = ResistanceMean / 4;
	std::normal_distribution<> ResistanceGen(ResistanceMean, ResistanceSD);

	float Resistance = FMath::Abs(ResistanceGen(e2)+ 1.f);


	float ResistancePerMeteoriteMean = 3000 + (1 + (1-PowerRatio) *0.5);
	float ResistancePerMeteoriteSD = ResistancePerMeteoriteMean / 10;
	std::normal_distribution<> ResistancePerMeteoriteGen(ResistancePerMeteoriteMean, ResistancePerMeteoriteSD);

	float ResistancePerMeteorite = FMath::Abs(ResistancePerMeteoriteGen(e2)) + 1.f;

	int32 Count = FMath::Max(1, FMath::RoundToInt(Resistance / ResistancePerMeteorite));


	std::normal_distribution<> MeteoriteResistanceGen(ResistancePerMeteorite, ResistancePerMeteorite/6);


	int32 MeshCount = IsMetal ? GetGame()->GetMeteoriteCatalog()->RockMeteorites.Num() : GetGame()->GetMeteoriteCatalog()->RockMeteorites.Num();


	std::normal_distribution<> AngularVelocityGen(0.f, 1.f);
	std::normal_distribution<> DaysGen(15.f, 3.f);

	FVector BaseLocation = TargetStation->GetData().Location + FMath::VRand() * FMath::FRandRange(1000000.f,1200000);

	int32 DaysBeforeImpact = FMath::Abs(DaysGen(e2)) + 1.f;


	std::normal_distribution<> OffsetGen(0.f, 1500.f);

	float TotalEffectiveResistance = 0;

	FVector VelocityVector = (TargetStation->GetData().Location - BaseLocation).GetUnsafeNormal() * Velocity;

	for(int i = 0; i< Count; i++)
	{
		FFlareMeteoriteSave Data;
		Data.TargetStation = TargetStation->GetImmatriculation();
		Data.MeteoriteMeshID = FMath::RandRange(0, MeshCount-1);
		Data.IsMetal = IsMetal;
		Data.BrokenDamage = FMath::Abs(MeteoriteResistanceGen(e2)+ 1.f);;
		Data.LinearVelocity = VelocityVector;
		Data.AngularVelocity = FMath::VRand() * AngularVelocityGen(e2);
		Data.Rotation = FRotator(FMath::FRandRange(0,360), FMath::FRandRange(0,360), FMath::FRandRange(0,360));

		Data.TargetOffset = FVector(OffsetGen(e2), OffsetGen(e2), OffsetGen(e2)) + Data.LinearVelocity.GetUnsafeNormal() * OffsetGen(e2) * 20;

		Data.Location = BaseLocation + Data.TargetOffset;




		Data.DaysBeforeImpact = DaysBeforeImpact;
		Data.Damage = 0;
		Data.HasMissed = false;

		SectorData.MeteoriteData.Add(Data);

		TotalEffectiveResistance += Data.BrokenDamage;
	}


	float EffectivePowerRatio = (TotalEffectiveResistance / BaseResistance) * (Velocity / BaseVelocity);

	if(TargetStation->GetCompany() == GetGame()->GetPC()->GetCompany())
	{
		if(GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("early-warning"))
		{
			GetGame()->GetPC()->Notify(LOCTEXT("MeteoriteDetected", "Meteorites detected"),
								FText::Format(LOCTEXT("MeteoriteDetectedFormat", "A meteorite group has been detected as potential danger for one of your stations at {0}"), GetSectorName()),
								FName("meteorite-detected"),
								EFlareNotification::NT_Military,
								false);
		}
	}
	else
	{
		GetGame()->GetQuestManager()->GetQuestGenerator()->GenerateMeteoriteQuest(TargetStation, EffectivePowerRatio, Count, DaysBeforeImpact);
	}

}

void UFlareSimulatedSector::UpdateFleetSupplyConsumptionStats()
{
	SectorData.FleetSupplyConsumptionStats.Append(SectorData.DailyFleetSupplyConsumption);
	SectorData.DailyFleetSupplyConsumption = 0;
}

void UFlareSimulatedSector::OnFleetSupplyConsumed(int32 Quantity)
{
	SectorData.DailyFleetSupplyConsumption += Quantity;
}

static const int32 MIN_SPAWN = 1;

void UFlareSimulatedSector::UpdateReserveShips()
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	int32 MaxShipsInSector = MyGameSettings->MaxShipsInSector;
	int32 TotalShipCount = GetSectorShips().Num();
	int32 TotalCargoShipCount = 0;
	int32 TotalMilitaryShipCount = 0;

	//FLOGV("UpdateReserveShips in %s: max=%d total=%d", *GetSectorName().ToString(), MaxShipsInSector, TotalShipCount);

	if (TotalShipCount <= MaxShipsInSector)
	{
		//FLOG("reserve no ship");
		for (auto Ship : GetSectorShips())
		{
			Ship->SetReserve(false);
		}
		return;
	}

	TArray<TArray<UFlareSimulatedSpacecraft*>> CargoShipListByCompanies;
	TArray<TArray<UFlareSimulatedSpacecraft*>> MilitaryShipListByCompanies;
	for (int32 CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		CargoShipListByCompanies.Add(TArray<UFlareSimulatedSpacecraft*>());
		MilitaryShipListByCompanies.Add(TArray<UFlareSimulatedSpacecraft*>());
	}

	for (UFlareSimulatedSpacecraft* Ship : GetSectorShips())
	{
		Ship->SetReserve(false);

		for (int32 CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			UFlareCompany* Company = GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
			if (Ship->GetCompany() == Company)
			{
				if(Ship->IsMilitary())
				{
					MilitaryShipListByCompanies[CompanyIndex].Add(Ship);
					TotalMilitaryShipCount++;
				}
				else
				{
					CargoShipListByCompanies[CompanyIndex].Add(Ship);
					TotalCargoShipCount++;
				}

				break;
			}
		}
	}


	// Count min ships to spawn
	int MinShipToSpawn = 0;
	int MinCargoShipToSpawn = 0;
	int MinMilitaryShipToSpawn = 0;

	for (int32 CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		int32 CargoCompanyShipCount = CargoShipListByCompanies[CompanyIndex].Num();
		int32 MilitaryCompanyShipCount = MilitaryShipListByCompanies[CompanyIndex].Num();

		if(CargoCompanyShipCount > 0)
		{
			MinShipToSpawn += MIN_SPAWN;
			MinCargoShipToSpawn += MIN_SPAWN;
		}

		if(MilitaryCompanyShipCount > 0)
		{
			MinShipToSpawn += MIN_SPAWN;
			MinMilitaryShipToSpawn += MIN_SPAWN;
		}
	}

	float MilitaryProportion = (GetSectorBattleState(Game->GetPC()->GetCompany()).InBattle ? 0.75f : 0.25);
	float CargoProportion = 1.f-MilitaryProportion;

	for (int32 CompanyIndex = 0; CompanyIndex < GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];

		int32 CargoCompanyShipCount = CargoShipListByCompanies[CompanyIndex].Num();
		if (CargoCompanyShipCount > 0)
		{
			float CompanyProportion = (float) (CargoCompanyShipCount - MIN_SPAWN) / (float) (TotalCargoShipCount - MinCargoShipToSpawn);
			int32 AllowedShipCount = FMath::Max(0, FMath::FloorToInt(CompanyProportion * CargoProportion * (MaxShipsInSector - MinShipToSpawn)));
			AllowedShipCount += MIN_SPAWN;
			//FLOGV("Allow %d/%d cargo for %s", AllowedShipCount, CargoCompanyShipCount, *Company->GetCompanyName().ToString());

			CargoShipListByCompanies[CompanyIndex].Sort(&ReserveShipComparator);
			for (int32 ShipIndex = AllowedShipCount; ShipIndex < CargoCompanyShipCount; ShipIndex++)
			{
				UFlareSimulatedSpacecraft* Ship = CargoShipListByCompanies[CompanyIndex][ShipIndex];
				Ship->SetReserve(true);
			}
		}

		int32 MilitaryCompanyShipCount = MilitaryShipListByCompanies[CompanyIndex].Num();
		if (MilitaryCompanyShipCount > 0)
		{
			float CompanyProportion = (float) (MilitaryCompanyShipCount - MIN_SPAWN) / (float) (TotalMilitaryShipCount - MinMilitaryShipToSpawn);
			int32 AllowedShipCount = FMath::Max(0, FMath::FloorToInt(CompanyProportion * MilitaryProportion * (MaxShipsInSector - MinShipToSpawn)));
			AllowedShipCount += MIN_SPAWN;
			//FLOGV("Allow %d/%d military for %s", AllowedShipCount, MilitaryCompanyShipCount, *Company->GetCompanyName().ToString());

			MilitaryShipListByCompanies[CompanyIndex].Sort(&ReserveShipComparator);
			for (int32 ShipIndex = AllowedShipCount; ShipIndex < MilitaryCompanyShipCount; ShipIndex++)
			{
				UFlareSimulatedSpacecraft* Ship = MilitaryShipListByCompanies[CompanyIndex][ShipIndex];
				Ship->SetReserve(true);
			}
		}
	}
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
			return DefaultPrice * 1.1 + Resource->TransportFee; // TODO dynamic
		break;
		case EFlareResourcePriceContext::MaintenanceConsumption:
			return DefaultPrice * 1.5  + Resource->TransportFee;
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
	if (SourceSpacecraft && SourceSpacecraft->IsStation())
	{
		Station = SourceSpacecraft;
	}
	else if (DestinationSpacecraft && DestinationSpacecraft->IsStation())
	{
		Station = DestinationSpacecraft;
	}
	else
	{
		return GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
	}

	// Get context
	EFlareResourcePriceContext::Type ResourceUsage = Station->GetResourceUseType(Resource);
	if (Station == DestinationSpacecraft && (ResourceUsage == EFlareResourcePriceContext::ConsumerConsumption ||
			ResourceUsage == EFlareResourcePriceContext::MaintenanceConsumption))
	{
		ResourceUsage = EFlareResourcePriceContext::FactoryInput;
	}

	// Get the usage and price
	return GetResourcePrice(Resource, ResourceUsage);
}

bool UFlareSimulatedSector::CanUpgrade(UFlareCompany* Company)
{
	// Can't upgrade during battles
	FFlareSectorBattleState BattleState = GetSectorBattleState(Company);
	if (BattleState.HasDanger)
	{
		return false;
	}

	// Look for a station with upgrade capability
	for (int StationIndex = 0 ; StationIndex < GetSectorStations().Num(); StationIndex ++ )
	{
		UFlareSimulatedSpacecraft* StationInterface = GetSectorStations()[StationIndex];
		if (StationInterface->GetCompany()->GetWarState(Company) != EFlareHostility::Hostile
			&& StationInterface->HasCapability(EFlareSpacecraftCapability::Upgrade))
		{
			return true;
		}
	}

	return false;
}

bool UFlareSimulatedSector::IsPlayerBattleInProgress()
{
	AFlarePlayerController* PC = GetGame()->GetPC();
	FFlareSectorBattleState BattleState = GetSectorBattleState(PC->GetCompany());

	if (BattleState.HasDanger)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int32 UFlareSimulatedSector::GetCompanyCapturePoints(UFlareCompany* Company) const
{
	int32 CapturePoints = 0;

	for (int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

		if (Ship->GetCompany() != Company)
		{
			continue;
		}

		if (Ship->GetDamageSystem()->IsDisarmed())
		{
			continue;
		}

		if (Ship->GetSize() ==  EFlarePartSize::S)
		{
			CapturePoints += 1;
		}

		if (Ship->GetSize() ==  EFlarePartSize::L)
		{
			CapturePoints += 10;
		}
	}
	return CapturePoints;
}
#undef LOCTEXT_NAMESPACE
