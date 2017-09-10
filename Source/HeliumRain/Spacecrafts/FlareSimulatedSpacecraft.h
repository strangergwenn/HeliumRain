#pragma once

#include "Object.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Economy/FlareResource.h"
#include "Subsystems/FlareSpacecraftDockingSystem.h"
#include "FlareSimulatedSpacecraft.generated.h"


class UFlareSimulatedSector;
class UFlareFleet;
class UFlareCargoBay;
class UFlareFactory;

UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraft : public UObject
{
	 GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the ship from a save file */
	virtual void Load(const FFlareSpacecraftSave& Data);

	/** Reload */
	void Reload();

	/** Save the ship to a save file */
	virtual FFlareSpacecraftSave* Save();

	/** Get the parent company */
	virtual UFlareCompany* GetCompany() const;

	/** Get the ship size class */
	virtual EFlarePartSize::Type GetSize();

	virtual FName GetImmatriculation() const;

	/** Check if this is a military ship */
	virtual bool IsMilitary() const;

	/** Check if this is a station ship */
	virtual bool IsStation() const;

	virtual bool CanFight() const;

	virtual bool CanTravel() const;


	/*----------------------------------------------------
		Sub system
	----------------------------------------------------*/

	virtual class UFlareSimulatedSpacecraftDamageSystem* GetDamageSystem() const;

	virtual class UFlareSimulatedSpacecraftWeaponsSystem* GetWeaponsSystem() const;

    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

	virtual void SetCurrentSector(UFlareSimulatedSector* Sector);

	virtual void SetCurrentFleet(UFlareFleet* Fleet)
	{
		CurrentFleet = Fleet;
	}

	virtual void SetSpawnMode(EFlareSpawnMode::Type SpawnMode);

	virtual bool CanBeFlown(FText& OutInfo) const;

	/** Set asteroid data from an asteroid save */
	void SetAsteroidData(FFlareAsteroidSave* Data);

	/** Set parent object name */
	void SetActorAttachment(FName ActorName);

	/** Set parent station name and connector */
	void SetComplexStationAttachment(FName StationName, FName ConnectorName);

	void SetDynamicComponentState(FName Identifier, float Progress = 0.f);

	void Upgrade();

	void SetActiveSpacecraft(AFlareSpacecraft* Spacecraft)
	{
		if(Spacecraft)
		{
			FCHECK(ActiveSpacecraft == NULL || ActiveSpacecraft == Spacecraft);
		}
		ActiveSpacecraft = Spacecraft;
	}

	void ForceUndock();

	void SetTrading(bool Trading);

	void SetIntercepted(bool Intercept);

	void Repair();

	void RecoveryRepair();

	void Stabilize();

	void Refill();

	void SetReserve(bool InReserve);

	/** This ship was harpooned */
	void SetHarpooned(UFlareCompany* OwnerCompany);

	bool IsHarpooned()
	{
		return SpacecraftData.HarpoonCompany != NAME_None;
	}

	void SetDestroyed(bool Destroyed)
	{
		SpacecraftData.IsDestroyed = Destroyed;
	}

	UFlareCompany* GetHarpoonCompany();

	void ResetCapture(UFlareCompany* Company = NULL);

	bool TryCapture(UFlareCompany* Company, int32 CapturePoint);

	bool UpgradePart(FFlareSpacecraftComponentDescription* NewPartDesc, int32 WeaponGroupIndex);

	bool CanUpgrade(EFlarePartType::Type Type);

	int64 GetUpgradeCost(FFlareSpacecraftComponentDescription* NewPart, FFlareSpacecraftComponentDescription* OldPart);

	FFlareSpacecraftComponentDescription* GetCurrentPart(EFlarePartType::Type Type, int32 WeaponGroupIndex);


	void FinishConstruction();

	void OrderRepairStock(float FS);
	void OrderRefillStock(float FS);

	bool NeedRefill();

	bool IsShipyard();

	void AutoFillConstructionCargoBay();

	bool CanScrapStation() const;

	TMap<FFlareResourceDescription*, int32> ComputeScrapResources() const;


	/*----------------------------------------------------
		Complexes
	----------------------------------------------------*/

	/** Is this station a complex */
	bool IsComplex() const;

	/** Is this station part of a complex, and not a standalone station */
	bool IsComplexElement() const;

	/** Get available connectors */
	TArray<FFlareDockingInfo>& GetStationConnectors();

	/** Get a specific connector */
	FFlareDockingInfo* GetStationConnector(FName Name);

	/** Add an element to this complex */
	void RegisterComplexElement(FFlareConnectionSave ConnectionData);

	/** Remove an element from this complex */
	void UnregisterComplexElement(UFlareSimulatedSpacecraft* Element);

	UFlareSimulatedSpacecraft* GetComplexMaster() const;

	TArray<UFlareSimulatedSpacecraft*> const& GetComplexChildren() const;

	void RegisterComplexMaster(UFlareSimulatedSpacecraft* master);

	/** Is this slot usable for special complex elements ? */
	static bool IsSpecialComplexSlot(FName ConnectorName);


	/*----------------------------------------------------
		Resources
	----------------------------------------------------*/

	bool CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft, FText& Reason);

	FFlareResourceUsage GetResourceUseType(FFlareResourceDescription* Resource);
	void LockResources();

	void ComputeConstructionCargoBaySize(int32& CargoBaySlotCapacity, int32& CargoBayCount);

	void ComputeProductionCargoBaySize(int32& CargoBaySlotCapacity, int32& CargoBayCount);

	int64 GetResourcePrice(FFlareResourceDescription* Resource, EFlareResourcePriceContext::Type PriceContext, int32 Age = 0);

	/*----------------------------------------------------
		Shipyard
	----------------------------------------------------*/

	bool ShipyardOrderShip(UFlareCompany* OrderCompany, FName ShipIdentifier);

	void CancelShipyardOrder(int32 OrderIndex);

	TArray<FFlareShipyardOrderSave>& GetShipyardOrderQueue();

	TArray<FFlareShipyardOrderSave> GetOngoingProductionList();

	FText GetNextShipyardOrderStatus();

	UFlareFactory* GetCompatibleIdleShipyardFactory(FName ShipIdentifier);

	void UpdateShipyardProduction();

	bool CanOrder(const FFlareSpacecraftDescription* ShipDescription, UFlareCompany* OrderCompany);

	const FFlareProductionData& GetCycleDataForShipClass(FName ShipIdentifier);

	int32 GetShipProductionTime(FName ShipIdentifier);

	int32 GetEstimatedQueueAndProductionDuration(FName ShipIdentifier, int32 OrderIndex);

	bool IsShipyardMissingResources();

	FText GetShipCost(FName ShipIdentifier);

	bool IsAllowExternalOrder();
	void SetAllowExternalOrder(bool Allow);

	const FFlareProductionData* GetNextOrderShipProductionData(EFlarePartSize::Type Size);

	void SetNickName(FText NewName)
	{
		SpacecraftData.NickName = NewName;
	}


