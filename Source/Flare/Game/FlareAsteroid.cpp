
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
		
		// Actor scale
		float CollisionSize = GetStaticMeshComponent()->GetCollisionShape().GetExtent().Size();
		FVector ScaleFactor = Data.Scale * (20000.0f / CollisionSize);
		SetActorScale3D(ScaleFactor);

		// Mass scale
		FBodyInstance* BodyInst = GetStaticMeshComponent()->GetBodyInstance();
		BodyInst->MassScale = ScaleFactor.Size();
		BodyInst->UpdateMassProperties();

		// Physics status
		GetStaticMeshComponent()->SetPhysicsLinearVelocity(Data.LinearVelocity);
		GetStaticMeshComponent()->SetPhysicsAngularVelocity(Data.AngularVelocity);

		// Material
		AsteroidMaterial = UMaterialInstanceDynamic::Create(GetStaticMeshComponent()->GetMaterial(0), GetWorld());
		GetStaticMeshComponent()->SetMaterial(0, AsteroidMaterial);
		AsteroidMaterial->SetScalarParameterValue("IceMask", 0);// TODO FRED : 1 or 0 depending on sector :) 
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
