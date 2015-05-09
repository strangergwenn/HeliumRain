
#include "../Flare.h"
#include "FlareShip.h"
#include "FlareShell.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareShell::AFlareShell(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	ShellComp = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = ShellComp;

	// Settings
	FlightEffects = NULL;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareShell::Initialize(UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer)
{
	ShellDescription = Description;
	TracerShell = Tracer;
	float AmmoVelocity = 0.f;
	// Get the power from description
	if (Description)
	{
		ShellPower = Description->GunCharacteristics.AmmoPower;
		AmmoVelocity = Description->GunCharacteristics.AmmoVelocity;

		ImpactSound = Description->GunCharacteristics.ImpactSound;
		DamageSound = Description->GunCharacteristics.DamageSound;

		ExplosionEffectTemplate = Description->GunCharacteristics.ExplosionEffect;
		FlightEffectsTemplate = Description->GunCharacteristics.TracerEffect;

		ExplosionEffectMaterial = Description->GunCharacteristics.ExplosionMaterial;
	}

	ShellVelocity = ParentVelocity + ShootDirection * AmmoVelocity * 100;
	ShellMass = 2 * ShellPower * 1000 / FMath::Square(AmmoVelocity); // ShellPower is in Kilo-Joule, reverse kinetic energy equation

	LastLocation = GetActorLocation();

	// Spawn the flight effects
	if(TracerShell)
	{
		FlightEffects = UGameplayStatics::SpawnEmitterAttached(
			FlightEffectsTemplate,
			RootComponent,
			NAME_None,
			FVector(0,0,0),
			FRotator(0,0,0),
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	SetLifeSpan(200000 / ShellVelocity.Size()); // 2km
}

void AFlareShell::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FVector ActorLocation = GetActorLocation();
	ActorLocation += ShellVelocity * DeltaSeconds;
	FVector NextActorLocation = ActorLocation + ShellVelocity * DeltaSeconds;
	SetActorLocation(NextActorLocation, false);
	SetActorRotation(ShellVelocity.Rotation());

	FHitResult HitResult(ForceInit);
	if (Trace(ActorLocation, NextActorLocation, HitResult))
	{
		OnImpact(HitResult, ShellVelocity);
	}
}

void AFlareShell::OnImpact(const FHitResult& HitResult, const FVector& HitVelocity)
{
	bool DestroyProjectile = true;
	
	if (HitResult.Actor.IsValid() && HitResult.Component.IsValid())
	{
		// Compute projectile energy.
		FVector ProjectileVelocity = HitVelocity / 100;
		FVector TargetVelocity = HitResult.Component->GetPhysicsLinearVelocity() / 100;
		FVector ImpactVelocity = ProjectileVelocity - TargetVelocity;
		FVector ImpactVelocityAxis = ImpactVelocity.GetUnsafeNormal();
	
		// Compute parameters
		float ShellEnergy = 0.5f * ShellMass * ImpactVelocity.SizeSquared() / 1000; // Damage in KJ
		float PenerationIncidenceLimit = 0.6f;
		float Incidence = FVector::DotProduct(HitResult.ImpactNormal, -ImpactVelocityAxis);
		float RemainingArmor = -1; // Negative value means undestructible

		if(Incidence < 0)
		{
			// Parasite hit after rebound, ignore
			return;
		}

		// Hit a component
		UFlareShipComponent* ShipComponent = Cast<UFlareShipComponent>(HitResult.Component.Get());
		if (ShipComponent)
		{
			 RemainingArmor = ShipComponent->GetRemainingArmorAtLocation(HitResult.Location);
		}

		// Check armor peneration
		int32 PenetrateArmor = false;
		if (Incidence > PenerationIncidenceLimit)
		{
			PenetrateArmor = true; // No ricochet
		}
		else if (RemainingArmor >= 0 && Incidence * ShellEnergy > RemainingArmor)
		{
			PenetrateArmor = true; // Armor destruction
		}	
		
		// Hit a component : damage in KJ
		float AbsorbedEnergy = (PenetrateArmor ? ShellEnergy : FMath::Square(Incidence) * ShellEnergy);
		IFlareShipInterface* Ship = Cast<IFlareShipInterface>(HitResult.Actor.Get());
		if (Ship)
		{
			Ship->ApplyDamage(AbsorbedEnergy, 0.75f, HitResult.Location);
			
			// Play sound
			AFlareShipBase* ShipBase = Cast<AFlareShipBase>(Ship);
			if (ShipBase && ShipBase->IsLocallyControlled())
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), PenetrateArmor ? DamageSound : ImpactSound, HitResult.Location, 1, 1);
			}
		}

		// FX data
		USceneComponent* Target = HitResult.GetComponent();
		float DecalSize = FMath::FRandRange(60, 90);
		
		// Spawn impact decal
		if (ShipComponent && ShipComponent->IsVisibleByPlayer())
		{
			UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
				ExplosionEffectMaterial,
				DecalSize * FVector(1, 1, 1),
				Target,
				NAME_None,
				HitResult.Location,
				HitResult.ImpactNormal.Rotation(),
				EAttachLocation::KeepWorldPosition);

			// Instanciate and configure the decal material
			UMaterialInterface* DecalMaterial = Decal->GetMaterial(0);
			UMaterialInstanceDynamic* DecalMaterialInst = UMaterialInstanceDynamic::Create(DecalMaterial, GetWorld());
			if (DecalMaterialInst)
			{
				DecalMaterialInst->SetScalarParameterValue("RandomParameter", FMath::FRandRange(1, 0));
				Decal->SetMaterial(0, DecalMaterialInst);
			}
		}

		// Spawn penetration effect
		if (PenetrateArmor)
		{
			if (ShipComponent && ShipComponent->IsVisibleByPlayer())
			{
				UGameplayStatics::SpawnEmitterAttached(
					ExplosionEffectTemplate,
					Target,
					NAME_None,
					HitResult.Location,
					HitResult.ImpactNormal.Rotation(),
					EAttachLocation::KeepWorldPosition,
					true);
			}

			// Remove flight effects
			if (FlightEffects)
			{
				FlightEffects->Deactivate();
			}
		}

		// Keep bouncing
		else
		{
			DestroyProjectile = false;
			float RemainingEnergy = ShellEnergy - AbsorbedEnergy;
			float RemainingVelocity = FMath::Sqrt(2 * RemainingEnergy * 1000 / ShellMass);
			FVector BounceDirection = ShellVelocity.GetUnsafeNormal().MirrorByVector(HitResult.ImpactNormal);
			ShellVelocity = BounceDirection * RemainingVelocity * 100;
			SetActorLocation(HitResult.Location);
		}
	
		// Physics impulse
		UMeshComponent* PhysMesh = Cast<UMeshComponent>(Target);
		if (PhysMesh)
		{
			PhysMesh->AddImpulseAtLocation( AbsorbedEnergy * (PenetrateArmor ? ImpactVelocityAxis : -HitResult.ImpactNormal), HitResult.Location);
		}
	}

	if (DestroyProjectile)
	{
		Destroy();
	}
}

bool AFlareShell::Trace(const FVector& Start, const FVector& End, FHitResult& HitOut)
{
	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, this);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	//Ignore Actors
	TraceParams.AddIgnoredActor(this);

	//Re-initialize hit info
	HitOut = FHitResult(ForceInit);

	ECollisionChannel CollisionChannel = (ECollisionChannel) (ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

	//Trace!
	GetWorld()->LineTraceSingleByChannel(
		HitOut,		//result
		Start,	//start
		End , //end
		CollisionChannel, //collision channel
		TraceParams
	);

	//Hit any Actor?
	return (HitOut.GetActor() != NULL) ;
}
