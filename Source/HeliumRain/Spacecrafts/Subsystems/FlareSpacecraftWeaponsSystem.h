#pragma once

#include "../../Flare.h"
#include "FlareSpacecraftWeaponsSystemInterface.h"
#include "FlareSpacecraftWeaponsSystem.generated.h"

class AFlareSpacecraft;
class UFlareWeapon;

struct FFlareSpacecraftComponentDescription;
struct FFlareSpacecraftDescription;
struct FFlareSpacecraftSave;

/** Structure holding all data for a weapon group */
struct FFlareWeaponGroup
{
	FFlareSpacecraftComponentDescription*           Description;
	TEnumAsByte <EFlareWeaponGroupType::Type>       Type;

	TArray <UFlareWeapon*>                          Weapons;
	int32                                           LastFiredWeaponIndex;
	AFlareSpacecraft*								Target;
};


/** Spacecraft weapons system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftWeaponsSystem : public UObject, public IFlareSpacecraftWeaponsSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	~UFlareSpacecraftWeaponsSystem();

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/
	
	virtual void ActivateWeapons(bool Activate);

	virtual void ActivateWeaponGroup(int32 Index);

	virtual void ToogleWeaponActivation();

	virtual void ActivateWeapons();
	virtual void DeactivateWeapons();

	/** Are we using the fire director mode where the player fires turret on capital ships ? */
	bool IsInFireDirector();

	virtual int32 GetGroupByWeaponIdentifer(FName Identifier) const;

	virtual bool HasUsableWeaponType(EFlareWeaponGroupType::Type Type) const;

	EFlareWeaponGroupType::Type GetActiveWeaponType() const;

	inline FFlareSpacecraftComponentDescription* GetWeaponDescription(int32 Index) const
	{
		return WeaponDescriptionList[Index];
	}

	inline int32 GetActiveWeaponGroupIndex() const
	{
		return ActiveWeaponGroupIndex;
	}

	inline FFlareWeaponGroup* GetActiveWeaponGroup() const
	{
		return ActiveWeaponGroup;
	}

	inline TArray<UFlareWeapon*>& GetWeaponList()
	{
		return WeaponList;
	}

	inline FFlareWeaponGroup* GetWeaponGroup(int32 Index)
	{
		return WeaponGroupList[Index];
	}

	inline TArray<FFlareWeaponGroup*>& GetWeaponGroupList()
	{
		return WeaponGroupList;
	}

	virtual int32 GetWeaponGroupCount() const override
	{
		return WeaponGroupList.Num();
	}

	virtual void SetActiveWeaponTarget(AFlareSpacecraft* Target);

	virtual AFlareSpacecraft* GetActiveWeaponTarget();
	
	virtual void StopAllWeapons();

	void GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableSmallMilitary, float* IsUncontrollableLargeMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned, FFlareWeaponGroup* RestrictGroup = NULL);

	int32 FindBestWeaponGroup(AFlareSpacecraft* Target);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Weapon components and descriptions
	TArray <UFlareWeapon*>                           WeaponList;
	TArray <FFlareSpacecraftComponentDescription*>   WeaponDescriptionList;
	TArray <FFlareWeaponGroup*>                      WeaponGroupList;

	// TODO save
	int32                                            LastActiveWeaponGroupIndex;
	int32                                            ActiveWeaponGroupIndex;
	FFlareWeaponGroup*                               ActiveWeaponGroup;
	bool                                             Armed;
	
	UPROPERTY()
	AFlareSpacecraft*                                Spacecraft;

	FFlareSpacecraftSave*                            Data;
	FFlareSpacecraftDescription*                     Description;
	TArray<UActorComponent*>                         Components;

};
