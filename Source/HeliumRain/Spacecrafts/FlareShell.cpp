
#include "FlareShell.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTypes.h"
#include "../Game/FlareSkirmishManager.h"

#include "../Player/FlarePlayerController.h"

#include "Components/DecalComponent.h"
#include "Components/DestructibleComponent.h"


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
	ManualTurret = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareShell::Initialize(UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer)
{
	ShellDescription = Description;
	TracerShell = Tracer;
	ParentWeapon = Weapon;
	Armed = false;
	MinEffectiveDistance = 0.f;

	// Can't exist without description, can't return
	FCHECK(Description);

	ImpactSound = Description->WeaponCharacteristics.ImpactSound;
	DamageSound = Description->WeaponCharacteristics.DamageSound;

	ExplosionEffectTemplate = Description->WeaponCharacteristics.ExplosionEffect;
	ImpactEffectTemplate = Description->WeaponCharacteristics.ImpactEffect;
	ExplosionEffectScale = Description->WeaponCharacteristics.ExplosionEffectScale;
	ImpactEffectScale = Description->WeaponCharacteristics.ImpactEffectScale;
	FlightEffectsTemplate = Description->WeaponCharacteristics.GunCharacteristics.TracerEffect;

	ExplosionEffectMaterial = Description->WeaponCharacteristics.GunCharacteristics.ExplosionMaterial;
	
	float AmmoVelocity = Description->WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
	float KineticEnergy = Description->WeaponCharacteristics.GunCharacteristics.KineticEnergy;
	
	ShellVelocity = ParentVelocity + ShootDirection * AmmoVelocity * 100;
	ShellMass = 2 * KineticEnergy * 1000 / FMath::Square(AmmoVelocity); // ShellPower is in Kilo-Joule, reverse kinetic energy equation

	LastLocation = GetActorLocation();

	// Spawn the flight effects
	if (TracerShell)
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

	SetLifeSpan(ShellDescription->WeaponCharacteristics.GunCharacteristics.AmmoRange * 100 / ShellVelocity.Size()); // 10km
	ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->RegisterShell(this);
	PC = ParentWeapon->GetSpacecraft()->GetGame()->GetPC();

	ManualTurret = ParentWeapon->GetSpacecraft()->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET;
}

void AFlareShell::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	FVector ActorLocation = GetActorLocation();
	FVector NextActorLocation = ActorLocation + ShellVelocity * DeltaSeconds;
	SetActorLocation(NextActorLocation, false);
	SetActorRotation(ShellVelocity.Rotation());
	// 1 at 100m or less
	float Scale = 1;
	float BaseDistance = 10000.f;
	float MinScale = 0.1f;
	if(PC->GetShipPawn())
	{
		float LifeRatio = GetLifeSpan() / InitialLifeSpan;

		float LifeRatioScale = 1.f;

		if(LifeRatio < 0.1f)
		{
			LifeRatioScale = LifeRatio * 10.f;
		}

		float Distance = (NextActorLocation - PC->GetShipPawn()->GetActorLocation()).Size();
		if(Distance > BaseDistance)
		{
			Scale = (Distance / BaseDistance) * ((1.f-MinScale) * BaseDistance / Distance +MinScale) * LifeRatioScale;
		}
	}

	SetActorRelativeScale3D(FVector(0.6 + Scale * 0.4 , Scale, Scale));

	if (ShellDescription)
	{
		FHitResult HitResult(ForceInit);

		if (Trace(ActorLocation, NextActorLocation, HitResult))
		{
			OnImpact(HitResult, ShellVelocity);
		}
		
		if (ShellDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
		{
			if (SecureTime > 0)
			{
				SecureTime -= DeltaSeconds;
			}
			else if (ActiveTime > 0)
			{
				CheckFuze(ActorLocation, NextActorLocation);
				ActiveTime -= DeltaSeconds;
			}
		}
	}
}

