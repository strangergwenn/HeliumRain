
#pragma once
#include "FlareResource.h"
#include "../Data/FlareResourceCatalogEntry.h"
#include "FlareFactory.generated.h"

class UFlareSimulatedSpacecraft;

/** Spacecraft factory save data */
USTRUCT()
struct FFlareFactorySave
{
	GENERATED_USTRUCT_BODY()

	/** Factory is active */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Active;

	/** Money locked by the factory */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 CostReserved;

	/** Timestamp of last production cycle */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 ProductionBeginTime;
};

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
	int32 ProductionCost;

	/** Input resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> InputResources;

	/** Output resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> OutputResources;


	// TODO add output actions

};

UCLASS()
class FLARE_API UFlareFactory : public UObject
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

	void Simulate(long Duration);

	bool HasInputMoney();

	bool HasInputResources();

	bool HasOutputFreeSpace();

	void BeginProduction(int64 SimulatedTime);

	void CancelProduction();

	void DoProduction();

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareFactorySave                        FactoryData;

	AFlareGame*                              Game;
	const FFlareFactoryDescription*          FactoryDescription;
	UFlareSimulatedSpacecraft*				 Parent;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareSimulatedSpacecraft* GetParent()
	{
		return Parent;
	}
};
