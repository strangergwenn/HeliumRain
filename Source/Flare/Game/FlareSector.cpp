#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSector.h"
#include "../Spacecrafts/FlareSpacecraft.h"

// TODO rework


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSector::UFlareSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SectorRepartitionCache = false;
}

/*----------------------------------------------------
  Save
----------------------------------------------------*/

void UFlareSector::Load(const FFlareSectorSave& Data, UFlareSimulatedSector* Sector)
{
	Destroy();

	Game = Cast<AFlareGame>(GetOuter());
	SectorData = Data;
	SimulatedSector = Sector;
	LocalTime = SectorData.LocalTime;

	for (int i = 0 ; i < SectorData.AsteroidData.Num(); i++)
	{
		LoadAsteroid(SectorData.AsteroidData[i]);
	}

	// Load safe location spacecrafts
	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);
		if (Spacecraft->Save()->SafeLocation)
		{
			LoadSpacecraft(*Spacecraft->Save());
		}
	}

	SectorRepartitionCache = false;

	// Load unsafe location spacecrafts
	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);
		if (!Spacecraft->Save()->SafeLocation)
		{
			LoadSpacecraft(*Spacecraft->Save());
		}
	}

	// Check docking once all spacecraft are loaded
	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		SectorSpacecrafts[i]->Redock();
	}

	for (int i = 0 ; i < SectorData.BombData.Num(); i++)
	{
		LoadBomb(SectorData.BombData[i]);
	}
}

FFlareSectorSave* UFlareSector::Save(TArray<FFlareSpacecraftSave>& SpacecraftData)
{
	SectorData.SpacecraftIdentifiers.Empty();
	SectorData.BombData.Empty();
	SectorData.AsteroidData.Empty();

	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		if(!SectorSpacecrafts[i]->GetDamageSystem()->IsAlive())
		{
			// Don't save destroyed ships
			FLOGV("UFlareSector::Save Don't save %s", *SectorSpacecrafts[i]->GetImmatriculation().ToString());
			continue;
		}
		FFlareSpacecraftSave* SpacecraftSave = SectorSpacecrafts[i]->Save();
		SectorData.SpacecraftIdentifiers.Add(SpacecraftSave->Immatriculation);
		SpacecraftData.Add(*SpacecraftSave);
		// TODO delete spacecraft
	}

	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorData.BombData.Add(*SectorBombs[i]->Save());
	}

	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorData.AsteroidData.Add(*SectorAsteroids[i]->Save());
	}

	SectorData.LocalTime = LocalTime + GetGame()->GetPlanetarium()->GetSmoothTime();
	return &SectorData;
}

void UFlareSector::Destroy()
{
	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		if(!SectorSpacecrafts[i]->GetDamageSystem()->IsAlive())
		{
			// Remove from world
			DestroySpacecraft(SectorSpacecrafts[i], true);
		}
		else
		{
			SectorSpacecrafts[i]->Destroy();
		}
	}

	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorBombs[i]->Destroy();
	}

	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorAsteroids[i]->Destroy();
	}

	for (int i = 0 ; i < SectorShells.Num(); i++)
	{
		SectorShells[i]->Destroy();
	}

	SectorSpacecrafts.Empty();
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

AFlareSpacecraft* UFlareSector::LoadSpacecraft(const FFlareSpacecraftSave& ShipData)
{
	AFlareSpacecraft* Spacecraft = NULL;
	FLOGV("AFlareGame::LoadSpacecraft ('%s')", *ShipData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipData.Identifier);
	if (Desc)
	{
		// Spawn parameters
		FActorSpawnParameters Params;
		Params.bNoFail = true;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Create and configure the ship
		Spacecraft = Game->GetWorld()->SpawnActor<AFlareSpacecraft>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
		if (Spacecraft && !Spacecraft->IsPendingKillPending())
		{
			Spacecraft->Load(ShipData);
			UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Spacecraft->GetRootComponent());

			if (Spacecraft->IsStation())
			{
				SectorStations.Add(Spacecraft);
			}
			else
			{
				SectorShips.Add(Spacecraft);
			}
			SectorSpacecrafts.Add(Spacecraft);


			if (!ShipData.SafeLocation)
			{
				// Secure location

				float RandomLocationRadius = 0;
				float RandomLocationRadiusIncrement = 1000; // 10m
				float EffectiveDistance = -1;
				FVector Location = ShipData.Location;

				while(EffectiveDistance < 0 && RandomLocationRadius < RandomLocationRadiusIncrement * 1000)
				{
					Location += FMath::VRand() * RandomLocationRadius;

					// Check if location is secure
					float Size = Spacecraft->GetMeshScale();


					float NearestDistance;
					if (GetNearestBody(Location, &NearestDistance, true, Spacecraft) == NULL)
					{
						// No other ship.
						break;
					}
					EffectiveDistance = NearestDistance - Size;

					RandomLocationRadius += RandomLocationRadiusIncrement;
				}
				Spacecraft->SetActorLocation(Location);
				RootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
				RootComponent->SetPhysicsAngularVelocity(FVector::ZeroVector, false);
			}
			else
			{
				RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
				RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);
			}
		}
		else
		{
			FLOG("AFlareGame::LoadSpacecraft fail to create AFlareSpacecraft");
		}
	}
	else
	{
		FLOG("AFlareGame::LoadSpacecraft failed (no description available)");
	}


	return Spacecraft;
}

