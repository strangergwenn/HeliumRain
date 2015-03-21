
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
	ExplosionEffectTemplate = ExplosionEffectObject.Object;

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
	MovementComp->InitialSpeed = 20000.0f;
	MovementComp->MaxSpeed = 200000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;
	MovementComp->Bounciness = 1.f;
	MovementComp->bShouldBounce = true;
	
	// Settings
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
	if (MovementComp)
	{
		MovementComp->Velocity = ParentVelocity + ShootDirection * MovementComp->InitialSpeed;
		ShellDirection = MovementComp->Velocity;
		ShellDirection.Normalize();
		// ShellPower is in Kilo-Joule
		ShellMass = 2 * ShellPower * 1000 / FMath::Square(MovementComp->InitialSpeed/100); // Reverse kinetic energy equation
	}
}

void AFlareProjectile::OnImpact(const FHitResult& HitResult, const FVector& HitVelocity)
{
	bool DestroyProjectile = true;
	
	if (HitResult.Actor.IsValid() && HitResult.Component.IsValid())
	{
		// Compute projectile energy.
		//MovementComp->UpdateComponentVelocity();
		FVector ProjectileVelocity = HitVelocity / 100;
		FVector TargetVelocity = HitResult.Component->GetPhysicsLinearVelocity() / 100;
		FVector ImpactVelocity = ProjectileVelocity - TargetVelocity;
		FVector ImpactVelocityAxis = ImpactVelocity.GetUnsafeNormal();
		
		float ShellEnergy = 0.5f * ShellMass * ImpactVelocity.SizeSquared();
		
		float PenerationIncidenceLimit = 0.3f;
		
		float Incidence = FVector::DotProduct(HitResult.ImpactNormal, -ImpactVelocityAxis);
		
		FLOGV("Incidence %f", Incidence);
		FLOGV("ImpactVelocityAxis %s", *ImpactVelocityAxis.ToString());
		FLOGV("ImpactNormal %s", *HitResult.ImpactNormal.ToString());
		
		
		float RemainingArmor = -1; // Negative value mean no destructable
		
		FLOGV("OnImpact ProjectileVelocity=%s TargetVelocity=%s ImpactVelocity=%s ImpactVelocityAxis=%s", *ProjectileVelocity.ToString(), *TargetVelocity.ToString(), *ImpactVelocity.ToString(), *ImpactVelocityAxis.ToString());
		
		FLOGV("OnImpact ShellMass=%f PenerationIncidenceLimit=%f RemainingArmor=%f HitResult.ImpactNormal=%s", ShellMass, PenerationIncidenceLimit, RemainingArmor, *(HitResult.ImpactNormal.ToString()));
		
		
		UFlareShipComponent* ShipComponent = Cast<UFlareShipComponent>(HitResult.Component.Get());
		if(ShipComponent) {
			// Hit a component
			 RemainingArmor = ShipComponent->GetRemainingArmorAtLocation(HitResult.Location);
		}
		
		
		
		
		// Check armor peneration
		int32 PenetrateArmor = false;
		if(Incidence > PenerationIncidenceLimit)
		{
			// No ricochet
			PenetrateArmor = true;
		}
		else if(RemainingArmor >= 0 && Incidence * ShellEnergy > RemainingArmor)
		{
			// Armor destruction
			PenetrateArmor = true;
		}
				
		
		// Calculate usefull energy
		float AbsorbedEnergy = (PenetrateArmor ? ShellEnergy : Incidence * ShellEnergy);
		
		FLOGV("OnImpact AbsorbedEnergy=%f PenetrateArmor=%d ShellEnergy=%f Incidence=%f", AbsorbedEnergy, PenetrateArmor, ShellEnergy, Incidence);
		
		// Apply damages
		IFlareShipInterface* Ship = Cast<IFlareShipInterface>(HitResult.Actor.Get());
		if(Ship) {
			// Hit a component
			 Ship->ApplyDamage(AbsorbedEnergy, 0.5f, HitResult.Location);
		}
		
		// Data
		USceneComponent* Target = HitResult.GetComponent();
		float DecalSize = FMath::FRandRange(250, 500);

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
		
		if(PenetrateArmor) {
			// Spawn penetration effect
			UGameplayStatics::SpawnEmitterAttached(
			ExplosionEffectTemplate,
			Target,
			NAME_None,
			HitResult.Location,
			HitResult.ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true);
		} else {
			DestroyProjectile = false;
			float RemainingEnergy = ShellEnergy - AbsorbedEnergy;
			float RemainingVelocity = FMath::Sqrt(2 * RemainingEnergy / ShellMass);
			MovementComp->Bounciness = RemainingEnergy / ShellEnergy;
			//MovementComp->Velocity = MovementComp->Velocity * (1.0f - Incidence);
			//MovementComp->UpdateComponentVelocity();
			FLOGV("OnImpact projectile velocity to %f ", MovementComp->Bounciness);
		}
	
		// Physics impulse
		UMeshComponent* PhysMesh = Cast<UMeshComponent>(Target);
		if (PhysMesh)
		{
			PhysMesh->AddImpulseAtLocation( AbsorbedEnergy * (PenetrateArmor ? ImpactVelocityAxis : -HitResult.ImpactNormal), HitResult.Location);
		}
	}

	if(DestroyProjectile) {
		Destroy();
	}
}
