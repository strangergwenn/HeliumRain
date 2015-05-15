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
	void Initialize(class UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer);

	virtual void Tick(float DeltaSeconds) override;

	/** Impact happened */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult, const FVector& ImpactVelocity);

	bool Trace(const FVector& Start, const FVector& End, FHitResult& HitOut);

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
	const FFlareShipComponentDescription*    ShellDescription;
	FVector                                  LastLocation;
	int32 ShellPower;
	float ShellMass;
	bool TracerShell;

};
