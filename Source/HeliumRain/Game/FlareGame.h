#pragma once


#include "GameFramework/GameMode.h"

// TODO, make it depend on player CA
#define AI_NERF_RATIO 0.5

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "../Player/FlareMenuPawn.h"

#include "FlarePlanetarium.h"
#include "FlareGameTools.h"
#include "FlareCompany.h"
#include "FlareWorld.h"
#include "FlareSector.h"
#include "Log/FlareLogApi.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareAsteroidCatalog.h"
#include "../Data/FlareCompanyCatalog.h"
#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareOrbitalMap.h"

#include "FlareGame.generated.h"


class UFlareSaveGame;
class UFlareSaveGameSystem;
class UFlareQuestManager;
class UFlareQuestCatalog;
class UFlareDebrisField;
class UFlareSectorCatalogEntry;
class UFlareScenarioTools;
struct FFlarePlayerSave;


USTRUCT()
struct FFlareSaveSlotInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() UFlareSaveGame*            Save;
	UPROPERTY() UMaterialInstanceDynamic*  Emblem;

	FSlateBrush                EmblemBrush;

	int32                      CompanyShipCount;
	int32                      CompanyValue;
	FText                      CompanyName;
};


UCLASS()
class HELIUMRAIN_API AFlareGame : public AGameMode
{
public:
	
	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void StartPlay() override;

	virtual void PostLogin(APlayerController* Player) override;

	virtual void Logout(AController* Player) override;

	virtual void ActivateSector(UFlareSimulatedSector* Sector);

	virtual void ActivateCurrentSector();

	virtual UFlareSimulatedSector* DeactivateSector();

	virtual void Recovery();

	virtual void SetWorldPause(bool Pause);

	virtual void Tick(float DeltaSeconds) override;

	virtual void Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation);

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
	virtual void CreateGame(AFlarePlayerController* PC, FFlareCompanyDescription CompanyData, int32 ScenarioIndex, bool PlayTutorial);

	/** Create a company */
	UFlareCompany* CreateCompany(int32 CatalogIdentifier);

    /** Load the game from this save file */
    virtual bool LoadGame(AFlarePlayerController* PC);

	/** Save the world to this save file */
	virtual bool SaveGame(AFlarePlayerController* PC, bool Async);

	/** Unload the game*/
	virtual void UnloadGame();
	
	virtual void Clean();
	
	/*----------------------------------------------------
		Level streaming
	----------------------------------------------------*/

	/** Load a streaming level */
	bool LoadStreamingLevel(FName SectorLevel);

	/** Unload a streaming level */
	void UnloadStreamingLevel(FName SectorLevel);

	/** Callback for a loaded streaming level */
	UFUNCTION(BlueprintCallable, Category = GameMode)
	void OnLevelLoaded();

	/** Callback for an unloaded streaming level */
	UFUNCTION(BlueprintCallable, Category = GameMode)
	void OnLevelUnLoaded();


	/*----------------------------------------------------
		Immatriculations
	----------------------------------------------------*/

	/** Build a unique immatriculation string for this object */
	void Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave);

	FName GenerateIdentifier(FName BaseName);

	/** Fill the database with capital ship names */
	void InitCapitalShipNameDatabase();

	/** Convert a number to roman */
	static FString ConvertToRoman(uint32 Val);

	/** Get a capship name */
	FText PickCapitalShipName();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
		
	/** Planetary system */
	UPROPERTY()
	AFlarePlanetarium*                         Planetarium;

    /** World */
    UPROPERTY()
    UFlareWorld*                               World;

	/** Quest manager*/
	UPROPERTY()
	UFlareQuestManager*                        QuestManager;

	/** Active sector */
	UPROPERTY()
	UFlareSector*                              ActiveSector;

	/** Activating sector */
	UPROPERTY()
	UFlareSimulatedSector*                     ActivatingSector;

	/** Debris field */
	UPROPERTY()
	UFlareDebrisField*                         DebrisFieldSystem;

	/** Player controller */
	UPROPERTY()
	AFlarePlayerController*			           PlayerController;

	/** Save game system*/
	UPROPERTY()
	UFlareSaveGameSystem*                      SaveGameSystem;

	/** Scenario tools */
	UPROPERTY()
	UFlareScenarioTools*                       ScenarioTools;
	
	// Post process volume
	UPROPERTY()
	APostProcessVolume*                        PostProcessVolume;
	

	/*----------------------------------------------------
		Catalogs
	----------------------------------------------------*/
	
	/** Reference to all available ship models */
	UPROPERTY()
	UFlareSpacecraftCatalog*                   SpacecraftCatalog;

	/** Reference to all available ship parts */
	UPROPERTY()
	UFlareSpacecraftComponentsCatalog*         ShipPartsCatalog;

	/** Reference to colors and patterns */
	UPROPERTY()
	UFlareCustomizationCatalog*                CustomizationCatalog;

	/** Reference to asteroid data */
	UPROPERTY()
	UFlareAsteroidCatalog*                     AsteroidCatalog;

	/** Reference to company data */
	UPROPERTY()
	UFlareCompanyCatalog*                      CompanyCatalog;

	/** Reference to the orbital setup */
	UPROPERTY()
	UFlareOrbitalMap*                          OrbitalBodies;

	/** Sector description list */
	UPROPERTY()
	TArray<UFlareSectorCatalogEntry*>          SectorList;

	/** Reference to all quests */
	UPROPERTY()
	UFlareQuestCatalog*                        QuestCatalog;

	/** Reference to all quests */
	UPROPERTY()
	UFlareResourceCatalog*                     ResourceCatalog;

	/** Default asteroid */
	UPROPERTY()
	UStaticMesh*                               DefaultAsteroid;
	

	/*----------------------------------------------------
		Immatriculation and save data
	----------------------------------------------------*/

	int32                                      CurrentImmatriculationIndex;
	int32                                      CurrentIdentifierIndex;
	TArray<FText>                              BaseImmatriculationNameList;

	FName                                      DefaultWeaponIdentifier;
	FName                                      DefaultTurretIdentifier;

	int32                                      CurrentSaveIndex;
	bool                                       LoadedOrCreated;
	int32                                      SaveSlotCount;
	int32                                      CurrentStreamingLevelIndex;
	bool                                       IsLoadingStreamingLevel;

	UPROPERTY()
	TArray<FFlareSaveSlotInfo>                 SaveSlots;


