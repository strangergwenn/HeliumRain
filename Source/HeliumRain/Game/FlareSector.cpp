
#include "FlareSector.h"
#include "../Flare.h"

#include "FlareGame.h"
#include "FlarePlanetarium.h"
#include "FlareSimulatedSector.h"
#include "FlareCollider.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareShell.h"
#include "../Spacecrafts/FlareSpacecraft.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSector::UFlareSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SectorRepartitionCache = false;
	IsDestroyingSector = false;
}

/*----------------------------------------------------
  Save
----------------------------------------------------*/

void UFlareSector::Load(UFlareSimulatedSector* Parent)
{
	DestroySector();
	ParentSector = Parent;
	LocalTime = Parent->GetData()->LocalTime;

	// Load asteroids
	for (int i = 0 ; i < ParentSector->GetData()->AsteroidData.Num(); i++)
	{
		LoadAsteroid(ParentSector->GetData()->AsteroidData[i]);
	}

	// Load meteorite
	for (FFlareMeteoriteSave& Meteorite : ParentSector->GetData()->MeteoriteData)
	{
		if (Meteorite.DaysBeforeImpact == 0 && Meteorite.Damage < Meteorite.BrokenDamage)
		{
			LoadMeteorite(Meteorite);
		}
	}

	// Load safe location spacecrafts
	for (int i = 0 ; i < ParentSector->GetSectorSpacecrafts().Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ParentSector->GetSectorSpacecrafts()[i];
		if (Spacecraft->GetData().SpawnMode == EFlareSpawnMode::Safe && (!Spacecraft->IsReserve() ||  Parent->GetGame()->GetPC()->GetPlayerShip() == Spacecraft))
		{
			LoadSpacecraft(Spacecraft);
		}
	}

	SectorRepartitionCache = false;

	// Load unsafe location spacecrafts
	for (int i = 0; i < ParentSector->GetSectorSpacecrafts().Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ParentSector->GetSectorSpacecrafts()[i];
		if (Spacecraft->GetData().SpawnMode != EFlareSpawnMode::Safe && (!Spacecraft->IsReserve() ||  Parent->GetGame()->GetPC()->GetPlayerShip() == Spacecraft))
		{
			LoadSpacecraft(Spacecraft);
		}
	}

	// Load bombs
	for (int i = 0; i < ParentSector->GetData()->BombData.Num(); i++)
	{
		LoadBomb(ParentSector->GetData()->BombData[i]);
	}
}

void UFlareSector::Save()
{
	FFlareSectorSave* SectorData  = GetSimulatedSector()->GetData();

	SectorData->BombData.Empty();
	SectorData->AsteroidData.Empty();
	// Meteorites have references but save must be call


	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorData->BombData.Add(*SectorBombs[i]->Save());
	}

	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorData->AsteroidData.Add(*SectorAsteroids[i]->Save());
	}

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		Meteorite->Save();
	}

	SectorData->LocalTime = LocalTime + GetGame()->GetPlanetarium()->GetSmoothTime();
}

void UFlareSector::DestroySector()
{
	FLOG("UFlareSector::DestroySector");

	IsDestroyingSector = true;

	// Remove spacecrafts from world
	for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		SectorSpacecrafts[SpacecraftIndex]->Destroy();
	}

	for (int BombIndex = 0 ; BombIndex < SectorBombs.Num(); BombIndex++)
	{
		SectorBombs[BombIndex]->Destroy();
	}

	for (int AsteroidIndex = 0 ; AsteroidIndex < SectorAsteroids.Num(); AsteroidIndex++)
	{
		SectorAsteroids[AsteroidIndex]->Destroy();
	}

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		Meteorite->Destroy();
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
	SectorMeteorites.Empty();
	SectorShells.Empty();

	IsDestroyingSector = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

AFlareAsteroid* UFlareSector::LoadAsteroid(const FFlareAsteroidSave& AsteroidData)
{
    FActorSpawnParameters Params;
    Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFlareAsteroid* Asteroid = GetGame()->GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), AsteroidData.Location, AsteroidData.Rotation, Params);
    Asteroid->Load(AsteroidData);

	SectorAsteroids.AddUnique(Asteroid);
    return Asteroid;
}

AFlareMeteorite* UFlareSector::LoadMeteorite(FFlareMeteoriteSave& MeteoriteData)
{
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFlareMeteorite* Meteorite = GetGame()->GetWorld()->SpawnActor<AFlareMeteorite>(AFlareMeteorite::StaticClass(), MeteoriteData.Location, MeteoriteData.Rotation, Params);
	Meteorite->Load(&MeteoriteData, this);

	SectorMeteorites.AddUnique(Meteorite);
	return Meteorite;
}

