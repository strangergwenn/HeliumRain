#pragma once

#include "FlareShipComponent.h"
#include "FlareWeapon.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareWeapon : public UFlareShipComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu) override;

	virtual FFlareShipComponentSave* Save() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void SetupEffectMesh() override;

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

	// Weapon properties
	float                       FiringRate;
	float                       FiringPeriod;
	float                       AmmoVelocity;
	int32                       MaxAmmo;
	UStaticMesh*                ShellMesh;
	FActorSpawnParameters       ProjectileSpawnParams;

	// State
	bool                        Firing;
	float                       TimeSinceLastShell;
	int32                       CurrentAmmo;


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
