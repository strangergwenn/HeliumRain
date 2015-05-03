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

	virtual void PostInitializeComponents() override;

	/** Properties setup */
	void Initialize(class UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity);

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
	USoundCue*                               ImpactSound; // TODO M3 : Move to characteristic (weapon description)

	/** Damage sound */
	UPROPERTY()
	USoundCue*                               DamageSound; // TODO M3 : Move to characteristic (weapon description)

	/** Mesh component */
	UPROPERTY()
	USceneComponent*                    ShellComp;

	/** Special effects on explosion */
	UPROPERTY()
	UParticleSystem*                         ExplosionEffectTemplate; // TODO M3 : Move to characteristic (weapon description)

	/** Special effects on flight */
	UPROPERTY()
	UParticleSystem*                         FlightEffectsTemplate; // TODO M3 : Move to characteristic (weapon description)

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

};
