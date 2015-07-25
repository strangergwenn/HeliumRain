#pragma once

#include "GameFramework/GameMode.h"

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "../Player/FlareMenuPawn.h"

#include "FlarePlanetarium.h"
#include "FlareCompany.h"
#include "FlareWorld.h"
#include "FlareSector.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareAsteroidCatalog.h"
#include "../Data/FlareCompanyCatalog.h"

#include "FlareGame.generated.h"


class UFlareSaveGame;
struct FFlarePlayerSave;

USTRUCT()
struct FFlareSaveSlotInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() UFlareSaveGame*            Save;
	UPROPERTY() UMaterialInstanceDynamic*  Emblem;

	FSlateBrush                EmblemBrush;

	int32                      CompanyShipCount;
	int32                      CompanyMoney;
	FText                      CompanyName;
};


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

	virtual void ActivateSector(AController* Player,UFlareSimulatedSector* Sector);

	virtual void DeactivateSector(AController* Player);

	/*----------------------------------------------------
		Save slots
	----------------------------------------------------*/

	/** Load metadata for all save slots */
	void ReadAllSaveSlots();

	/** Get the number of save slots */
	int32 GetSaveSlotCount() const;

	/** Get the current save slot number */
	int32 GetCurrentSaveSlot() const;

	/** Set the current slot to use for loading or creating a game */
	void SetCurrentSlot(int32 Index);

	/** Is this an existing save slot ? */
	bool DoesSaveSlotExist(int32 Index) const;

	/** Get metadata for this save slot */
	const FFlareSaveSlotInfo& GetSaveSlotInfo(int32 Index);

	/** Load a game save */
	UFlareSaveGame* ReadSaveSlot(int32 Index);

	/** Remove a game save */
	bool DeleteSaveSlot(int32 Index);


	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Create a new game from scratch */
	virtual void CreateGame(AFlarePlayerController* PC, FString CompanyName, int32 ScenarioIndex, bool PlayTutorial);

	/** Create a company */
	UFlareCompany* CreateCompany(int32 CatalogIdentifier);

    /** Load the game from this save file */
    virtual bool LoadGame(AFlarePlayerController* PC);

	/** Save the world to this save file */
	virtual bool SaveGame(AFlarePlayerController* PC);

	/** Unload the game*/
	virtual void UnloadGame();


	/*----------------------------------------------------
		Creation command line
	----------------------------------------------------*/

	/** Create a ship in a sector for the current player*/
	UFUNCTION(exec)
	UFlareSimulatedSpacecraft* CreateShipForMeInSector(FName ShipClass, FName SectorIdentifier);

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

	/** Empty the active sector. Let only the current played ship */
	UFUNCTION(exec)
	virtual void EmptySector();

	/** Make war between two company */
	UFUNCTION(exec)
	void DeclareWar(FName Company1ShortName, FName Company2ShortName);

	/** Make peace two company */
	UFUNCTION(exec)
	void MakePeace(FName Company1ShortName, FName Company2ShortName);

	/** Force sector activation */
	UFUNCTION(exec)
	void ForceSectorActivation(FName SectorIdentifier);

	/** Force sector deactivation */
	UFUNCTION(exec)
	void ForceSectorDeactivation();

	/** Get world time */
	UFUNCTION(exec)
	int64 GetWorldTime();

	/** Set world time */
	UFUNCTION(exec)
	void SetWorldTime(int64 Time);


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

    /** World */
    UPROPERTY()
    UFlareWorld*                   World;

	/** Active sector */
	UPROPERTY()
	UFlareSector*                   ActiveSector;


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

	/** Reference to company data */
	UPROPERTY()
	UFlareCompanyCatalog*                    CompanyCatalog;

	/** Company emblems */
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*>        CompanyEmblems;

	/** Company emblem brushes */
	UPROPERTY()
	TArray<FSlateBrush>                      CompanyEmblemBrushes;


	/*----------------------------------------------------
		Immatriculation and save data
	----------------------------------------------------*/

	int32                                    CurrentImmatriculationIndex;
	TArray<FName>                            BaseImmatriculationNameList;

	FName                                    DefaultWeaponIdentifier;
	FName                                    DefaultTurretIdentifier;

	int32                                    CurrentSaveIndex;
	bool                                     LoadedOrCreated;
	int32                                    SaveSlotCount;

	UPROPERTY()
	TArray<FFlareSaveSlotInfo>               SaveSlots;


public:

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Add a new emblem for this company - Be careful to respect the array indexes */
	void AddEmblem(const FFlareCompanyDescription* Company);


	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UFlareWorld* GetGameWorld() const
	{
		return World;
	}

	inline UFlareSector* GetActiveSector() const
	{
		return ActiveSector;
	}

	inline const FFlareCompanyDescription* GetCompanyDescription(int32 Index) const
	{
		return (CompanyCatalog ? &CompanyCatalog->Companies[Index] : NULL);
	}

	inline const FFlareCompanyDescription* GetPlayerCompanyDescription() const;

	inline const FSlateBrush* GetCompanyEmblem(int32 Index) const;

	inline const int32 GetCompanyCatalogCount() const
	{
		return (CompanyCatalog ? CompanyCatalog->Companies.Num() : 0);
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

	inline UFlareCompanyCatalog* GetCompanyCatalog() const
	{
		return CompanyCatalog;
	}

	inline bool IsLoadedOrCreated() const
	{
		return LoadedOrCreated;
	}

	inline FName GetDefaultWeaponIdentifier() const
	{
		return DefaultWeaponIdentifier;
	}

	inline FName GetDefaultTurretIdentifier() const
	{
		return DefaultTurretIdentifier;
	}

};
