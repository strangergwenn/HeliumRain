#pragma once

#include "FlareWeapon.h"
#include "FlareBomb.generated.h"

class UFlareBombComponent;

/** Bomb save data */
USTRUCT()
struct FFlareBombSave
{
	GENERATED_USTRUCT_BODY()

	/** Bomb location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Bomb rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Bomb linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Bomb angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Parent weapon slot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName WeaponSlotIdentifier;

	/** Parent weapon spacecraft */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ParentSpacecraft;

	/** Activated */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Activated;

	/** Activated */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Dropped;

	/** Distance to parent on drop */
	UPROPERTY(EditAnywhere, Category = Save)
	float DropParentDistance;

	/** Distance to parent on drop */
	UPROPERTY(EditAnywhere, Category = Save)
	float LifeTime;
};

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
	void Initialize(const FFlareBombSave* Data, UFlareWeapon* Weapon);

	/** Launch the weapon */
	virtual void OnLaunched();

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Spacecraft hit */
	void OnSpacecraftHit(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal);

	/** Bomb detonated */
	void OnBombDetonated(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal);

	/** Save the bomb to a save file */
	virtual FFlareBombSave* Save();

	/** Game pause */
	virtual void SetPause(bool Paused);

	/** Get the distance from the parent ship */
	virtual float GetParentDistance() const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Mesh component */
	UPROPERTY()
	UFlareBombComponent*               BombComp;

	/** Damage sound */
	UPROPERTY()
	USoundCue*                               DamageSound;

	UPROPERTY()
	UFlareWeapon*                            ParentWeapon;

	/** Special effects on explosion */
	UPROPERTY()
	UParticleSystem*                         ExplosionEffectTemplate;

	/** Burn mark decal */
	UPROPERTY()
	UMaterialInterface*                      ExplosionEffectMaterial;


	const FFlareSpacecraftComponentDescription*    WeaponDescription;

	FFlareBombSave                          BombData;

	bool                                    Paused;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareSpacecraft* GetFiringSpacecraft() const
	{
		check(ParentWeapon);

		AFlareSpacecraft* Spacecraft = ParentWeapon->GetSpacecraft();
		check(Spacecraft);

		return Spacecraft;
	}

	inline bool IsDropped() const
	{
		return BombData.Dropped;
	}

	inline bool IsPaused()
	{
		return Paused;
	}
};
