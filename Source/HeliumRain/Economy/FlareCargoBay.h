
#pragma once
#include "FlareCargoBay.generated.h"

struct FFlareResourceDescription;

UCLASS()
class HELIUMRAIN_API UFlareCargoBay : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/*----------------------------------------------------
	   Save
	----------------------------------------------------*/

	/** Load the factory from a save file */
	virtual void Load(IFlareSpacecraftInterface* ParentSpacecraft, TArray<FFlareCargoSave>& Data);

	/** Save the factory to a save file */
	virtual TArray<FFlareCargoSave>* Save();

	/*----------------------------------------------------
	   Gameplay
	----------------------------------------------------*/

	bool HasResources(FFlareResourceDescription* Resource, uint32 Quantity);

	uint32 TakeResources(FFlareResourceDescription* Resource, uint32 Quantity);

	uint32 GiveResources(FFlareResourceDescription* Resource, uint32 Quantity);

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	TArray<FFlareCargoSave>               CargoBayData;
	IFlareSpacecraftInterface*				 Parent;

	TArray<FFlareCargo>                      CargoBay;

	// Cache
	uint32								CargoBayCount;
	uint32								CargoBayCapacity;
	AFlareGame*                              Game;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline IFlareSpacecraftInterface* GetParent()
	{
		return Parent;
	}

	uint32 GetSlotCount();

	FFlareCargo* GetSlot(uint32 Index);

	uint32 GetCapacity();

	uint32 GetSlotCapacity()
	{
		return CargoBayCapacity;
	}

	uint32 GetResourceQuantity(FFlareResourceDescription* Resource);

	uint32 GetFreeSpaceForResource(FFlareResourceDescription* Resource);




};
