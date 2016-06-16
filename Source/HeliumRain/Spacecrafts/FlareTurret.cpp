
#include "../Flare.h"
#include "FlareTurret.h"
#include "FlareSpacecraft.h"
#include "FlareShell.h"
#include "FlareSpacecraftSubComponent.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTurret::UFlareTurret(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, TurretComponent(NULL)
	, BarrelComponent(NULL)
{
	HasFlickeringLights = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareTurret::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);
	AimDirection = FVector::ZeroVector;

	// Initialize pilot
	Pilot = NewObject<UFlareTurretPilot>(this, UFlareTurretPilot::StaticClass());
	Pilot->Initialize(&(Data->Pilot), Company, this);
}

void UFlareTurret::SetupFiringEffects()
{
	if (FiringEffect == NULL && FiringEffectTemplate)
	{
		FiringEffects.Empty();

		for (int32 i = 0; i < ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount; i++)
		{
			// Create the effect
			UParticleSystemComponent* TempFiringEffect = UGameplayStatics::SpawnEmitterAttached(
				FiringEffectTemplate,
				BarrelComponent,
				NAME_None,
				GetMuzzleLocation(i),
				GetComponentRotation(),
				EAttachLocation::KeepWorldPosition,
				false);

			// Additional setup
			TempFiringEffect->DeactivateSystem();
			TempFiringEffect->SetTickGroup(ETickingGroup::TG_PostPhysics);
			FiringEffects.Add(TempFiringEffect);
		}
	}
}

void UFlareTurret::SetupComponentMesh()
{
	Super::SetupComponentMesh();

	if (TurretComponent)
	{
		TurretComponent->DestroyComponent();
		TurretComponent = NULL;
	}

	if (BarrelComponent)
	{
		BarrelComponent->DestroyComponent();
		BarrelComponent = NULL;
	}

	// Turret Mesh
	if (Spacecraft && ComponentDescription && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMesh)
	{

		TurretComponent = NewObject<UFlareSpacecraftSubComponent>(this, UFlareSpacecraftSubComponent::StaticClass());
		 if (TurretComponent)
		 {
			TurretComponent->SetParentSpacecraftComponent(this);
			TurretComponent->RegisterComponent();
			TurretComponent->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
			TurretComponent->SetStaticMesh(ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMesh);
			TurretComponent->SetMaterial(0, ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMesh->GetMaterial(0));
			TurretComponent->Initialize(NULL, PlayerCompany, Spacecraft, false);
			Spacecraft->AddOwnedComponent(TurretComponent);
		}
	}

	// Barrel Mesh
	if (Spacecraft && ComponentDescription && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMesh)
	{

		BarrelComponent = NewObject<UFlareSpacecraftSubComponent>(this, UFlareSpacecraftSubComponent::StaticClass());
		 if (BarrelComponent)
		 {
			BarrelComponent->SetParentSpacecraftComponent(this);
			BarrelComponent->RegisterComponent();
			if (TurretComponent)
			{
				BarrelComponent->AttachToComponent(TurretComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("Axis"));
			}
			else
			{
				BarrelComponent->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
			}
			BarrelComponent->SetStaticMesh(ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMesh);
			BarrelComponent->SetMaterial(0, ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMesh->GetMaterial(0));
			Spacecraft->AddOwnedComponent(BarrelComponent);
		}
	}
}



