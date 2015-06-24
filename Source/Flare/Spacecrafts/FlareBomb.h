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
	FString ParentSpacecraft;

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

	/** Save the bomb to a save file */
	virtual FFlareBombSave* Save();

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Drop();

	virtual void Tick(float DeltaSeconds) override;

	virtual float GetParentDistance() const;

	virtual void SetPause(bool Paused);

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

	inline bool IsDropped() const
	{
		return BombData.Dropped;
	}

	inline bool IsPaused()
	{
		return Paused;
	}
};
