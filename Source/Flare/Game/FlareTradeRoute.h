#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareTradeRoute.generated.h"

class UFlareSimulatedSector;
class UFlareFleet;
struct FFlareResourceDescription;

/** Fleet save data */
USTRUCT()
struct FFlareTradeRouteSectorSave
{
	GENERATED_USTRUCT_BODY()

	/** Trade route sector */
	UPROPERTY(EditAnywhere, Category = Save)
	FName SectorIdentifier;

	/** Resource to load in this sector
	 *  The quantity represent the quantity to let in the sector
	 */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ResourcesToLoad;

	/** Resource to unload for this sector
	 *  The quantity represent the max quantity present in the sector
	 *	0 mean no limit.
	 */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ResourcesToUnload;
};


/** Fleet save data */
USTRUCT()
struct FFlareTradeRouteSave
{
	GENERATED_USTRUCT_BODY()

	/** Given Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Name;

	/** Trade route identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Trade route fleets */
	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> FleetIdentifiers;

	/** Trade route sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTradeRouteSectorSave> Sectors;
};

UCLASS()
class FLARE_API UFlareTradeRoute : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the trade route from a save file */
	virtual void Load(const FFlareTradeRouteSave& Data);

	/** Save the trade route to a save file */
	virtual FFlareTradeRouteSave* Save();


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void Simulate(int64 Duration);

	virtual void AddFleet(UFlareFleet* Ship);

	virtual void RemoveFleet(UFlareFleet* Ship);

	virtual void AddSector(UFlareSimulatedSector* Sector);

	virtual void SetSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 QuantityToLeft);

	virtual void ClearSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource);

	virtual void SetSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 LimitQuantity);

	virtual void ClearSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource);


	/** Remove all fleet from the trade route and delete it. */
	virtual void Dissolve();

	virtual void InitFleetList();

    virtual void SetTradeRouteName(FText NewName)
    {
        TradeRouteData.Name = NewName;
    }

protected:

	TArray<UFlareFleet*>                  TradeRouteFleets;

	UFlareCompany*			               TradeRouteCompany;
	FFlareTradeRouteSave                   TradeRouteData;
	AFlareGame*                            Game;
	bool                                   IsFleetListLoaded;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FText GetTradeRouteName() const
	{
		return TradeRouteData.Name;
	}

	UFlareCompany* GetTradeRouteCompany() const
	{
		return TradeRouteCompany;
	}

	FName GetIdentifier() const
	{
		return TradeRouteData.Identifier;
	}

	FFlareTradeRouteSave* GetData()
	{
		return &TradeRouteData;
	}

	TArray<UFlareFleet*>& GetFleets();

    TArray<FFlareTradeRouteSectorSave>& GetSectors()
    {
        return TradeRouteData.Sectors;
    }

	FFlareTradeRouteSectorSave* GetSectorOrders(UFlareSimulatedSector* Sector);

	UFlareSimulatedSector* GetNextTradeSector(UFlareSimulatedSector* Sector);

    bool IsVisiting(UFlareSimulatedSector *Sector);
};
