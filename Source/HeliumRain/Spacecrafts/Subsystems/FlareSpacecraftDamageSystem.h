#pragma once
#include "FlareSpacecraftDamageSystem.generated.h"

class AFlareSpacecraft;
class UFlareSimulatedSpacecraftDamageSystem;
struct FFlareSpacecraftSave;
struct FFlareSpacecraftDescription;


/** Spacecraft damage system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftDamageSystem : public UObject
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

	/** Set who's doing the damage */
	virtual void SetLastDamageCauser(AFlareSpacecraft* Ship);


public:

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/


	virtual float GetWeaponGroupHealth(int32 GroupIndex, bool WithArmor = false, bool WithAmmo = true) const;

	virtual bool IsPowered() const;

	virtual float GetOverheatRatio(float HalfRatio) const;

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/

	/** Update power status for all components */
	virtual void UpdatePower();

	/** Method call if a electric component had been damaged */
	virtual void OnElectricDamage(float DamageRatio);

	virtual void OnCollision(class AActor* Other, FVector HitLocation, FVector NormalImpulse);

	virtual void ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType, UFlareCompany* DamageSource);



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
	UFlareSimulatedSpacecraftDamageSystem*          Parent;
	TArray<UActorComponent*>                        Components;

	bool                                            WasAlive; // True if was alive at the last tick
	float											TimeSinceLastExternalDamage;

	AFlareSpacecraft*                               LastDamageCauser;


public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	virtual float GetTimeSinceLastExternalDamage() const
	{
		return TimeSinceLastExternalDamage;
	}
};
