#pragma once

#include "FlareCargoBay.generated.h"


struct FFlareCargo;
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
	virtual void Load(UFlareSimulatedSpacecraft* ParentSpacecraft, TArray<FFlareCargoSave>& Data);

	/** Save the factory to a save file */
	virtual TArray<FFlareCargoSave>* Save();


	/*----------------------------------------------------
	   Gameplay
	----------------------------------------------------*/

	/* If client is not null, restriction are used*/
	bool HasResources(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Client);

	/* If client is not null, restriction are used*/
	uint32 TakeResources(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Client);

	void DumpCargo(FFlareCargo* Cargo);

	/* If client is not null, restriction are used*/
	uint32 GiveResources(FFlareResourceDescription* Resource, uint32 Quantity, UFlareCompany* Client);

	void UnlockAll(bool IgnoreManualLock = true);

	bool LockSlot(FFlareResourceDescription* Resource, EFlareResourceLock::Type LockType, bool ManualLock);

	void SetSlotRestriction(int32 SlotIndex, EFlareResourceRestriction::Type RestrictionType);

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	TArray<FFlareCargoSave>                    CargoBayData;
	UFlareSimulatedSpacecraft*				   Parent;

	TArray<FFlareCargo>                        CargoBay;

	// Cache
	uint32								       CargoBayCount;
	uint32								       CargoBayBaseCapacity;
	AFlareGame*                                Game;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	uint32 GetSlotCount() const;

	uint32 GetCapacity() const;

	uint32 GetUsedCargoSpace() const;

	uint32 GetFreeCargoSpace() const;

	/* If client is not null, restriction are used*/
	uint32 GetResourceQuantity(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	/* If client is not null, restriction are used*/
	uint32 GetFreeSpaceForResource(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	bool HasRestrictions() const;

	FFlareCargo* GetSlot(uint32 Index);

	TArray<FFlareCargo>& GetSlots()
	{
		return CargoBay;
	}

	uint32 GetSlotCapacity() const;

	inline UFlareSimulatedSpacecraft* GetParent() const
	{
		return Parent;
	}

	/* If client is not null, restriction are used*/
	bool WantSell(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	/* If client is not null, restriction are used*/
	bool WantBuy(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	bool CheckRestriction(const FFlareCargo* Cargo, UFlareCompany* Client) const;
};
