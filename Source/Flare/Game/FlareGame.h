#pragma once

#include "GameFramework/GameMode.h"

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "../Player/FlareMenuPawn.h"

#include "FlarePlanetarium.h"
#include "FlareCompany.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareAsteroidCatalog.h"

#include "FlareGame.generated.h"


class UFlareSaveGame;


UCLASS()
class FLARE_API AFlareGame : public AGameMode
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void StartPlay() override;

	virtual void PostLogin(APlayerController* Player) override;

	virtual void Logout(AController* Player) override;


	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Create a new world from scratch */
	virtual void CreateWorld(AFlarePlayerController* PC);

	/** Create a station in the level  for a specific company */
	AFlareSpacecraft* CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition);

	/** Create a ship in the level  for a specific company */
	AFlareSpacecraft* CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition);

	/** Create a ship or station in the level  for a specific company */
	AFlareSpacecraft* CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition);

	/** Load a game save */
	static UFlareSaveGame* LoadSaveFile(int32 Index);

	/** Load the world from this save file */
	virtual bool LoadWorld(AFlarePlayerController* PC, int32 Index);

	/** Spawn a company from save data */
	virtual UFlareCompany* LoadCompany(const FFlareCompanySave& CompanyData);

	/** Spawn an asteroid from save data */
	virtual AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	/** Spawn a ship from save data */
	virtual AFlareSpacecraft* LoadShip(const FFlareSpacecraftSave& ShipData);

	/** Spawn a bomb from save data */
	virtual AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	/** Save the world to this save file */
	virtual bool SaveWorld(AFlarePlayerController* PC);


	/*----------------------------------------------------
		Creation command line
	----------------------------------------------------*/

	/** Create a company */
	UFUNCTION(exec)
	UFlareCompany* CreateCompany(FString CompanyName);

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareSpacecraft* CreateStationForMe(FName StationClass);

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareSpacecraft* CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance);

	/** Create a ship in the level for the current player*/
	UFUNCTION(exec)
	AFlareSpacecraft* CreateShipForMe(FName ShipClass);

	/** Create a ship in the level for a specific company identified by its short name*/
	UFUNCTION(exec)
	AFlareSpacecraft* CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance);

	/** Create ships in the level for a specific company identified by its short name*/
	UFUNCTION(exec)
	void CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count);

	/** Create 2 fleets for 2 companies At a defined distance */
	UFUNCTION(exec)
	void CreateQuickBattle(float Distance, FName Company1, FName Company2, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count);

	/** Set the default weapon for new created ship */
	UFUNCTION(exec)
	void SetDefaultWeapon(FName NewDefaultWeaponIdentifier);

	/** Set the default turret for new created ship */
	UFUNCTION(exec)
	void SetDefaultTurret(FName NewDefaultTurretIdentifier);

	/** Add an asteroid to the world */
	UFUNCTION(exec)
	void CreateAsteroid(int32 ID);


	/*----------------------------------------------------
		Immatriculations
	----------------------------------------------------*/

	/** Build a unique immatriculation string for this object */
	void Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave);

	/** Fill the database with capital ship names */
	void InitCapitalShipNameDatabase();

	/** Get a capship name */
	FName PickCapitalShipName();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** The default pawn class used by players. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<class AFlareMenuPawn>        MenuPawnClass;

	/** Planetary system class */
	UPROPERTY(EditAnywhere, Category = GameMode)
	TSubclassOf<class AFlarePlanetarium>     PlanetariumClass;

	/** Planetary system */
	UPROPERTY()
	AFlarePlanetarium*                       Planetarium;

	/** Companies */
	UPROPERTY()
	TArray<UFlareCompany*>                   Companies;


	/*----------------------------------------------------
		Catalogs
	----------------------------------------------------*/

	/** Reference to all available ship models */
	UPROPERTY()
	UFlareSpacecraftCatalog*                 SpacecraftCatalog;

	/** Reference to all available ship parts */
	UPROPERTY()
	UFlareSpacecraftComponentsCatalog*       ShipPartsCatalog;

	/** Reference to colors and patterns */
	UPROPERTY()
	UFlareCustomizationCatalog*              CustomizationCatalog;

	/** Reference to asteroid data */
	UPROPERTY()
	UFlareAsteroidCatalog*                   AsteroidCatalog;


	/*----------------------------------------------------
		Immatriculation and save data
	----------------------------------------------------*/

	int32                                    CurrentImmatriculationIndex;
	TArray<FName>                            BaseImmatriculationNameList;

	FName                                    DefaultWeaponIdentifer;
	FName                                    DefaultTurretIdentifer;

	int32                                    CurrentSaveIndex;
	bool                                     LoadedOrCreated;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UFlareCompany* FindCompany(FName Identifier) const
	{
		for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
		{
			UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
			if (Company && Company->GetIdentifier() == Identifier)
			{
				return Company;
			}
		}
		return NULL;
	}

	inline UClass* GetMenuPawnClass() const
	{
		return MenuPawnClass;
	}

	inline UFlareSpacecraftCatalog* GetSpacecraftCatalog() const
	{
		return SpacecraftCatalog;
	}

	inline UFlareSpacecraftComponentsCatalog* GetShipPartsCatalog() const
	{
		return ShipPartsCatalog;
	}

	inline UFlareCustomizationCatalog* GetCustomizationCatalog() const
	{
		return CustomizationCatalog;
	}

	inline UFlareAsteroidCatalog* GetAsteroidCatalog() const
	{
		return AsteroidCatalog;
	}

	inline bool IsLoadedOrCreated() const
	{
		return LoadedOrCreated;
	}

};