void AFlareShell::CheckFuze(FVector ActorLocation, FVector NextActorLocation)
{
	FVector Center = (NextActorLocation + ActorLocation) / 2;
	float NearThresoldSquared = FMath::Square(100000); // 1km
	UFlareSector* Sector = ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector();
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Sector->GetSpacecrafts()[SpacecraftIndex];


		if (ShipCandidate == ParentWeapon->GetSpacecraft())
		{
			// Ignore parent spacecraft
			continue;
		}

		// First check if near to filter distant ship
		if ((Center - ShipCandidate->GetActorLocation()).SizeSquared() > NearThresoldSquared)
		{
			continue;
		}

		/*FLOG("=================");
		FLOGV("Proximity fuze near ship for %s",*GetHumanReadableName());
*/

		FVector ShellDirection = ShellVelocity.GetUnsafeNormal();
		FVector CandidateOffset = ShipCandidate->GetActorLocation() - ActorLocation;
		FVector NextCandidateOffset = ShipCandidate->GetActorLocation() - NextActorLocation;

		// Min distance
		float MinDistance = FVector::CrossProduct(CandidateOffset, ShellDirection).Size() / ShellDirection.Size();

		// Check if the min distance is not in the past
		if (FVector::DotProduct(CandidateOffset, ShellDirection) < 0)
		{
			// The target is behind the shell
			MinDistance = CandidateOffset.Size();
		}

		bool MinInFuture = false;
		// Check if the min distance is not in the future
		if (FVector::DotProduct(NextCandidateOffset, ShellDirection) > 0)
		{
			// The target is before the shell
			MinDistance = NextCandidateOffset.Size();
			MinInFuture = true;
		}



		float DistanceToMinDistancePoint;
		if (CandidateOffset.Size() == MinDistance)
		{
			DistanceToMinDistancePoint = 0;
		}
		else if (NextCandidateOffset.Size() == MinDistance)
		{
			DistanceToMinDistancePoint = (NextActorLocation - ActorLocation).Size();
		}
		else
		{
			DistanceToMinDistancePoint = FMath::Sqrt(CandidateOffset.SizeSquared() - FMath::Square(MinDistance));
		}

	/*	FLOGV("ShipCandidate->GetMeshScale() %f",ShipCandidate->GetMeshScale());
		FLOGV("DistanceToMinDistancePoint %f",DistanceToMinDistancePoint);
		FLOGV("Step distance %f",(NextActorLocation - ActorLocation).Size());
		FLOGV("MinDistance %f",MinDistance);
		FLOGV("Start Distance %f",(ActorLocation - ShipCandidate->GetActorLocation()).Size());
		FLOGV("End Distance %f",(NextActorLocation - ShipCandidate->GetActorLocation()).Size());
*/

		// Check if need to detonnate
		float EffectiveDistance = MinDistance - ShipCandidate->GetMeshScale();


		if (EffectiveDistance < ShellDescription->WeaponCharacteristics.FuzeMinDistanceThresold *100)
		{
			// Detonate because of too near. Find the detonate point.


			float MinThresoldDistance = ShellDescription->WeaponCharacteristics.FuzeMinDistanceThresold *100 + ShipCandidate->GetMeshScale();

		// 	FLOGV("MinThresoldDistance %f",MinThresoldDistance);
			float DistanceToMinThresoldDistancePoint = FMath::Sqrt(FMath::Square(MinThresoldDistance) - FMath::Square(MinDistance));
		// 	FLOGV("DistanceToMinThresoldDistancePoint %f",DistanceToMinThresoldDistancePoint);

			float DistanceToDetonatePoint = DistanceToMinDistancePoint - DistanceToMinThresoldDistancePoint;
		// 	FLOGV("DistanceToDetonatePoint %f",DistanceToDetonatePoint);
			FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToDetonatePoint;

			DetonateAt(DetonatePoint);
		}
		else if (Armed && EffectiveDistance > MinEffectiveDistance)
		{
			// We are armed and the distance as increase, detonate at nearest point
			FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToMinDistancePoint;
			DetonateAt(DetonatePoint);
		}
		else if (EffectiveDistance < ShellDescription->WeaponCharacteristics.FuzeMaxDistanceThresold *100)
		{
			if (MinInFuture)
			{
				// In activation zone but we will be near in future, arm the fuze
				Armed = true;
				MinEffectiveDistance = EffectiveDistance;
			}
			else
			{
				// In activation zone and the min distance is reach in this step, detonate
				FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToMinDistancePoint;
			// 	FLOGV("DistanceToMinDistancePoint %f",DistanceToMinDistancePoint);
				DetonateAt(DetonatePoint);
			}
		}
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
		

		float AbsorbedEnergy = ApplyDamage(HitResult.Actor.Get(), HitResult.GetComponent(), HitResult.Location, ImpactVelocityAxis, HitResult.ImpactNormal, ShellEnergy, ShellDescription->WeaponCharacteristics.AmmoDamageRadius, EFlareDamage::DAM_ArmorPiercing);
		bool Richochet = (AbsorbedEnergy < ShellEnergy);

		if (Richochet)
		{
			DestroyProjectile = false;
			float RemainingEnergy = ShellEnergy - AbsorbedEnergy;
			float RemainingVelocity = FMath::Sqrt(2 * RemainingEnergy * 1000 / ShellMass);
			FVector BounceDirection = ShellVelocity.GetUnsafeNormal().MirrorByVector(HitResult.ImpactNormal);
			ShellVelocity = BounceDirection * RemainingVelocity * 100;
			SetActorLocation(HitResult.Location);
		}
		else
		{
			AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(HitResult.Actor.Get());
			if (ShellDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
			{
				AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(HitResult.Actor.Get());
				AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(HitResult.Actor.Get());
				if (Spacecraft)
				{
					Spacecraft->GetDamageSystem()->ApplyDamage(ShellDescription->WeaponCharacteristics.ExplosionPower,
						ShellDescription->WeaponCharacteristics.AmmoDamageRadius, HitResult.Location, EFlareDamage::DAM_HEAT, ParentWeapon->GetSpacecraft()->GetParent(), GetName());

					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;

					// Physics impulse
					Spacecraft->Airframe->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
				}
				else if (Asteroid)
				{
					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;
					Asteroid->GetAsteroidComponent()->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
				}
				else if (Meteorite)
				{
					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;
					Meteorite->GetMeteoriteComponent()->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
					Meteorite->ApplyDamage(ShellDescription->WeaponCharacteristics.ExplosionPower,
										   ShellDescription->WeaponCharacteristics.AmmoDamageRadius, HitResult.Location, EFlareDamage::DAM_HEAT, ParentWeapon->GetSpacecraft()->GetParent(), GetName());
				}

			}

			// Spawn penetration effect
			if(!(Spacecraft && Spacecraft->IsInImmersiveMode()))
			{
				UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
					ExplosionEffectTemplate,
					HitResult.GetComponent(),
					NAME_None,
					HitResult.Location,
					HitResult.ImpactNormal.Rotation(),
					EAttachLocation::KeepWorldPosition,
					true);
				if (PSC)
				{
					PSC->SetWorldScale3D(ExplosionEffectScale * FVector(1, 1, 1));
				}
			}

			// Spawn hull damage effect
			UFlareSpacecraftComponent* HullComp = Cast<UFlareSpacecraftComponent>(HitResult.GetComponent());
			if (HullComp)
			{
				HullComp->StartDamagedEffect(HitResult.Location, HitResult.ImpactNormal.Rotation(), ParentWeapon->GetDescription()->Size);
			}

			// Remove flight effects
			if (FlightEffects)
			{
				FlightEffects->Deactivate();
			}
		}
	}

	if (DestroyProjectile)
	{
		Destroy();
	}
}

