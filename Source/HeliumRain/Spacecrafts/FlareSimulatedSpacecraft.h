
#pragma once

#include "Object.h"
#include "FlareSpacecraftInterface.h"
#include "Subsystems/FlareSimulatedSpacecraftDamageSystem.h"
#include "Subsystems/FlareSimulatedSpacecraftNavigationSystem.h"
#include "Subsystems/FlareSimulatedSpacecraftDockingSystem.h"
#include "Subsystems/FlareSimulatedSpacecraftWeaponsSystem.h"
#include "FlareSimulatedSpacecraft.generated.h"

class UFlareGame;
class UFlareSimulatedSector;
class UFlareFleet;
class UFlareCargoBay;

UCLASS()
class HELIUMRAIN_API UFlareSimulatedSpacecraft : public UObject, public IFlareSpacecraftInterface
{
	 GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the ship from a save file */
	virtual void Load(const FFlareSpacecraftSave& Data) override;

	/** Save the ship to a save file */
	virtual FFlareSpacecraftSave* Save() override;

	/** Get the parent company */
	virtual UFlareCompany* GetCompany() override;

	/** Get the ship size class */
	virtual EFlarePartSize::Type GetSize() override;

	virtual FName GetImmatriculation() const override;

	/** Check if this is a military ship */
	virtual bool IsMilitary() const override;

	/** Check if this is a station ship */
	virtual bool IsStation() const override;

	virtual bool CanFight() const override;

	virtual bool CanTravel() const override;


	/*----------------------------------------------------
		Sub system
	----------------------------------------------------*/

	virtual UFlareSimulatedSpacecraftDamageSystem* GetDamageSystem() const override;

	virtual UFlareSimulatedSpacecraftNavigationSystem* GetNavigationSystem() const override;

	virtual UFlareSimulatedSpacecraftDockingSystem* GetDockingSystem() const override;

	virtual UFlareSimulatedSpacecraftWeaponsSystem* GetWeaponsSystem() const override;

    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

	virtual void SetCurrentSector(UFlareSimulatedSector* Sector);

	virtual void SetCurrentFleet(UFlareFleet* Fleet)
	{
		CurrentFleet = Fleet;
	}

	virtual void SetSpawnMode(EFlareSpawnMode::Type SpawnMode);

	virtual bool CanBeFlown(FText& OutInfo) const override;

	virtual bool IsAssignedToSector() const override
	{
		return SpacecraftData.IsAssigned;
	}

	void AssignToSector(bool Assign) override;

	/** Set asteroid data from an asteroid save */
	void SetAsteroidData(FFlareAsteroidSave* Data);

	void SetDynamicComponentState(FName Identifier, float Progress = 0.f);

	void Upgrade()
	{
		SpacecraftData.Level++;
	}

	/*----------------------------------------------------
		Resources
	----------------------------------------------------*/

	bool CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft);

protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
	FFlareSpacecraftSave          SpacecraftData;
	FFlareSpacecraftDescription*  SpacecraftDescription;

	AFlareGame*                   Game;

	UFlareFleet*                  CurrentFleet;
	UFlareSimulatedSector*        CurrentSector;

	// Systems
	UPROPERTY()
	UFlareSimulatedSpacecraftDamageSystem*                  DamageSystem;
	UPROPERTY()
	UFlareSimulatedSpacecraftNavigationSystem*              NavigationSystem;
	UPROPERTY()
	UFlareSimulatedSpacecraftDockingSystem*                 DockingSystem;
	UPROPERTY()
	UFlareSimulatedSpacecraftWeaponsSystem*                 WeaponsSystem;

	UPROPERTY()
	TArray<UFlareFactory*>                                  Factories;

	UPROPERTY()
	UFlareCargoBay*                                         CargoBay;
public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

	inline AFlareGame* GetGame() const override
	{
		return Game;
	}	

	inline FText GetNickName() const override
	{
		return SpacecraftData.NickName;
	}

	/** Return null if traveling */
	inline UFlareSimulatedSector* GetCurrentSector()
	{
		return CurrentSector;
	}

	/** Return null if not in a fleet */
	inline UFlareFleet* GetCurrentFleet()
	{
		return CurrentFleet;
	}

	/** Return null if not in a trade route */
	inline UFlareTradeRoute* GetCurrentTradeRoute()
	{
		return (CurrentFleet ? CurrentFleet->GetCurrentTradeRoute() : NULL);
	}

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return SpacecraftDescription;
	}

	inline UFlareCargoBay* GetCargoBay() override
	{
		return CargoBay;
	}

	inline UFlareSectorInterface* GetCurrentSectorInterface() override
	{
		return Cast<UFlareSectorInterface>(CurrentSector);
	}

	inline TArray<UFlareFactory*>& GetFactories()
	{
		return Factories;
	}

	inline FVector GetSpawnLocation() const
	{
		return SpacecraftData.Location;
	}

	inline int32 GetLevel() const override
	{
		return SpacecraftData.Level;
	}

	inline bool IsConsumeResource(FFlareResourceDescription* Resource) const
	{
		return HasCapability(EFlareSpacecraftCapability::Consumer) && !SpacecraftData.SalesExcludedResources.Contains(Resource->Identifier);
	}

	int64 GetStationUpgradeFee() const
	{
		return SpacecraftData.Level * SpacecraftDescription->CycleCost.ProductionCost;
	}

};