AFlareSpacecraft* UFlareSector::LoadSpacecraft(UFlareSimulatedSpacecraft* ParentSpacecraft)
{
	FCHECK(ParentSpacecraft);
	FCHECK(ParentSpacecraft);
	FCHECK(ParentSpacecraft->GetDescription());
	FCHECK(ParentSpacecraft->GetDescription()->SpacecraftTemplate);

	/*FLOGV("UFlareSector::LoadSpacecraft : Start loading '%s' (template '%s' at %x)",
		*ParentSpacecraft->GetImmatriculation().ToString(),
		*ParentSpacecraft->GetDescription()->SpacecraftTemplate->GetName(),
		ParentSpacecraft->GetDescription()->SpacecraftTemplate);
	*/
	// Spawn parameters
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Create and configure the ship
	AFlareSpacecraft* Spacecraft = GetGame()->GetWorld()->SpawnActor<AFlareSpacecraft>(ParentSpacecraft->GetDescription()->SpacecraftTemplate,
		ParentSpacecraft->GetData().Location,
		ParentSpacecraft->GetData().Rotation,
		Params);

	if (Spacecraft && !Spacecraft->IsPendingKillPending())
	{
		Spacecraft->Load(ParentSpacecraft);
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

		switch (ParentSpacecraft->GetData().SpawnMode)
		{
			// Already known to be correct
			case EFlareSpawnMode::Safe:

				/*FLOGV("UFlareSector::LoadSpacecraft : Safe spawn '%s' at (%f,%f,%f)",
					*ParentSpacecraft->GetImmatriculation().ToString(),
					ParentSpacecraft->GetData().Location.X, ParentSpacecraft->GetData().Location.Y, ParentSpacecraft->GetData().Location.Z);
				*/
				RootComponent->SetPhysicsLinearVelocity(ParentSpacecraft->GetData().LinearVelocity, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(ParentSpacecraft->GetData().AngularVelocity, false);
				break;

			// First spawn
			case EFlareSpawnMode::Spawn:

				PlaceSpacecraft(Spacecraft, ParentSpacecraft->GetData().Location);
				{
					FVector NewLocation = Spacecraft->GetActorLocation();
					//FLOGV("UFlareSector::LoadSpacecraft : Placing '%s' at (%f,%f,%f)",
					//	*ParentSpacecraft->GetImmatriculation().ToString(),
					//	NewLocation.X, NewLocation.Y, NewLocation.Z);
				}

				RootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
				break;

			// Incoming in sector
			case EFlareSpawnMode::Travel:
			{

				//FLOGV("UFlareSector::LoadSpacecraft : Travel '%s' at (%f, %f, %f)",
				//	*ParentSpacecraft->GetImmatriculation().ToString(),
				//	ParentSpacecraft->GetData().Location.X, ParentSpacecraft->GetData().Location.Y, ParentSpacecraft->GetData().Location.Z);

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

				float SpawnDistance = GetSectorRadius() + 1;

				if (GetSimulatedSector()->GetSectorBattleState(Spacecraft->GetCompany()).InBattle)
				{
					SpawnDistance += 500000; // 5 km
				}

				SpawnDistance = FMath::Min(SpawnDistance, GetSectorLimits());

				FVector Location = GetSectorCenter() + SpawnDirection * SpawnDistance;

				FVector CenterDirection = (GetSectorCenter() - Location).GetUnsafeNormal();
				Spacecraft->SetActorRotation(CenterDirection.Rotation());

				PlaceSpacecraft(Spacecraft, Location);

				float SpawnVelocity = 0;

				if (GetSimulatedSector()->GetSectorBattleState(Spacecraft->GetCompany()).InBattle)
				{
						SpawnVelocity = 10000;
				}

				RootComponent->SetPhysicsLinearVelocity(CenterDirection * SpawnVelocity, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
			}
			break;
			case EFlareSpawnMode::Exit:
			{
				float SpawnDistance = GetSectorLimits() * 0.9;
				float SpawnVelocity = ParentSpacecraft->GetData().LinearVelocity.Size() * 0.6;
				FVector SpawnDirection = ParentSpacecraft->GetData().Location.GetUnsafeNormal();
				FVector Location = SpawnDirection * SpawnDistance;
				FVector CenterDirection = (GetSectorCenter() - Location).GetUnsafeNormal();

				//FLOGV("UFlareSector::LoadSpacecraft : Exit '%s' at (%f, %f, %f)",
				//	*ParentSpacecraft->GetImmatriculation().ToString(),
				//	Location.X, Location.Y, Location.Z);

				PlaceSpacecraft(Spacecraft, Location);
				Spacecraft->SetActorRotation(CenterDirection.Rotation());

				RootComponent->SetPhysicsLinearVelocity(CenterDirection * SpawnVelocity, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);

			}
			break;
		}

		if (ParentSpacecraft->GetData().SpawnMode == EFlareSpawnMode::Travel && GetSimulatedSector()->IsTravelSector())
		{
			FLOG("UFlareSector::LoadSpacecraft : ship is still traveling");
		}
		else
		{
			ParentSpacecraft->SetSpawnMode(EFlareSpawnMode::Safe);
		}
	}
	else
	{
		FLOG("UFlareSector::LoadSpacecraft : failed to create AFlareSpacecraft");
	}

	if (ParentSpacecraft->IsComplex())
	{
		for(UFlareSimulatedSpacecraft* Child: ParentSpacecraft->GetComplexChildren())
		{
			LoadSpacecraft(Child);
		}
	}

	return Spacecraft;
}

AFlareBomb* UFlareSector::LoadBomb(const FFlareBombSave& BombData)
{
    AFlareBomb* Bomb = NULL;
    FLOG("UFlareSector::LoadBomb");

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
			Bomb = GetGame()->GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombData.Location, BombData.Rotation, Params);
            if (Bomb)
            {
                Bomb->Initialize(&BombData, ParentWeapon);

                UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());

                RootComponent->SetPhysicsLinearVelocity(BombData.LinearVelocity, false);
                RootComponent->SetPhysicsAngularVelocityInDegrees(BombData.AngularVelocity, false);

				SectorBombs.Add(Bomb);
            }
            else
            {
                FLOG("UFlareSector::LoadBomb fail to create AFlareBom");
            }
        }
        else
        {
            FLOG("UFlareSector::LoadBomb failed (no parent weapon)");
        }
    }
    else
    {
        FLOG("UFlareSector::LoadBomb failed (no parent ship)");
    }

    return Bomb;
}

