#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSector.h"

// TODO rework


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSector::UFlareSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*----------------------------------------------------
  Save
----------------------------------------------------*/

void UFlareSector::Load(const FFlareSectorSave& Data)
{
	Destroy();

	Game = Cast<AFlareGame>(GetOuter());
	SectorData = Data;

	for(int i = 0 ; i < SectorData.AsteroidData.Num(); i++)
	{
		LoadAsteroid(SectorData.AsteroidData[i]);
	}

	for(int i = 0 ; i < SectorData.ShipData.Num(); i++)
	{
		LoadShip(SectorData.ShipData[i]);
	}

	for(int i = 0 ; i < SectorData.StationData.Num(); i++)
	{
		LoadShip(SectorData.StationData[i]);
	}

	for(int i = 0 ; i < SectorData.BombData.Num(); i++)
	{
		LoadBomb(SectorData.BombData[i]);
	}
}

FFlareSectorSave* UFlareSector::Save()
{
	SectorData.ShipData.Empty();
	SectorData.StationData.Empty();
	SectorData.BombData.Empty();
	SectorData.AsteroidData.Empty();

	for(int i = 0 ; i < SectorShips.Num(); i++)
	{
		SectorData.ShipData.Add(*SectorShips[i]->Save());
	}

	for(int i = 0 ; i < SectorStations.Num(); i++)
	{
		SectorData.StationData.Add(*SectorStations[i]->Save());
	}

	for(int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorData.BombData.Add(*SectorBombs[i]->Save());
	}

	for(int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorData.AsteroidData.Add(*SectorAsteroids[i]->Save());
	}

	return &SectorData;
}

void UFlareSector::Destroy()
{
	for(int i = 0 ; i < SectorShips.Num(); i++)
	{
		SectorShips[i]->Destroy();
	}

	for(int i = 0 ; i < SectorStations.Num(); i++)
	{
		SectorStations[i]->Destroy();
	}

	for(int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorBombs[i]->Destroy();
	}

	for(int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorAsteroids[i]->Destroy();
	}

	for(int i = 0 ; i < SectorShells.Num(); i++)
	{
		SectorShells[i]->Destroy();
	}


	SectorShips.Empty();
	SectorStations.Empty();
	SectorBombs.Empty();
	SectorAsteroids.Empty();
	SectorShells.Empty();

	Game = NULL;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


AFlareAsteroid* UFlareSector::LoadAsteroid(const FFlareAsteroidSave& AsteroidData)
{
    FActorSpawnParameters Params;
    Params.bNoFail = true;

	AFlareAsteroid* Asteroid = Game->GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), AsteroidData.Location, AsteroidData.Rotation, Params);
    Asteroid->Load(AsteroidData);

	// TODO Check double add
	SectorAsteroids.Add(Asteroid);
    return Asteroid;
}

// TODO rename spacecraft
AFlareSpacecraft* UFlareSector::LoadShip(const FFlareSpacecraftSave& ShipData)
{
    AFlareSpacecraft* Ship = NULL;
    FLOGV("AFlareGame::LoadShip ('%s')", *ShipData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipData.Identifier);
	if (Desc)
	{
		// Spawn parameters
		FActorSpawnParameters Params;
		Params.bNoFail = true;

		// Create and configure the ship
		Ship = Game->GetWorld()->SpawnActor<AFlareSpacecraft>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
		if (Ship)
		{
			Ship->Load(ShipData);
			UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
			RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
			RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);

			if(Ship->IsStation())
			{
				SectorStations.Add(Ship);
			}
			else
			{
				SectorShips.Add(Ship);
			}
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


    return Ship;
}

AFlareBomb* UFlareSector::LoadBomb(const FFlareBombSave& BombData)
{
    AFlareBomb* Bomb = NULL;
    FLOG("AFlareGame::LoadBomb");

    AFlareSpacecraft* ParentSpacecraft = NULL;

	// TODO iterate on SectorShips
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
			Bomb = Game->GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombData.Location, BombData.Rotation, Params);
            if (Bomb)
            {
                Bomb->Initialize(&BombData, ParentWeapon);

                UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());

                RootComponent->SetPhysicsLinearVelocity(BombData.LinearVelocity, false);
                RootComponent->SetPhysicsAngularVelocity(BombData.AngularVelocity, false);

				SectorBombs.Add(Bomb);
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



AFlareSpacecraft* UFlareSector::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
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

AFlareSpacecraft* UFlareSector::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
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


AFlareSpacecraft* UFlareSector::CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
    AFlareSpacecraft* ShipPawn = NULL;
	UFlareCompany* Company = Game->GetGameWorld()->FindCompany(CompanyIdentifier);

	FLOG("UFlareSector::CreateShip");

    if (ShipDescription && Company)
    {
		FLOG("UFlareSector::CreateShip ShipDescription && Company");
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
        ShipPawn = LoadShip(ShipData);
        FLOGV("AFlareGame::CreateShip : Created ship '%s' at %s", *ShipPawn->GetImmatriculation(), *TargetPosition.ToString());
    }

    return ShipPawn;
}

void UFlareSector::CreateAsteroidAt(int32 ID, FVector Location)
{
	if(ID >= Game->GetAsteroidCatalog()->Asteroids.Num())
	{
		FLOGV("Astroid create fail : Asteroid max ID is %d", Game->GetAsteroidCatalog()->Asteroids.Num() -1);
		return;
	}

	// Spawn parameters
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	FFlareAsteroidSave Data;
	Data.AsteroidMeshID = ID;
	Data.LinearVelocity = FVector::ZeroVector;
	Data.AngularVelocity = FMath::VRand() * FMath::FRandRange(-1.f,1.f);
	Data.Scale = FVector(1,1,1) * FMath::FRandRange(0.9,1.1);
	FRotator Rotation = FRotator(FMath::FRandRange(0,360), FMath::FRandRange(0,360), FMath::FRandRange(0,360));

	// Spawn and setup
	AFlareAsteroid* Asteroid = Game->GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), Location, Rotation, Params);
	Asteroid->Load(Data);

}

void UFlareSector::EmptySector()
{
	FLOG("UFlareSector::EmptySector");

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	AFlareSpacecraft* CurrentPlayedShip = NULL;

	if (PC)
	{
		// Current played ship
		CurrentPlayedShip = PC->GetShipPawn();
	}


	for(int i = 0 ; i < SectorShips.Num(); i++)
	{
		if (SectorShips[i] != CurrentPlayedShip)
		{
			SectorShips[i]->Destroy();
		}
	}

	for(int i = 0 ; i < SectorStations.Num(); i++)
	{
		SectorStations[i]->Destroy();
	}

	for(int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorBombs[i]->Destroy();
	}

	for(int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorAsteroids[i]->Destroy();
	}

	for(int i = 0 ; i < SectorShells.Num(); i++)
	{
		SectorShells[i]->Destroy();
	}




	// TODO don't use iterator
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode() && SpacecraftCandidate != CurrentPlayedShip)
		{
			SpacecraftCandidate->Destroy();
		}

		AFlareBomb* BombCandidate = Cast<AFlareBomb>(*ActorItr);
		if (BombCandidate)
		{
			BombCandidate->Destroy();
		}

		AFlareShell* ShellCandidate = Cast<AFlareShell>(*ActorItr);
		if (ShellCandidate)
		{
			ShellCandidate->Destroy();
		}

		AFlareAsteroid* AsteroidCandidate = Cast<AFlareAsteroid>(*ActorItr);
		if (AsteroidCandidate)
		{
			AsteroidCandidate->Destroy();
		}
	}
}
