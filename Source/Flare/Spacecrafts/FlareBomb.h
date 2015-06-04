#pragma once

#include "FlareWeapon.h"
#include "FlareBomb.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareBomb : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void PostInitializeComponents() override;

	/** Properties setup */
	void Initialize(class UFlareWeapon* Weapon, const FFlareShipComponentDescription* Description);

	/** Impact happened */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult, const FVector& ImpactVelocity);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Mesh component */
	UPROPERTY()
	UFlareSpacecraftComponent*               BombComp;

	UPROPERTY()
	const FFlareShipComponentDescription*    WeaponDescription;

	UPROPERTY()
	UFlareWeapon*                            ParentWeapon;

	/** Special effects on explosion */
	UPROPERTY()
	UParticleSystem*                         ExplosionEffectTemplate;

	/** Burn mark decal */
	UPROPERTY()
	UMaterialInterface*                      ExplosionEffectMaterial;


};
