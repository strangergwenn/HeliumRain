#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareWeapon.generated.h"

class AFlareShell;
class AFlareBomb;
struct FFlareWeaponGroup;

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

	/** Setup this weapon's effects */
	virtual void SetupFiringEffects();

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

	/** Show the special effects on firing */
	virtual void ShowFiringEffects(int GunIndex);

	virtual bool FireBomb();

	virtual void ConfigureShellFuze(AFlareShell* Shell);

	virtual void SetTarget(AActor *NewTarget);

	virtual void OnAttachmentChanged();

	virtual void FillBombs();

	virtual FText GetSlotName() const;

	inline void SetWeaponGroup(FFlareWeaponGroup* Group)
	{
		WeaponGroup = Group;
	}

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

	UPROPERTY()
	TArray<AFlareBomb*>         Bombs;

	// State
	bool                        Firing;
	float                       TimeSinceLastShell;
	int32                       CurrentAmmo;
	int                         LastFiredGun;
	FFlareWeaponGroup*          WeaponGroup;
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

	virtual UStaticMesh* GetMesh(bool PresentationMode) const;

	inline FFlareWeaponGroup* GetWeaponGroup()
	{
		return WeaponGroup;
	}

};
