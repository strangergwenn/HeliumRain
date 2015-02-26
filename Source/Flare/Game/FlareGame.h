#pragma once

#include "GameFramework/GameMode.h"
#include "../Ships/FlareShip.h"
#include "../Data/FlareShipCatalog.h"
#include "../Data/FlareStationCatalog.h"
#include "../Data/FlareShipPartsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Player/FlareMenuPawn.h"
#include "FlarePlanetarium.h"
#include "FlareCompany.h"
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


	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the world from this save file */
	virtual bool LoadWorld(AFlarePlayerController* PC, FString SaveFile);

	/** Spawn a company from save data */
	virtual UFlareCompany* LoadCompany(const FFlareCompanySave& CompanyData);

	/** Spawn a station from save data */
	virtual AFlareStation* LoadStation(const FFlareStationSave& StationData);

	/** Spawn a ship from save data */
	virtual AFlareShip* LoadShip(const FFlareShipSave& ShipData);

	/** Save the world to this save file */
	virtual bool SaveWorld(AFlarePlayerController* PC, FString SaveFile);


	/*----------------------------------------------------
		Creation tools
	----------------------------------------------------*/

	/** Create a new world from scratch */
	virtual void CreateWorld(AFlarePlayerController* PC);

	/** Create a company */
	UFUNCTION(exec)
	UFlareCompany* CreateCompany(FString CompanyName);

	/** Create a station in the level */
	UFUNCTION(exec)
	AFlareStation* CreateStation(FName StationClass);

	/** Create a ship in the level */
	UFUNCTION(exec)
	AFlareShip* CreateShip(FName ShipClass);

	/** Build a unique immatriculation string for this object */
	FName Immatriculate(FName Company, FName TargetClass);


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

	/** Comapnies */
	TArray<UFlareCompany*> Companies;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UFlareCompany* FindCompany(FName Identifier) const
	{
		for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
		{
			UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
			if (Company)
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
