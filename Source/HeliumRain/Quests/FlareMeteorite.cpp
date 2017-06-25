
#include "FlareMeteorite.h"
#include "../Flare.h"
#include "../Game/FlareGame.h"
#include "../Data/FlareMeteoriteCatalog.h"
#include "Components/DestructibleComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareMeteorite::AFlareMeteorite(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh
	Meteorite = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Meteorite"));
	RootComponent = Meteorite;
	//Meteorite = GetDestructibleComponent();

	Meteorite->bTraceComplexOnMove = true;
	Meteorite->SetSimulatePhysics(true);
	Meteorite->SetLinearDamping(0);
	Meteorite->SetAngularDamping(0);
	Meteorite->PrimaryComponentTick.bCanEverTick = true;

	SetActorEnableCollision(true);

	// Settings
	PrimaryActorTick.bCanEverTick = true;
	RootComponent->SetMobility(EComponentMobility::Movable);
	Paused = false;
	//EffectsMultiplier = 1;
}

void AFlareMeteorite::Load(const FFlareMeteoriteSave& Data)
{
	FLOGV("AFlareMeteorite::Load vel=%s", *Data.LinearVelocity.ToString());
	FLOGV("- Meteorite.BodyInstance.bSimulatePhysics=%d", Meteorite->BodyInstance.bSimulatePhysics);

	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	MeteoriteData = Data;

	// TODO, icy in Meteorite Data


	//Meteorite->BodyInstance.bSimulatePhysics = true;


	SetupMeteoriteMesh(Game, Meteorite, Data, false);
	Meteorite->SetPhysicsLinearVelocity(Data.LinearVelocity);
	Meteorite->SetPhysicsAngularVelocity(Data.AngularVelocity);
}

FFlareMeteoriteSave* AFlareMeteorite::Save()
{
	// Physical data
	MeteoriteData.Location = GetActorLocation();
	MeteoriteData.Rotation = GetActorRotation();
	if (!Paused)
	{
		MeteoriteData.LinearVelocity = Meteorite->GetPhysicsLinearVelocity();
		MeteoriteData.AngularVelocity = Meteorite->GetPhysicsAngularVelocity();
	}

	return &MeteoriteData;
}

void AFlareMeteorite::BeginPlay()
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	/*if (Game && Game->GetActiveSector())
	{
		Asteroid->SetupEffects(Game->GetActiveSector()->GetSimulatedSector()->GetDescription()->IsIcy);
	}*/

	Super::BeginPlay();
}


void AFlareMeteorite::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*FLOGV("Meteorite %s vel=%s", *GetName(), *Meteorite->GetPhysicsLinearVelocity().ToString());
	FLOGV(" - IsPhysicsCollisionEnabled %d", Meteorite->IsPhysicsCollisionEnabled());
	FLOGV(" - IsPhysicsStateCreated %d", Meteorite->IsPhysicsStateCreated());
	FLOGV(" - IsAnySimulatingPhysics %d", Meteorite->IsAnySimulatingPhysics());
	FLOGV(" - IsAnyRigidBodyAwake %d", Meteorite->IsAnyRigidBodyAwake());
	FLOGV(" - IsCollisionEnabled %d", Meteorite->IsCollisionEnabled());
	FLOGV(" - IsSimulatingPhysics %d", Meteorite->IsSimulatingPhysics());
*/


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


void AFlareMeteorite::SetPause(bool Pause)
{
	FLOGV("AFlareMeteorite::SetPause Pause=%d", Pause);


	if (Paused == Pause)
	{
		return;
	}

	CustomTimeDilation = (Pause ? 0.f : 1.0);
	if (Pause)
	{
		Save(); // Save must be performed with the old pause state
	}

	Meteorite->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		Meteorite->SetPhysicsLinearVelocity(MeteoriteData.LinearVelocity);
		Meteorite->SetPhysicsAngularVelocity(MeteoriteData.AngularVelocity);
	}
}

void AFlareMeteorite::SetupMeteoriteMesh(AFlareGame* Game, UStaticMeshComponent* Component, const FFlareMeteoriteSave& Data, bool IsIcy)
{
	/*if (Game->GetMeteoriteCatalog())
	{
		// TODO Icy
		FCHECK(Data.MeteoriteMeshID >= 0 && Data.MeteoriteMeshID < Game->GetMeteoriteCatalog()->DustyMeteorites.Num());
		Component->SetDestructibleMesh(Game->GetMeteoriteCatalog()->DustyMeteorites[Data.MeteoriteMeshID]);
	}
	else
	{
		return;
	}*/

	if (Game->GetAsteroidCatalog())
	{
		FCHECK(Data.MeteoriteMeshID >= 0 && Data.MeteoriteMeshID < Game->GetAsteroidCatalog()->Asteroids.Num());
		Component->SetStaticMesh(Game->GetAsteroidCatalog()->Asteroids[Data.MeteoriteMeshID]);
	}
	else
	{
		Component->SetStaticMesh(Game->GetDefaultAsteroid());
	}


	// Actor scale
	Component->SetWorldScale3D(FVector(1, 1, 1));
	float CollisionSize = Component->GetCollisionShape().GetExtent().Size();
	FVector ScaleFactor = Component->GetOwner()->GetActorScale3D() * 0.1f * (20000.0f / CollisionSize);
	Component->SetWorldScale3D(ScaleFactor);

	// Mass scale
	FBodyInstance* BodyInst = Component->GetBodyInstance();
	BodyInst->MassScale = ScaleFactor.Size();
	BodyInst->UpdateMassProperties();


	// Material
	UMaterialInstanceDynamic* MeteoriteMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), Component->GetWorld());
	//for (int32 LodIndex = 0; LodIndex < Component->GetDestructibleMesh()->RenderData->LODResources.Num(); LodIndex++)
	//{
//		Component->SetMaterial(LodIndex, MeteoriteMaterial);
	//}

	// Sector style
	MeteoriteMaterial->SetScalarParameterValue("IceMask", false);
}