void UFlareTurret::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (Spacecraft->IsPresentationMode())
	{
		TurretComponent->SetRelativeRotation(FRotator(0, 0, 0));
		BarrelComponent->SetRelativeRotation(FRotator(15, 0, 0));

		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive() && Pilot)
	{

		Pilot->TickPilot(DeltaTime);
		//FLOGV("Pilot exist WantFire %d", Pilot->IsWantFire());
		if (Pilot->IsWantFire())
		{
			StartFire();
		}
		else
		{
			StopFire();
		}
		AimDirection = Pilot->GetTargetAimAxis();
		//FLOGV("Pilot AimDirection %s", *AimDirection.ToString());
	}

	if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive() && GetUsableRatio() > 0)
	{

		if (TurretComponent && ComponentDescription)
		{

			float TargetTurretAngle = 0;
			if (AimDirection != FVector::ZeroVector)
			{
				FVector LocalTurretAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);
				TargetTurretAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.X)));
			}

			// Clamp movements
			TargetTurretAngle = FMath::Clamp(TargetTurretAngle, ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMinAngle, ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMaxAngle);

			float UsableTurretVelocity = GetUsableRatio() * ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretAngularVelocity;

			float TurretAngleDiff = FMath::UnwindDegrees(TargetTurretAngle - ShipComponentData->Turret.TurretAngle);

			if (FMath::Abs(TurretAngleDiff) <= UsableTurretVelocity * DeltaTime)
			{
				ShipComponentData->Turret.TurretAngle = TargetTurretAngle;
			}
			else if (TurretAngleDiff < 0)
			{
				ShipComponentData->Turret.TurretAngle -= UsableTurretVelocity * DeltaTime;
			}
			else
			{
				ShipComponentData->Turret.TurretAngle += UsableTurretVelocity * DeltaTime;
			}

			TurretComponent->SetRelativeRotation(FRotator(0, ShipComponentData->Turret.TurretAngle, 0));
		}

		if (BarrelComponent)
		{

			float TargetBarrelAngle = 15;

			if (AimDirection != FVector::ZeroVector)
			{
				FVector LocalBarrelAimDirection;
				if (TurretComponent)
				{
					LocalBarrelAimDirection = TurretComponent->GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);
				}
				else
				{
					LocalBarrelAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(AimDirection);
				}

				TargetBarrelAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
			}

			// Clamp movements
			TargetBarrelAngle = FMath::Clamp(TargetBarrelAngle, GetMinLimitAtAngle(ShipComponentData->Turret.TurretAngle), ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMaxAngle);


			// TODO Add ship specific bound

			float UsableBarrelsVelocity = GetUsableRatio() * ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretAngularVelocity;
			float BarrelAngleDiff = FMath::UnwindDegrees(TargetBarrelAngle - ShipComponentData->Turret.BarrelsAngle);

			if (FMath::Abs(BarrelAngleDiff) <= UsableBarrelsVelocity * DeltaTime) {
				ShipComponentData->Turret.BarrelsAngle = TargetBarrelAngle;
			} else if (BarrelAngleDiff < 0) {
				ShipComponentData->Turret.BarrelsAngle -= UsableBarrelsVelocity * DeltaTime;
			} else {
				ShipComponentData->Turret.BarrelsAngle += UsableBarrelsVelocity * DeltaTime;
			}
			BarrelComponent->SetRelativeRotation(FRotator(ShipComponentData->Turret.BarrelsAngle, 0, 0));

		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UFlareTurret::GetFireAxis() const
{
	if (BarrelComponent)
	{
		return BarrelComponent->GetComponentRotation().RotateVector(FVector(1, 0, 0));
	}
	else if (TurretComponent)
	{
		return TurretComponent->GetComponentRotation().RotateVector(FVector(1, 0, 0));
	}
	else
	{
		return Super::GetFireAxis();
	}
}

FVector UFlareTurret::GetIdleAxis() const
{
	// Ship front
	return Spacecraft->Airframe->GetComponentRotation().RotateVector(FVector(1, 0, 0));
}


FVector UFlareTurret::GetMuzzleLocation(int GunIndex) const
{
	const UStaticMeshComponent* GunComponent = this;
	if (BarrelComponent)
	{
		GunComponent = BarrelComponent;
	}
	else if (TurretComponent)
	{
		GunComponent = TurretComponent;
	}

	if (ComponentDescription->WeaponCharacteristics.GunCharacteristics.GunCount <= 1)
	{
		return GunComponent->GetSocketLocation(FName("Muzzle"));
	}
	else
	{
		return GunComponent->GetSocketLocation(FName(*(FString("Muzzle") + FString::FromInt(GunIndex))));
	}
}

FVector UFlareTurret::GetTurretBaseLocation() const
{
	if (BarrelComponent)
	{
		return BarrelComponent->GetComponentLocation();
	}
	else if (TurretComponent)
	{
		return TurretComponent->GetComponentLocation();
	}
	return GetComponentLocation();
}

