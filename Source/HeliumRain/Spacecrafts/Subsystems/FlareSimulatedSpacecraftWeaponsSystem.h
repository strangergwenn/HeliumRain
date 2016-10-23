#pragma once

#include "FlareSpacecraftWeaponsSystemInterface.h"
#include "FlareSimulatedSpacecraftWeaponsSystem.generated.h"

/** Structure holding all data for a weapon group */
struct FFlareSimulatedWeaponGroup
{
	FFlareSpacecraftComponentDescription*           Description;
	TEnumAsByte <EFlareWeaponGroupType::Type>       Type;

	TArray <FFlareSpacecraftComponentSave*>         Weapons;
};

/** Spacecraft weapons system class */
UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraftWeaponsSystem : public UObject, public IFlareSpacecraftWeaponsSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:
	~UFlareSimulatedSpacecraftWeaponsSystem();

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/


	/** Initialize this system */
	virtual void Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	void GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned);

	int32 FindBestWeaponGroup(UFlareSimulatedSpacecraft* Target);
public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	virtual int32 GetWeaponGroupCount() const;

	virtual EFlareWeaponGroupType::Type GetActiveWeaponType() const;

	inline FFlareSimulatedWeaponGroup* GetWeaponGroup(int32 Index)
	{
		return WeaponGroupList[Index];
	}

	virtual int32 GetGroupByWeaponIdentifer(FName Identifier) const;

protected:


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSpacecraft*                       Spacecraft;
	FFlareSpacecraftSave*                            Data;
	FFlareSpacecraftDescription*                     Description;
	TArray <FFlareSpacecraftComponentSave*>          WeaponList;
	TArray <FFlareSpacecraftComponentDescription*>   WeaponDescriptionList;
	TArray <FFlareSimulatedWeaponGroup*>                      WeaponGroupList;
};
