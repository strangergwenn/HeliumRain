#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareWeapon.generated.h"

class AFlareShell;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareWeapon : public UFlareSpacecraftComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu) override;

	virtual FFlareSpacecraftComponentSave* Save() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void SetupComponentMesh() override;

	/** Start firing */
	virtual void StartFire();

	/** Stop firing */
	virtual void StopFire();

	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const override;

	/** Apply damage to this component only it is used. */
	virtual void ApplyHeatDamage(float OverheatEnergy, float BurnEnergy) override;

	/** Reset the current ammo to max ammo.	*/
	virtual void RefillAmmo();

	virtual FVector GetFireAxis() const;

	virtual FVector GetMuzzleLocation(int GunIndex) const;

	virtual int GetGunCount() const;

	virtual bool IsTurret() const;

	virtual bool IsSafeToFire(int GunIndex) const;

	/** Return the aim need minimum radius. 0 if not proximity fuze */
	virtual float GetAimRadius() const;

	virtual bool FireGun(int GunIndex);

	virtual void ConfigureShellFuze(AFlareShell* Shell);

	virtual void SetTarget(AActor *NewTarget);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Firing sound */
	UPROPERTY()
	USoundCue*                  FiringSound;

	/** Special effects on firing (template) */
	UPROPERTY()
	UParticleSystem*            FiringEffectTemplate;

	/** Special effects on firing (component) */
	UPROPERTY()
	UParticleSystemComponent*   FiringEffect;

	UPROPERTY()
	AActor *                    Target;
	// Weapon properties
	float                       FiringRate;
	float                       FiringPeriod;
	float                       AmmoVelocity;
	int32                       MaxAmmo;
	FActorSpawnParameters       ProjectileSpawnParams;

	// State
	bool                        Firing;
	float                       TimeSinceLastShell;
	int32                       CurrentAmmo;
	int                         LastFiredGun;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline int32 GetCurrentAmmo() const
	{
		return CurrentAmmo;
	}

	inline int32 GetMaxAmmo() const
	{
		return MaxAmmo;
	}

	inline float GetAmmoVelocity() const
	{
		return AmmoVelocity;
	}

	inline bool isFiring() const
	{
		return Firing && CurrentAmmo > 0;
	}

};
