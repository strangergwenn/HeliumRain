#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "FlareSimulatedSector.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;
class AFlareAsteroid;

UCLASS()
class HELIUMRAIN_API UFlareSector : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(UFlareSimulatedSector* Parent);

	/** Save the sector to a save file */
	virtual void Save();

	/** Destroy the sector */
	virtual void DestroySector();


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	AStaticMeshActor* AddDebris(UStaticMesh* Mesh, float Debris, float SectorScale);

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareSpacecraft* LoadSpacecraft(UFlareSimulatedSpacecraft* ParentSpacecraft);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	void RegisterBomb(AFlareBomb* Bomb);

	void UnregisterBomb(AFlareBomb* Bomb);

	void RegisterShell(AFlareShell* Shell);

	void UnregisterShell(AFlareShell* Shell);

	/** Destroy a ship or a station*/
	virtual void DestroySpacecraft(AFlareSpacecraft* Spacecraft, bool Destroying = false);

	virtual void SetPause(bool Pause);

	AActor* GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize = true, AActor* ActorToIgnore = NULL);

	void PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector*         ParentSector;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorStations;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorShips;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorSpacecrafts;
	
	UPROPERTY()
	TArray<AFlareAsteroid*>        SectorAsteroids;
	UPROPERTY()
	TArray<AStaticMeshActor*>      SectorDebrisField;
	UPROPERTY()
	TArray<AFlareBomb*>            SectorBombs;
	UPROPERTY()
	TArray<AFlareShell*>           SectorShells;

	int64						   LocalTime;
	bool						   SectorRepartitionCache;
	FVector                        SectorCenter;
	float                          SectorRadius;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame()
	{
		return ParentSector->GetGame();
	}

	UFlareSimulatedSector* GetSimulatedSector()
	{
		return ParentSector;
	}

	TArray<AFlareSpacecraft*> GetCompanyShips(UFlareCompany* Company);

	TArray<AFlareSpacecraft*> GetCompanySpacecrafts(UFlareCompany* Company);

	AFlareSpacecraft* FindSpacecraft(FName Immatriculation);

	inline const FFlareSectorDescription* GetDescription() const
	{
		return ParentSector->GetDescription();
	}


	/*inline FName GetIdentifier() const
	{
		return SectorData.Identifier;
	}*/

	inline TArray<AFlareSpacecraft*>& GetSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	/*inline FFlarePeopleSave* GetPeople(){
		return &SectorData.PeopleData;
	}*/

	inline TArray<AFlareSpacecraft*>& GetStations()
	{
		return SectorStations;
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

	float GetSectorLimits()
	{
		return 1500000; // 15 km
	}

};
