#pragma once

#include "../FlareSpacecraftTypes.h"
#include "../FlareSimulatedSpacecraft.h"
#include "HeliumRain/Spacecrafts/Subsystems/FlareSpacecraftWeaponsSystemInterface.h"
#include "FlareSimulatedSpacecraftWeaponsSystem.generated.h"

/** Structure holding all data for a weapon group */
struct FFlareSimulatedWeaponGroup
{
	struct FFlareSpacecraftComponentDescription*    Description;
	TEnumAsByte <EFlareWeaponGroupType::Type>       Type;

	TArray <struct FFlareSpacecraftComponentSave*>  Weapons;
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

	void GetTargetPreference(float* IsSmall, float* IsLarge, float* IsUncontrollableCivil, float* IsUncontrollableSmallMilitary, float* IsUncontrollableLargeMilitary, float* IsNotUncontrollable, float* IsStation, float* IsHarpooned);

	int32 FindBestWeaponGroup(UFlareSimulatedSpacecraft* Target);


	/*----------------------------------------------------
		Helper
	----------------------------------------------------*/

	static FName GetSlotIdentifierFromWeaponGroupIndex(const FFlareSpacecraftDescription* ShipDesc, int32 WeaponGroupIndex);

	static  int32 GetGroupIndexFromSlotIdentifier(const FFlareSpacecraftDescription* ShipDesc, FName SlotName);

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

	bool HasAntiLargeShipWeapon();

	bool HasAntiSmallShipWeapon();

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
