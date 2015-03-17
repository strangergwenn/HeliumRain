#pragma once

#include "FlareWeapon.h"
#include "FlareProjectile.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareProjectile : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void PostInitializeComponents() override;

	/** Properties setup */
	void Initialize(class UFlareWeapon* Weapon, const FFlareShipModuleDescription* Description, FVector ShootDirection, FVector ParentVelocity);

	/** Impact happened */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Mesh component */
	UPROPERTY()
	UStaticMeshComponent*                    ShellComp;

	/** Movement component */
	UPROPERTY()
	UProjectileMovementComponent*            MovementComp;

	/** Special effects on explosion */
	UPROPERTY()
	UParticleSystem*                         ExplosionEffectTemplate;

	/** Burn mark decal */
	UPROPERTY()
	UMaterialInterface*                      ExplosionEffectMaterial;

	// Shell data
	FVector                                  ShellDirection;
	const FFlareShipModuleDescription*       ShellDescription;

};