AFlareBomb* UFlareSector::LoadBomb(const FFlareBombSave& BombData)
{
    AFlareBomb* Bomb = NULL;
    FLOG("AFlareGame::LoadBomb");

    AFlareSpacecraft* ParentSpacecraft = NULL;

	for (int i = 0 ; i < SectorShips.Num(); i++)
	{
		AFlareSpacecraft* SpacecraftCandidate = SectorShips[i];
		if (SpacecraftCandidate->GetImmatriculation() == BombData.ParentSpacecraft)
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

void UFlareSector::DestroySpacecraft(AFlareSpacecraft* Spacecraft, bool Destroying)
{
	FLOGV("UFlareSector::DestroySpacecraft %s", *Spacecraft->GetImmatriculation().ToString());

	if(!Destroying)
	{
		SectorSpacecrafts.Remove(Spacecraft);
		SectorShips.Remove(Spacecraft);
		SectorStations.Remove(Spacecraft);
	}

	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Game->GetGameWorld()->FindSpacecraft(Spacecraft->GetImmatriculation());
	SimulatedSpacecraft->GetCompany()->DestroySpacecraft(SimulatedSpacecraft);

	Spacecraft->Destroy();
}

void UFlareSector::SetPause(bool Pause)
{
	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		SectorSpacecrafts[i]->SetPause(Pause);
	}

	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorBombs[i]->SetPause(Pause);
	}

	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorAsteroids[i]->SetPause(Pause);
	}

	for (int i = 0 ; i < SectorShells.Num(); i++)
	{
		SectorShells[i]->SetPause(Pause);
	}
}


AActor* UFlareSector::GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize, AActor* ActorToIgnore)
{
	AActor* NearestCandidateActor = NULL;
	float NearestCandidateActorDistance = 0;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* SpacecraftCandidate = GetSpacecrafts()[SpacecraftIndex];
		float Distance = FVector::Dist(SpacecraftCandidate->GetActorLocation(), Location) - SpacecraftCandidate->GetMeshScale();
		if (SpacecraftCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = SpacecraftCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	for (int32 AsteroidIndex = 0; AsteroidIndex < GetAsteroids().Num(); AsteroidIndex++)
	{
		AFlareAsteroid* AsteroidCandidate = GetAsteroids()[AsteroidIndex];

		FBox CandidateBox = AsteroidCandidate->GetComponentsBoundingBox();
		float CandidateSize = FMath::Max(CandidateBox.GetExtent().Size(), 1.0f);

		float Distance = FVector::Dist(AsteroidCandidate->GetActorLocation(), Location) - CandidateSize;
		if (AsteroidCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = AsteroidCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	*NearestDistance = NearestCandidateActorDistance;
	return NearestCandidateActor;
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/


TArray<AFlareSpacecraft*> UFlareSector::GetCompanyShips(UFlareCompany* Company)
{
	TArray<AFlareSpacecraft*> CompanyShips;
	// TODO Cache

	for (int i = 0 ; i < SectorShips.Num(); i++)
	{
		if (SectorShips[i]->GetCompany() == Company)
		{
			CompanyShips.Add(SectorShips[i]);
		}
	}
	return CompanyShips;
}

TArray<AFlareSpacecraft*> UFlareSector::GetCompanySpacecrafts(UFlareCompany* Company)
{
	TArray<AFlareSpacecraft*> CompanySpacecrafts;
	// TODO Cache

	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		if (SectorSpacecrafts[i]->GetCompany() == Company)
		{
			CompanySpacecrafts.Add(SectorSpacecrafts[i]);
		}
	}
	return CompanySpacecrafts;
}

AFlareSpacecraft* UFlareSector::FindSpacecraft(FName Immatriculation)
{
	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		if (SectorSpacecrafts[i]->GetImmatriculation() == Immatriculation)
		{
			return SectorSpacecrafts[i];
		}
	}
	return NULL;
}


void UFlareSector::GenerateSectorRepartitionCache()
{
	if(!SectorRepartitionCache)
	{
		SectorRepartitionCache = true;
		SectorRadius = 0.0f;
		SectorCenter = FVector::ZeroVector;

		// TODO compute better center and radius including sector content
	}
}

FVector UFlareSector::GetSectorCenter()
{
	GenerateSectorRepartitionCache();
	return SectorCenter;
}

float UFlareSector::GetSectorRadius()
{
	GenerateSectorRepartitionCache();
	return SectorRadius;
}
