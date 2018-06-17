
#include "FlareAIBehavior.h"
#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../FlareGame.h"
#include "../FlareCompany.h"
#include "../FlareScenarioTools.h"

#include "../../Quests/FlareQuest.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


//#define DEBUG_AI_NO_WAR

DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior Load"), STAT_FlareAIBehavior_Load, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior Simulate"), STAT_FlareAIBehavior_Simulate, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior SimulateGeneralBehavior"), STAT_FlareAIBehavior_SimulateGeneralBehavior, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior SimulatePirateBehavior"), STAT_FlareAIBehavior_SimulatePirateBehavior, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior UpdateDiplomacy"), STAT_FlareAIBehavior_UpdateDiplomacy, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "UFlareAIBehavior"


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareAIBehavior::UFlareAIBehavior(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareAIBehavior::Load(UFlareCompany* ParentCompany)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_Load);

	if(!Company)
	{
		Company = ParentCompany;
		check(Company);
		Game = Company->GetGame();
		ST = Game->GetScenarioTools();
		check(ST);

		GenerateAffilities();

		ProposeTributeToPlayer = false;
	}
}

inline static bool CompanyValueComparator(const UFlareCompany& ip1, const UFlareCompany& ip2)
{
	return ip1.GetCompanyValue().TotalValue < ip2.GetCompanyValue().TotalValue;
}


void UFlareAIBehavior::Simulate()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_Simulate);

	// See how the player is doing
	TArray<UFlareCompany*> SortedCompany = Game->GetGameWorld()->GetCompanies();
	SortedCompany.Sort(&CompanyValueComparator);
	int32 PlayerCompanyIndex = SortedCompany.IndexOfByKey(GetGame()->GetPC()->GetCompany());
	int32 PlayerArmy = GetGame()->GetPC()->GetCompany()->GetCompanyValue().ArmyCurrentCombatPoints;

	// Pirates hate you
	if (PlayerCompanyIndex > 0 && Company == ST->Pirates)
	{
		Company->GivePlayerReputation(-0.5);
	}

	// Competitors hate you more if you're doing well
	float ReputationLoss = PlayerCompanyIndex / 30.f;
	if (PlayerArmy > 0 && Company != ST->AxisSupplies && Company->GetPlayerReputation() > ReputationLoss)
	{
		Company->GivePlayerReputation(-ReputationLoss);
	}

	// Simulate the day
	if (Company == ST->Pirates)
	{
		SimulatePirateBehavior();
	}
	else
	{
		SimulateGeneralBehavior();
	}

}

void UFlareAIBehavior::SimulateGeneralBehavior()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_SimulateGeneralBehavior);

	// First make cargo evasion to avoid them to lock themselve trading
	Company->GetAI()->CargosEvasion();

	// Repair and refill ships and stations
	Company->GetAI()->RepairAndRefill();

	Company->GetAI()->ProcessBudget(Company->GetAI()->AllBudgets);

	// Buy ships
	//Company->GetAI()->UpdateShipAcquisition(IdleCargoCapacity);

	Company->GetAI()->UpdateMilitaryMovement();
}