protected:

	void RemoveCapturePoint(FName CompanyIdentifier, int32 CapturePoint);

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
	FFlareSpacecraftSave          SpacecraftData;
	FFlareSpacecraftDescription*  SpacecraftDescription;

	UFlareCompany*				  Company;
	AFlareGame*                   Game;

	AFlareSpacecraft*             ActiveSpacecraft;

	UFlareFleet*                  CurrentFleet;
	UFlareSimulatedSector*        CurrentSector;

	// Systems
	UPROPERTY()
	class UFlareSimulatedSpacecraftDamageSystem*                  DamageSystem;
	UPROPERTY()
	class UFlareSimulatedSpacecraftWeaponsSystem*                 WeaponsSystem;

	UPROPERTY()
	TArray<UFlareFactory*>                                  Factories;

	UPROPERTY()
	UFlareCargoBay*                                         ProductionCargoBay;

	UPROPERTY()
	UFlareCargoBay*                                         ConstructionCargoBay;

	TArray<FFlareDockingInfo>                               ConnectorSlots;

	UFlareSimulatedSpacecraft*								ComplexMaster;
	TArray<UFlareSimulatedSpacecraft*>						ComplexChildren;

public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}	

	inline bool IsActive() const
	{
		return ActiveSpacecraft != NULL;
	}

	inline AFlareSpacecraft* GetActive() const
	{
		return ActiveSpacecraft;
	}

	inline FFlareSpacecraftSave& GetData()
	{
		return SpacecraftData;
	}

	inline FFlareSpacecraftSave const& GetDataConst() const
	{
		return SpacecraftData;
	}

	inline FText GetNickName() const
	{
		return SpacecraftData.NickName;
	}

	/** Return null if traveling */
	inline UFlareSimulatedSector* GetCurrentSector() const
	{
		return CurrentSector;
	}

	/** Return null if not in a fleet */
	inline UFlareFleet* GetCurrentFleet() const
	{
		return CurrentFleet;
	}

	/** Return null if not in a trade route */
	inline UFlareTradeRoute* GetCurrentTradeRoute( )const
	{
		return (CurrentFleet ? CurrentFleet->GetCurrentTradeRoute() : NULL);
	}

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return SpacecraftDescription;
	}

	inline UFlareCargoBay* GetProductionCargoBay() const
	{
		return ProductionCargoBay;
	}

	inline UFlareCargoBay* GetConstructionCargoBay() const
	{
		return ConstructionCargoBay;
	}

	UFlareCargoBay* GetActiveCargoBay() const;

	TArray<UFlareFactory*>& GetFactories();

	inline FVector GetSpawnLocation() const
	{
		return SpacecraftData.Location;
	}

	inline int32 GetLevel() const
	{
		return SpacecraftData.Level;
	}

	inline bool IsTrading() const
	{
		return SpacecraftData.IsTrading;
	}

	inline bool IsIntercepted() const
	{
		return SpacecraftData.IsIntercepted;
	}


	inline float GetRepairStock() const
	{
		return SpacecraftData.RepairStock;
	}

	inline float GetRefillStock() const
	{
		return SpacecraftData.RefillStock;
	}

	inline bool IsReserve() const
	{
		return SpacecraftData.IsReserve;
	}

	inline bool IsConsumeResource(FFlareResourceDescription* Resource) const
	{
		return HasCapability(EFlareSpacecraftCapability::Consumer) && !SpacecraftData.SalesExcludedResources.Contains(Resource->Identifier);
	}

	int64 GetStationUpgradeFee() const
	{
		return SpacecraftDescription->CycleCost.ProductionCost;
	}

	bool HasCapability(EFlareSpacecraftCapability::Type Capability) const;

	EFlareHostility::Type GetPlayerWarState() const;

	int32 GetCapturePoint(UFlareCompany* Company) const;

	bool IsBeingCaptured() const;

	int32 GetCapturePointThreshold() const;

	float GetStationEfficiency();

	int32 GetCombatPoints(bool ReduceByDamage);

	bool IsDestroyed()
	{
		return SpacecraftData.IsDestroyed;
	}

	bool IsUnderConstruction(bool local = false) const;

	bool IsPlayerShip();

	bool IsRepairing();

	bool IsRefilling();

	int32 GetRepairDuration();
	int32 GetRefillDuration();

	bool IsLastPlayerShip();

	bool IsResponsible(EFlareDamage::Type DamageType);

};

