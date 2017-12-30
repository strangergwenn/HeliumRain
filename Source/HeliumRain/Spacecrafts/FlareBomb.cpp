
#include "FlareBomb.h"
#include "../Flare.h"

#include "../Game/FlareAsteroid.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareSkirmishManager.h"

#include "../Player/FlarePlayerController.h"

#include "FlareWeapon.h"
#include "FlareBombComponent.h"
#include "FlareSpacecraft.h"
#include "FlareShell.h"

#include "Components/DecalComponent.h"
#include "Components/DestructibleComponent.h"

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
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	Paused = false;
	BombLockedInCollision = 0;
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
		BombData.Locked = false;
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
	if (BombData.AimTargetSpacecraft != NAME_None)
	{
		TargetSpacecraft = ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->FindSpacecraft(BombData.AimTargetSpacecraft);
	}

	BombComp->BodyInstance.bUseCCD = true;

	ParentWeapon->MoveIgnoreActors.Add(this);
}

void AFlareBomb::OnLaunched(AFlareSpacecraft* Target)
{
	FLOG("AFlareBomb::OnLaunched");

	CombatLog::BombDropped(this);

	// Detach
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	DetachFromActor(DetachRules);

	ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->RegisterBomb(this);

	// Spin to stabilize
	FVector FrontVector = BombComp-> GetComponentTransform().TransformVector(FVector(1, 0, 0));
	BombComp->SetPhysicsAngularVelocity(FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropAngularVelocity);
	BombComp->SetPhysicsLinearVelocity(ParentWeapon->GetSpacecraft()->Airframe->GetPhysicsLinearVelocity() + FrontVector * WeaponDescription->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity * 100);

	BombData.DropParentDistance = GetParentDistance();
	BombData.Dropped = true;
	BombData.LifeTime = 0;
	BombData.BurnDuration = 0;
	TargetSpacecraft = Target;
}

void AFlareBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(BombLockedInCollision > 0)
	{
		BombLockedInCollision -= 0.5;
	}

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

	float GimbalRangeDot = 0.99;
	float DirectionCorrectionThresold = 0.999; // In dot


	// Auto-destroy
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		if (PlayerShip)
		{
			// no fuel and 5 km and 30s auto-destroy
			float Distance = (GetActorLocation() - PlayerShip->GetActorLocation()).Size();
			if (!IsActive() && Distance > 500000 && BombData.LifeTime > 30)
			{
				OnBombDetonated(NULL, NULL, FVector(), FVector());
				//DrawDebugSphere(GetWorld(), GetActorLocation(), 1000, 32, FColor::Red, true);

				// Test Player ship avoidance
				if(WeaponDescription->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration > 0 && TargetSpacecraft == PlayerShip && ParentWeapon->GetSpacecraft()->GetCompany()->IsAtWar(PC->GetCompany()))
				{
					PC->SetAchievementProgression("ACHIEVEMENT_MISSILE_ESCAPE", 1);
				}
			}

			// Parent removed destroy
			if (!ParentWeapon || !ParentWeapon->IsValidLowLevel() || !ParentWeapon->GetSpacecraft()->IsValidLowLevel())
			{
				OnBombDetonated(NULL, NULL, FVector(), FVector());
				//DrawDebugSphere(GetWorld(), GetActorLocation(), 1000, 32, FColor::Red, true);
			}
		}
	}

	float NeededAcceleration = 0;


	
	if (TargetSpacecraft && BombData.LifeTime > WeaponDescription->WeaponCharacteristics.BombCharacteristics.ActivationTime && BombData.BurnDuration < WeaponDescription->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration)
	{
		//ProcessGuidance(DeltaSeconds);
		//v2
		FVector TargetPredictedLocation = TargetSpacecraft->GetActorLocation();
		FVector TargetDeltaLocation = TargetPredictedLocation - GetActorLocation();
		FVector TargetDirection = TargetDeltaLocation.GetUnsafeNormal();


		FVector TargetVelocity = TargetSpacecraft->GetVelocity();

		FVector BombVelocityRefTarget = BombComp->GetPhysicsLinearVelocity() - TargetVelocity;
		FVector BombVelocityDirectionRefTarget = BombVelocityRefTarget.GetUnsafeNormal();


		FVector AimVelocityRefTarget = TargetDirection * WeaponDescription->WeaponCharacteristics.BombCharacteristics.NominalVelocity;


	
		float MaxDeltaV = WeaponDescription->WeaponCharacteristics.BombCharacteristics.MaxAcceleration * DeltaSeconds;

		float Dot = FVector::DotProduct(BombVelocityDirectionRefTarget, TargetDirection);
		FVector EffectiveDeltaVelocity = FVector::ZeroVector;

		FVector FineAimVelocityRefTarget = AimVelocityRefTarget;


		/*FLOGV("TargetDeltaLocation %s", *TargetDeltaLocation.ToString());
		FLOGV("TargetDirection %s", *TargetDirection.ToString());
		FLOGV("TargetVelocity %s", *TargetVelocity.ToString());
		FLOGV("BombVelocityRefTarget %s", *BombVelocityRefTarget.ToString());
		FLOGV("BombVelocityDirectionRefTarget %s", *BombVelocityDirectionRefTarget.ToString());
		FLOGV("AimVelocityRefTarget %s", *AimVelocityRefTarget.ToString());


		FLOGV("Dot %f", Dot);*/

		/*if (!BombVelocityRefTarget.IsNearlyZero())
		{

			float ConvergenceSpeed = FMath::Max(WeaponDescription->WeaponCharacteristics.BombCharacteristics.NominalVelocity /50.f, FVector::DotProduct(TargetDirection, BombVelocityRefTarget));
			//FLOGV("ConvergenceSpeed %f", ConvergenceSpeed);

			if(Dot < DirectionCorrectionThresold)
			{
				// Bad alignement, don't speed up
				FineAimVelocityRefTarget = TargetDirection * ConvergenceSpeed;
			}
			else
			{
				FineAimVelocityRefTarget = TargetDirection * FMath::Max(ConvergenceSpeed, WeaponDescription->WeaponCharacteristics.BombCharacteristics.NominalVelocity);
			}
		}*/
		
		//FLOGV("FineAimVelocityRefTarget Dot %f", FVector::DotProduct(FineAimVelocityRefTarget.GetUnsafeNormal(), TargetDirection));

		FVector DeltaVelocity = FineAimVelocityRefTarget - BombVelocityRefTarget;



		FVector AngularVelocityTarget = FVector::ZeroVector;


		if (!DeltaVelocity.IsNearlyZero())
		{
			FVector DeltaVelocityDirection = DeltaVelocity.GetUnsafeNormal();
			FVector WorldBombAxis = BombComp->GetComponentToWorld().GetRotation().RotateVector(FVector::ForwardVector);

			//FLOGV("GimbalRangeDot %f", FVector::DotProduct(DeltaVelocityDirection, WorldBombAxis));

			if (!BombData.Locked && FVector::DotProduct(DeltaVelocityDirection, WorldBombAxis) > GimbalRangeDot)
			{
				BombData.Locked = true;
			}

			// Bomb orientation
			AngularVelocityTarget = GetAngularVelocityToAlignAxis(DeltaVelocityDirection,WeaponDescription->WeaponCharacteristics.BombCharacteristics.AngularAcceleration, DeltaSeconds);

			//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DeltaVelocityDirection * 100, FColor::Green, false);
		}


		if (BombData.Locked)
		{
			EffectiveDeltaVelocity = DeltaVelocity.GetClampedToSize(0, MaxDeltaV);
		}

		NeededAcceleration = EffectiveDeltaVelocity.Size() / MaxDeltaV;



		BombComp->SetPhysicsLinearVelocity(EffectiveDeltaVelocity, true); // Multiply by 100 because UE4 works in cm
		//BombComp->SetRelativeRotation(FRotator(FQuat::FastLerp(BombComp->RelativeRotation.Quaternion(), BombVelocityDirection.Rotation().Quaternion(), DeltaSeconds)));

		// Angular physics
		FVector DeltaAngularV = AngularVelocityTarget - BombComp->GetPhysicsAngularVelocity();

		if (!DeltaAngularV.IsNearlyZero())
		{
			FVector	DeltaAngularVAxis = DeltaAngularV.GetUnsafeNormal();
			FVector Acceleration = DeltaAngularVAxis * WeaponDescription->WeaponCharacteristics.BombCharacteristics.AngularAcceleration * DeltaSeconds;
			FVector ClampedAcceleration = Acceleration.GetClampedToMaxSize(DeltaAngularV.Size());
			BombComp->SetPhysicsAngularVelocity(ClampedAcceleration, true);
		}


		/*float TargetDistance = TargetDeltaLocation.Size();

		FLOGV("FineAimVelocityRefTarget %s", *FineAimVelocityRefTarget.ToString());

		FLOGV("DeltaVelocity %s", *DeltaVelocity.ToString());

	
		FLOGV("EffectiveDeltaVelocity %s", *EffectiveDeltaVelocity.ToString());
		FLOGV("NeededAcceleration %f", NeededAcceleration);
		FLOGV("NeededAcceleration optimal %f", MaxDeltaV/ DeltaVelocity.Size());
		FLOGV("BurnDuration %f", BombData.BurnDuration);
		FLOGV("TargetDistance %f", TargetDistance);
		
		if (LastLocation != FVector::ZeroVector)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), LastLocation, FColor::Green, true);
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + EffectiveDeltaVelocity, FColor::Blue, true);
			DrawDebugLine(GetWorld(), TargetSpacecraft->GetActorLocation(), LastTargetLocation, FColor::Red, true);
		}*/
		LastLocation = GetActorLocation();
		LastTargetLocation = TargetSpacecraft->GetActorLocation();
		

		BombData.BurnDuration += NeededAcceleration * DeltaSeconds;
	}

	BombComp->UpdateEffects(NeededAcceleration);

}



