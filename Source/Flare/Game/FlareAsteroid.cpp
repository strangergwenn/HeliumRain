
#include "../Flare.h"
#include "FlareAsteroid.h"
#include "FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareAsteroid::AFlareAsteroid(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	GetStaticMeshComponent()->bTraceComplexOnMove = true;
	//GetStaticMeshComponent()->LDMaxDrawDistance = 1000000; // 10km
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	GetStaticMeshComponent()->SetLinearDamping(0);
	GetStaticMeshComponent()->SetAngularDamping(0);
	SetActorEnableCollision(true);

	// Settings
	PrimaryActorTick.bCanEverTick = true;
	SetMobility(EComponentMobility::Movable);
	Paused = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareAsteroid::Load(const FFlareAsteroidSave& Data)
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	if (Game)
	{
		AsteroidData = Data;
		GetStaticMeshComponent()->SetStaticMesh(Game->GetAsteroidCatalog()->Asteroids[Data.AsteroidMeshID]);
		SetActorScale3D(Data.Scale);
		GetStaticMeshComponent()->SetPhysicsLinearVelocity(Data.LinearVelocity);
		GetStaticMeshComponent()->SetPhysicsAngularVelocity(Data.AngularVelocity);
	}
}

FFlareAsteroidSave* AFlareAsteroid::Save()
{
	// Physical data
	AsteroidData.Location = GetActorLocation();
	AsteroidData.Rotation = GetActorRotation();
	if (!Paused)
	{
		AsteroidData.LinearVelocity = GetStaticMeshComponent()->GetPhysicsLinearVelocity();
		AsteroidData.AngularVelocity = GetStaticMeshComponent()->GetPhysicsAngularVelocity();
	}
	
	return &AsteroidData;
}

void AFlareAsteroid::SetPause(bool Pause)
{
	if (Paused == Pause)
	{
		return;
	}



	CustomTimeDilation = (Pause ? 0.f : 1.0);
	if (Pause)
	{
		Save(); // Save must be performed with the old pause state
	}

	GetStaticMeshComponent()->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		GetStaticMeshComponent()->SetPhysicsLinearVelocity(AsteroidData.LinearVelocity);
		GetStaticMeshComponent()->SetPhysicsAngularVelocity(AsteroidData.AngularVelocity);
	}
}
