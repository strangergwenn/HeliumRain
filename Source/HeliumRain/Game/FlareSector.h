#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "FlareSimulatedSector.h"
#include "FlareSectorInterface.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;
class AFlareAsteroid;

UCLASS()
class HELIUMRAIN_API UFlareSector : public UFlareSectorInterface
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(UFlareSimulatedSector* ParentSector, const FFlareSectorSave& Data);

	/** Save the sector to a save file */
	virtual FFlareSectorSave* Save(TArray<FFlareSpacecraftSave>& SpacecraftData);

	/** Destroy the sector */
	virtual void DestroySector();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Create a station in the level  for a specific company */
	// AFlareSpacecraft* CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Create a ship in the level  for a specific company */
	// AFlareSpacecraft* CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company. No null parameter accepted */
	// AFlareSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation = FRotator::ZeroRotator);

	/** Add an asteroid to the world at a specific location */
	// void CreateAsteroidAt(int32 ID, FVector Location);

	// virtual void EmptySector();

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareSpacecraft* LoadSpacecraft(const FFlareSpacecraftSave& ShipData);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	void RegisterBomb(AFlareBomb* Bomb);

	void UnregisterBomb(AFlareBomb* Bomb);

	/** Destroy a ship or a station*/
	virtual void DestroySpacecraft(AFlareSpacecraft* Spacecraft, bool Destroying = false);

	virtual void SetPause(bool Pause);

	AActor* GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize = true, AActor* ActorToIgnore = NULL);

	void PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	UFlareSimulatedSector*      SimulatedSector;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorStations;
	TArray<IFlareSpacecraftInterface*>      SectorStationInterfaces;
	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorShips;
	TArray<IFlareSpacecraftInterface*>      SectorShipInterfaces;
	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorSpacecrafts;

	UPROPERTY()
	TArray<AFlareAsteroid*>      SectorAsteroids;
	UPROPERTY()
	TArray<AFlareBomb*>      SectorBombs;
	UPROPERTY()
	TArray<AFlareShell*>      SectorShells;
	// TODO shell register

	int64						  LocalTime;
	bool						  SectorRepartitionCache;
	FVector                       SectorCenter;
	float                         SectorRadius;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	UFlareSimulatedSector* GetSimulatedSector() override
	{
		return SimulatedSector;
	}

	TArray<AFlareSpacecraft*> GetCompanyShips(UFlareCompany* Company);

	TArray<AFlareSpacecraft*> GetCompanySpacecrafts(UFlareCompany* Company);

	AFlareSpacecraft* FindSpacecraft(FName Immatriculation);

	inline FName GetIdentifier() const
	{
		return SectorData.Identifier;
	}

	inline TArray<AFlareSpacecraft*>& GetSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	inline TArray<IFlareSpacecraftInterface*>& GetSectorShipInterfaces() override
	{
		return SectorShipInterfaces;
	}

	inline TArray<AFlareSpacecraft*>& GetStations()
	{
		return SectorStations;
	}

	inline TArray<IFlareSpacecraftInterface*>& GetSectorStationInterfaces() override
	{
		return SectorStationInterfaces;
	}

	inline TArray<AFlareAsteroid*>& GetAsteroids()
	{
		return SectorAsteroids;
	}

	inline TArray<AFlareBomb*>& GetBombs()
	{
		return SectorBombs;
	}

	inline int64 GetLocalTime()
	{
		return LocalTime;
	}

	void GenerateSectorRepartitionCache();

	FVector GetSectorCenter();

	float GetSectorRadius();

};
