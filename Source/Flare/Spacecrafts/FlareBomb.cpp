
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "../Game/FlareAsteroid.h"
#include "FlareWeapon.h"
#include "FlareBombComponent.h"
#include "FlareBomb.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareBomb::AFlareBomb(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	//FLOG("AFlareBomb");
	// Mesh data
	BombComp = PCIP.CreateDefaultSubobject<UFlareBombComponent>(this, TEXT("Root"));
	BombComp->bTraceComplexOnMove = true;
	BombComp->LDMaxDrawDistance = 100000; // 1km*/
	BombComp->SetSimulatePhysics(true);
	BombComp->SetLinearDamping(0);
	BombComp->SetAngularDamping(0);
	RootComponent = BombComp;

	SetActorEnableCollision(false);
	// Settings
	PrimaryActorTick.bCanEverTick = true;
	Paused = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareBomb::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFlareBomb::Initialize(const FFlareBombSave* Data, UFlareWeapon* Weapon)
{
	ParentWeapon = Weapon;
	WeaponDescription = Weapon->GetDescription();

	// Get the power from description
	if (WeaponDescription)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = WeaponDescription->Identifier;
		BombComp->Initialize(&ComponentData, Weapon->GetSpacecraft()->GetCompany(), Weapon->GetSpacecraft(), false);

		DamageSound = WeaponDescription->WeaponCharacteristics.DamageSound;

		ExplosionEffectTemplate = WeaponDescription->WeaponCharacteristics.ExplosionEffect;
		ExplosionEffectMaterial = WeaponDescription->WeaponCharacteristics.GunCharacteristics.ExplosionMaterial;

	}

	SetActorScale3D(ParentWeapon->GetSpacecraft()->GetActorScale3D());

	if (Data)
	{
		BombData = *Data;
	}
	else
	{
		BombData.Activated = false;
		BombData.Dropped = false;
		BombData.LifeTime = 0;
		BombData.DropParentDistance = 0;
	}

	if (BombData.Activated)
	{
		SetActorEnableCollision(true);
	}
}

void AFlareBomb::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	FLOG("AFlareBomb Hit");

	if (Other && OtherComp)
	{
		//TODO Investigate on NULL ParentWeapon
		if (!ParentWeapon || Other == ParentWeapon->GetSpacecraft())
		{
			// Avoid auto hit
			return;
		}


		AFlareBomb* BombCandidate = Cast<AFlareBomb>(Other);
		if (BombCandidate)
		{
			// Avoid bomb hit
			return;
		}

		// Spawn penetration effect
		if (ExplosionEffectTemplate)
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
		AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(Other);
		if (Spacecraft)
		{
			Spacecraft->GetDamageSystem()->ApplyDamage(WeaponDescription->WeaponCharacteristics.ExplosionPower , WeaponDescription->WeaponCharacteristics.AmmoDamageRadius, HitLocation, EFlareDamage::DAM_HEAT);


			float ImpulseForce = 1000 * WeaponDescription->WeaponCharacteristics.ExplosionPower * WeaponDescription->WeaponCharacteristics.AmmoDamageRadius;

			FVector ImpulseDirection = (HitLocation - GetActorLocation()).GetUnsafeNormal();

			// Physics impulse
			Spacecraft->Airframe->AddImpulseAtLocation( ImpulseForce * ImpulseDirection, HitLocation);

			// Play sound
			AFlareSpacecraftPawn* ShipBase = Cast<AFlareSpacecraftPawn>(Spacecraft);
			if (ShipBase && ShipBase->IsLocallyControlled())
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamageSound, HitLocation, 1, 1);
			}
		}
		else if (Asteroid)
		{
			float ImpulseForce = 1000 * WeaponDescription->WeaponCharacteristics.ExplosionPower * WeaponDescription->WeaponCharacteristics.AmmoDamageRadius;

			FVector ImpulseDirection = (HitLocation - GetActorLocation()).GetUnsafeNormal();

			// Physics impulse
			Asteroid->GetStaticMeshComponent()->AddImpulseAtLocation( ImpulseForce * ImpulseDirection, HitLocation);
		}

		UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(OtherComp);
		// Spawn impact decal
		if (ShipComponent && ShipComponent->IsVisibleByPlayer() && ExplosionEffectMaterial)
		{
			// FX data
			float DecalSize = FMath::FRandRange(50, 150);

			UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
				ExplosionEffectMaterial,
				DecalSize * FVector(1, 1, 1),
				ShipComponent,
				NAME_None,
				HitLocation,
				HitNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				120);

			// Instanciate and configure the decal material
			UMaterialInterface* DecalMaterial = Decal->GetMaterial(0);
			UMaterialInstanceDynamic* DecalMaterialInst = UMaterialInstanceDynamic::Create(DecalMaterial, GetWorld());
			if (DecalMaterialInst)
			{
				DecalMaterialInst->SetScalarParameterValue("RandomParameter", FMath::FRandRange(1, 0));
				Decal->SetMaterial(0, DecalMaterialInst);
			}
		}

		Spacecraft->GetGame()->GetActiveSector()->UnregisterBomb(this);
		Destroy();
	}


}

