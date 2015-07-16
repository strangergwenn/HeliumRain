
#include "../Flare.h"
#include "FlareSimulatedSector.h"
#include "FlareGame.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSector::UFlareSimulatedSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareSimulatedSector::Load(const FFlareSectorSave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();

	SectorData = Data;
	SectorShips.Empty();
	SectorStations.Empty();

	for(int i = 0 ; i < SectorData.ShipData.Num(); i++)
	{
		LoadSpacecraft(SectorData.ShipData[i]);
	}

	for(int i = 0 ; i < SectorData.StationData.Num(); i++)
	{
		LoadSpacecraft(SectorData.StationData[i]);
	}
}

FFlareSectorSave* UFlareSimulatedSector::Save()
{
	SectorData.ShipData.Empty();
	SectorData.StationData.Empty();

	for(int i = 0 ; i < SectorShips.Num(); i++)
	{
		SectorData.ShipData.Add(*SectorShips[i]->Save());
	}

	for(int i = 0 ; i < SectorStations.Num(); i++)
	{
		SectorData.StationData.Add(*SectorStations[i]->Save());
	}

	return &SectorData;
}



UFlareSimulatedSpacecraft* UFlareSimulatedSector::LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	FLOGV("AFlareGame::LoadSpacecraft ('%s')", *SpacecraftData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(SpacecraftData.Identifier);
	if (Desc)
	{
		Spacecraft = NewObject<UFlareSimulatedSpacecraft>(this, UFlareSimulatedSpacecraft::StaticClass());
		Spacecraft->Load(SpacecraftData);
		// TODO station/ship dispatch
		SectorShips.Add(Spacecraft);
	}
	else
	{
		FLOG("UFlareSimulatedSector::LoadSpacecraft failed (no description available)");
	}

	return Spacecraft;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(StationClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition, TargetRotation);
	}
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition);
	}
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	UFlareCompany* Company = Game->GetGameWorld()->FindCompany(CompanyIdentifier);

    if (ShipDescription && Company)
    {
        // Default data
        FFlareSpacecraftSave ShipData;
        ShipData.Location = TargetPosition;
        ShipData.Rotation = TargetRotation;
        ShipData.LinearVelocity = FVector::ZeroVector;
        ShipData.AngularVelocity = FVector::ZeroVector;
		Game->Immatriculate(Company, ShipDescription->Identifier, &ShipData);
        ShipData.Identifier = ShipDescription->Identifier;
        ShipData.Heat = 600 * ShipDescription->HeatCapacity;
        ShipData.PowerOutageDelay = 0;
        ShipData.PowerOutageAcculumator = 0;

        FName RCSIdentifier;
        FName OrbitalEngineIdentifier;

        // Size selector
        if (ShipDescription->Size == EFlarePartSize::S)
        {
            RCSIdentifier = FName("rcs-piranha");
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
        ShipData.Pilot.Name = "Chewbaca";

        // Init company
        ShipData.CompanyIdentifier = CompanyIdentifier;

        // Create the ship
		Spacecraft = LoadSpacecraft(ShipData);
		FLOGV("UFlareSimulatedSector::CreateShip : Created ship '%s' at %s", *Spacecraft->GetImmatriculation(), *TargetPosition.ToString());
    }

	return Spacecraft;
}

