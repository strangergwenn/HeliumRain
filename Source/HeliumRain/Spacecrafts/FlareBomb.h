#pragma once

#include "FlareWeapon.h"
#include "../Flare.h"
#include "FlareBomb.generated.h"

class UFlareBombComponent;

/** Bomb save data */
USTRUCT()
struct FFlareBombSave
{
	GENERATED_USTRUCT_BODY()

	/** Bomb identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

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

	/** Spacecraft onto attache the bomb (for harpoon) */
	UPROPERTY(EditAnywhere, Category = Save)
	FName AttachTarget;

	/** Activated */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Activated;

	/** Activated */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Dropped;

	/** Activated */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Locked;

	/** Distance to parent on drop */
	UPROPERTY(EditAnywhere, Category = Save)
	float DropParentDistance;

	/** Distance to parent on drop */
	UPROPERTY(EditAnywhere, Category = Save)
	float LifeTime;

	/** Guided bomb burn duration */
	UPROPERTY(EditAnywhere, Category = Save)
	float BurnDuration;

	/** Guided bomb target spacecraft */
	UPROPERTY(EditAnywhere, Category = Save)
	FName AimTargetSpacecraft;


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
	virtual void OnLaunched(AFlareSpacecraft* Target);

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
		FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Spacecraft hit */
	void OnSpacecraftHit(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal);

	/** Bomb detonated */
	void OnBombDetonated(AFlareSpacecraft* HitSpacecraft, UFlareSpacecraftComponent* HitComponent, FVector HitLocation, FVector InertialNormal);

	/** Attach bomb */
	void AttachBomb(AFlareSpacecraft* HitSpacecraft);

	/** Save the bomb to a save file */
	virtual FFlareBombSave* Save();

	/** Game pause */
	virtual void SetPause(bool Paused);

	/** Get the distance from the parent ship */
	virtual float GetParentDistance() const;

	FVector GetAngularVelocityToAlignAxis(FVector TargetAxis, float AngularAcceleration, float DeltaSeconds) const;

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

	float                                    ExplosionEffectScale;
	
	const FFlareSpacecraftComponentDescription*    WeaponDescription;

	FFlareBombSave                          BombData;

	bool                                    Paused;

	FRotator								LastTickRotation;

	AFlareSpacecraft*  TargetSpacecraft;

	FVector LastLocation;
	FVector LastTargetLocation;
	float BombLockedInCollision;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareSpacecraft* GetFiringSpacecraft() const
	{
		FCHECK(ParentWeapon);

		AFlareSpacecraft* Spacecraft = ParentWeapon->GetSpacecraft();
		FCHECK(Spacecraft);

		return Spacecraft;
	}

	FName GetIdentifier() const
	{
		return BombData.Identifier;
	}

	AFlareSpacecraft* GetTargetSpacecraft()
	{
		return TargetSpacecraft;
	}

	inline UFlareWeapon* GetFiringWeapon() const
	{
		FCHECK(ParentWeapon);

		return ParentWeapon;
	}

	inline bool IsHarpooned() const
	{
		return (BombData.AttachTarget != NAME_None);
	}

	inline bool IsDropped() const
	{
		return BombData.Dropped;
	}

	inline bool IsPaused()
	{
		return Paused;
	}

	bool IsActive() const;
};
