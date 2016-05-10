
#include "../Flare.h"
#include "FlareAsteroid.h"
#include "FlareGame.h"
#include "FlarePlanetarium.h"
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

	// FX
	static ConstructorHelpers::FObjectFinder<UParticleSystem> IceEffectTemplateObj(TEXT("/Game/Master/Particles/PS_IceEffect.PS_IceEffect"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_DustEffect.PS_DustEffect"));
	IceEffectTemplate = IceEffectTemplateObj.Object;
	DustEffectTemplate = DustEffectTemplateObj.Object;

	// Settings
	PrimaryActorTick.bCanEverTick = true;
	SetMobility(EComponentMobility::Movable);
	IsIcyAsteroid = false;
	Paused = false;
	EffectsCount = FMath::RandRange(1, 5);
	EffectsScale = 0.05;
	EffectsUpdatePeriod = 0.17f;
	EffectsUpdateTimer = 0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareAsteroid::BeginPlay()
{
	Super::BeginPlay();

	// Create random effects
	for (int32 Index = 0; Index < EffectsCount; Index++)
	{
		EffectsKernels.Add(FMath::VRand());

		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			DustEffectTemplate,
			GetRootComponent(),
			NAME_None,
			GetActorLocation(),
			FRotator(),
			EAttachLocation::KeepWorldPosition,
			false);

		PSC->SetWorldScale3D(EffectsScale * FVector(1, 1, 1));
		Effects.Add(PSC);
	}
}

void AFlareAsteroid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	float CollisionSize = GetStaticMeshComponent()->GetCollisionShape().GetExtent().Size();
	EffectsUpdateTimer += DeltaSeconds;

	/*if (SpawnLocation.Size() <= 0.1)
	{
		SpawnLocation = GetActorLocation();
		DrawDebugSphere(GetWorld(), SpawnLocation, CollisionSize / 2, 16, FColor::Red, true);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), CollisionSize / 2, 16, FColor::Blue, false);
		DrawDebugLine(GetWorld(), GetActorLocation(), SpawnLocation, FColor::Green, false);
	}*/

	if (EffectsUpdateTimer > EffectsUpdatePeriod)
	{
		// World data
		AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
		FVector AsteroidLocation = GetActorLocation();
		FVector SunDirection = Game->GetPlanetarium()->GetSunDirection();
		SunDirection.Normalize();
	
		// Compute new FX locations
		for (int32 Index = 0; Index < EffectsKernels.Num(); Index++)
		{
			FVector RandomDirection = FVector::CrossProduct(SunDirection, EffectsKernels[Index]);
			RandomDirection.Normalize();
			FVector StartPoint = AsteroidLocation + RandomDirection * CollisionSize;

			// Trace params
			FHitResult HitResult(ForceInit);
			FCollisionQueryParams TraceParams(FName(TEXT("Asteroid Trace")), false, NULL);
			TraceParams.bTraceComplex = true;
			TraceParams.bReturnPhysicalMaterial = false;
			ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldDynamic;

			// Trace
			bool FoundHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, AsteroidLocation, CollisionChannel, TraceParams);
			if (FoundHit && HitResult.Actor == this && Effects[Index])
			{
				FVector EffectLocation = HitResult.Location;
				//DrawDebugLine(GetWorld(), EffectLocation, EffectLocation + SunDirection * 10000, FColor::Blue, false);

				if (Game->GetActiveSector()->GetDescription()->IsIcy != IsIcyAsteroid)
				{
					IsIcyAsteroid = Game->GetActiveSector()->GetDescription()->IsIcy;
					Effects[Index]->SetTemplate(IsIcyAsteroid ? IceEffectTemplate : DustEffectTemplate);
				}

				if (!Effects[Index]->IsActive())
				{
					Effects[Index]->Activate();
				}
				Effects[Index]->SetWorldLocation(EffectLocation);
				Effects[Index]->SetWorldRotation(SunDirection.Rotation());
			}
			else
			{
				Effects[Index]->Deactivate();
			}
		}

		EffectsUpdateTimer = 0;
	}
}

void AFlareAsteroid::Load(const FFlareAsteroidSave& Data)
{
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	AsteroidData = Data;

	SetupAsteroidMesh(Game, GetStaticMeshComponent(), Data, Game->GetActiveSector()->GetDescription()->IsIcy);
	GetStaticMeshComponent()->SetPhysicsLinearVelocity(Data.LinearVelocity);
	GetStaticMeshComponent()->SetPhysicsAngularVelocity(Data.AngularVelocity);
}

void AFlareAsteroid::SetupAsteroidMesh(AFlareGame* Game, UStaticMeshComponent* Component, const FFlareAsteroidSave& Data, bool IsIcy)
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
