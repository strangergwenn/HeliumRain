#pragma once

#include "FlareGameTools.generated.h"

class UFlareWorld;
class AFlarePlayerController;
class UFlareSector;
class AFlareGame;

UCLASS(Within=PlayerController)
class ENGINE_API UFlareGameTools : public UCheatManager
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Game tools
	----------------------------------------------------*/

	/** Force sector activation */
	UFUNCTION(exec)
	void ForceSectorActivation(FName SectorIdentifier);

	/** Force sector deactivation */
	UFUNCTION(exec)
	void ForceSectorDeactivation();


	/** Set the default weapon for new created ship */
	UFUNCTION(exec)
	void SetDefaultWeapon(FName NewDefaultWeaponIdentifier);

	/** Set the default turret for new created ship */
	UFUNCTION(exec)
	void SetDefaultTurret(FName NewDefaultTurretIdentifier);

	/*----------------------------------------------------
		World tools
	----------------------------------------------------*/

	/** Get world time */
	UFUNCTION(exec)
	int64 GetWorldTime();

	/** Set world time */
	UFUNCTION(exec)
	void SetWorldTime(int64 Time);

	/** Simulate a world duration */
	UFUNCTION(exec)
	void Simulate(int64 Duration);


	/*----------------------------------------------------
		Company tools
	----------------------------------------------------*/

	/** Make war between two company */
	UFUNCTION(exec)
	void DeclareWar(FName Company1ShortName, FName Company2ShortName);

	/** Make peace two company */
	UFUNCTION(exec)
	void MakePeace(FName Company1ShortName, FName Company2ShortName);

	UFUNCTION(exec)
	void PrintCompanyList();

	UFUNCTION(exec)
	void PrintCompany(FName CompanyShortName);

	UFUNCTION(exec)
	void PrintCompanyByIndex(int32 Index);

	/*----------------------------------------------------
		Fleet tools
	----------------------------------------------------*/

	/** Create a fleet */
	UFUNCTION(exec)
	void CreateFleet(FString FleetName, FName FirstShipImmatriculation);

	/** Disband a fleet. Not possible during travelling */
	UFUNCTION(exec)
	void DisbandFleet(FName FleetIdentifier);

	/** Add a ship to a fleet. Not possible during travelling*/
	UFUNCTION(exec)
	void AddToFleet(FName FleetIdentifier, FName ShipImmatriculation);

	/** Remove a ship from a fleet. Not possible during travelling*/
	UFUNCTION(exec)
	void RemoveFromFleet(FName FleetIdentifier, FName ShipImmatriculation);

	/** Merge fleet 2 in fleet 1. Not possible during travelling*/
	UFUNCTION(exec)
	void MergeFleets(FName Fleet1Identifier, FName Fleet2Identifier);

	/*----------------------------------------------------
		Travel tools
	----------------------------------------------------*/

	/** Start travel. Not possible during travelling */
	UFUNCTION(exec)
	void StartTravel(FName FleetIdentifier, FName SectorIdentifier);

	UFUNCTION(exec)
	void PrintTravelList();

	UFUNCTION(exec)
	void PrintTravelByIndex(int32 Index);

	/*----------------------------------------------------
		Sector tools
	----------------------------------------------------*/

	/** Create a ship in a sector for the current player*/
	UFUNCTION(exec)
	UFlareSimulatedSpacecraft* CreateShipForMeInSector(FName ShipClass, FName SectorIdentifier);

	UFUNCTION(exec)
	void PrintSectorList();

	UFUNCTION(exec)
	void PrintSector(FName SectorIdentifier);

	UFUNCTION(exec)
	void PrintSectorByIndex(int32 Index);


	/*----------------------------------------------------
		Active Sector tools
	----------------------------------------------------*/

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareSpacecraft* CreateStationForMe(FName StationClass);

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareSpacecraft* CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance);

	/** Create a ship in the level for the current player*/
	UFUNCTION(exec)
	UFlareSimulatedSpacecraft* CreateShipForMe(FName ShipClass);

	/** Create a ship in the level for a specific company identified by its short name*/
	UFUNCTION(exec)
	AFlareSpacecraft* CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance);

	/** Create ships in the level for a specific company identified by its short name*/
	UFUNCTION(exec)
	void CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count);

	/** Create 2 fleets for 2 companies At a defined distance */
	UFUNCTION(exec)
	void CreateQuickBattle(float Distance, FName Company1, FName Company2, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count);

	/** Add an asteroid to the world */
	UFUNCTION(exec)
	void CreateAsteroid(int32 ID);

	/** Empty the active sector. Let only the current played ship */
	UFUNCTION(exec)
	virtual void EmptySector();

	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	AFlarePlayerController* GetPC() const;

	AFlareGame* GetGame() const;

	UFlareWorld* GetGameWorld() const;

	UFlareSector* GetActiveSector() const;

};
