#pragma once

#include "GameFramework/GameMode.h"

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "../Player/FlareMenuPawn.h"

#include "FlarePlanetarium.h"
#include "FlareGameTools.h"
#include "FlareCompany.h"
#include "FlareWorld.h"
#include "FlareSector.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareAsteroidCatalog.h"
#include "../Data/FlareCompanyCatalog.h"
#include "../Data/FlareSectorCatalog.h"
#include "../Data/FlareResourceCatalog.h"

#include "FlareGame.generated.h"


class UFlareSaveGame;
class UFlareSaveGameSystem;
class UFlareQuestManager;
class UFlareQuestCatalog;
class UFlareDebrisField;
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
	virtual void CreateGame(AFlarePlayerController* PC, FText CompanyName, int32 ScenarioIndex, bool PlayTutorial);

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
	void LoadStreamingLevel(FName SectorLevel);

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

	/** The default pawn class used by players. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<class AFlareMenuPawn>          MenuPawnClass;

	/** Planetary system class */
	UPROPERTY(EditAnywhere, Category = GameMode)
	TSubclassOf<class AFlarePlanetarium>       PlanetariumClass;

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

	/** Debris field */
	UPROPERTY()
	UFlareDebrisField*                         DebrisFieldSystem;

	/** Player controller */
	UPROPERTY()
	AFlarePlayerController*			           PlayerController;

	/** Save game system*/
	UPROPERTY()
	UFlareSaveGameSystem*                      SaveGameSystem;

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

	/** Reference to all sector descriptions */
	UPROPERTY()
	UFlareSectorCatalog*                       SectorCatalog;

	/** Reference to all quests*/
	UPROPERTY()
	UFlareQuestCatalog*                        QuestCatalog;

	/** Reference to all quests*/
	UPROPERTY()
	UFlareResourceCatalog*                     ResourceCatalog;
	

	/*----------------------------------------------------
		Immatriculation and save data
	----------------------------------------------------*/

	int32                                      CurrentImmatriculationIndex;
	TArray<FText>                              BaseImmatriculationNameList;

	FName                                      DefaultWeaponIdentifier;
	FName                                      DefaultTurretIdentifier;

	int32                                      CurrentSaveIndex;
	bool                                       LoadedOrCreated;
	int32                                      SaveSlotCount;
	int32                                      CurrentStreamingLevelIndex;

	UPROPERTY()
	TArray<FFlareSaveSlotInfo>                 SaveSlots;


public:
	
	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UFlareWorld* GetGameWorld() const
	{
		return World;
	}

	AFlarePlayerController* GetPC() const
	{
		return PlayerController;
	}

	inline UFlareSector* GetActiveSector() const
	{
		return ActiveSector;
	}

	inline AFlarePlanetarium* GetPlanetarium() const
	{
		return Planetarium;
	}

	inline UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
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

	inline UFlareSectorCatalog* GetSectorCatalog() const
	{
		return SectorCatalog;
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

};
