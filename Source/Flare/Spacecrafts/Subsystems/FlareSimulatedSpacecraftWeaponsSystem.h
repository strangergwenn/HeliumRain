#pragma once

#include "FlareSpacecraftWeaponsSystemInterface.h"
#include "FlareSimulatedSpacecraftWeaponsSystem.generated.h"


/** Spacecraft weapons system class */
UCLASS()
class FLARE_API UFlareSimulatedSpacecraftWeaponsSystem : public UObject, public IFlareSpacecraftWeaponsSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/


	/** Initialize this system */
	virtual void Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	virtual int32 GetWeaponGroupCount() const;

	virtual EFlareWeaponGroupType::Type GetActiveWeaponType() const;

protected:


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSpacecraft*                                Spacecraft;
	FFlareSpacecraftSave*                            Data;
	FFlareSpacecraftDescription*                     Description;
};
