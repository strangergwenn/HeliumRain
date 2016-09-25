#pragma once
#include "../../Flare.h"

struct FFlareSpacecraftDescription;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;

class CombatLog
{


	/*
		FlareLog::Combat->DamageDone(23);
		FlareLog::Combat->WeaponEmpty(23);


		FlareLog::Game->Loaded();
		FlareLog::Game->DamageDone(23);
		FlareLog::Game->Unloaded();

		FlareLog::Game->Trade();
		FlareLog::Game->Trade();
		FlareLog::Game->ReputationChange();*/


};


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