void UFlareAIBehavior::UpdateDiplomacy()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_UpdateDiplomacy);

	TArray<UFlareCompany*> OtherCompanies = Company->GetOtherCompanies(true); // true for Shuffle
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	int32 MaxCombatPoint = 0;
	UFlareCompany* MaxCombatPointCompany = NULL;

	for (UFlareCompany* OtherCompany : GetGame()->GetGameWorld()->GetCompanies())
	{
		struct CompanyValue Value = OtherCompany->GetCompanyValue();

		if (MaxCombatPoint < Value.ArmyCurrentCombatPoints)
		{
			MaxCombatPoint = Value.ArmyCurrentCombatPoints;
			MaxCombatPointCompany = OtherCompany;
		}
	}

	float MaxCombatPointRatio = float(MaxCombatPoint) / GetGame()->GetGameWorld()->GetTotalWorldCombatPoint();

	//FLOGV("MaxCombatPointRatio %f", MaxCombatPointRatio);
	//FLOGV("MaxCombatPoint %d", MaxCombatPoint);
	//FLOGV("TotalWorldCombatPoint %d", TotalWorldCombatPoint);

	
	// If a non-player company is not far from half world power, a global alliance is formed
	if (MaxCombatPointCompany != Company && MaxCombatPointCompany != PlayerCompany && MaxCombatPointRatio > 0.3 && MaxCombatPoint > 500)
	{
		bool NewWar = false;

		for (UFlareCompany* OtherCompany : OtherCompanies)
		{
			if (MaxCombatPointCompany == OtherCompany && Company->GetCompanyValue().ArmyCurrentCombatPoints > 0)
			{
				if (Company->GetWarState(OtherCompany) != EFlareHostility::Hostile)
				{
					NewWar = true;
				}

				Company->GetAI()->GetData()->Pacifism = 0;
				Company->SetHostilityTo(OtherCompany, true);
			}
			else
			{
				Company->SetHostilityTo(OtherCompany, false);
			}
		}

		// Notify the player that this is happening
		if (NewWar)
		{
			GetGame()->GetPC()->Notify(LOCTEXT("GlobalWar", "Global War"),
				FText::Format(LOCTEXT("AIStartGlobalWarInfo", "All companies are now allied to stop the militarization of {0}."),
					MaxCombatPointCompany->GetCompanyName()),
				FName("global-war"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Leaderboard);
		}

		return;
	}

	// Not global war
	if(Company->GetAI()->GetData()->Pacifism >= 100)
	{
		// Want peace
		for (UFlareCompany* OtherCompany : OtherCompanies)
		{
			Company->SetHostilityTo(OtherCompany, false);

			TArray<UFlareCompany*> Allies;

			if (Company->GetWarState(OtherCompany) == EFlareHostility::Hostile && Company->GetConfidenceLevel(OtherCompany, Allies) < PayTributeConfidence)
			{
				if (OtherCompany == PlayerCompany)
				{
					ProposeTributeToPlayer = true;
				}
				else
				{
					Company->PayTribute(OtherCompany, false);
				}
			}
		}
	}
	else if (Company->GetAI()->GetData()->Pacifism == 0)
	{
		//Want war

		if (Company->GetWarCount(PlayerCompany) == 0)
		{
			// Don't fight if already at war

			// Fill target company sorted by weight (only with weight higther than mine)
			TArray<TPair<UFlareCompany*, float>> TargetCompanies;

			for (UFlareCompany* OtherCompany : OtherCompanies)
			{
				if (Company->IsAtWar(OtherCompany))
				{
					continue;
				}


				//FLOGV("- %s Weight=%f", *OtherCompany->GetCompanyName().ToString(), Weight);
				//FLOGV("-    GetConfidenceLevel=%f", Company->GetConfidenceLevel(OtherCompany));

				if(Company->WantWarWith(OtherCompany))
				{
					//FLOGV("%s add %s as target", *Company->GetCompanyName().ToString(), *OtherCompany->GetCompanyName().ToString());

					TargetCompanies.Add(TPairInitializer<UFlareCompany*, float>(OtherCompany, OtherCompany->ComputeCompanyDiplomaticWeight()));
				}
			}

			TargetCompanies.Sort([&](const TPair<UFlareCompany*, float>& Lhs, const TPair<UFlareCompany*, float>& Rhs){

				if(Lhs.Key == PlayerCompany)
				{
					return true;
				}

				if(Rhs.Key == PlayerCompany)
				{
					return false;
				}

				return Lhs.Value > Rhs.Value;
			});


			auto DeclareWarConfidenceMean = [](TArray<UFlareCompany*> &Allies)
			{
				float DeclareWarConfidenceSum = 0;
				float DeclareWarConfidenceCount = 0;

				for (UFlareCompany* Ally : Allies)
				{
					DeclareWarConfidenceSum += Ally->GetAI()->GetBehavior()->DeclareWarConfidence;
					++DeclareWarConfidenceCount;
				}

				return DeclareWarConfidenceSum / DeclareWarConfidenceCount;
			};

			// For each target
			// find allies list
			// compute allies confidence level
			// compute mean of DeclareWarConfidence
			// if confidente, rand skip else declare war with all allies

			int i = 0;
			for(TPair<UFlareCompany*, float> Target : TargetCompanies)
			{
				if(i >= 3)
				{
					break;
				}
				++i;

				UFlareCompany* TargetCompany = Target.Key;

				//FLOGV("Target for %s: %s", *Company->GetCompanyName().ToString(), *TargetCompany->GetCompanyName().ToString());

				TArray<UFlareCompany*> Allies;

				float Confidence = Company->GetConfidenceLevel(TargetCompany, Allies);

				if(FMath::FRand() <= 0.2f)
				{
					continue;
				}

				if(Confidence > DeclareWarConfidenceMean(Allies))
				{
					FLOGV("Declare alliance agains %s with:", *TargetCompany->GetCompanyName().ToString());
					for (UFlareCompany* Ally : Allies)
					{
						FLOGV("- %s", *Ally->GetCompanyName().ToString());
						Ally->SetHostilityTo(TargetCompany, true);

					}
				}
			}
		}
	}
}

