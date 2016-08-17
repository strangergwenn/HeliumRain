#pragma once

#include "Object.h"
#include "FlareFleet.generated.h"


class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
class AFlareGame;
class UFlareCompany;
class UFlareTravel;
class UFlareTradeRoute;
struct FFlareSpacecraftSave;

/** Fleet save data */
USTRUCT()
struct FFlareFleetSave
{
	GENERATED_USTRUCT_BODY()

	/** Given Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Name;

	/** Fleet identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Fleet ships */
	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> ShipImmatriculations;
};

UCLASS()
class HELIUMRAIN_API UFlareFleet : public UObject
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

	virtual void RemoveShip(UFlareSimulatedSpacecraft* Ship, bool destroyed = false);

	/** Remove all ship from the fleet and delete it. Not possible during travel */
	virtual void Disband();

	/** Tell us if we can merge, and why */
	virtual bool CanMerge(UFlareFleet* Fleet, FText& OutInfo);

	/** Tell us if we are travelling */
	bool IsTraveling() const;

	/** Tell us if we can travel */
	bool CanTravel();

	/** Tell us if we can travel, and why */
	bool CanTravel(FText& OutInfo);

	virtual void Merge(UFlareFleet* Fleet);

	virtual void SetCurrentSector(UFlareSimulatedSector* Sector);

	void SetCurrentTravel(UFlareTravel* Travel);

	virtual void SetCurrentTradeRoute(UFlareTradeRoute* TradeRoute)
	{
		CurrentTradeRoute = TradeRoute;
	}

	virtual void InitShipList();

	void RemoveImmobilizedShips();

protected:

	TArray<UFlareSimulatedSpacecraft*>     FleetShips;

	UFlareCompany*			               FleetCompany;
	FFlareFleetSave                        FleetData;
	AFlareGame*                            Game;
	bool                                   IsShipListLoaded;
	UFlareSimulatedSector*                 CurrentSector;
	UFlareTravel*                          CurrentTravel;
	UFlareTradeRoute*                      CurrentTradeRoute;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FText GetFleetName() const
	{
		return FleetData.Name;
	}

	void SetFleetName(FText Name)
	{
		FleetData.Name = Name;
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

	uint32 GetImmobilizedShipCount();

	/** Get the current ship count in the fleet */
	uint32 GetShipCount();

	/** Get the maximum ship count in a fleet */
	uint32 GetMaxShipCount();

	/** Get information about the current travel, if any */
	FText GetStatusInfo() const;

	/** Return null if traveling */
	inline UFlareSimulatedSector* GetCurrentSector() const
	{
		return CurrentSector;
	}

	/** Return null if not traveling */
	inline UFlareTravel* GetCurrentTravel() const
	{
		return CurrentTravel;
	}

	/** Return null if not in a trade route */
	inline UFlareTradeRoute* GetCurrentTradeRoute() const
	{
		return CurrentTradeRoute;
	}

};
