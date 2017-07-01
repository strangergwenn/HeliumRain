
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
	Meteorite = PCIP.CreateDefaultSubobject<UDestructibleComponent>(this, TEXT("Meteorite"));
	Meteorite->bTraceComplexOnMove = true;
	Meteorite->SetLinearDamping(0);
	Meteorite->SetAngularDamping(0);
	RootComponent = Meteorite;
	SetActorEnableCollision(true);

	// Physics
	Meteorite->SetMobility(EComponentMobility::Movable);
	Meteorite->SetCollisionProfileName("Destructible");
	Meteorite->GetBodyInstance()->SetUseAsyncScene(false);
	Meteorite->SetNotifyRigidBodyCollision(true);

	// Settings
	Meteorite->PrimaryComponentTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = true;
	Paused = false;
}

void AFlareMeteorite::Load(const FFlareMeteoriteSave& Data)
{
	FLOGV("AFlareMeteorite::Load vel=%s", *Data.LinearVelocity.ToString());

	MeteoriteData = Data;

	SetupMeteoriteMesh(Data);
	Meteorite->SetPhysicsLinearVelocity(Data.LinearVelocity);
	Meteorite->SetPhysicsAngularVelocity(Data.AngularVelocity);

	FLOGV("- Meteorite.BodyInstance.bSimulatePhysics=%d", Meteorite->BodyInstance.bSimulatePhysics);
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
	FLOGV(" - IsSimulatingPhysics %d", Meteorite->IsSimulatingPhysics());*/



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

void AFlareMeteorite::SetupMeteoriteMesh(const FFlareMeteoriteSave& Data)
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	
	if (Game->GetMeteoriteCatalog())
	{
		const TArray<UDestructibleMesh*>& MeteoriteList = MeteoriteData.IsIcy ? Game->GetMeteoriteCatalog()->IcyMeteorites : Game->GetMeteoriteCatalog()->DustyMeteorites;
		FCHECK(Data.MeteoriteMeshID >= 0 && Data.MeteoriteMeshID < MeteoriteList.Num());

		Meteorite->SetDestructibleMesh(MeteoriteList[Data.MeteoriteMeshID]);
		Meteorite->SetSimulatePhysics(true);
	}
	else
	{
		return;
	}
}