void UFlareAIBehavior::SimulatePirateBehavior()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_SimulatePirateBehavior);

	// Repair and refill ships and stations
	Company->GetAI()->RepairAndRefill();

	// First make cargo evasion to avoid them to lock themselve trading
	Company->GetAI()->CargosEvasion();

	Company->GetAI()->ProcessBudget(Company->GetAI()->AllBudgets);

	Company->GetAI()->UpdateMilitaryMovement();

}

void UFlareAIBehavior::GenerateAffilities()
{
	// Reset resource affilities
	ResourceAffilities.Empty();
	
	// Default behavior
	SetResourceAffilities(1.f);

	// If above 5, will become exclusive
	SetSectorAffilities(1.f);

	StationCapture = 1.f;
	TradingBuy = 1.f;
	TradingSell = 1.f;
	TradingBoth = 0.5f;

	ShipyardAffility = 1.0;
	ConsumerAffility = 0.5;
	MaintenanceAffility = 0.1;

	// Budget
	BudgetTechnologyWeight = 0.2;
	BudgetMilitaryWeight = 0.5;
	BudgetStationWeight = 1.0;
	BudgetTradeWeight = 1.0;

	ConfidenceTarget = -0.1;
	DeclareWarConfidence = 0.2;
	RequestPeaceConfidence = -0.5;
	PayTributeConfidence = -0.8;

	AttackThreshold = 0.99;
	RetreatThreshold = 0.5;
	DefeatAdaptation = 0.01;

	ArmySize = 5.0;
	DiplomaticReactivity = 1;

	PacifismIncrementRate = 0.8;
	PacifismDecrementRate = 0.6;

	// Pirate base
	SetSectorAffility(ST->Boneyard, 0.f);


	if(Company == ST->Pirates)
	{
		// Pirates
		// -------
		//
		// Pirate don't trade, and are only interested in getting money by force
		// All resource affilities are set to 0
		// They never build stations
		// They never research science

		// They have 2 mode : agressive or moderate

		// They need a shipyard, if they have a shipyard they will steal resources,
		// or buy one to build ship, and fill and defend their base
		// They always want graveyard

		// They need some cargo to buy resources for their shipyard and arsenal

		SetResourceAffilities(0.1f);


		SetSectorAffilities(0.f);

		SetSectorAffility(ST->Boneyard, 10.f);


		ShipyardAffility = 0.0;
		ConsumerAffility = 0.0;
		MaintenanceAffility = 0.0;


		// Only buy
		TradingSell = 0.f;
		TradingBoth = 0.f;

		// Doesn't capture station. Change in recovery
		StationCapture = 0.f;

		DeclareWarConfidence = -0.2;
		RequestPeaceConfidence = -0.8;
		PayTributeConfidence = -1.0;

		ArmySize = 50.0;
		AttackThreshold = 0.8;
		RetreatThreshold = 0.2;
		DefeatAdaptation = 0.001;

		// Budget
		BudgetTechnologyWeight = 0.0;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 0.1;
		BudgetTradeWeight = 0.1;

		PacifismIncrementRate = 1.f;
		PacifismDecrementRate = 0.8f;

	}
	else if(Company == ST->GhostWorksShipyards)
	{
		// Loves Hela and doesn't like Nema

		ShipyardAffility = 5.0;

		SetSectorAffilitiesByMoon(ST->Nema,0.5f);
		SetSectorAffilitiesByMoon(ST->Hela, 6.f);

		// Budget
		BudgetTechnologyWeight = 0.2;
		BudgetMilitaryWeight = 1.0;
		BudgetStationWeight = 1.0;
		BudgetTradeWeight = 1.0;
	}
	else if(Company == ST->MiningSyndicate)
	{
		// Mining specialist.
		// Loves raw materials
		// Likes hard work
		SetResourceAffility(ST->Water, 10.f);
		SetResourceAffility(ST->Silica, 10.f);
		SetResourceAffility(ST->IronOxyde, 10.f);
		SetResourceAffility(ST->Hydrogen, 2.f);

		// Budget
		BudgetTechnologyWeight = 0.2;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;
	}
	else if(Company == ST->HelixFoundries)
	{
		// Likes hard factory, and Anka.
		// Base at Outpost
		SetResourceAffility(ST->Steel, 10.f);
		SetResourceAffility(ST->Tools, 10.f);
		SetResourceAffility(ST->Tech, 5.f);
		SetSectorAffilitiesByMoon(ST->Anka, 6.f);
		SetSectorAffility(ST->Outpost, 10.f);
		
		// Budget
		BudgetTechnologyWeight = 0.4;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;
	}
	else if(Company == ST->Sunwatch)
	{
		// Likes hard factory, and Anka.
		// Base at Outpost
		SetResourceAffility(ST->Fuel, 10.f);
		
		// Budget
		BudgetTechnologyWeight = 0.6;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;
	}
	else if(Company == ST->IonLane)
	{
		BudgetTechnologyWeight = 0.2;
		BudgetMilitaryWeight = 1.0;
		BudgetStationWeight = 0.1;
		BudgetTradeWeight = 2.0;

		ArmySize = 10.0;
		DeclareWarConfidence = 0.1;
		RequestPeaceConfidence = -0.4;
		PayTributeConfidence = -0.85;
	}
	else if(Company == ST->UnitedFarmsChemicals)
	{
		// Likes chemisty
		SetResourceAffility(ST->Food, 10.f);
		SetResourceAffility(ST->Carbon, 5.f);
		SetResourceAffility(ST->Methane, 5.f);

		// Budget
		BudgetTechnologyWeight = 0.4;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;
	}
	else if(Company == ST->NemaHeavyWorks)
	{
		// Likes Nema and heavy work
		SetResourceAffility(ST->FleetSupply, 2.f);
		SetResourceAffility(ST->Steel, 5.f);
		SetResourceAffility(ST->Tools, 5.f);
		SetResourceAffility(ST->Tech, 5.f);
		SetSectorAffilitiesByMoon(ST->Nema, 5.f);

		ShipyardAffility = 3.0;

		// Budget
		BudgetTechnologyWeight = 0.4;
		BudgetMilitaryWeight = 0.5;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;
	}
	else if(Company == ST->AxisSupplies)
	{
		// Assures fleet supply availability
		SetResourceAffility(ST->FleetSupply, 5.f);
		SetResourceAffility(ST->Food, 2.f);

		ShipyardAffility = 0.0;
		ConsumerAffility = 1.0;
		MaintenanceAffility = 10.0;

		// Budget
		BudgetTechnologyWeight = 0.2;
		BudgetMilitaryWeight = 0.25;
		BudgetStationWeight = 2.0;
		BudgetTradeWeight = 2.0;

		ArmySize = 1.0;
		DeclareWarConfidence = 1.0;
		RequestPeaceConfidence = 0.0;
		PayTributeConfidence = -0.1;

		DiplomaticReactivity = 0.1;
	}

}

