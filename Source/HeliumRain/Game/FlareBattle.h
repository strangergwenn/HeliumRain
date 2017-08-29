#pragma once

#include "Object.h"
#include "FlareSimulatedSector.h"
#include "FlareBattle.generated.h"

class UFlareSpacecraftComponentsCatalog;


UCLASS()
class HELIUMRAIN_API UFlareBattle : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the battle state */
	virtual void Load(UFlareSimulatedSector* BattleSector);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void Simulate();

	bool SimulateTurn();

	bool SimulateShipTurn(UFlareSimulatedSpacecraft* Ship);

	bool SimulateSmallShipTurn(UFlareSimulatedSpacecraft* Ship);

	bool SimulateLargeShipTurn(UFlareSimulatedSpacecraft* Ship);

	UFlareSimulatedSpacecraft* GetBestTarget(UFlareSimulatedSpacecraft* Ship, struct BattleTargetPreferences Preferences);

	bool SimulateShipAttack(UFlareSimulatedSpacecraft* Ship, int32 WeaponGroupIndex, UFlareSimulatedSpacecraft* Target);

	bool SimulateShipWeaponAttack(UFlareSimulatedSpacecraft* Ship, FFlareSpacecraftComponentDescription* WeaponDescription, FFlareSpacecraftComponentSave* Weapon, UFlareSimulatedSpacecraft* Target);

	void SimulateBulletDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareSimulatedSpacecraft* DamageSource);

	void SimulateBombDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareSimulatedSpacecraft* DamageSource);

	void ApplyDamage(UFlareSimulatedSpacecraft* Target, float Energy, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource);

	int32 GetBestTargetComponent(UFlareSimulatedSpacecraft* TargetSpacecraft);

protected:

	UFlareSimulatedSector*                  Sector;
	AFlareGame*                             Game;
	UFlareCompany*                          PlayerCompany;
	UFlareSpacecraftComponentsCatalog*      Catalog;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

        bool HasBattle();
};
