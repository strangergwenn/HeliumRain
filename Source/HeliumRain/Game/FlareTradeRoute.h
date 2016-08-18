#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareTradeRoute.generated.h"

class UFlareSimulatedSector;
class UFlareFleet;
struct FFlareResourceDescription;

/** Hostility status */
UENUM()
namespace EFlareTradeRouteOperation
{
	enum Type
	{
		Load,
		Unload,
		Buy,
		Sell,
		LoadOrBuy,
		UnloadOrSell,
	};
}

/** Trade route sector operation data */
USTRUCT()
struct FFlareTradeRouteSectorOperationSave
{
	GENERATED_USTRUCT_BODY()

	/** Operation type */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareTradeRouteOperation::Type> Type;

	/** Resource */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ResourceIdentifier;

	/** Max quantity before next operation. -1 Mean no limit */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 MaxQuantity;

	/** Max wait time before next operation. -1 Mean no limit */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 MaxWait;

};


/** Trade route sector data */
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
	TArray<FFlareTradeRouteSectorOperationSave> Operations;
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
	FName FleetIdentifier;

	/** Trade route sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTradeRouteSectorSave> Sectors;

	/** Trade route current target sector */
	UPROPERTY(EditAnywhere, Category = Save)
	FName TargetSectorIdentifier;

	/** Trade route current target sector */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CurrentOperationIndex;

	/** Trade route current operation progress*/
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CurrentOperationProgress;

	/** Trade route current operation duration*/
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CurrentOperationDuration;

	/** Trade route current pause status*/
	UPROPERTY(EditAnywhere, Category = Save)
	bool IsPaused;
};

UCLASS()
class HELIUMRAIN_API UFlareTradeRoute : public UObject
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

	void Simulate();

	UFlareSimulatedSector* UpdateTargetSector();

	bool ProcessCurrentOperation(FFlareTradeRouteSectorOperationSave* Operation);

	bool ProcessLoadOperation(FFlareTradeRouteSectorOperationSave* Operation);

	bool ProcessUnloadOperation(FFlareTradeRouteSectorOperationSave* Operation);

	int32 GetOperationRemainingQuantity(FFlareTradeRouteSectorOperationSave* Operation);

	bool IsOperationQuantityLimitReach(FFlareTradeRouteSectorOperationSave* Operation);

	void SetTargetSector(UFlareSimulatedSector* Sector);

	virtual void AssignFleet(UFlareFleet* Fleet);

	virtual void RemoveFleet(UFlareFleet* Fleet);

	virtual void AddSector(UFlareSimulatedSector* Sector);

	virtual void RemoveSector(UFlareSimulatedSector* Sector);

	virtual void AddSectorOperation(int32 SectorIndex, EFlareTradeRouteOperation::Type Type, FFlareResourceDescription* Resource);

	virtual void RemoveSectorOperation(int32 SectorIndex, int32 OperationIndex);

	void DeleteOperation(FFlareTradeRouteSectorOperationSave* Operation);

	FFlareTradeRouteSectorOperationSave* MoveOperationUp(FFlareTradeRouteSectorOperationSave* Operation);

	FFlareTradeRouteSectorOperationSave* MoveOperationDown(FFlareTradeRouteSectorOperationSave* Operation);

	/** Remove all fleet from the trade route and delete it. */
	virtual void Dissolve();

	virtual void InitFleetList();

	void SkipCurrentOperation();

    virtual void SetTradeRouteName(FText NewName)
    {
        TradeRouteData.Name = NewName;
    }

	void SetPaused(bool Paused)
	{
		TradeRouteData.IsPaused = Paused;
	}

protected:

	UFlareFleet*                  TradeRouteFleet;

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

	UFlareFleet* GetFleet();

    TArray<FFlareTradeRouteSectorSave>& GetSectors()
    {
        return TradeRouteData.Sectors;
    }

	FFlareTradeRouteSectorSave* GetSectorOrders(UFlareSimulatedSector* Sector);

	UFlareSimulatedSector* GetNextTradeSector(UFlareSimulatedSector* Sector);

    bool IsVisiting(UFlareSimulatedSector *Sector);

    int32 GetSectorIndex(UFlareSimulatedSector *Sector);

	UFlareSimulatedSector* GetTargetSector() const;

	FFlareTradeRouteSectorOperationSave* GetActiveOperation();

	bool IsPaused()
	{
		return TradeRouteData.IsPaused;
	}
};