void UFlareAIBehavior::SetResourceAffilities(float Value)
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		SetResourceAffility(Resource, Value);
	}
}

void UFlareAIBehavior::SetResourceAffility(FFlareResourceDescription* Resource, float Value)
{
	if(ResourceAffilities.Contains(Resource)){
		ResourceAffilities[Resource] = Value;
	}
	else
	{
		ResourceAffilities.Add(Resource, Value);
	}
}


void UFlareAIBehavior::SetSectorAffilities(float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];
		SetSectorAffility(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffility(UFlareSimulatedSector* Sector, float Value)
{
	if(SectorAffilities.Contains(Sector)){
		SectorAffilities[Sector] = Value;
	}
	else
	{
		SectorAffilities.Add(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffilitiesByMoon(FFlareCelestialBody *CelestialBody, float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		if(Sector->GetOrbitParameters()->CelestialBodyIdentifier == CelestialBody->Identifier)
		{
			SetSectorAffility(Sector, Value);
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

float UFlareAIBehavior::GetBudgetWeight(EFlareBudget::Type Budget)
{
	switch (Budget) {
	case EFlareBudget::Military:
		return (Company->AtWar() ? BudgetMilitaryWeight * 10 : BudgetMilitaryWeight);
		break;
	case EFlareBudget::Trade:
		return BudgetTradeWeight;
		break;
	case EFlareBudget::Station:
		return BudgetStationWeight;
		break;
	case EFlareBudget::Technology:
		return BudgetTechnologyWeight;
		break;
	default:
		break;
	}

	return 0;
}

float UFlareAIBehavior::GetSectorAffility(UFlareSimulatedSector* Sector)
{
	if(SectorAffilities.Contains(Sector))
	{
		return SectorAffilities[Sector];
	}
	return 1.f;
}

float UFlareAIBehavior::GetResourceAffility(FFlareResourceDescription* Resource)
{
	if(ResourceAffilities.Contains(Resource))
	{
		return ResourceAffilities[Resource];
	}
	return 1.f;
}

float UFlareAIBehavior::GetAttackThreshold()
{
	return AttackThreshold + Company->GetAI()->GetData()->Caution;
}

#undef LOCTEXT_NAMESPACE
