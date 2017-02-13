
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "../Game/FlareAsteroid.h"
#include "../Game/FlareGame.h"
#include "../Player/FlarePlayerController.h"
#include "FlareWeapon.h"
#include "FlareBombComponent.h"
#include "FlareBomb.h"


#define LOCTEXT_NAMESPACE "FlareBomb"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareBomb::AFlareBomb(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	TargetSpacecraft = NULL;

	// Mesh data
	BombComp = PCIP.CreateDefaultSubobject<UFlareBombComponent>(this, TEXT("Root"));
	BombComp->bTraceComplexOnMove = true;
	BombComp->LDMaxDrawDistance = 100000; // 1km*/
	BombComp->SetSimulatePhysics(true);
	BombComp->SetLinearDamping(0);
	BombComp->SetAngularDamping(0);
	RootComponent = BombComp;

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
		BombComp->Initialize(&ComponentData, Weapon->GetSpacecraft()->GetParent()->GetCompany(), Weapon->GetSpacecraft(), false);

		DamageSound = WeaponDescription->WeaponCharacteristics.DamageSound;

		ExplosionEffectTemplate = WeaponDescription->WeaponCharacteristics.ExplosionEffect;
		ExplosionEffectScale = WeaponDescription->WeaponCharacteristics.ExplosionEffectScale;
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
		BombData.Identifier = Weapon->GetSpacecraft()->GetGame()->GenerateIdentifier(TEXT("bomb"));
	}

	SetActorEnableCollision(BombData.Activated);

	if (BombData.AttachTarget != NAME_None)
	{
		AFlareSpacecraft* AttachTarget = ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->FindSpacecraft(BombData.AttachTarget);
		AttachBomb(AttachTarget);
	}
}

void AFlareBomb::OnLaunched(AFlareSpacecraft* Target)
{
	FLOG("AFlareBomb::OnLaunched");

	CombatLog::BombDropped(this);

	DetachRootComponentFromParent(true);
	ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->RegisterBomb(this);

	// Spin to stabilize
	FVector FrontVector = BombComp->ComponentToWorld.TransformVector(FVector(1, 0, 0));
	BombComp->SetPhysicsAngularVelocity(FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropAngularVelocity);
	BombComp->SetPhysicsLinearVelocity(ParentWeapon->GetSpacecraft()->Airframe->GetPhysicsLinearVelocity() + FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity * 100);

	BombData.DropParentDistance = GetParentDistance();
	BombData.Dropped = true;
	BombData.LifeTime = 0;
	TargetSpacecraft = Target;
}

void AFlareBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Activate after few centimeters
	if (BombData.Dropped && !BombData.Activated)
	{
		if (GetParentDistance() > BombData.DropParentDistance + WeaponDescription->WeaponCharacteristics.BombCharacteristics.ActivationDistance * 100)
		{
			SetActorEnableCollision(true);
			BombData.Activated = true;
		}
	}

	if (BombData.Dropped && BombData.Activated)
	{
		BombData.LifeTime += DeltaSeconds;
		LastTickRotation = GetActorRotation();
	}

	// Auto-destroy
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		if (PlayerShip)
		{
			// 5 km and 30s auto-destroy
			float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
			if (Distance > 500000 && BombData.LifeTime > 30)
			{
				OnBombDetonated(NULL, NULL, FVector(), FVector());
			}

			// Parent removed destroy
			if (!ParentWeapon || !ParentWeapon->IsValidLowLevel() || !ParentWeapon->GetSpacecraft()->IsValidLowLevel())
			{
				OnBombDetonated(NULL, NULL, FVector(), FVector());
			}
		}
	}

	if (TargetSpacecraft)
	{
		FVector TargetDelta = TargetSpacecraft->GetActorLocation() - GetActorLocation();
		FVector TargetDirection = TargetDelta.GetUnsafeNormal();
		FVector TargetVelocity = TargetSpacecraft->GetVelocity();
		FVector BombVelocity = BombComp->GetPhysicsLinearVelocity();
		FVector BombVelocityDirection = BombVelocity.GetUnsafeNormal();
		float DeltaVelocity = (BombVelocity - TargetVelocity).Size();


		float TimeToImpact = TargetDelta.Size() / DeltaVelocity;
		if(TimeToImpact < 0 || TimeToImpact > 2)
		{
			TimeToImpact = DeltaSeconds;
		}
		FVector AimDelta = (TargetSpacecraft->GetActorLocation() + TimeToImpact * TargetVelocity)  - GetActorLocation();
		FVector AimDirection = AimDelta.GetUnsafeNormal();


		FVector DirectionDelta = TargetDirection - BombVelocityDirection;

		FVector DeltaV;
		FVector AccelerationDirection;
		FVector DirectionCorrectionDirection = AimDirection;

		if (!DirectionDelta.IsNearlyZero())
		{
			DirectionCorrectionDirection = (AimDirection - BombVelocityDirection).GetUnsafeNormal();

		}

		float MaxAcceleration = 5000; // in m.s-2
		float DirectionCorrectionThresold = 0.999;
		float NominalVelocity = 10000; // In cm/s

		float NeededAcceleration;
		float Dot = FVector::DotProduct(BombVelocityDirection, AimDirection);
		FLOGV("Dot %f", Dot);

		FLOGV("TimeToImpact %f", TimeToImpact);
		FLOGV("DeltaSeconds %f", DeltaSeconds);

		if (Dot < DirectionCorrectionThresold)
		{



			DeltaV = (AimDirection * BombVelocity.Size() - BombVelocity).GetClampedToSize(0, MaxAcceleration * DeltaSeconds);

			FLOGV("Direction DeltaVelocity %s", *(AimDirection * BombVelocity.Size() - BombVelocity).ToString());
			FLOGV("Direction DeltaVelocity  size %f", (AimDirection * BombVelocity.Size() - BombVelocity).Size());

			AccelerationDirection = DirectionCorrectionDirection;
			NeededAcceleration = 1; // Max correction
		}
		else
		{
			float Alpha = (1 - Dot) / (1 - DirectionCorrectionThresold);

			FLOGV("Alpha %f", Alpha);

			NeededAcceleration = Alpha; // Direction correction Need




			FLOGV("DeltaVelocity %f", DeltaVelocity);

			if(DeltaVelocity < NominalVelocity)
			{
				NeededAcceleration = 1;
				AccelerationDirection = DirectionCorrectionDirection * Alpha + AimDirection * (1 - Alpha);
			}
			else
			{
				AccelerationDirection = DirectionCorrectionDirection;
			}

			DeltaV = MaxAcceleration * DeltaSeconds * NeededAcceleration * AccelerationDirection;
		}


		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + BombVelocity  * 1, FColor::White, true);
		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + AimDirection  * 500, FColor::Yellow, true);
		//DrawDebugLine(GetWorld(), GetActorLocation(), TargetSpacecraft->GetActorLocation(), FColor::Red, true);

		//Acceleration = MaxAcceleration * NeededAcceleration * AccelerationDirection;

		if (Dot < DirectionCorrectionThresold)
		{
		//	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DeltaV  * 1, FColor::Blue, true);
		}
		else
		{
		//	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DeltaV  * 1, FColor::Green, true);
		}

		FLOGV("DeltaV %s", *DeltaV.ToString());
		FLOGV("AccelerationDirection %s", *AccelerationDirection.ToString());
		FLOGV("NeededAcceleration %f", NeededAcceleration);

		BombComp->SetPhysicsLinearVelocity(DeltaV, true); // Multiply by 100 because UE4 works in cm
		FRotator(FQuat::FastLerp(BombComp->RelativeRotation.Quaternion(), BombVelocityDirection.Rotation().Quaternion(), DeltaSeconds));
	}

}

void AFlareBomb::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	AFlareBomb* BombCandidate = Cast<AFlareBomb>(Other);
	AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(Other);
	AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(Other);
	UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(OtherComp);

	// Forget uninteresting hits
	if (!Other || !OtherComp || !ParentWeapon || Other == ParentWeapon->GetSpacecraft() || BombCandidate || (BombData.AttachTarget != NAME_None))
	{
		FLOG("AFlareBomb::NotifyHit : invalid hit");
		return;
	}

	// Spawn penetration effect
	if (ExplosionEffectTemplate && !(Spacecraft && Spacecraft->IsInImmersiveMode()))
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			ExplosionEffectTemplate,
			OtherComp,
			NAME_None,
			HitLocation,
			HitNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true);
		if (PSC)
		{
			PSC->SetWorldScale3D(ExplosionEffectScale * FVector(1, 1, 1));
		}
	}

	// Physics impulse
	float ImpulseForce = 1000 * WeaponDescription->WeaponCharacteristics.ExplosionPower * WeaponDescription->WeaponCharacteristics.AmmoDamageRadius;
	FVector ImpulseDirection = (HitLocation - GetActorLocation()).GetUnsafeNormal();

	// Process damage
	if (Spacecraft)
	{
		Spacecraft->Airframe->AddImpulseAtLocation(ImpulseForce * ImpulseDirection, HitLocation);
		OnSpacecraftHit(Spacecraft, ShipComponent, HitLocation, NormalImpulse);
	}
	else if (Asteroid)
	{
		Asteroid->GetAsteroidComponent()->AddImpulseAtLocation(ImpulseForce * ImpulseDirection, HitLocation);
	}

	// Spawn impact decal
	if (ShipComponent && ExplosionEffectMaterial)
	{
		// Spawn
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			ExplosionEffectMaterial,
			FMath::FRandRange(50, 150) * FVector(1, 1, 1),
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
			DecalMaterialInst->SetScalarParameterValue("RandomParameter2", FMath::FRandRange(1, 0));
			DecalMaterialInst->SetScalarParameterValue("IsShipHull", ShipComponent->IsA(UFlareSpacecraftComponent::StaticClass()));
			Decal->SetMaterial(0, DecalMaterialInst);
		}
	}

	// Bomb has done its job, good job bomb
	OnBombDetonated(Spacecraft, ShipComponent, HitLocation, ImpulseDirection);
}

