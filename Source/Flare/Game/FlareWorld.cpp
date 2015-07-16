
#include "../Flare.h"
#include "FlareWorld.h"


/*----------------------------------------------------
    Constructor
----------------------------------------------------*/


void UFlareWorld::Load(const FFlareWorldSave& Data)
{
    FLOGV("UFlareWorld::Load");
    WorldData = Data;

    // Load all companies
    for (int32 i = 0; i < WorldData.CompanyData.Num(); i++)
    {
        LoadCompany(Save->CompanyData[i]);
    }

    // Load all stations
    for (int32 i = 0; i < WorldData.StationData.Num(); i++)
    {
        LoadShip(Save->StationData[i]);
    }

    // Load all ships
    for (int32 i = 0; i < WorldData.ShipData.Num(); i++)
    {
        LoadShip(Save->ShipData[i]);
    }

    // Load all bombs
    for (int32 i = 0; i < WorldData.BombData.Num(); i++)
    {
        LoadBomb(Save->BombData[i]);
    }

    // Load all asteroids
    for (int32 i = 0; i < WorldData.AsteroidData.Num(); i++)
    {
        LoadAsteroid(Save->AsteroidData[i]);
    }
}


UFlareCompany* AFlareGame::LoadCompany(const FFlareCompanySave& CompanyData)
{
    UFlareCompany* Company = NULL;

    // Create the new company
    Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
    Company->Load(CompanyData);
    Companies.AddUnique(Company);

	FLOGV("AFlareGame::LoadCompany : loaded '%s'", *Company->GetCompanyName());

    return Company;
}

AFlareAsteroid* AFlareGame::LoadAsteroid(const FFlareAsteroidSave& AsteroidData)
{
    FActorSpawnParameters Params;
    Params.bNoFail = true;

    AFlareAsteroid* Asteroid = GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), AsteroidData.Location, AsteroidData.Rotation, Params);
    Asteroid->Load(AsteroidData);
    return Asteroid;
}

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

AFlareBomb* AFlareGame::LoadBomb(const FFlareBombSave& BombData)
{
    AFlareBomb* Bomb = NULL;
    FLOG("AFlareGame::LoadBomb");

    AFlareSpacecraft* ParentSpacecraft = NULL;

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
        if (SpacecraftCandidate && SpacecraftCandidate->GetImmatriculation() == BombData.ParentSpacecraft)
        {
            ParentSpacecraft = SpacecraftCandidate;
            break;
        }
    }

    if (ParentSpacecraft)
    {
        UFlareWeapon* ParentWeapon = NULL;
        TArray<UActorComponent*> Components = ParentSpacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
        for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
        {
            UFlareWeapon* WeaponCandidate = Cast<UFlareWeapon>(Components[ComponentIndex]);
            if (WeaponCandidate && WeaponCandidate->SlotIdentifier == BombData.WeaponSlotIdentifier)
            {

                ParentWeapon = WeaponCandidate;
                break;
            }
        }

        if (ParentWeapon)
        {
            // Spawn parameters
            FActorSpawnParameters Params;
            Params.bNoFail = true;

            // Create and configure the ship
            Bomb = GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombData.Location, BombData.Rotation, Params);
            if (Bomb)
            {
                Bomb->Initialize(&BombData, ParentWeapon);

                UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());

                RootComponent->SetPhysicsLinearVelocity(BombData.LinearVelocity, false);
                RootComponent->SetPhysicsAngularVelocity(BombData.AngularVelocity, false);
            }
            else
            {
                FLOG("AFlareGame::LoadBomb fail to create AFlareBom");
            }
        }
        else
        {
            FLOG("AFlareGame::LoadBomb failed (no parent weapon)");
        }
    }
    else
    {
        FLOG("AFlareGame::LoadBomb failed (no parent ship)");
    }

    return Bomb;
}
