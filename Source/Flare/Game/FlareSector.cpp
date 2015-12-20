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
		if (Spacecraft->Save()->SpawnMode == EFlareSpawnMode::Safe)
		{
			LoadSpacecraft(*Spacecraft->Save());
		}
	}

	SectorRepartitionCache = false;

	// Load unsafe location spacecrafts
	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);
		if (Spacecraft->Save()->SpawnMode != EFlareSpawnMode::Safe)
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
		if (!SectorSpacecrafts[i]->GetDamageSystem()->IsAlive())
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
	for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		if (!SectorSpacecrafts[SpacecraftIndex]->GetDamageSystem()->IsAlive())
		{
			// Remove from world
			DestroySpacecraft(SectorSpacecrafts[SpacecraftIndex], true);
		}
		else
		{
			SectorSpacecrafts[SpacecraftIndex]->Destroy();
		}
	}

	for (int BombIndex = 0 ; BombIndex < SectorBombs.Num(); BombIndex++)
	{
		SectorBombs[BombIndex]->Destroy();
	}

	for (int AsteroidIndex = 0 ; AsteroidIndex < SectorAsteroids.Num(); AsteroidIndex++)
	{
		SectorAsteroids[AsteroidIndex]->Destroy();
	}

	for (int ShellIndex = 0 ; ShellIndex < SectorShells.Num(); ShellIndex++)
	{
		SectorShells[ShellIndex]->Destroy();
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


			switch(ShipData.SpawnMode)
			{
				case EFlareSpawnMode::Safe:
					RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
					RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);
					break;
				case EFlareSpawnMode::Spawn:
					if (Desc->NeedAttachPoint)
					{
						AFlareAsteroid* AttachPoint = NULL;
						for (int AsteroidIndex = 0 ; AsteroidIndex < SectorAsteroids.Num(); AsteroidIndex++)
						{
							AFlareAsteroid* AsteroidCandidate = SectorAsteroids[AsteroidIndex];
							if (AsteroidCandidate->Save()->Identifier == ShipData.AttachPoint)
							{
								AttachPoint = AsteroidCandidate;
								break;
							}
						}
						if (AttachPoint)
						{
							bool SpacecraftAttachPointLocationFound = false;
							FVector Position = Spacecraft->GetActorLocation() + FVector(1,0,0);
							TArray<UActorComponent*> Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
							for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
							{
								UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
								if (Component->DoesSocketExist(FName("AttachPoint")))
								{
									SpacecraftAttachPointLocationFound = true;
									Position = Component->GetSocketLocation(FName("AttachPoint"));
									break;
								}
							}
							if (! SpacecraftAttachPointLocationFound)
							{
								FLOGV("AFlareGame::LoadSpacecraft failed to find 'AttachPoint' socket on '%s'", *ShipData.Immatriculation.ToString());
							}

							FBox AttachPointBox = AttachPoint->GetComponentsBoundingBox();
							float AttachPointSize = FMath::Max(AttachPointBox.GetExtent().Size(), 1.0f);

							FVector StationOffset = Position - Spacecraft->GetActorLocation();
							FVector StationDirection = StationOffset.GetUnsafeNormal();
							float AttachDistance = StationOffset.Size() + AttachPointSize;

							FVector StationLocation = AttachPoint->GetActorLocation() + StationDirection * AttachDistance;

							Spacecraft->SetActorLocation(StationLocation);
						}
					}
					else
					{
						PlaceSpacecraft(Spacecraft, ShipData.Location);
					}
					RootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
					RootComponent->SetPhysicsAngularVelocity(FVector::ZeroVector, false);
					break;
				case EFlareSpawnMode::Travel:

					FVector SpawnDirection;
					TArray<AFlareSpacecraft*> FriendlySpacecrafts = GetCompanySpacecrafts(Spacecraft->GetCompany());
					FVector FriendlyShipLocationSum = FVector::ZeroVector;
					int FriendlyShipCount = 0;

					for (int SpacecraftIndex = 0 ; SpacecraftIndex < FriendlySpacecrafts.Num(); SpacecraftIndex++)
					{
						AFlareSpacecraft *SpacecraftCandidate = FriendlySpacecrafts[SpacecraftIndex];
						if (!SpacecraftCandidate->IsStation() && SpacecraftCandidate != Spacecraft)
						{
							FriendlyShipLocationSum += SpacecraftCandidate->GetActorLocation();
							FriendlyShipCount++;
						}
					}

					if (FriendlyShipCount == 0)
					{
						FVector NotFriendlyShipLocationSum = FVector::ZeroVector;
						int NotFriendlyShipCount = 0;
						for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
						{
							AFlareSpacecraft *SpacecraftCandidate = SectorShips[SpacecraftIndex];
							if (SpacecraftCandidate != Spacecraft && SpacecraftCandidate->GetCompany() != Spacecraft->GetCompany())
							{
								NotFriendlyShipLocationSum += SpacecraftCandidate->GetActorLocation();
								NotFriendlyShipCount++;
							}
						}

						if (NotFriendlyShipCount == 0)
						{
							SpawnDirection = FMath::VRand();
						}
						else
						{
							FVector	NotFriendlyShipLocationMean = NotFriendlyShipLocationSum / NotFriendlyShipCount;
							SpawnDirection = (GetSectorCenter() - NotFriendlyShipLocationMean).GetUnsafeNormal();
						}
					}
					else
					{
						FVector	FriendlyShipLocationMean = FriendlyShipLocationSum / FriendlyShipCount;
						SpawnDirection = (FriendlyShipLocationMean - GetSectorCenter()).GetUnsafeNormal() ;
					}

					FVector Location = GetSectorCenter() + SpawnDirection * (500000 + GetSectorRadius());

					FVector CenterDirection = (GetSectorCenter() - Location).GetUnsafeNormal();
					Spacecraft->SetActorRotation(CenterDirection.Rotation());

					PlaceSpacecraft(Spacecraft, Location);

					RootComponent->SetPhysicsLinearVelocity(CenterDirection * 10000, false);
					RootComponent->SetPhysicsAngularVelocity(FVector::ZeroVector, false);
					break;
			}

			if (Desc->NeedAttachPoint)
			{
				// Attach the station to the attach point
				AFlareAsteroid* AttachPoint = NULL;
				for (int AsteroidIndex = 0 ; AsteroidIndex < SectorAsteroids.Num(); AsteroidIndex++)
				{
					AFlareAsteroid* AsteroidCandidate = SectorAsteroids[AsteroidIndex];
					if (AsteroidCandidate->Save()->Identifier == ShipData.AttachPoint)
					{
						AttachPoint = AsteroidCandidate;
						break;
					}
				}

				if (AttachPoint)
				{
					AttachPoint->AttachRootComponentToActor(Spacecraft,"", EAttachLocation::KeepWorldPosition, true);
				}
				else
				{
					FLOGV("AFlareGame::LoadSpacecraft failed to attach '%s' to attach point '%s'", *ShipData.Immatriculation.ToString(), *ShipData.AttachPoint.ToString());
				}
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

	if (!Destroying)
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

void UFlareSector::PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location)
{
	float RandomLocationRadius = 0;
	float RandomLocationRadiusIncrement = 50000; // 500m
	float EffectiveDistance = -1;

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
	if (!SectorRepartitionCache)
	{
		SectorRepartitionCache = true;
		SectorRadius = 0.0f;
		SectorCenter = FVector::ZeroVector;

		int SignificantObjectCount = 0;

		FVector SectorMin = FVector(INFINITY, INFINITY, INFINITY);
		FVector SectorMax = FVector(-INFINITY, -INFINITY, -INFINITY);

		for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft *Spacecraft = SectorSpacecrafts[SpacecraftIndex];
			if (Spacecraft->IsStation())
			{
				SectorMin = SectorMin.ComponentMin(Spacecraft->GetActorLocation());
				SectorMax = SectorMax.ComponentMax(Spacecraft->GetActorLocation());
				SignificantObjectCount++;
			}
		}

		for (int AsteroidsIndex = 0 ; AsteroidsIndex < SectorAsteroids.Num(); AsteroidsIndex++)
		{
			AFlareAsteroid* Asteroid = SectorAsteroids[AsteroidsIndex];
			SectorMin = SectorMin.ComponentMin(Asteroid->GetActorLocation());
			SectorMax = SectorMax.ComponentMax(Asteroid->GetActorLocation());
			SignificantObjectCount++;
		}

		if (SignificantObjectCount > 0)
		{
			// At least one station or asteroid in sector
			FVector BoxSize = SectorMax - SectorMin;
			SectorRadius = BoxSize.Size() / 2;
			SectorCenter = (SectorMax + SectorMin) / 2;
		}
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
