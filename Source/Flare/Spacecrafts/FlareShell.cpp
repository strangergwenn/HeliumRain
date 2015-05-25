
#include "../Flare.h"
#include "FlareSpacecraft.h"
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

void AFlareShell::Initialize(UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer)
{
	ShellDescription = Description;
	TracerShell = Tracer;
	ParentWeapon = Weapon;
	float AmmoVelocity = 0.f;
	// Get the power from description
	if (Description)
	{
		ShellPower = Description->GunCharacteristics.AmmoPower;
		AmmoVelocity = Description->GunCharacteristics.AmmoVelocity;

		ImpactSound = Description->GunCharacteristics.ImpactSound;
		DamageSound = Description->GunCharacteristics.DamageSound;

		ExplosionEffectTemplate = Description->GunCharacteristics.ExplosionEffect;
		ImpactEffectTemplate = Description->GunCharacteristics.ImpactEffect;
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

	if(ShellDescription)
	{
		if(ShellDescription->GunCharacteristics.FuzeType == EFlareShellFuzeType::Contact)
		{
			FHitResult HitResult(ForceInit);
			if (Trace(ActorLocation, NextActorLocation, HitResult))
			{
				OnImpact(HitResult, ShellVelocity);
			}
		}
		else if(ShellDescription->GunCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
		{

			FVector Center = (NextActorLocation + ActorLocation) / 2;
			float NearThresoldSquared = FMath::Square(100000); // 1km
			for (TActorIterator<AActor> ActorItr(ParentWeapon->GetSpacecraft()->GetWorld()); ActorItr; ++ActorItr)
			{

				// Ship
				AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
				if (ShipCandidate)
				{
					if(ShipCandidate == ParentWeapon->GetSpacecraft())
					{
						//Ignore parent spacecraft
						continue;
					}

					// First check if near to filter distant ship
					if((Center - ShipCandidate->GetActorLocation()).SizeSquared() > NearThresoldSquared)
					{
						continue;
					}

					/*FLOG("=================");
					FLOGV("Proximity fuze near ship for %s",*GetHumanReadableName());
*/

					FVector ShellDirection = ShellVelocity.GetUnsafeNormal();
					FVector CandidateOffset = ShipCandidate->GetActorLocation() - ActorLocation;

					// Min distance
					float MinDistance = FVector::CrossProduct(CandidateOffset, ShellDirection).Size() / ShellDirection.Size();
					if(MinDistance > ShellDescription->GunCharacteristics.FuzeMaxDistanceThresold *100 + ShipCandidate->GetMeshScale())
					{
						// Too far
						continue;
					}

					// Check if the min distance is not in the past
					if(FVector::DotProduct(CandidateOffset, ShellDirection) < 0)
					{
						// The target is behind the shell
						MinDistance = CandidateOffset.Size();
					}

					// Check if need to detonnate
					float EffectiveDistance = MinDistance - ShipCandidate->GetMeshScale();

					float DistanceToMinDistancePoint;
					if(CandidateOffset.Size() == MinDistance)
					{
						DistanceToMinDistancePoint = 0;
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

					if(DistanceToMinDistancePoint > (NextActorLocation - ActorLocation).Size())
					{
						FLOG("In future, wait");
						// In future
						continue;
					}

					if (EffectiveDistance < ShellDescription->GunCharacteristics.FuzeMinDistanceThresold *100)
					{
						// Detonate because of too near. Find the detonate point.
					//	FLOGV("Detonate because too near EffectiveDistance=%f FuzeMinDistanceThresold=%f", EffectiveDistance, ShellDescription->GunCharacteristics.FuzeMinDistanceThresold *100)



						float MinThresoldDistance = ShellDescription->GunCharacteristics.FuzeMinDistanceThresold *100 + ShipCandidate->GetMeshScale();

					//	FLOGV("MinThresoldDistance %f",MinThresoldDistance);
						float DistanceToMinThresoldDistancePoint = FMath::Sqrt(FMath::Square(MinThresoldDistance) - FMath::Square(MinDistance));
					//	FLOGV("DistanceToMinThresoldDistancePoint %f",DistanceToMinThresoldDistancePoint);

						float DistanceToDetonatePoint = DistanceToMinDistancePoint - DistanceToMinThresoldDistancePoint;
					//	FLOGV("DistanceToDetonatePoint %f",DistanceToDetonatePoint);
						FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToDetonatePoint;

						DetonateAt(DetonatePoint);
					}
					else if (EffectiveDistance < ShellDescription->GunCharacteristics.FuzeMaxDistanceThresold *100)
					{
					//	FLOGV("Detonate because min distance reach EffectiveDistance=%f FuzeMaxDistanceThresold=%f", EffectiveDistance, ShellDescription->GunCharacteristics.FuzeMaxDistanceThresold *100)
						FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToMinDistancePoint;
					//	FLOGV("DistanceToMinDistancePoint %f",DistanceToMinDistancePoint);
						DetonateAt(DetonatePoint);

					}
				}
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
		float PenerationIncidenceLimit = 0.6f;
		float Incidence = FVector::DotProduct(HitResult.ImpactNormal, -ImpactVelocityAxis);
		float RemainingArmor = -1; // Negative value means undestructible

		if(Incidence < 0)
		{
			// Parasite hit after rebound, ignore
			return;
		}

		// Hit a component
		UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(HitResult.Component.Get());
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
		AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(HitResult.Actor.Get());
		if (Spacecraft)
		{
			// TODO Shell distance
			Spacecraft->GetDamageSystem()->ApplyDamage(AbsorbedEnergy, ShellDescription->GunCharacteristics.AmmoDamageRadius, HitResult.Location);
			
			// Play sound
			AFlareSpacecraftPawn* ShipBase = Cast<AFlareSpacecraftPawn>(Spacecraft);
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
			UGameplayStatics::SpawnEmitterAttached(
				ImpactEffectTemplate,
				Target,
				NAME_None,
				HitResult.Location,
				HitResult.ImpactNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				true);
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

void AFlareShell::DetonateAt(FVector DetonatePoint)
{
	/*FLOG("-------------");
	FLOGV("Detonate at %s",*DetonatePoint.ToString());
	FLOGV("AmmoExplosionRadius %f",ShellDescription->GunCharacteristics.AmmoExplosionRadius * 100);
	if(ShellDescription && ShellDescription->GunCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)*/
	{
		UGameplayStatics::SpawnEmitterAtLocation(this,
			ExplosionEffectTemplate,
			DetonatePoint);

		// TODO Arm time to avoid to hit allied

		//DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, ShellDescription->GunCharacteristics.AmmoExplosionRadius * 100, 12, FColor::Red, true);
		for (TActorIterator<AActor> ActorItr(ParentWeapon->GetSpacecraft()->GetWorld()); ActorItr; ++ActorItr)
		{
			// Ship
			AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if (ShipCandidate)
			{
				// First check if in radius area
				FVector CandidateOffset = ShipCandidate->GetActorLocation() - DetonatePoint;
				float CandidateDistance = CandidateOffset.Size();
				float CandidateSize = ShipCandidate->GetMeshScale();

				/*FLOGV("CandidateDistance %f",CandidateDistance);
				FLOGV("CandidateSize %f",CandidateSize);*/

				if(CandidateDistance > ShellDescription->GunCharacteristics.AmmoExplosionRadius * 100 + CandidateSize)
				{
					//FLOG("Too far");
					continue;
				}

				//DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), ShipCandidate->GetActorLocation(), CandidateSize, 12, FColor::Magenta, true);

				//FLOGV("CandidateOffset at %s",*CandidateOffset.ToString());

				// Find exposed surface
				//Apparent radius
				float ApparentRadius = FMath::Sqrt(FMath::Square(CandidateDistance) + FMath::Square(CandidateSize));

				float Angle = FMath::Acos(CandidateDistance/ApparentRadius);

				//DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, ApparentRadius, 12, FColor::Yellow, true);

				float ExposedSurface = 2 * PI * ApparentRadius * (ApparentRadius - CandidateDistance);
				float TotalSurface = 4 * PI * FMath::Square(ApparentRadius);

				float ExposedSurfaceRatio = ExposedSurface / TotalSurface;


				int FragmentCount = ShellDescription->GunCharacteristics.AmmoFragmentCount * ExposedSurfaceRatio;

				/*FLOGV("ApparentRadius %f",ApparentRadius);
				FLOGV("Angle %f",FMath::RadiansToDegrees(Angle));
				FLOGV("ExposedSurface %f",ExposedSurface);
				FLOGV("TotalSurface %f",TotalSurface);
				FLOGV("ExposedSurfaceRatio %f",ExposedSurfaceRatio);
				FLOGV("FragmentCount %d",FragmentCount);*/


				TArray<UActorComponent*> Components = ShipCandidate->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());

				//FLOGV("Component cont %d",Components.Num());
				for(int i = 0; i < FragmentCount; i ++)
				{

					FVector HitDirection = FMath::VRandCone(CandidateOffset, Angle);

					bool HasHit = false;
					FHitResult BestHitResult;
					float BestHitDistance;

					for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
					{
						UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
						if(Component)
						{
							FHitResult HitResult(ForceInit);
							FCollisionQueryParams TraceParams(FName(TEXT("Fragment Trace")), true, this);
							TraceParams.bTraceComplex = true;
							TraceParams.bReturnPhysicalMaterial = false;
							Component->LineTraceComponent(HitResult, DetonatePoint, DetonatePoint + HitDirection * 2* CandidateDistance, TraceParams);

							if(HitResult.Actor.IsValid()){
								float HitDistance = (HitResult.Location - DetonatePoint).Size();
								if(!HasHit || HitDistance < BestHitDistance)
								{
									BestHitDistance = HitDistance;
									BestHitResult = HitResult;

								}

								//FLOGV("Fragment %d hit %s at a distance=%f",i, *Component->GetReadableName(), HitDistance);
								HasHit = true;
							}
						}

					}

					if(!HasHit)
					{
						//DrawDebugLine(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, DetonatePoint + HitDirection * ShellDescription->GunCharacteristics.AmmoExplosionRadius * 100, FColor::Blue, true);
					}
					else
					{
						//DrawDebugLine(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, BestHitResult.Location, FColor::Green, true);

						AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(BestHitResult.Actor.Get());
						if (Spacecraft)
						{
							// TODO Make common with onImpact
							Spacecraft->GetDamageSystem()->ApplyDamage(ShellDescription->GunCharacteristics.AmmoPower, ShellDescription->GunCharacteristics.AmmoDamageRadius, BestHitResult.Location);


							// Play sound
							AFlareSpacecraftPawn* ShipBase = Cast<AFlareSpacecraftPawn>(Spacecraft);
							if (ShipBase && ShipBase->IsLocallyControlled())
							{
								UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, BestHitResult.Location, 1, 1);
							}
						}

						UGameplayStatics::SpawnEmitterAttached(
							ImpactEffectTemplate,
							BestHitResult.GetComponent(),
							NAME_None,
							BestHitResult.Location,
							BestHitResult.ImpactNormal.Rotation(),
							EAttachLocation::KeepWorldPosition,
							true);
					}


				}
			}
		}

	}
	Destroy();
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
