#include "../Flare.h"
#include "FlareSector.h"

// TODO rework


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



AFlareSpacecraft* AFlareGame::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(StationClass);

	if (!Desc)
	{
		Desc = GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition, TargetRotation);
	}
	return NULL;
}

AFlareSpacecraft* AFlareGame::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition);
	}
	return NULL;
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


// TODO Save
static void Save()
{
	// Save all physical ships
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Tentative casts
		AFlareMenuPawn* MenuPawn = PC->GetMenuPawn();
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(*ActorItr);
		AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(*ActorItr);

		// Ship
		if (Ship && Ship->GetDescription() && !Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentSpacecraft()))
		{
			FLOGV("AFlareGame::SaveWorld : saving ship ('%s')", *Ship->GetImmatriculation());
			FFlareSpacecraftSave* TempData = Ship->Save();
			Save->ShipData.Add(*TempData);
		}

		// Station
		else if (Ship && Ship->GetDescription() && Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentSpacecraft()))
		{
			FLOGV("AFlareGame::SaveWorld : saving station ('%s')", *Ship->GetImmatriculation());
			FFlareSpacecraftSave* TempData = Ship->Save();
			Save->StationData.Add(*TempData);
		}

		// Asteroid
		else if (Asteroid)
		{
			FLOGV("AFlareGame::SaveWorld : saving asteroid ('%s')", *Asteroid->GetName());
			FFlareAsteroidSave* TempData = Asteroid->Save();
			Save->AsteroidData.Add(*TempData);
		}
	}
}


// TODO
void AFlareGame::DeleteSector()
{
	FLOG("AFlareGame::DeleteWorld");
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode())
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

	Companies.Empty();
	LoadedOrCreated = false;
}


void AFlareSector::CreateAsteroidAt(int32 ID, FVector Location)
{
	if(ID >= GetAsteroidCatalog()->Asteroids.Num())
	{
		FLOGV("Astroid create fail : Asteroid max ID is %d", GetAsteroidCatalog()->Asteroids.Num() -1);
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
	AFlareAsteroid* Asteroid = GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), Location, Rotation, Params);
	Asteroid->Load(Data);

}

void AFlareSector::EmptySector()
{
	FLOG("AFlareGame::EmptySector");

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	AFlareSpacecraft* CurrentPlayedShip = NULL;

	if (PC)
	{
		// Current played ship
		CurrentPlayedShip = PC->GetShipPawn();
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

	UFlareCompany* CurrentShipCompany = NULL;

	if(CurrentPlayedShip)
	{
		CurrentShipCompany = CurrentPlayedShip->GetCompany();
	}

	Companies.Empty();
	Companies.Add(CurrentShipCompany);
}