void AFlareShell::DetonateAt(FVector DetonatePoint)
{
	{
		UGameplayStatics::SpawnEmitterAtLocation(this,
			ExplosionEffectTemplate,
			DetonatePoint);
		UFlareSector* Sector = ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector();
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* ShipCandidate = Sector->GetSpacecrafts()[SpacecraftIndex];

			// First check if in radius area
			FVector CandidateOffset = ShipCandidate->GetActorLocation() - DetonatePoint;
			float CandidateDistance = CandidateOffset.Size();
			float CandidateSize = ShipCandidate->GetMeshScale();

			if (CandidateDistance > ShellDescription->WeaponCharacteristics.AmmoExplosionRadius * 100 + CandidateSize)
			{
				//FLOG("Too far");
				continue;
			}

			// DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), ShipCandidate->GetActorLocation(), CandidateSize, 12, FColor::Magenta, true);

			//FLOGV("CandidateOffset at %s",*CandidateOffset.ToString());

			// Find exposed surface
			// Apparent radius
			float ApparentRadius = FMath::Sqrt(FMath::Square(CandidateDistance) + FMath::Square(CandidateSize));

			float Angle = FMath::Acos(CandidateDistance/ApparentRadius);

			// DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, ApparentRadius, 12, FColor::Yellow, true);

			float ExposedSurface = 2 * PI * ApparentRadius * (ApparentRadius - CandidateDistance);
			float TotalSurface = 4 * PI * FMath::Square(ApparentRadius);

			float ExposedSurfaceRatio = ExposedSurface / TotalSurface;


			int FragmentCount =  FMath::RandRange(0,2) + ShellDescription->WeaponCharacteristics.AmmoFragmentCount * ExposedSurfaceRatio;

			/*FLOGV("CandidateDistance %f",CandidateDistance);
			FLOGV("CandidateSize %f",CandidateSize);
			FLOGV("ApparentRadius %f",ApparentRadius);
			FLOGV("Angle %f",FMath::RadiansToDegrees(Angle));
			FLOGV("ExposedSurface %f",ExposedSurface);
			FLOGV("TotalSurface %f",TotalSurface);
			FLOGV("ExposedSurfaceRatio %f",ExposedSurfaceRatio);
			FLOGV("FragmentCount %d",FragmentCount);*/


			TArray<UActorComponent*> Components = ShipCandidate->GetComponentsByClass(UStaticMeshComponent::StaticClass());

			//FLOGV("Component cont %d",Components.Num());
			for (int i = 0; i < FragmentCount; i ++)
			{

				FVector HitDirection = FMath::VRandCone(CandidateOffset, Angle);

				bool HasHit = false;
				FHitResult BestHitResult;
				float BestHitDistance = 0;

				for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
				{
					UStaticMeshComponent* Component = Cast<UStaticMeshComponent>(Components[ComponentIndex]);
					if (Component)
					{
						FHitResult HitResult(ForceInit);
						FCollisionQueryParams TraceParams(FName(TEXT("Fragment Trace")), true, this);
						TraceParams.bTraceComplex = true;
						TraceParams.bReturnPhysicalMaterial = false;
						Component->LineTraceComponent(HitResult, DetonatePoint, DetonatePoint + HitDirection * 2* CandidateDistance, TraceParams);

						if (HitResult.Actor.IsValid()){
							float HitDistance = (HitResult.Location - DetonatePoint).Size();
							if (!HasHit || HitDistance < BestHitDistance)
							{
								BestHitDistance = HitDistance;
								BestHitResult = HitResult;
							}

							//FLOGV("Fragment %d hit %s at a distance=%f",i, *Component->GetReadableName(), HitDistance);
							HasHit = true;
						}
					}

				}

				if (HasHit)
				{
					// DrawDebugLine(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, BestHitResult.Location, FColor::Green, true);

					AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(BestHitResult.Actor.Get());
					if (Spacecraft)
					{
						float FragmentPowerEffet = FMath::FRandRange(0.f, 2.f);
						float FragmentRangeEffet = FMath::FRandRange(0.5f, 1.5f);
						ApplyDamage(Spacecraft, BestHitResult.GetComponent()
									, BestHitResult.Location
									, HitDirection
									, BestHitResult.ImpactNormal
									, FragmentPowerEffet * ShellDescription->WeaponCharacteristics.ExplosionPower
									, FragmentRangeEffet  * ShellDescription->WeaponCharacteristics.AmmoDamageRadius
									, EFlareDamage::DAM_HighExplosive);

						// Play sound
						AFlareSpacecraftPawn* SpacecraftPawn = Cast<AFlareSpacecraftPawn>(Spacecraft);
						if (SpacecraftPawn->IsPlayerShip())
						{
							SpacecraftPawn->GetPC()->PlayLocalizedSound(ImpactSound, BestHitResult.Location);
						}
					}
				}
			}
		}

	}
	Destroy();
}

