
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
	MovementComp->MaxSpeed = 20000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;

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
	MovementComp->OnProjectileStop.AddDynamic(this, &AFlareProjectile::OnImpact);
	ShellComp->MoveIgnoreActors.Add(Instigator);
	SetLifeSpan(1000000 / MovementComp->InitialSpeed);
}

void AFlareProjectile::Initialize(UFlareWeapon* Weapon, const FFlareShipModuleDescription* Description, FVector ShootDirection, FVector ParentVelocity)
{
	ShellDirection = ShootDirection;
	ShellDescription = Description;

	if (Description)
	{
		ShellComp->SetStaticMesh(Description->EffectMesh);
	}
	if (MovementComp)
	{
		MovementComp->Velocity = ParentVelocity + ShootDirection * MovementComp->InitialSpeed;
	}
}

void AFlareProjectile::OnImpact(const FHitResult& HitResult)
{
	if (HitResult.Actor.IsValid() && HitResult.Component.IsValid())
	{
		// Data
		USceneComponent* Target = HitResult.GetComponent();
		float DecalSize = FMath::FRandRange(250, 500);

		// Spawn effect
		UGameplayStatics::SpawnEmitterAttached(
			ExplosionEffectTemplate,
			Target,
			NAME_None,
			HitResult.Location,
			HitResult.ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true);

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

		// Physics impulse
		UMeshComponent* PhysMesh = Cast<UMeshComponent>(Target);
		if (PhysMesh)
		{
			int32 ShellPower = 0;
			for (int32 i = 0; i < ShellDescription->Characteristics.Num(); i++)
			{
				const FFlarePartCharacteristic& Characteristic = ShellDescription->Characteristics[i];
				switch (Characteristic.CharacteristicType)
				{
					case EFlarePartAttributeType::AmmoPower:
						ShellPower = Characteristic.CharacteristicValue;
						break;
				}
			}
			PhysMesh->AddImpulseAtLocation(1000 * ShellPower * ShellDirection, HitResult.Location);
		}
	}

	Destroy();
}
