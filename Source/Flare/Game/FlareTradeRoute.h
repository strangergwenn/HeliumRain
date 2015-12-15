#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareTradeRoute.generated.h"

class UFlareSimulatedSector;
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

	/** Trade route ships */
	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FName> ShipImmatriculations;

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

	virtual void AddShip(UFlareSimulatedSpacecraft* Ship);

	virtual void RemoveShip(UFlareSimulatedSpacecraft* Ship);

	virtual void AddSector(UFlareSimulatedSector* Sector);

	virtual void SetSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 QuantityToLeft);

	virtual void ClearSectorLoadOrder(int32 SectorIndex, FFlareResourceDescription* Resource);

	virtual void SetSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource, uint32 LimitQuantity);

	virtual void ClearSectorUnloadOrder(int32 SectorIndex, FFlareResourceDescription* Resource);


	/** Remove all ship from the trade route and delete it. */
	virtual void Dissolve();

	virtual void InitShipList();


protected:

	TArray<UFlareSimulatedSpacecraft*>     TradeRouteShips;

	UFlareCompany*			               TradeRouteCompany;
	FFlareTradeRouteSave                   TradeRouteData;
	AFlareGame*                            Game;
	bool                                   IsShipListLoaded;

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

	TArray<UFlareSimulatedSpacecraft*>& GetShips();
};