void AFlareBomb::Drop()
{
	FLOG("AFlareBomb Drop");
	DetachRootComponentFromParent(true);
	ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->RegisterBomb(this);

	FVector FrontVector = BombComp->ComponentToWorld.TransformVector(FVector(1,0,0));

	// Spin to stabilize
	BombComp->SetPhysicsAngularVelocity(FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropAngularVelocity);
	BombComp->SetPhysicsLinearVelocity(ParentWeapon->GetSpacecraft()->Airframe->GetPhysicsLinearVelocity() + FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity * 100);

	BombData.DropParentDistance = GetParentDistance();
	BombData.Dropped = true;
	BombData.LifeTime = 0;

}


void AFlareBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BombData.Dropped && !BombData.Activated)
	{
		if (GetParentDistance() > BombData.DropParentDistance + WeaponDescription->WeaponCharacteristics.BombCharacteristics.ActivationDistance*100)
		{
			// Activate after few centimeters
			SetActorEnableCollision(true);
			BombData.Activated = true;
		}
	}

	if (BombData.Dropped && BombData.Activated)
	{
		BombData.LifeTime += DeltaSeconds;
	}

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		if (PlayerShip)
		{
			float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
			if (Distance > 500000 && BombData.LifeTime > 30)
			{
				// 5 km and 30s
				Destroy();
				PlayerShip->GetGame()->GetActiveSector()->UnregisterBomb(this);
			}
		}
	}

}


float AFlareBomb::GetParentDistance() const
{
	return (ParentWeapon->GetComponentLocation() - GetActorLocation()).Size();
}

FFlareBombSave* AFlareBomb::Save()
{
	// Physical data
	BombData.Location = GetActorLocation();
	BombData.Rotation = GetActorRotation();
	if (!Paused)
	{
		BombData.LinearVelocity = BombComp->GetPhysicsLinearVelocity();
		BombData.AngularVelocity = BombComp->GetPhysicsAngularVelocity();
	}

	// TODO Investigate on NULL ParentWeapon
	if (ParentWeapon)
	{
		BombData.ParentSpacecraft = ParentWeapon->GetSpacecraft()->GetImmatriculation();
		BombData.WeaponSlotIdentifier = ParentWeapon->SlotIdentifier;
	}

	return &BombData;
}

void AFlareBomb::SetPause(bool Pause)
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
	BombComp->SetSimulatePhysics(!Pause);

	Paused = Pause;
	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		BombComp->SetPhysicsLinearVelocity(BombData.LinearVelocity);
		BombComp->SetPhysicsAngularVelocity(BombData.AngularVelocity);
		if (!BombData.Dropped && ParentWeapon)
		{
			AttachRootComponentToActor(ParentWeapon->GetSpacecraft(),"", EAttachLocation::KeepWorldPosition, true);
		}
	}
}
