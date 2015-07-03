#pragma once

#include "FlareWeapon.h"
#include "FlareShell.generated.h"

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareShell : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Properties setup */
	void Initialize(class UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer);

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetPause(bool Pause);

	/** Impact happened */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult, const FVector& ImpactVelocity);

	virtual void DetonateAt(FVector DetonatePoint);

	bool Trace(const FVector& Start, const FVector& End, FHitResult& HitOut);

	virtual float ApplyDamage(AActor *ActorToDamage, UPrimitiveComponent* ImpactComponent, FVector ImpactLocation,  FVector ImpactAxis,  FVector ImpactNormal, float ImpactPower, float ImpactRadius, EFlareDamage::Type DamageType);

	virtual void SetFuzeTimer(float TargetSecureTime, float TargetActiveTime);

	virtual void CheckFuze(FVector ActorLocation, FVector NextActorLocation);
protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Impact sound */
	UPROPERTY()
	USoundCue*                               ImpactSound;

	/** Damage sound */
	UPROPERTY()
	USoundCue*                               DamageSound;

	/** Mesh component */
	UPROPERTY()
	USceneComponent*                    ShellComp;

	/** Special effects on explosion */
	UPROPERTY()
	UParticleSystem*                         ExplosionEffectTemplate;

	/** Special effects on impact */
	UPROPERTY()
	UParticleSystem*                         ImpactEffectTemplate;

	/** Special effects on flight */
	UPROPERTY()
	UParticleSystem*                         FlightEffectsTemplate;

	// Flight effects
	UParticleSystemComponent*                FlightEffects;

	/** Burn mark decal */
	UPROPERTY()
	UMaterialInterface*                      ExplosionEffectMaterial;

	// Shell data
	FVector                                  ShellVelocity;
	const FFlareSpacecraftComponentDescription*    ShellDescription;
	FVector                                  LastLocation;	
	float ShellMass;
	bool TracerShell;
	bool Armed;
	float MinEffectiveDistance;
	float SecureTime;
	float ActiveTime;

	UFlareWeapon* ParentWeapon;

};