public:
	
	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareWorld* GetGameWorld() const
	{
		return World;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	AFlarePlayerController* GetPC() const
	{
		return PlayerController;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	APostProcessVolume* GetPostProcessVolume() const
	{
		return PostProcessVolume;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareSector* GetActiveSector() const
	{
		return ActiveSector;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	AFlarePlanetarium* GetPlanetarium() const
	{
		return Planetarium;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
	}

	inline const FFlareCompanyDescription* GetCompanyDescription(int32 Index) const
	{
		return (CompanyCatalog ? &CompanyCatalog->Companies[Index] : NULL);
	}

	inline const FFlareCompanyDescription* GetPlayerCompanyDescription() const;
	
	inline const int32 GetCompanyCatalogCount() const
	{
		return (CompanyCatalog ? CompanyCatalog->Companies.Num() : 0);
	}

	inline UFlareSpacecraftCatalog* GetSpacecraftCatalog() const
	{
		return SpacecraftCatalog;
	}

	inline UFlareOrbitalMap* GetOrbitalBodies() const
	{
		return OrbitalBodies;
	}

	inline TArray<UFlareSectorCatalogEntry*>& GetSectorCatalog()
	{
		return SectorList;
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

	inline UFlareQuestCatalog* GetQuestCatalog() const
	{
		return QuestCatalog;
	}

	inline UFlareResourceCatalog* GetResourceCatalog() const
	{
		return ResourceCatalog;
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

	inline void SetDefaultWeaponIdentifier(FName Identifier)
	{
		DefaultWeaponIdentifier = Identifier;
	}

	inline void SetDefaultTurretIdentifier(FName Identifier)
	{
		DefaultTurretIdentifier = Identifier;
	}

	inline UStaticMesh* GetDefaultAsteroid() const
	{
		return DefaultAsteroid;
	}

	UFlareScenarioTools* GetScenarioTools()
	{
		return ScenarioTools;
	}

	bool IsLoadingLevel() const
	{
		return IsLoadingStreamingLevel;
	}


};
