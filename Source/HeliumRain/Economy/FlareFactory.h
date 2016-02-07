
#pragma once
#include "FlareResource.h"
#include "../Data/FlareResourceCatalogEntry.h"
#include "../Game/FlareWorld.h"
#include "FlareFactory.generated.h"

class UFlareSimulatedSpacecraft;

/** Factory input or output resource */
USTRUCT()
struct FFlareFactoryResource
{
	GENERATED_USTRUCT_BODY()

	/** Resource */
	UPROPERTY(EditAnywhere, Category = Content)
	UFlareResourceCatalogEntry* Resource;

	/** Quantity for the resource */
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 Quantity;
};

/** Factory action type values */
UENUM()
namespace EFlareFactoryAction
{
	enum Type
	{
		CreateShip,
		GainTechnology,
		DiscoverSector
	};
}

/** Factory output action */
USTRUCT()
struct FFlareFactoryAction
{
	GENERATED_USTRUCT_BODY()
	/** Faction action. */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareFactoryAction::Type> Action;

	/** Generic identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Quantity for this action */
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 Quantity;
};



/** Factory description */
USTRUCT()
struct FFlareFactoryDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	/** Resource identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Time for 1 production cycle */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 ProductionTime;

	/** Cost for 1 production cycle */
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 ProductionCost;

	/** Input resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> InputResources;

	/** Output resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> OutputResources;

	/** Output actions */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryAction> OutputActions;

	/** Sun impact factory cost */
	UPROPERTY(EditAnywhere, Category = Content)
	bool NeedSun;
};

UCLASS()
class HELIUMRAIN_API UFlareFactory : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/*----------------------------------------------------
	   Save
	----------------------------------------------------*/

	/** Load the factory from a save file */
	virtual void Load(UFlareSimulatedSpacecraft* ParentSpacecraft, const FFlareFactoryDescription* Description, const FFlareFactorySave& Data);

	/** Save the factory to a save file */
	virtual FFlareFactorySave* Save();

	/*----------------------------------------------------
	   Gameplay
	----------------------------------------------------*/

	void Simulate();

	void TryBeginProduction();

	void Start();

	void Pause();

	void Stop();

	void SetInfiniteCycle(bool Mode);

	void SetCycleCount(uint32 Count);

	void SetOutputLimit(FFlareResourceDescription* Resource, uint32 MaxSlot);

	void ClearOutputLimit(FFlareResourceDescription* Resource);

	bool HasInputMoney();

	bool HasInputResources();

	bool HasOutputFreeSpace();

	bool HasCostReserved();

	void BeginProduction();

	void CancelProduction();

	void DoProduction();

	FFlareWorldEvent *GenerateEvent();

	void PerformCreateShipAction(const FFlareFactoryAction* Action);

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareFactorySave                        FactoryData;

	AFlareGame*                              Game;
	const FFlareFactoryDescription*          FactoryDescription;
	UFlareSimulatedSpacecraft*				 Parent;
	FFlareWorldEvent                         NextEvent;
	uint32                                   ScaledProductionCost;
	bool                                     ProductionCostInit;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline const FFlareFactoryDescription*GetDescription() const
	{
		return FactoryDescription;
	}

	inline UFlareSimulatedSpacecraft* GetParent()
	{
		return Parent;
	}

	uint32 GetProductionCost();

	int64 GetRemainingProductionDuration();

	TArray<FFlareFactoryResource> GetLimitedOutputResources();

	inline int64 GetProductedDuration()
	{
		return FactoryData.ProductedDuration;
	}

	inline bool IsActive()
	{
		return FactoryData.Active;
	}

	inline bool IsPaused()
	{
		return !FactoryData.Active && FactoryData.ProductedDuration > 0;
	}

	inline bool HasInfiniteCycle()
	{
		return FactoryData.InfiniteCycle;
	}

	inline uint32 GetCycleCount()
	{
		return FactoryData.CycleCount;
	}

	inline bool IsNeedProduction()
	{
		return HasInfiniteCycle() || GetCycleCount() > 0;
	}

	uint32 GetOutputLimit(FFlareResourceDescription* Resource);

	bool HasOutputLimit(FFlareResourceDescription* Resource);

	int32 GetInputResourcesCount();

	FFlareResourceDescription* GetInputResource(int32 Index);

	uint32 GetInputResourceQuantity(int32 Index);

	bool HasOutputResource(FFlareResourceDescription* Resource);

	bool HasInputResource(FFlareResourceDescription* Resource);

	/** Get tgis factory's cycle data */
	FText GetFactoryCycleInfo();

	/** Get this factory's status text */
	FText GetFactoryStatus();

};