void AFlareBomb::OnSpacecraftHit(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal)
{
	UFlareCompany* OwnerCompany = ParentWeapon->GetSpacecraft()->GetCompany();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	// Apply damage
	HitSpacecraft->GetDamageSystem()->ApplyDamage(WeaponDescription->WeaponCharacteristics.ExplosionPower,
		WeaponDescription->WeaponCharacteristics.AmmoDamageRadius,
		HitLocation,
		SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType),
		ParentWeapon->GetSpacecraft()->GetParent());

	// Play sound
	if (HitSpacecraft->IsPlayerShip())
	{
		HitSpacecraft->GetPC()->PlayLocalizedSound(DamageSound, HitLocation);
	}

	// Ship salvage
	if (!HitSpacecraft->IsStation() &&
		((WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage && HitSpacecraft->GetDescription()->Size == EFlarePartSize::S)
	 || (WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage && HitSpacecraft->GetDescription()->Size == EFlarePartSize::L)))
	{
		FLOGV("AFlareBomb::OnSpacecraftHit : salvaging %s for %s", *HitSpacecraft->GetImmatriculation().ToString(), *OwnerCompany->GetCompanyName().ToString());
		HitSpacecraft->GetParent()->SetHarpooned(OwnerCompany);

		if (OwnerCompany == PC->GetCompany())
		{
			FFlareMenuParameterData Data;
			Data.Spacecraft = HitSpacecraft->GetParent();
			PC->Notify(LOCTEXT("HeavyShipHarpooned", "Ship harpooned !"),
				FText::Format(LOCTEXT("HeavyShipHarpoonedFormat", "If it is uncontrollable, you will retrieve {0} on the next day, if you are still in this sector."),
					FText::FromString(HitSpacecraft->GetImmatriculation().ToString())),
				FName("ship-harpooned"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Ship,
				Data);
		}
	}
}

void AFlareBomb::OnBombDetonated(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal)
{
	// Attach to the hull if it's a salvage harpoon
	if (HitSpacecraft && (
			(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage)
	 || (WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage)))
	{
		if (HitSpacecraft && !HitSpacecraft->IsStation() && HitComponent && WeaponDescription &&
		   ((WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage && HitSpacecraft->GetDescription()->Size == EFlarePartSize::S)
		 || (WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage && HitSpacecraft->GetDescription()->Size == EFlarePartSize::L)))
		{
			SetActorLocation(HitLocation);
			FLOGV("Attach actor rotation %s", *GetActorRotation().ToString());
			SetActorRotation(LastTickRotation);
			AttachBomb(HitSpacecraft);
		}
	}
	else
	{
		if (ParentWeapon)
		{
			ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->UnregisterBomb(this);
		}
		CombatLog::BombDestroyed(GetIdentifier());
		Destroy();
	}
}

void AFlareBomb::AttachBomb(AFlareSpacecraft* HitSpacecraft)
{
	if(HitSpacecraft)
	{
		AttachToActor(HitSpacecraft, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
		BombData.AttachTarget =  HitSpacecraft->GetImmatriculation();
	}
	else
	{
		BombData.AttachTarget =  NAME_None;
	}
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

	// On pause, save the state
	if (Pause)
	{
		Save();
	}

	// Toggle pause
	Paused = Pause;
	SetActorHiddenInGame(Pause);
	BombComp->SetSimulatePhysics(!Pause);
	CustomTimeDilation = (Pause ? 0.f : 1.0);

	// On unpause, restore physics
	if (!Pause)
	{
		BombComp->SetPhysicsLinearVelocity(BombData.LinearVelocity);
		BombComp->SetPhysicsAngularVelocity(BombData.AngularVelocity);
		if (!BombData.Dropped && ParentWeapon)
		{
			AttachToActor(ParentWeapon->GetSpacecraft(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
		}
	}
}

float AFlareBomb::GetParentDistance() const
{
	return (ParentWeapon->GetComponentLocation() - GetActorLocation()).Size();
}


#undef LOCTEXT_NAMESPACE
