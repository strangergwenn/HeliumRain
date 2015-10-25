#pragma once

#include "Object.h"
#include "FlareFleet.generated.h"


class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
class AFlareGame;
class UFlareCompany;
class UFlareTravel;
struct FFlareSpacecraftSave;

/** Fleet save data */
USTRUCT()
struct FFlareFleetSave
{
	GENERATED_USTRUCT_BODY()

	/** Given Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

	/** Sector identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Fleet ships */
	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> ShipImmatriculations;
};

UCLASS()
class FLARE_API UFlareFleet : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the fleet from a save file */
	virtual void Load(const FFlareFleetSave& Data);

	/** Save the fleet to a save file */
	virtual FFlareFleetSave* Save();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Get a name for this fleet (capital ship's name, etc) */
	FText GetName();

	virtual void AddShip(UFlareSimulatedSpacecraft* Ship);

	virtual void RemoveShip(UFlareSimulatedSpacecraft* Ship);

	/** Remove all ship from the fleet and delete it. Not possible during travel */
	virtual void Disband();

	virtual void Merge(UFlareFleet* Fleet);

	virtual void SetCurrentSector(UFlareSimulatedSector* Sector);

	void SetCurrentTravel(UFlareTravel* Travel);

	virtual void InitShipList();


protected:

	TArray<UFlareSimulatedSpacecraft*>     FleetShips;


	UFlareCompany*			               FleetCompany;
	FFlareFleetSave                        FleetData;
	AFlareGame*                            Game;
	bool                                   IsShipListLoaded;
	UFlareSimulatedSector*                 CurrentSector;
	UFlareTravel*                           CurrentTravel;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FString GetFleetName() const
	{
		return FleetData.Name;
	}

	UFlareCompany* GetFleetCompany() const
	{
		return FleetCompany;
	}

	FName GetIdentifier() const
	{
		return FleetData.Identifier;
	}

	FFlareFleetSave* GetData()
	{
		return &FleetData;
	}

	TArray<UFlareSimulatedSpacecraft*>& GetShips();


	bool IsTraveling();

	/** Return null if traveling */
	inline UFlareSimulatedSector* GetCurrentSector()
	{
		return CurrentSector;
	}

	/** Return null if not traveling */
	inline UFlareTravel* GetCurrentTravel()
	{
		return CurrentTravel;
	}


};