float AFlareShell::ApplyDamage(AActor *ActorToDamage, UPrimitiveComponent* HitComponent, FVector ImpactLocation,  FVector ImpactAxis,  FVector ImpactNormal, float ImpactPower, float ImpactRadius, EFlareDamage::Type DamageType)
{
	float Incidence = FVector::DotProduct(ImpactNormal, -ImpactAxis);
	float Armor = 1; // Full armored

	if (Incidence < 0)
	{
		// Parasite hit after rebound, ignore
		return 0;
	}

	// Hit a component
	UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(HitComponent);
	if (ShipComponent)
	{
		 Armor = ShipComponent->GetArmorAtLocation(ImpactLocation);
	}

	// Check armor peneration
	int32 PenetrateArmor = false;
	float PenerationIncidenceLimit = 0.7f;
	if (Incidence > PenerationIncidenceLimit)
	{
		PenetrateArmor = true; // No ricochet
	}
	else if (Armor == 0)
	{
		PenetrateArmor = true; // Armor destruction
	}

	// Hit a component : damage in KJ
	float AbsorbedEnergy = (PenetrateArmor ? ImpactPower : FMath::Square(Incidence) * ImpactPower);
	AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(ActorToDamage);
	AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(ActorToDamage);
	AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(ActorToDamage);
	AFlareBomb* Bomb = Cast<AFlareBomb>(ActorToDamage);
	if (Spacecraft)
	{
		DamageCause Cause(Cast<AFlareSpacecraft>(ParentWeapon->GetOwner())->GetParent(), DamageType);
		Cause.ManualTurret = ManualTurret;
		Spacecraft->GetDamageSystem()->SetLastDamageCause(Cause);
		Spacecraft->GetDamageSystem()->ApplyDamage(AbsorbedEnergy, ImpactRadius, ImpactLocation, DamageType, ParentWeapon->GetSpacecraft()->GetParent(), GetName());

		// Physics impulse
		Spacecraft->Airframe->AddImpulseAtLocation( 5000	 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);
		
		// Play sound
		AFlareSpacecraftPawn* SpacecraftPawn = Cast<AFlareSpacecraftPawn>(Spacecraft);
		if (SpacecraftPawn->IsPlayerShip())
		{
			SpacecraftPawn->GetPC()->PlayLocalizedSound(PenetrateArmor ? DamageSound : ImpactSound, ImpactLocation);
		}

		// Quest progress
		if (Spacecraft->GetGame()->GetQuestManager()
			&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
		{
			ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-ship").PutName("immatriculation", Spacecraft->GetImmatriculation()));
		}

		// Skirmish scoring
		if (Spacecraft->GetGame()->IsSkirmish())
		{
			bool HitByPlayer = ParentWeapon->GetSpacecraft()->GetCompany() == ParentWeapon->GetSpacecraft()->GetPC()->GetCompany();
			Spacecraft->GetGame()->GetSkirmishManager()->AmmoHit(HitByPlayer);
		}
	}
	else if (Asteroid)
	{
		// Physics impulse
		Asteroid->GetAsteroidComponent()->AddImpulseAtLocation( 5000	 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);
		if (Spacecraft->GetGame()->GetQuestManager()
			&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
		{
			ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-asteroid"));
		}
	}
	else if (Meteorite)
	{
		// Physics impulse
		Meteorite->GetMeteoriteComponent()->AddImpulseAtLocation( 5000	 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);
		if (Spacecraft->GetGame()->GetQuestManager()
			&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
		{
			ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-meteorite"));
		}
		Meteorite->ApplyDamage(AbsorbedEnergy, ImpactRadius, ImpactLocation, DamageType, ParentWeapon->GetSpacecraft()->GetParent(), GetName());
	}



	else if (Bomb)
	{
		FHitResult Hit;
		Bomb->NotifyHit(HitComponent, this, NULL, false, ImpactLocation, ImpactNormal, FVector::ZeroVector, Hit);
	}

	// Spawn impact decal
	if (HitComponent)
	{
		float DecalSize = FMath::FRandRange(50, 100);
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			ExplosionEffectMaterial,
			DecalSize * FVector(1, 1, 1),
			HitComponent,
			NAME_None,
			ImpactLocation,
			ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			120);
		if (Decal)
		{
			// Instanciate and configure the decal material
			UMaterialInterface* DecalMaterial = Decal->GetMaterial(0);
			UMaterialInstanceDynamic* DecalMaterialInst = UMaterialInstanceDynamic::Create(DecalMaterial, GetWorld());
			if (DecalMaterialInst)
			{
				DecalMaterialInst->SetScalarParameterValue("RandomParameter", FMath::FRandRange(1, 0));
				DecalMaterialInst->SetScalarParameterValue("RandomParameter2", FMath::FRandRange(1, 0));
				DecalMaterialInst->SetScalarParameterValue("IsShipHull", HitComponent->IsA(UFlareSpacecraftComponent::StaticClass()));
				Decal->SetMaterial(0, DecalMaterialInst);
			}
		}
	}

	// Apply FX
	if (HitComponent && !(Spacecraft && Spacecraft->IsInImmersiveMode()))
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			ImpactEffectTemplate,
			HitComponent,
			NAME_None,
			ImpactLocation,
			ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true);
		if (PSC)
		{
			PSC->SetWorldScale3D(ImpactEffectScale * FVector(1, 1, 1));
		}
	}

	return AbsorbedEnergy;
}

bool AFlareShell::Trace(const FVector& Start, const FVector& End, FHitResult& HitOut)
{
	// Ignore Actors
	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(ParentWeapon->GetSpacecraft());

	// Re-initialize hit info
	HitOut = FHitResult(ForceInit);

	ECollisionChannel CollisionChannel = (ECollisionChannel) (ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

	// Trace!
	GetWorld()->LineTraceSingleByChannel(
		HitOut,		// result
		Start,	// start
		End , // end
		CollisionChannel, // collision channel
		TraceParams
	);

	// Hit any Actor?
	return (HitOut.GetActor() != NULL) ;
}

void AFlareShell::Destroyed()
{
	Super::Destroyed();

	AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	FCHECK(Game);

	UFlareSector* Sector = Game->GetActiveSector();
	if (Sector->IsValidLowLevel())
	{
		Sector->UnregisterShell(this);
	}
}

void AFlareShell::SetFuzeTimer(float TargetSecureTime, float TargetActiveTime)
{
	SecureTime = TargetSecureTime;
	ActiveTime = TargetActiveTime;
}

void AFlareShell::SetPause(bool Pause)
{
	SetActorHiddenInGame(Pause);
	CustomTimeDilation = (Pause ? 0.f : 1.0);
}
