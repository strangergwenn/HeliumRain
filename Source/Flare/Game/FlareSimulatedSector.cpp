
#include "../Flare.h"
#include "FlareSimulatedSector.h"

// TODO rework


AFlareSpacecraft* AFlareGame::LoadShip(const FFlareSpacecraftSave& ShipData)
{
    AFlareSpacecraft* Ship = NULL;
    FLOGV("AFlareGame::LoadShip ('%s')", *ShipData.Immatriculation.ToString());

    if (SpacecraftCatalog)
    {
        FFlareSpacecraftDescription* Desc = SpacecraftCatalog->Get(ShipData.Identifier);
        if (Desc)
        {
            // Spawn parameters
            FActorSpawnParameters Params;
            Params.bNoFail = true;

            // Create and configure the ship
            Ship = GetWorld()->SpawnActor<AFlareSpacecraft>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
            if (Ship)
            {
                Ship->Load(ShipData);
                UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
                RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
                RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);
            }
            else
            {
                FLOG("AFlareGame::LoadShip fail to create AFlareSpacecraft");
            }
        }
        else
        {
            FLOG("AFlareGame::LoadShip failed (no description available)");
        }
    }
    else
    {
        FLOG("AFlareGame::LoadShip failed (no catalog data)");
    }

    return Ship;
}


AFlareSpacecraft* AFlareGame::CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
    AFlareSpacecraft* ShipPawn = NULL;
    UFlareCompany* Company = World->FindCompany(CompanyIdentifier);

    if (ShipDescription && Company)
    {
        // Default data
        FFlareSpacecraftSave ShipData;
        ShipData.Location = TargetPosition;
        ShipData.Rotation = TargetRotation;
        ShipData.LinearVelocity = FVector::ZeroVector;
        ShipData.AngularVelocity = FVector::ZeroVector;
        Immatriculate(Company, ShipDescription->Identifier, &ShipData);
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
            ComponentData.ComponentIdentifier = DefaultWeaponIdentifer;
            ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
            ComponentData.Damage = 0.f;
            ComponentData.Weapon.FiredAmmo = 0;
            ShipData.Components.Add(ComponentData);
        }

        for (int32 i = 0; i < ShipDescription->TurretSlots.Num(); i++)
        {
            FFlareSpacecraftComponentSave ComponentData;
            ComponentData.ComponentIdentifier = DefaultTurretIdentifer;
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
        ShipPawn = LoadShip(ShipData);
        FLOGV("AFlareGame::CreateShip : Created ship '%s' at %s", *ShipPawn->GetImmatriculation(), *TargetPosition.ToString());
    }

    return ShipPawn;
}

