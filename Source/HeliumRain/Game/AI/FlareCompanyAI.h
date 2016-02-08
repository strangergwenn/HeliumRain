#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"
#include "FlareCompanyAI.generated.h"

class UFlareCompany;


UCLASS()
class HELIUMRAIN_API UFlareCompanyAI : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the company AI from a save file */
	virtual void Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data);

	/** Save the company AI to a save file */
	virtual FFlareCompanyAISave* Save();




	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void Simulate();

protected:

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	virtual void UnassignShipsFromSector(UFlareSimulatedSector* Sector, uint32 Capacity);

	virtual void AssignShipsToSector(UFlareSimulatedSector* Sector, uint32 Capacity);

	protected:

	UFlareCompany*			               Company;
	FFlareCompanyAISave					   AIData;
	AFlareGame*                            Game;

	public:

	/*----------------------------------------------------
	  Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
	  return Game;
	}

};