bool UFlareTurret::IsSafeToFire(int GunIndex) const
{
	FVector FiringLocation = GetMuzzleLocation(GunIndex);
	FVector FiringDirection = GetFireAxis();
	FVector TargetLocation = FiringLocation + FiringDirection * 100000;

	FHitResult HitResult(ForceInit);
	if (Trace(FiringLocation, TargetLocation, HitResult))
	{
		if (HitResult.Actor.IsValid() && HitResult.Actor == Spacecraft)
		{
			//FLOG("!!!!!!!!!Not safe to fire !");
			return false;
		}
	}
	return true;
}

bool UFlareTurret::Trace(const FVector& Start, const FVector& End, FHitResult& HitOut) const
{
	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, NULL);
	TraceParams.bTraceComplex = true;
	// TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

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

bool UFlareTurret::IsReacheableAxis(FVector TargetAxis) const
{
	float TargetTurretAngle = 0;
	if (TurretComponent && ComponentDescription)
	{

		FVector LocalTurretAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		TargetTurretAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalTurretAimDirection.Y, LocalTurretAimDirection.X)));

		if (TargetTurretAngle > ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMaxAngle
				|| TargetTurretAngle < ComponentDescription->WeaponCharacteristics.TurretCharacteristics.TurretMinAngle)
		{
			return false;
		}
	}

	if (BarrelComponent && ComponentDescription)
	{

		FVector LocalBarrelAimDirection;
		if (TurretComponent)
		{
			LocalBarrelAimDirection = TurretComponent->GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		}
		else
		{
			LocalBarrelAimDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(TargetAxis);
		}

		float TargetBarrelAngle = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalBarrelAimDirection.Z, LocalBarrelAimDirection.X)));
		if (TargetBarrelAngle > ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMaxAngle
				|| TargetBarrelAngle < GetMinLimitAtAngle(TargetTurretAngle))
		{
			return false;
		}




	}
	return true;
}

static inline int PositiveModulo(int i, int n)
{
	return (i % n + n) % n;
}

float UFlareTurret::GetMinLimitAtAngle(float Angle) const
{
	float BarrelsMinAngle = ComponentDescription->WeaponCharacteristics.TurretCharacteristics.BarrelsMinAngle;
	FFlareSpacecraftDescription* Desc = Spacecraft->GetParent()->GetDescription();

	// Fine Local slot check
	for (int32 i = 0; i < Desc->TurretSlots.Num(); i++)
	{
		// TODO optimize and store that in cache
		if (Desc->TurretSlots[i].SlotIdentifier == ShipComponentData->ShipSlotIdentifier)
		{
			int LimitStepCount = Desc->TurretSlots[i].TurretBarrelsAngleLimit.Num();


			if (LimitStepCount > 0)
			{
				float StepAngle = 360.f / (float) LimitStepCount;


				float AngleInStep = Angle / StepAngle;
				int NearestStep = FMath::FloorToInt(AngleInStep + 0.5f);
				int SecondNearestStep;
				if (AngleInStep > NearestStep)
				{
					SecondNearestStep = NearestStep+1;
				}
				else
				{
					SecondNearestStep = NearestStep-1;
				}

				float Ratio = FMath::Abs(Angle - NearestStep * StepAngle) /  StepAngle;

				float LocalMin = Desc->TurretSlots[i].TurretBarrelsAngleLimit[PositiveModulo(NearestStep, LimitStepCount)] * (1.f - Ratio)
									+ Desc->TurretSlots[i].TurretBarrelsAngleLimit[PositiveModulo(SecondNearestStep,LimitStepCount)] * Ratio;

				BarrelsMinAngle = FMath::Max(BarrelsMinAngle, LocalMin);
			}
		}

	}
	return BarrelsMinAngle;
}

void UFlareTurret::GetBoundingSphere(FVector& Location, float& SphereRadius)
{
	Super::GetBoundingSphere(Location, SphereRadius);
	if (TurretComponent || BarrelComponent)
	{
		SphereRadius = 0;
	}
}

void UFlareTurret::ShowFiringEffects(int GunIndex)
{
	if (FiringEffects[GunIndex])
	{
		FiringEffects[GunIndex]->ActivateSystem();
	}
}