void UFlareSector::RegisterBomb(AFlareBomb* Bomb)
{
	SectorBombs.AddUnique(Bomb);
}

void UFlareSector::UnregisterBomb(AFlareBomb* Bomb)
{
	if (!IsDestroyingSector)
	{
		SectorBombs.Remove(Bomb);
	}
}

void UFlareSector::RegisterShell(AFlareShell* Shell)
{
	SectorShells.AddUnique(Shell);
}

void UFlareSector::UnregisterShell(AFlareShell* Shell)
{
	if (!IsDestroyingSector)
	{
		SectorShells.Remove(Shell);
	}
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

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		Meteorite->SetPause(Pause);
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

	TArray<AActor*> ColliderActorList;
	UGameplayStatics::GetAllActorsOfClass(GetGame()->GetWorld(), AFlareCollider::StaticClass(), ColliderActorList);
	for (int32 ColliderIndex = 0; ColliderIndex < ColliderActorList.Num(); ColliderIndex++)
	{
		AActor* ColliderCandidate = ColliderActorList[ColliderIndex];

		float CandidateSize = Cast<UStaticMeshComponent>(ColliderCandidate->GetRootComponent())->Bounds.SphereRadius;

		float Distance = FVector::Dist(ColliderCandidate->GetActorLocation(), Location) - CandidateSize;
		if (ColliderCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = ColliderCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	*NearestDistance = NearestCandidateActorDistance;
	return NearestCandidateActor;
}

void UFlareSector::PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location)
{
	float RandomLocationRadiusIncrement = 100000; // 1000m
	float RandomLocationRadius = RandomLocationRadiusIncrement;
	float EffectiveDistance = -1;

	do 
	{
		Location += FMath::VRand() * RandomLocationRadius;
		float Size = (Spacecraft->IsStation() ? 80000 : Spacecraft->GetMeshScale());
		float NearestDistance;

		// Check if location is secure
		if (GetNearestBody(Location, &NearestDistance, true, Spacecraft) == NULL)
		{
			// No other ship.
			break;
		}

		EffectiveDistance = NearestDistance - Size;
		RandomLocationRadius += RandomLocationRadiusIncrement;
	}
	while (EffectiveDistance <= 0 && RandomLocationRadius < RandomLocationRadiusIncrement * 1000);

#if !UE_BUILD_SHIPPING
	{
		TArray<AActor*> ColliderActorList;
		UGameplayStatics::GetAllActorsOfClass(GetGame()->GetWorld(), AFlareCollider::StaticClass(), ColliderActorList);
		for (int32 ColliderIndex = 0; ColliderIndex < ColliderActorList.Num(); ColliderIndex++)
		{
			AActor* ColliderCandidate = ColliderActorList[ColliderIndex];

			float CandidateSize = Cast<UStaticMeshComponent>(ColliderCandidate->GetRootComponent())->Bounds.SphereRadius;
			float SpacecraftSize = Spacecraft->GetSimpleCollisionRadius();
			float Distance = FVector::Dist(ColliderCandidate->GetActorLocation(), Location);
			
			if (Distance < CandidateSize + SpacecraftSize)
			{
				FLOGV("UFlareSector::PlaceSpacecraft : %s was placed inside collider '%s'", *Spacecraft->GetImmatriculation().ToString(), *ColliderCandidate->GetName());
			}
		}
	}
#endif

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
