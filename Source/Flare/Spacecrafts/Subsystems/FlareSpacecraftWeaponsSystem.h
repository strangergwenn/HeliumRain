#pragma once

#include "FlareSpacecraftWeaponsSystem.generated.h"

class AFlareSpacecraft;

/** Status of the ship */
UENUM()
namespace EFlareWeaponGroupType
{
	enum Type
	{
		WG_NONE,
		WG_GUN,
		WG_BOMB,
		WG_TURRET
	};
}

/** Structure holding all data for a weapon group */
struct FFlareWeaponGroup
{
	FFlareSpacecraftComponentDescription*           Description;
	TEnumAsByte <EFlareWeaponGroupType::Type>       Type;

	TArray <UFlareWeapon*>                          Weapons;
	int32                                           LastFiredWeaponIndex;
};


/** Spacecraft weapons system class */
UCLASS()
class FLARE_API UFlareSpacecraftWeaponsSystem : public UObject
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

	virtual void StartFire();

	virtual void StopFire();

	virtual void ActivateWeapons(bool Activate);

	virtual void ActivateWeaponGroup(int32 Index);

	virtual void ToogleWeaponActivation();

	virtual void ActivateWeapons();
	virtual void DesactivateWeapons();

	virtual int32 GetGroupByWeaponIdentifer(FName Identifier) const;

	EFlareWeaponGroupType::Type getActiveWeaponType();

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



	UPROPERTY()
	AFlareSpacecraft*                                Spacecraft;
	FFlareSpacecraftSave*                            Data;
	FFlareSpacecraftDescription*                     Description;
	TArray<UActorComponent*>                         Components;
};
