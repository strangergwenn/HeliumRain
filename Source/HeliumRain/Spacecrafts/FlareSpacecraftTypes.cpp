#include "../Flare.h"
#include "FlareSpacecraftTypes.h"

DECLARE_CYCLE_STAT(TEXT("SpacecraftHelper GetIntersectionPosition"), STAT_SpacecraftHelper_GetIntersectionPosition, STATGROUP_Flare);

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftTypes::UFlareSpacecraftTypes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float SpacecraftHelper::GetIntersectionPosition(FVector TargetLocation, FVector TargetVelocity, FVector SourceLocation, FVector SourceVelocity, float ProjectileSpeed, float PredictionDelay, FVector* ResultPosition)
{

	SCOPE_CYCLE_COUNTER(STAT_SpacecraftHelper_GetIntersectionPosition);


	/*FLOGV("GetIntersectionPosition at %f s",PredictionDelay);
	FLOGV("  TargetLocation=%s",*TargetLocation.ToString());
	FLOGV("  TargetVelocity=%s",*TargetVelocity.ToString());
	FLOGV("  SourceLocation=%s",*SourceLocation.ToString());
	FLOGV("  SourceVelocity=%s",*SourceVelocity.ToString());
	FLOGV("  ProjectileSpeed=%f",ProjectileSpeed);*/

	// Relative Target Speed
	FVector PredictedTargetLocation = TargetLocation + TargetVelocity * PredictionDelay;
	FVector BulletLocation = SourceLocation + SourceVelocity * PredictionDelay;

	// Find the relative speed in the axis of target
	FVector TargetDirection = (PredictedTargetLocation - BulletLocation).GetUnsafeNormal();
	FVector BonusVelocity = SourceVelocity;
	float BonusVelocityInTargetAxis = FVector::DotProduct(TargetDirection, BonusVelocity);
	float EffectiveProjectileSpeed = ProjectileSpeed + BonusVelocityInTargetAxis;

	float Divisor = FMath::Square(EffectiveProjectileSpeed) - TargetVelocity.SizeSquared();


	/*FLOGV("  PredictedTargetLocation=%s",*PredictedTargetLocation.ToString());
	FLOGV("  BulletLocation=%s",*BulletLocation.ToString());
	FLOGV("  TargetDirection=%s",*TargetDirection.ToString());
	FLOGV("  BonusVelocity=%s",*BonusVelocity.ToString());

	FLOGV("  BonusVelocityInTargetAxis=%f",BonusVelocityInTargetAxis);
	FLOGV("  EffectiveProjectileSpeed=%f",EffectiveProjectileSpeed);
	FLOGV("  Divisor=%f",Divisor);*/

	if (EffectiveProjectileSpeed < 0 || FMath::IsNearlyZero(Divisor))
	{
		// Intersect at an infinite time
		return -1;
	}

	float A = -1;
	float B = 2 * (TargetVelocity.X * (PredictedTargetLocation.X - BulletLocation.X) + TargetVelocity.Y * (PredictedTargetLocation.Y - BulletLocation.Y) + TargetVelocity.Z * (PredictedTargetLocation.Z - BulletLocation.Z)) / Divisor;
	float C = (PredictedTargetLocation - BulletLocation).SizeSquared() / Divisor;

	float Delta = FMath::Square(B) - 4 * A * C;

	float InterceptTime1 = (- B - FMath::Sqrt(Delta)) / (2 * A);
	float InterceptTime2 = (- B + FMath::Sqrt(Delta)) / (2 * A);

	float InterceptTime;
	if (InterceptTime1 > 0 && InterceptTime2 > 0)
	{
		InterceptTime = FMath::Min(InterceptTime1, InterceptTime2);
	}
	else
	{
		InterceptTime = FMath::Max(InterceptTime1, InterceptTime2);
	}

	if (InterceptTime > 0)
	{
		FVector InterceptLocation = PredictedTargetLocation + TargetVelocity * InterceptTime;
		*ResultPosition = InterceptLocation;
	}
	return InterceptTime;
}

EFlareDamage::Type SpacecraftHelper::GetWeaponDamageType(EFlareShellDamageType::Type ShellDamageType)
{
	switch(ShellDamageType) {
	case EFlareShellDamageType::HighExplosive:
		return EFlareDamage::DAM_HighExplosive;
	case EFlareShellDamageType::ArmorPiercing:
	case EFlareShellDamageType::LightSalvage:
	case EFlareShellDamageType::HeavySalvage:
		return EFlareDamage::DAM_ArmorPiercing;
	case EFlareShellDamageType::HEAT:
		return EFlareDamage::DAM_HEAT;
	default:
		return EFlareDamage::DAM_None;
	};
}

int32 FFlareSpacecraftDescription::GetCapacity() const
{
	return CargoBayCapacity * CargoBayCount;
}


bool FFlareSpacecraftDescription::IsStation() const
{
	return OrbitalEngineCount == 0;
}

bool FFlareSpacecraftDescription::IsMilitary() const
{
	return GunSlots.Num() > 0 || TurretSlots.Num() > 0;
}

bool FFlareSpacecraftDescription::IsResearch() const
{
	for (int32 FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		FFlareFactoryDescription* FactoryDescription = &Factories[FactoryIndex]->Data;

		if(FactoryDescription->IsResearch())
		{
			return true;
		}
	}
	return false;
}
