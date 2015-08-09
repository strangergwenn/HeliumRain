#pragma once
#include "FlareSpacecraftDamageSystemInterface.h"
#include "FlareSpacecraftDamageSystem.generated.h"

class AFlareSpacecraft;
struct FFlareSpacecraftSave;
struct FFlareSpacecraftDescription;


/** Spacecraft damage system class */
UCLASS()
class FLARE_API UFlareSpacecraftDamageSystem : public UObject, public IFlareSpacecraftDamageSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

public:

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/

	virtual float GetTemperature() const;

	virtual float GetOverheatTemperature() const { return 1200; }

	virtual float GetBurnTemperature() const { return 1500; }

	virtual float GetSubsystemHealth(EFlareSubsystem::Type Type, bool WithArmor = false, bool WithAmmo = false) const;

	virtual float GetWeaponGroupHealth(int32 GroupIndex, bool WithArmor = false, bool WithAmmo = true) const;


	virtual bool IsAlive() const;

	virtual bool IsPowered() const;

	virtual bool HasPowerOutage() const;

	virtual float GetPowerOutageDuration() const;


/*----------------------------------------------------
	System Interface
----------------------------------------------------*/

	/** Update power status for all components */
	virtual void UpdatePower();

	/** Method call if a electric component had been damaged */
	virtual void OnElectricDamage(float DamageRatio);

	virtual void OnCollision(class AActor* Other, FVector HitLocation, FVector NormalImpulse);

	virtual void ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType);



protected:

	/** Our ship was destroyed */
	virtual void OnControlLost();



	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	TArray<UActorComponent*>                        Components;
	bool                                            WasAlive; // True if was alive at the last tick
	float											TimeSinceLastExternalDamage;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	virtual float GetTimeSinceLastExternalDamage() const
	{
		return TimeSinceLastExternalDamage;
	}
};
