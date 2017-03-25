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
	bool HasResources(FFlareResourceDescription* Resource, int32 Quantity, UFlareCompany* Client);

	/* If client is not null, restriction are used*/
	int32 TakeResources(FFlareResourceDescription* Resource, int32 Quantity, UFlareCompany* Client);

	void DumpCargo(FFlareCargo* Cargo);

	/* If client is not null, restriction are used*/
	int32 GiveResources(FFlareResourceDescription* Resource, int32 Quantity, UFlareCompany* Client);

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
	int32								       CargoBayCount;
	int32								       CargoBayBaseCapacity;
	AFlareGame*                                Game;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	int32 GetSlotCount() const;

	int32 GetCapacity() const;

	int32 GetFreeSlotCount() const;

	int32 GetUsedCargoSpace() const;

	int32 GetFreeCargoSpace() const;

	/* If client is not null, restriction are used*/
	int32 GetResourceQuantity(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	/* If client is not null, restriction are used*/
	int32 GetFreeSpaceForResource(FFlareResourceDescription* Resource, UFlareCompany* Client) const;

	bool HasRestrictions() const;

	FFlareCargo* GetSlot(int32 Index);

	TArray<FFlareCargo>& GetSlots()
	{
		return CargoBay;
	}

	int32 GetSlotCapacity() const;

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
