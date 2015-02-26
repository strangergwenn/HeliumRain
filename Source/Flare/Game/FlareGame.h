#pragma once

#include "GameFramework/GameMode.h"
#include "../Ships/FlareShip.h"
#include "../Data/FlareShipCatalog.h"
#include "../Data/FlareStationCatalog.h"
#include "../Data/FlareShipPartsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Player/FlareMenuPawn.h"
#include "FlarePlanetarium.h"
#include "FlareGame.generated.h"


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

	/** Load the world from this save file */
	virtual bool LoadWorld(AFlarePlayerController* PC, FString SaveFile);

	/** Save the world to this save file */
	virtual bool SaveWorld(AFlarePlayerController* PC, FString SaveFile);


	/*----------------------------------------------------
		Creation tools
	----------------------------------------------------*/

	/** Create a ship in the level */
	UFUNCTION(exec)
	AFlareShip* CreateShip(FName ShipClass);

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareStation* CreateStation(FName StationClass);

	/** Build a unique immatriculation string for this object */
	FName Immatriculate(FName Company, FName TargetClass);


	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Spawn a ship from save data */
	virtual AFlareShip* LoadShip(const FFlareShipSave& ShipData);

	/** Spawn a station from save data */
	virtual AFlareStation* LoadStation(const FFlareStationSave& ShipData);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** The default pawn class used by players. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<class AFlareMenuPawn> MenuPawnClass;

	/** Planetary system class */
	UPROPERTY(EditAnywhere, Category = GameMode)
	TSubclassOf<class AFlarePlanetarium> PlanetariumClass;

	/** Planetary system */
	UPROPERTY()
	AFlarePlanetarium* Planetarium;

	/** Reference to all available ship models */
	UPROPERTY()
	UFlareShipCatalog* ShipCatalog;

	/** Reference to all available station models */
	UPROPERTY()
	UFlareStationCatalog* StationCatalog;

	/** Reference to all available ship parts */
	UPROPERTY()
	UFlareShipPartsCatalog* ShipPartsCatalog;

	/** Reference to colors and patterns */
	UPROPERTY()
	UFlareCustomizationCatalog* CustomizationCatalog;

	/** Immatriculation index */
	int32 CurrentImmatriculationIndex;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UClass* GetMenuPawnClass() const
	{
		return MenuPawnClass;
	}

	inline UFlareShipCatalog* GetShipCatalog() const
	{
		return ShipCatalog;
	}

	inline UFlareStationCatalog* GetStationCatalog() const
	{
		return StationCatalog;
	}

	inline UFlareShipPartsCatalog* GetShipPartsCatalog() const
	{
		return ShipPartsCatalog;
	}

	inline UFlareCustomizationCatalog* GetCustomizationCatalog() const
	{
		return CustomizationCatalog;
	}


};
