
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

	void DumpCargo(FFlareCargo* Cargo);

	uint32 GiveResources(FFlareResourceDescription* Resource, uint32 Quantity);

	void UnlockAll();

	bool LockSlot(FFlareResourceDescription* Resource);

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	TArray<FFlareCargoSave>                    CargoBayData;
	IFlareSpacecraftInterface*				   Parent;

	TArray<FFlareCargo>                        CargoBay;

	// Cache
	uint32								       CargoBayCount;
	uint32								       CargoBayCapacity;
	AFlareGame*                                Game;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	uint32 GetSlotCount() const;

	uint32 GetCapacity() const;

	uint32 GetUsedCargoSpace() const;

	uint32 GetFreeCargoSpace() const;

	uint32 GetResourceQuantity(FFlareResourceDescription* Resource) const;

	uint32 GetFreeSpaceForResource(FFlareResourceDescription* Resource) const;

	FFlareCargo* GetSlot(uint32 Index);

	TArray<FFlareCargo>& GetSlots()
	{
		return CargoBay;
	}

	uint32 GetSlotCapacity() const
	{
		return CargoBayCapacity;
	}

	inline IFlareSpacecraftInterface* GetParent() const
	{
		return Parent;
	}


};
