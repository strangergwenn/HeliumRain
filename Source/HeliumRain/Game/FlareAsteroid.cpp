
#include "../Flare.h"
#include "FlareAsteroid.h"
#include "FlareGame.h"
#include "FlareSimulatedSector.h"

#include "StaticMeshResources.h"


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
	AsteroidData = Data;

	SetupAsteroidMesh(Game, Game->GetActiveSector(), GetStaticMeshComponent(), Data);
	GetStaticMeshComponent()->SetPhysicsLinearVelocity(Data.LinearVelocity);
	GetStaticMeshComponent()->SetPhysicsAngularVelocity(Data.AngularVelocity);
}

void AFlareAsteroid::SetupAsteroidMesh(AFlareGame* Game, UFlareSector* Sector, UStaticMeshComponent* Component, const FFlareAsteroidSave& Data)
{
	Component->SetStaticMesh(Game->GetAsteroidCatalog()->Asteroids[Data.AsteroidMeshID]);

	// Actor scale
	Component->SetWorldScale3D(FVector(1, 1, 1));
	float CollisionSize = Component->GetCollisionShape().GetExtent().Size();
	FVector ScaleFactor = Component->GetOwner()->GetActorScale3D() * Data.Scale * (20000.0f / CollisionSize);
	Component->SetWorldScale3D(ScaleFactor);

	FLOGV("AFlareAsteroid::SetupAsteroidMesh : ID=%d Scale=%f,%f,%f CollisionSize=%f",
		Data.AsteroidMeshID, Data.Scale.X, Data.Scale.Y, Data.Scale.Z, CollisionSize);

	// Mass scale
	FBodyInstance* BodyInst = Component->GetBodyInstance();
	BodyInst->MassScale = ScaleFactor.Size();
	BodyInst->UpdateMassProperties();

	// Material
	UMaterialInstanceDynamic* AsteroidMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), Component->GetWorld());
	for (int32 LodIndex = 0; LodIndex < Component->StaticMesh->RenderData->LODResources.Num(); LodIndex++)
	{
		Component->SetMaterial(LodIndex, AsteroidMaterial);
	}
	

	AsteroidMaterial->SetScalarParameterValue("IceMask", Sector->GetDescription()->IsIcy);
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
