#pragma once
#include "../../Flare.h"

struct FFlareSpacecraftDescription;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
class AFlareBomb;
class AFlareSpacecraft;
class UFlareSpacecraftComponent;

class GameLog
{
public:
	/**
	 * The game has been loaded
	 *
	 * Event: GAME_LOADED
	 * Params: None
	 */
	static void GameLoaded();

	/**
	 * The game has been unloaded
	 *
	 * Event: GAME_UNLOADED
	 * Params: None
	 */
	static void GameUnloaded();

	/**
	 * A new day has been simulated
	 *
	 * Event: DAY_SIMULATED
	 * Params:
	 *   - int : new date
	 */
	static void DaySimulated(int64 NewDate);

	/**
	 * A AI started a new construction
	 *
	 * Event: AI_CONSTRUCTION_STARTED
	 * Params:
	 *   - string : company
	 *   - string : sector
	 *   - string : station description
	 *   - string : upgraded station (empty if no upgrade)
	 */
	static void AIConstructionStart(UFlareCompany* Company,
									UFlareSimulatedSector* ConstructionProjectSector,
									FFlareSpacecraftDescription* ConstructionProjectStationDescription,
									UFlareSimulatedSpacecraft* ConstructionProjectStation);
};

class CombatLog
{
public:
	/**
	 * The sector has been activated
	 *
	 * Event: SECTOR_ACTIVATED
	 * Params:
	 *  - string : sector
	 */
	static void SectorActivated(UFlareSimulatedSector* Sector);

	/**
	 * The sector has been deactivated
	 *
	 * Event: SECTOR_DEACTIVATED
	 * Params:
	 *  - string : sector
	 */
	static void SectorDeactivated(UFlareSimulatedSector* Sector);

	/**
	 * An automatic battle started
	 *
	 * Event: AUTOMATIC_BATTLE_STARTED
	 * Params:
	 *  - string : sector
	 */
	static void AutomaticBattleStarted(UFlareSimulatedSector* Sector);

	/**
	 * An automatic battle ended
	 *
	 * Event: AUTOMATIC_BATTLE_ENDED
	 * Params:
	 *  - string : sector
	 */
	static void AutomaticBattleEnded(UFlareSimulatedSector* Sector);

	/**
	 * A bomb has been dropped
	 *
	 * Event: BOMB_DROPPED
	 * Params:
	 *  - string : bombId
	 *  - string : sourceSpacecraft
	 *  - string : sourceWeapon
	 *  - string : bombType
	 */
	static void BombDropped(AFlareBomb *Bomb);

	/**
	 * A bomb has been destroyed
	 *
	 * Event: BOMB_DESTROYED
	 * Params:
	 *  - string : bombId
	 */
	static void BombDestroyed(FName BombIdentifier);

	/**
	 * A spacecraft has been damaged
	 *
	 * Event: SPACECRAFT_DAMAGED
	 * Params:
	 *  - string : spacecraft
	 *  - string : damageType
	 *  - float : energy
	 *  - float : radius
	 *  - vector3 : relative location
	 *  - string : damageSourceCompany
	 */
	static void SpacecraftDamaged(UFlareSimulatedSpacecraft* Spacecraft, float Energy, float Radius, FVector RelativeLocation, EFlareDamage::Type DamageType, UFlareCompany* DamageSource);


	/**
	 * A spacecraft component has been damaged
	 *
	 * Event: SPACECRAFT_COMPONENT_DAMAGED
	 * Params:
	 *  - string : spacecraft
	 *  - string : componentSlot
	 *  - string : componentType
	 *  - float : energy
	 *  - float : effectiveEnergy
	 *  - string : damageType
	 *  - float : InitialDamageRatio
	 *  - float : TerminalDamageRatio
	 */
	static void SpacecraftComponentDamaged(UFlareSimulatedSpacecraft* Spacecraft, FFlareSpacecraftComponentSave* ComponentData, FFlareSpacecraftComponentDescription* ComponentDescription, float Energy, float EffectiveEnergy, EFlareDamage::Type DamageType, float InitialDamageRatio, float TerminalDamageRatio);

	/**
	 * A spacecraft has been harpooned
	 *
	 * Event: SPACECRAFT_HARPOONED
	 * Params:
	 *  - string : spacecraft
	 *  - string : harpoonOwner
	 */
	static void SpacecraftHarpooned(UFlareSimulatedSpacecraft* Spacecraft, UFlareCompany* HarpoonOwner);

};