FVector AFlareBomb::GetAngularVelocityToAlignAxis(FVector TargetAxis, float AngularAcceleration, float DeltaSeconds) const
{
	FVector AngularVelocity = BombComp->GetPhysicsAngularVelocity();
	FVector WorldBombAxis = BombComp->GetComponentToWorld().GetRotation().RotateVector(FVector::ForwardVector);

	WorldBombAxis.Normalize();
	TargetAxis.Normalize();

	FVector RotationDirection = FVector::CrossProduct(WorldBombAxis, TargetAxis);
	RotationDirection.Normalize();
	float Dot = FVector::DotProduct(WorldBombAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	FVector DeltaVelocity = - AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
		FVector Acceleration = DeltaVelocityAxis * AngularAcceleration;
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, RotationDirection));

		TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (FMath::Max(TimeToFinalVelocity,DeltaSeconds));

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = FVector::ZeroVector;
	}
	else
	{
		float MaxPreciseSpeed = (angle - AngleToStop) / (DeltaSeconds * 0.75f);

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
}

void AFlareBomb::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	AFlareBomb* BombCandidate = Cast<AFlareBomb>(Other);
	AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(Other);
	AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(Other);
	AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(Other);
	AFlareShell* Shell = Cast<AFlareShell>(Other);
	UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(OtherComp);

	// Forget uninteresting hits
	if ( !Other || (!Shell && !OtherComp) || !ParentWeapon || Other == ParentWeapon->GetSpacecraft() || BombCandidate || (BombData.AttachTarget != NAME_None))
	{
		FLOG("AFlareBomb::NotifyHit : invalid hit");
		if(Other == ParentWeapon->GetSpacecraft())
		{
			BombLockedInCollision += 1;
			// Random 1m move
			if (BombLockedInCollision > 1)
			{
				SetActorLocation(GetActorLocation() + FMath::VRand() * 10);
			}
		}
		return;
	}

	// Spawn penetration effect
	if (ExplosionEffectTemplate && !(Spacecraft && Spacecraft->GetCameraMode() == EFlareCameraMode::Immersive))
	{
		UParticleSystemComponent* PSC;
		if(Shell)
		{
			PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ExplosionEffectTemplate,
				HitLocation,
				HitNormal.Rotation(),
				true);
		}
		else
		{
			PSC = UGameplayStatics::SpawnEmitterAttached(
				ExplosionEffectTemplate,
				OtherComp,
				NAME_None,
				HitLocation,
				HitNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				true);
		}
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
		if(!Spacecraft->IsStation() && WeaponDescription->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration == 0 && ParentWeapon->GetSpacecraft()->GetParent() == Spacecraft->GetPC()->GetPlayerShip())
		{
			if(!Spacecraft->GetParent()->GetDamageSystem()->IsUncontrollable() && Spacecraft->GetCompany()->GetPlayerWarState() == EFlareHostility::Hostile)
			{
				if(Spacecraft->GetSize() == EFlarePartSize::S)
				{
					Spacecraft->GetPC()->SetAchievementProgression("ACHIEVEMENT_BOMB_LIGHT", 1);
				}
				else if(Spacecraft->GetSize() == EFlarePartSize::L)
				{
					Spacecraft->GetPC()->SetAchievementProgression("ACHIEVEMENT_BOMB_HEAVY", 1);
				}
			}
		}

		OnSpacecraftHit(Spacecraft, ShipComponent, HitLocation, NormalImpulse);
	}
	else if (Asteroid)
	{
		Asteroid->GetAsteroidComponent()->AddImpulseAtLocation(ImpulseForce * ImpulseDirection, HitLocation);
	}
	else if (Meteorite)
	{
		Meteorite->GetMeteoriteComponent()->AddImpulseAtLocation(ImpulseForce * ImpulseDirection, HitLocation);
		Meteorite->ApplyDamage(WeaponDescription->WeaponCharacteristics.ExplosionPower,
			WeaponDescription->WeaponCharacteristics.AmmoDamageRadius,
			HitLocation,
			SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType),
			ParentWeapon->GetSpacecraft()->GetParent(),
			GetName());
	}

	// Spawn impact decal
	if (ShipComponent && ExplosionEffectMaterial)
	{
		// Spawn
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			ExplosionEffectMaterial,
			FMath::FRandRange(100, 150) * FVector(1, 1, 1),
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

	// Skirmish scoring
	if (Spacecraft->GetGame()->IsSkirmish())
	{
		bool HitByPlayer = ParentWeapon->GetSpacecraft()->GetCompany() == ParentWeapon->GetSpacecraft()->GetPC()->GetCompany();
		Spacecraft->GetGame()->GetSkirmishManager()->AmmoHit(HitByPlayer);
	}

	// Bomb has done its job, good job bomb
	OnBombDetonated(Spacecraft, ShipComponent, HitLocation, ImpulseDirection);
}

void AFlareBomb::OnSpacecraftHit(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal)
{
	UFlareCompany* OwnerCompany = ParentWeapon->GetSpacecraft()->GetCompany();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	//DrawDebugSphere(GetWorld(), GetActorLocation(), 1000, 32, FColor::Green, true);

	// Apply damage
	HitSpacecraft->GetDamageSystem()->SetLastDamageCause(DamageCause(Cast<AFlareSpacecraft>(ParentWeapon->GetOwner())->GetParent(), SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType)));
	HitSpacecraft->GetDamageSystem()->ApplyDamage(WeaponDescription->WeaponCharacteristics.ExplosionPower,
		WeaponDescription->WeaponCharacteristics.AmmoDamageRadius,
		HitLocation,
		SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType),
		ParentWeapon->GetSpacecraft()->GetParent(),
		GetName());

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
					UFlareGameTools::DisplaySpacecraftName(HitSpacecraft->GetParent())),
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

	if(TargetSpacecraft)
	{
		BombData.AimTargetSpacecraft = TargetSpacecraft->GetImmatriculation();
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

bool AFlareBomb::IsActive() const
{
	return BombData.BurnDuration < WeaponDescription->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration;
}

#undef LOCTEXT_NAMESPACE
