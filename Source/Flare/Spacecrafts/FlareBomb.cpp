
#include "../Flare.h"
#include "FlareBombComponent.h"
#include "FlareBomb.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareBomb::AFlareBomb(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	FLOG("AFlareBomb");
	// Mesh data
	BombComp = PCIP.CreateDefaultSubobject<UFlareBombComponent>(this, TEXT("Root"));
	BombComp->bTraceComplexOnMove = true;
	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/*BombComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	BombComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);*/
	BombComp->LDMaxDrawDistance = 100000; // 1km*/
	BombComp->SetSimulatePhysics(true);
	BombComp->SetLinearDamping(0);
	BombComp->SetAngularDamping(0);
	RootComponent = BombComp;

	SetActorEnableCollision(false);
	// Settings
	PrimaryActorTick.bCanEverTick = true;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFlareBomb::Initialize(UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description)
{
	ParentWeapon = Weapon;
	WeaponDescription = Description;

	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Get the power from description
	if (Description)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Description->Identifier;
		BombComp->Initialize(&ComponentData, Weapon->GetSpacecraft()->GetCompany(), Weapon->GetSpacecraft(), false);

		DamageSound = Description->WeaponCharacteristics.DamageSound;

		ExplosionEffectTemplate = Description->WeaponCharacteristics.ExplosionEffect;
		ExplosionEffectMaterial = Description->WeaponCharacteristics.GunCharacteristics.ExplosionMaterial;

	}

	SetActorScale3D(ParentWeapon->GetSpacecraft()->GetActorScale3D());

	Activated = false;
	Dropped = false;
}

void AFlareBomb::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	FLOG("AFlareBomb Hit");

	if (Other && OtherComp)
	{
		if(Other == ParentWeapon->GetSpacecraft())
		{
			// Avoid auto hit
			return;
		}


		AFlareBomb* BombCandidate = Cast<AFlareBomb>(Other);
		if(BombCandidate)
		{
			// Avoid bomb hit
			return;
		}

		// Spawn penetration effect
		if(ExplosionEffectTemplate)
		{
		UGameplayStatics::SpawnEmitterAttached(
			ExplosionEffectTemplate,
			OtherComp,
			NAME_None,
			HitLocation,
			HitNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true);
		}

		//TODO Explosion radius

		AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(Other);
		if (Spacecraft)
		{
			Spacecraft->GetDamageSystem()->ApplyDamage(WeaponDescription->WeaponCharacteristics.ExplosionPower , WeaponDescription->WeaponCharacteristics.AmmoDamageRadius, HitLocation);

			// Physics impulse
			Spacecraft->Airframe->AddImpulseAtLocation( 5000	 * WeaponDescription->WeaponCharacteristics.ExplosionPower * WeaponDescription->WeaponCharacteristics.AmmoDamageRadius * -HitNormal, HitLocation);


			// Play sound
			AFlareSpacecraftPawn* ShipBase = Cast<AFlareSpacecraftPawn>(Spacecraft);
			if (ShipBase && ShipBase->IsLocallyControlled())
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamageSound, HitLocation, 1, 1);
			}
		}

		UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(OtherComp);
		// Spawn impact decal
		if (ShipComponent && ShipComponent->IsVisibleByPlayer() && ExplosionEffectMaterial)
		{
			// FX data
			float DecalSize = FMath::FRandRange(60, 90);

			UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
				ExplosionEffectMaterial,
				DecalSize * FVector(1, 1, 1),
				ShipComponent,
				NAME_None,
				HitLocation,
				HitNormal.Rotation(),
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

		Destroy();
	}


}

void AFlareBomb::Drop()
{
	FLOG("AFlareBomb Drop");
	DetachRootComponentFromParent(true);

	FVector FrontVector = BombComp->ComponentToWorld.TransformVector(FVector(1,0,0));




	// Spin to stabilize
	BombComp->SetPhysicsAngularVelocity(FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropAngularVelocity);
	BombComp->SetPhysicsLinearVelocity(ParentWeapon->GetSpacecraft()->Airframe->GetPhysicsLinearVelocity() + FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity * 100);
	//BombComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FLOGV("AFlareBomb FrontVector=%s", *FrontVector.ToString());

	/*BombComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BombComp->SetCollisionResponseToAllChannels(ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	BombComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);*/
	//BombComp->SetSimulatePhysics(true);


	DropParentDistance = GetParentDistance();
	Dropped = true;

}


void AFlareBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	if(Dropped && !Activated)
	{
		FLOG("AFlareBomb Tick Dropped && !Activated");
		FLOGV("AFlareBomb DropParentDistance=%f", DropParentDistance);
		FLOGV("AFlareBomb GetParentDistance()=%f", GetParentDistance());
		if(GetParentDistance() > DropParentDistance + WeaponDescription->WeaponCharacteristics.BombCharacteristics.ActivationDistance/100)
		{
			// Activate after 30 cm

			SetActorEnableCollision(true);
			Activated = true;
		}
	}



}


float AFlareBomb::GetParentDistance() const
{
	FLOGV("AFlareBomb GetParentDistance GetComponentLocation()=%s GetActorLocation()=%s", *ParentWeapon->GetComponentLocation().ToString(), *GetActorLocation().ToString());

	return (ParentWeapon->GetComponentLocation() - GetActorLocation()).Size();
}
