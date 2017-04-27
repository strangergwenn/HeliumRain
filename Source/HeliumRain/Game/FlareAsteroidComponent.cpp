
#include "FlareAsteroidComponent.h"
#include "../Flare.h"

#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareAsteroidComponent::UFlareAsteroidComponent(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	bTraceComplexOnMove = true;
	SetSimulatePhysics(true);
	SetLinearDamping(0);
	SetAngularDamping(0);

	// FX
	static ConstructorHelpers::FObjectFinder<UParticleSystem> IceEffectTemplateObj(TEXT("/Game/Master/Particles/PS_IceEffect.PS_IceEffect"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_DustEffect.PS_DustEffect"));
	IceEffectTemplate = IceEffectTemplateObj.Object;
	DustEffectTemplate = DustEffectTemplateObj.Object;

	// Settings
	PrimaryComponentTick.bCanEverTick = true;
	IsIcyAsteroid = true;
	EffectsCount = FMath::RandRange(2, 5);
	EffectsScale = 0.05;
	EffectsUpdatePeriod = 0.5f;
	EffectsUpdateTimer = 0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareAsteroidComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFlareAsteroidComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	float CollisionSize = GetCollisionShape().GetExtent().Size();
	EffectsUpdateTimer += DeltaTime;

	// Get player ship
	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	FCHECK(Game);
	AFlarePlayerController* PC = Game->GetPC();
	FCHECK(PC);
	AFlareSpacecraft* ShipPawn = PC->GetShipPawn();

	// Update if close to player and visible
	if (ShipPawn
	 && (ShipPawn->GetActorLocation() - GetComponentLocation()).Size() < 500000
	 && (GetWorld()->TimeSeconds - LastRenderTime) < 0.5)
	{
		if (EffectsUpdateTimer > EffectsUpdatePeriod)
		{
			// World data
			FVector AsteroidLocation = GetComponentLocation();
			FVector SunDirection = Game->GetPlanetarium()->GetSunDirection();
			SunDirection.Normalize();
	
			// Compute new FX locations
			for (int32 Index = 0; Index < Effects.Num(); Index++)
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
				if (FoundHit && HitResult.Component == this && Effects[Index])
				{
					FVector EffectLocation = HitResult.Location;

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

	// Disable all
	else
	{
		for (int32 Index = 0; Index < Effects.Num(); Index++)
		{
			Effects[Index]->Deactivate();
		}
	}
}

void UFlareAsteroidComponent::SetupEffects(bool Icy)
{
	IsIcyAsteroid = Icy;
	Effects.Empty();
	EffectsKernels.Empty();

	AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(GetOwner());
	int32 Multiplier = Asteroid ? Asteroid->EffectsMultiplier : 1;

	// Create random effects
	for (int32 Index = 0; Index < Multiplier * EffectsCount; Index++)
	{
		EffectsKernels.Add(FMath::VRand());

		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			IceEffectTemplate,
			this,
			NAME_None,
			GetComponentLocation(),
			FRotator::ZeroRotator,
			EAttachLocation::KeepWorldPosition,
			false);

		PSC->SetWorldScale3D(EffectsScale * FVector(1, 1, 1));
		Effects.Add(PSC);

		if (IsIcyAsteroid)
		{
			PSC->SetTemplate(IceEffectTemplate);
		}
		else
		{
			PSC->SetTemplate(DustEffectTemplate);
		}
	}
}
