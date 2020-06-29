#pragma once

#include <UObject/Object.h>
#include "../FlareGameTypes.h"
#include "FlareAIBehavior.generated.h"


class UFlareCompany;
class UFlareScenarioTools;
struct FFlareCelestialBody;

UCLASS()
class HELIUMRAIN_API UFlareAIBehavior : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public API
	----------------------------------------------------*/

	/** Load the company behavior */
	virtual void Load(UFlareCompany* ParentCompany);

	virtual void Simulate();

	void UpdateDiplomacy();

protected:

	/*----------------------------------------------------
		Internal subsystems
	----------------------------------------------------*/

	virtual void SimulateGeneralBehavior();

	virtual void SimulatePirateBehavior();



	void GenerateAffilities();

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/


	void SetResourceAffilities(float Value);

	void SetResourceAffility(FFlareResourceDescription* Resource, float Value);

	void SetSectorAffilities(float Value);

	void SetSectorAffility(UFlareSimulatedSector* Sector, float Value);

	void SetSectorAffilitiesByMoon(FFlareCelestialBody *CelestialBody, float Value);

protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

	// Gameplay data
	UFlareCompany*			               Company;
	AFlareGame*                            Game;
	UFlareScenarioTools*                   ST;


	TMap<FFlareResourceDescription*, float> ResourceAffilities;
	TMap<UFlareSimulatedSector*, float> SectorAffilities;

public:

	/*----------------------------------------------------
		Public Data
	----------------------------------------------------*/

	float StationCapture;
	float TradingBuy;
	float TradingSell;
	float TradingBoth;
	float ShipyardAffility;
	float ConsumerAffility;
	float MaintenanceAffility;
	float BudgetTechnologyWeight;
	float BudgetMilitaryWeight;
	float BudgetStationWeight;
	float BudgetTradeWeight;
	float ArmySize;

	float AttackThreshold;
	float RetreatThreshold;

	/* Amount of caution gain at each defeat of loose at each victory */
	float DefeatAdaptation;

	float ConfidenceTarget;
	float DeclareWarConfidence;
	float RequestPeaceConfidence;
	float PayTributeConfidence;

	float DiplomaticReactivity;

	float PacifismIncrementRate;
	float PacifismDecrementRate;

	bool ProposeTributeToPlayer = false;


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	float GetBudgetWeight(EFlareBudget::Type Budget);

	float GetSectorAffility(UFlareSimulatedSector* Sector);
	float GetResourceAffility(FFlareResourceDescription* Resource);

	float GetAttackThreshold();

};

