
#include "FlareAsteroid.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "FlarePlanetarium.h"
#include "FlareSimulatedSector.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareAsteroid::AFlareAsteroid(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh
	Asteroid = PCIP.CreateDefaultSubobject<UFlareAsteroidComponent>(this, TEXT("Asteroid"));
	Asteroid->bTraceComplexOnMove = true;
	Asteroid->SetSimulatePhysics(true);
	Asteroid->SetLinearDamping(0);
	Asteroid->SetAngularDamping(0);
	RootComponent = Asteroid;
	SetActorEnableCollision(true);

	// Settings
	PrimaryActorTick.bCanEverTick = true;
	RootComponent->SetMobility(EComponentMobility::Movable);
	Paused = false;
	EffectsMultiplier = 1;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareAsteroid::BeginPlay()
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	if (Game && Game->GetActiveSector())
	{
		Asteroid->SetupEffects(Game->GetActiveSector()->GetSimulatedSector()->GetDescription()->IsIcy);
	}

	Super::BeginPlay();
}

void AFlareAsteroid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*float CollisionSize = Asteroid->GetCollisionShape().GetExtent().Size();
	if (SpawnLocation.Size() <= 0.1)
	{
		SpawnLocation = GetActorLocation();
		DrawDebugSphere(GetWorld(), SpawnLocation, CollisionSize / 2, 16, FColor::Red, true);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), CollisionSize / 2, 16, FColor::Blue, false);
		DrawDebugLine(GetWorld(), GetActorLocation(), SpawnLocation, FColor::Green, false);
	}*/
}

void AFlareAsteroid::Load(const FFlareAsteroidSave& Data)
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	AsteroidData = Data;

	SetupAsteroidMesh(Game, Asteroid, Data, Game->GetActiveSector()->GetSimulatedSector()->GetDescription()->IsIcy);
	Asteroid->SetPhysicsLinearVelocity(Data.LinearVelocity);
	Asteroid->SetPhysicsAngularVelocity(Data.AngularVelocity);
}

void AFlareAsteroid::SetupAsteroidMesh(AFlareGame* Game, UStaticMeshComponent* Component, const FFlareAsteroidSave& Data, bool IsIcy)
{
	if (Game->GetAsteroidCatalog())
	{
		FCHECK(Data.AsteroidMeshID >= 0 && Data.AsteroidMeshID < Game->GetAsteroidCatalog()->Asteroids.Num());
		Component->SetStaticMesh(Game->GetAsteroidCatalog()->Asteroids[Data.AsteroidMeshID]);
	}
	else
	{
		Component->SetStaticMesh(Game->GetDefaultAsteroid());
	}

	// Actor scale
	Component->SetWorldScale3D(FVector(1, 1, 1));
	float CollisionSize = Component->GetCollisionShape().GetExtent().Size();
	FVector ScaleFactor = Component->GetOwner()->GetActorScale3D() * Data.Scale * (20000.0f / CollisionSize);
	Component->SetWorldScale3D(ScaleFactor);
	
	// Mass scale
	FBodyInstance* BodyInst = Component->GetBodyInstance();
	BodyInst->MassScale = ScaleFactor.Size();
	BodyInst->UpdateMassProperties();

	// Material
	UMaterialInstanceDynamic* AsteroidMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), Component->GetWorld());
	for (int32 LodIndex = 0; LodIndex < Component->GetStaticMesh()->RenderData->LODResources.Num(); LodIndex++)
	{
		Component->SetMaterial(LodIndex, AsteroidMaterial);
	}
	
	// Sector style
	AsteroidMaterial->SetScalarParameterValue("IceMask", IsIcy);
}

FFlareAsteroidSave* AFlareAsteroid::Save()
{
	// Physical data
	AsteroidData.Location = GetActorLocation();
	AsteroidData.Rotation = GetActorRotation();
	if (!Paused)
	{
		AsteroidData.LinearVelocity = Asteroid->GetPhysicsLinearVelocity();
		AsteroidData.AngularVelocity = Asteroid->GetPhysicsAngularVelocity();
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

	Asteroid->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		Asteroid->SetPhysicsLinearVelocity(AsteroidData.LinearVelocity);
		Asteroid->SetPhysicsAngularVelocity(AsteroidData.AngularVelocity);
	}
}
