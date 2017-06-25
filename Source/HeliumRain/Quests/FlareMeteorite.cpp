
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
	/*Asteroid = PCIP.CreateDefaultSubobject<UFlareAsteroidComponent>(this, TEXT("Asteroid"));*/

	Meteorite = GetDestructibleComponent();

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

	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	MeteoriteData = Data;

	// TODO, icy in Meteorite Data

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

void AFlareMeteorite::SetPause(bool Pause)
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

	Meteorite->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		Meteorite->SetPhysicsLinearVelocity(MeteoriteData.LinearVelocity);
		Meteorite->SetPhysicsAngularVelocity(MeteoriteData.AngularVelocity);
	}
}

void AFlareMeteorite::SetupMeteoriteMesh(AFlareGame* Game, UDestructibleComponent* Component, const FFlareMeteoriteSave& Data, bool IsIcy)
{
	if (Game->GetMeteoriteCatalog())
	{
		// TODO Icy
		FCHECK(Data.MeteoriteMeshID >= 0 && Data.MeteoriteMeshID < Game->GetMeteoriteCatalog()->DustyMeteorites.Num());
		Component->SetDestructibleMesh(Game->GetMeteoriteCatalog()->DustyMeteorites[Data.MeteoriteMeshID]);
	}
	else
	{
		return;
	}



	// Mass scale
	/*FBodyInstance* BodyInst = Component->GetBodyInstance();
	BodyInst->MassScale = ScaleFactor.Size();
	BodyInst->UpdateMassProperties();*/

	// Material
	UMaterialInstanceDynamic* MeteoriteMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), Component->GetWorld());
	//for (int32 LodIndex = 0; LodIndex < Component->GetDestructibleMesh()->RenderData->LODResources.Num(); LodIndex++)
	//{
//		Component->SetMaterial(LodIndex, MeteoriteMaterial);
	//}

	// Sector style
	MeteoriteMaterial->SetScalarParameterValue("IceMask", false);
}
