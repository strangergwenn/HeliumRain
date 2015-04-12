
#include "../Flare.h"
#include "FlareProjectile.h"
#include "FlareShipBase.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareProjectile::AFlareProjectile(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// FX particles
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionEffectObject(TEXT("/Game/Master/Particles/PS_Explosion"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FlightEffectsObject(TEXT("/Game/Master/Particles/PS_FlightTrail"));
	ExplosionEffectTemplate = ExplosionEffectObject.Object;
	FlightEffectsTemplate = FlightEffectsObject.Object;
	
	// FX material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ExplosionMaterialObject(TEXT("/Game/Master/Materials/MT_Impact_Decal"));
	ExplosionEffectMaterial = ExplosionMaterialObject.Object;

	// Mesh data
	ShellComp = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	ShellComp->bTraceComplexOnMove = true;
	ShellComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ShellComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ShellComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ShellComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ShellComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = ShellComp;

	// Setup movement
	MovementComp = PCIP.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("Movement"));
	MovementComp->UpdatedComponent = ShellComp;
	MovementComp->InitialSpeed = 50000;
	MovementComp->MaxSpeed = 50000;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0;
	MovementComp->Bounciness = 1;
	MovementComp->bShouldBounce = true;
	
	// Settings
	FlightEffects = NULL;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MovementComp->OnProjectileBounce.AddDynamic(this, &AFlareProjectile::OnImpact);
	ShellComp->MoveIgnoreActors.Add(Instigator);
	SetLifeSpan(1000000 / MovementComp->InitialSpeed);
}

void AFlareProjectile::Initialize(UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity)
{
	ShellDirection = ShootDirection;
	ShellDescription = Description;
	FVector FinalDirection = ParentVelocity + ShootDirection * MovementComp->InitialSpeed;

	// Get the power from description
	if (Description)
	{
		ShellComp->SetStaticMesh(Description->EffectMesh);
		
		for (int32 i = 0; i < ShellDescription->Characteristics.Num(); i++)
		{
			const FFlareShipComponentCharacteristic& Characteristic = ShellDescription->Characteristics[i];
			switch (Characteristic.CharacteristicType)
			{
				case EFlarePartCharacteristicType::AmmoPower:
					ShellPower = Characteristic.CharacteristicValue;
					break;
			}
		}
	}

	// Set the speed
	if (MovementComp)
	{
		MovementComp->Velocity = FinalDirection;
		ShellDirection = MovementComp->Velocity;
		ShellDirection.Normalize();
		ShellMass = 2 * ShellPower * 1000 / FMath::Square(MovementComp->InitialSpeed/100); // ShellPower is in Kilo-Joule, reverse kinetic energy equation
	}

	// Spawn the flight effects
	FlightEffects = UGameplayStatics::SpawnEmitterAttached(
		FlightEffectsTemplate,
		RootComponent,
		NAME_None,
		RootComponent->GetComponentLocation(),
		FinalDirection.Rotation(),
		EAttachLocation::KeepWorldPosition, 
		true);
}

void AFlareProjectile::OnImpact(const FHitResult& HitResult, const FVector& HitVelocity)
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
		float PenerationIncidenceLimit = 0.5f;
		float Incidence = FVector::DotProduct(HitResult.ImpactNormal, -ImpactVelocityAxis);
		float RemainingArmor = -1; // Negative value means undestructible

		// Hit a component
		UFlareShipComponent* ShipComponent = Cast<UFlareShipComponent>(HitResult.Component.Get());
		if (ShipComponent)
		{
			 RemainingArmor = ShipComponent->GetRemainingArmorAtLocation(HitResult.Location);
		}

		// Debug
		FLOGV("Incidence %f", Incidence);
		FLOGV("ImpactVelocityAxis %s", *ImpactVelocityAxis.ToString());
		FLOGV("ImpactNormal %s", *HitResult.ImpactNormal.ToString());
		FLOGV("OnImpact on %s", *(HitResult.Component.Get()->GetReadableName()));
		FLOGV("OnImpact ProjectileVelocity=%s TargetVelocity=%s ImpactVelocity=%s ImpactVelocityAxis=%s",
			*ProjectileVelocity.ToString(), *TargetVelocity.ToString(), *ImpactVelocity.ToString(), *ImpactVelocityAxis.ToString());
		FLOGV("OnImpact ShellMass=%f PenerationIncidenceLimit=%f RemainingArmor=%f HitResult.ImpactNormal=%s",
			ShellMass, PenerationIncidenceLimit, RemainingArmor, *(HitResult.ImpactNormal.ToString()));

		// Check armor peneration
		int32 PenetrateArmor = false;
		if (Incidence > PenerationIncidenceLimit)
		{
			PenetrateArmor = true; // No ricochet
		}
		else if(RemainingArmor >= 0 && Incidence * ShellEnergy > RemainingArmor)
		{
			PenetrateArmor = true; // Armor destruction
		}	
		
		// Calculate useful energy
		float AbsorbedEnergy = (PenetrateArmor ? ShellEnergy : Incidence * ShellEnergy);		
		FLOGV("OnImpact AbsorbedEnergy=%f PenetrateArmor=%d ShellEnergy=%f Incidence=%f", AbsorbedEnergy, PenetrateArmor, ShellEnergy, Incidence);

		// Hit a component : damage in KJ
		IFlareShipInterface* Ship = Cast<IFlareShipInterface>(HitResult.Actor.Get());
		if (Ship)
		{
			 Ship->ApplyDamage(AbsorbedEnergy, 0.75f, HitResult.Location);
		}
		
		// FX data
		USceneComponent* Target = HitResult.GetComponent();
		float DecalSize = FMath::FRandRange(60, 90);
		
		// Spawn decal
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

		// Spawn penetration effect
		if (PenetrateArmor)
		{
			UGameplayStatics::SpawnEmitterAttached(
				ExplosionEffectTemplate,
				Target,
				NAME_None,
				HitResult.Location,
				HitResult.ImpactNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				true);

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
			float RemainingVelocity = FMath::Sqrt(2 * RemainingEnergy / ShellMass);
			MovementComp->Bounciness = RemainingEnergy / ShellEnergy;
			FLOGV("OnImpact projectile velocity to %f ", MovementComp->Bounciness);
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
