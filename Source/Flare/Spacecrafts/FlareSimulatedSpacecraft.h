
#pragma once

#include "Object.h"
#include "FlareSpacecraftInterface.h"
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

	virtual FString GetImmatriculation() const override;

	/** Check if this is a military ship */
	virtual bool IsMilitary() override;

	/** Check if this is a station ship */
	virtual bool IsStation() override;

	/*----------------------------------------------------
		Sub system
	----------------------------------------------------*/

	virtual UFlareSpacecraftDamageSystem* GetDamageSystem() const override;

	virtual UFlareSpacecraftNavigationSystem* GetNavigationSystem() const override;

	virtual UFlareSpacecraftDockingSystem* GetDockingSystem() const override;

	virtual UFlareSpacecraftWeaponsSystem* GetWeaponsSystem() const override;

    /*----------------------------------------------------
        Gameplay
    ----------------------------------------------------*/

	virtual void SetCurrentSector(UFlareSimulatedSector* Sector)
	{
		CurrentSector = Sector;
	}

	virtual void SetCurrentFleet(UFlareFleet* Fleet)
	{
		CurrentFleet = Fleet;
	}

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

public:

    /*----------------------------------------------------
        Getters
    ----------------------------------------------------*/

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
};
