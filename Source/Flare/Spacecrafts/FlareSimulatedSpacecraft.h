
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

UCLASS()
class FLARE_API UFlareSimulatedSpacecraft : public UObject, public IFlareSpacecraftInterface
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

	virtual bool CanBeFlown() const override
	{
		return !IsStation() && (CurrentSector != NULL);
	}

	/*----------------------------------------------------
		Resources
	----------------------------------------------------*/

	bool HasResources(FFlareResourceDescription* Resource, uint32 Quantity);

	uint32 TakeResources(FFlareResourceDescription* Resource, uint32 Quantity);

	uint32 GiveResources(FFlareResourceDescription* Resource, uint32 Quantity);

	bool CanTradeWith(UFlareSimulatedSpacecraft* OtherSpacecraft);

protected:

    /*----------------------------------------------------
        Protected data
    ----------------------------------------------------*/

    // Gameplay data
	FFlareSpacecraftSave          SpacecraftData;
	FFlareSpacecraftDescription*  SpacecraftDescription;

	AFlareGame*                   Game;

	UFlareFleet*                   CurrentFleet;
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
	TArray<FFlareCargo>                                     CargoBay;
public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}	

	inline FName GetNickName() const override
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

	inline FFlareSpacecraftDescription* GetDescription() const
	{
		return SpacecraftDescription;
	}

	TArray<FFlareCargo>& GetCargoBay()
	{
		return CargoBay;
	}

	TArray<UFlareFactory*>& GetFactories()
	{
		return Factories;
	}

	inline FVector GetSpawnLocation() const
	{
		return SpacecraftData.Location;
	}

};
