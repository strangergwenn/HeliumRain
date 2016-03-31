
#include "../../Flare.h"
#include "FlareCompanyAI.h"
#include "../FlareCompany.h"
#include "../FlareGame.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompanyAI::UFlareCompanyAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCompanyAI::Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data)
{
	Company = ParentCompany;
	Game = Company->GetGame();
	ResetShipGroup(EFlareCombatTactic::AttackMilitary);
}

FFlareCompanyAISave* UFlareCompanyAI::Save()
{
	return &AIData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareCompanyAI::Simulate()
{
	if (Company == Game->GetPC()->GetCompany())
	{
		return;
	}


	FLOGV("Simulate AI for %s", *Company->GetCompanyName().ToString());


	SimulateDiplomacy();

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		int32 TransportCapacityBalance = Sector->GetTransportCapacityBalance(Company);
		//FLOGV("Sector %s, transport capacity=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacity(Company));
		//FLOGV("Sector %s, transport needs=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacityNeeds(Company));
		//FLOGV("Sector %s, transport balance=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacityBalance(Company));

		if (TransportCapacityBalance > 0)
		{
			// TODO tolerate few more ship
			UnassignShipsFromSector(Sector, (uint32) TransportCapacityBalance);
			//FLOGV("AI %s ACTION : Unassign ships from sector %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TransportCapacityBalance)
		}
		else if (TransportCapacityBalance < 0)
		{
			AssignShipsToSector(Sector, (uint32) (- TransportCapacityBalance));
			//FLOGV("AI %s ACTION : Assign ships to sector %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TransportCapacityBalance)
		}
		// TODO reassign large ships


		// Assign ship for trade

		int32 TradeTransportCapacityBalance = Sector->GetTransportCapacityBalance(Company, true);
		if (TradeTransportCapacityBalance < 0)
		{
			AssignShipsToSector(Sector, (uint32) (- TradeTransportCapacityBalance));
			FLOGV("AI %s ACTION : Assign ships to sector for trade %s %d units", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(), TradeTransportCapacityBalance)
		}
		FLOGV("Sector %s, final transport capacity=%d", *Sector->GetSectorName().ToString(), Sector->GetTransportCapacity(Company));
	}


	// TODO Move unassign ship un sector that have not enough ship
	// Substract ship that are currently traveling to the sector and are not in a trade route


	// Trade route creation

	// For all current trade route in a sector (if not in a sector, it's not possible to modify then)
	//      -> Compute the resource balance in the dest sector and the resource balance in the source sector
	//			-> If the balance is negative in the dest sector, and positive un the source add a cargo
	//      -> Compute the current transport rate for the resource (resource/day)(mean on multiple travel) and the max transport rate
	//			-> If current is a lot below the max, remove a cargo

	// If inactive cargo
	// compute max negative balance. Find nearest sector with a positive balance.
	// create a route.
	// assign enought capacity to match the min(negative balance, positive balance)



	// TODO hub by stock, % of world production max




	/*
	TArray<UFlareSimulatedSpacecraft*> CompanyShips = Company->GetCompanyShips();

	// Assign ships to current sector
	for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = CompanyShips[ShipIndex];
		Ship->AssignToSector(true);
	}*/
}

void UFlareCompanyAI::Tick()
{
	if (Company == Game->GetPC()->GetCompany())
	{
		return;
	}

	ResetShipGroup(EFlareCombatTactic::AttackMilitary);

	SimulateDiplomacy();
}

void UFlareCompanyAI::SimulateDiplomacy()
{
	// Declare war or make peace
	for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

		if(OtherCompany == Company)
		{
			continue;
		}

		if(Company->GetHostility(OtherCompany) == EFlareHostility::Hostile && Company->GetReputation(OtherCompany) > -100)
		{
			Company->SetHostilityTo(OtherCompany, false);
		}
		else if(Company->GetHostility(OtherCompany) != EFlareHostility::Hostile && Company->GetReputation(OtherCompany) <= -100)
		{
			Company->SetHostilityTo(OtherCompany, true);
			if (OtherCompany == Game->GetPC()->GetCompany())
			{
				OtherCompany->SetHostilityTo(Company, true);
			}
		}
	}
}

void UFlareCompanyAI::UnassignShipsFromSector(UFlareSimulatedSector* Sector, uint32 MaxCapacity)
{
	uint32 RemainingCapacity = MaxCapacity;

	for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
		if(Ship->GetCompany() != Company || !Ship->IsAssignedToSector())
		{
			continue;
		}

		if(Ship->GetCargoBay()->GetCapacity() <= RemainingCapacity)
		{
			Ship->AssignToSector(false);
			RemainingCapacity-= Ship->GetCargoBay()->GetCapacity();
		}

		if(RemainingCapacity == 0)
		{
			return;
		}
	}
}

void UFlareCompanyAI::AssignShipsToSector(UFlareSimulatedSector* Sector, uint32 MinCapacity)
{
	int32 RemainingCapacity = MinCapacity;

	for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
		if(Ship->GetCompany() != Company || Ship->IsAssignedToSector() || Ship->GetCurrentTradeRoute() != NULL || Ship->GetCargoBay()->GetCapacity() == 0)
		{
			continue;
		}


		Ship->AssignToSector(true);
		RemainingCapacity-= Ship->GetCargoBay()->GetCapacity();


		if(RemainingCapacity <= 0)
		{
			return;
		}
	}
}

/*----------------------------------------------------
	Command groups
----------------------------------------------------*/

void UFlareCompanyAI::SetCurrentShipGroup(EFlareCombatGroup::Type Type)
{
	CurrentShipGroup = Type;
}

void UFlareCompanyAI::SetTacticForCurrentShipGroup(EFlareCombatTactic::Type Tactic)
{
	check(CurrentShipGroup < CurrentCombatTactics.Num());
	CurrentCombatTactics[CurrentShipGroup] = Tactic;
	if (CurrentShipGroup == EFlareCombatGroup::AllMilitary)
	{
		CurrentCombatTactics[EFlareCombatGroup::Capitals] = Tactic;
		CurrentCombatTactics[EFlareCombatGroup::Fighters] = Tactic;
	}
}

EFlareCombatGroup::Type UFlareCompanyAI::GetCurrentShipGroup() const
{
	return CurrentShipGroup;
}

EFlareCombatTactic::Type UFlareCompanyAI::GetCurrentTacticForShipGroup(EFlareCombatGroup::Type Type) const
{
	check(Type < CurrentCombatTactics.Num());
	return CurrentCombatTactics[Type];
}

int32 UFlareCompanyAI::GetShipCountForShipGroup(EFlareCombatGroup::Type Type) const
{
	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:
			return CurrentMilitaryShipCount;

		case EFlareCombatGroup::Capitals:
			return CurrentCapitalShipCount;

		case EFlareCombatGroup::Fighters:
			return CurrentFighterCount;

		case EFlareCombatGroup::Civilan:
		default:
			return CurrentCivilianShipCount;
	}
}

void UFlareCompanyAI::ResetControlGroups(UFlareSector* Sector)
{
	// Reset ship count values
	CurrentMilitaryShipCount = 0;
	CurrentCapitalShipCount = 0;
	CurrentFighterCount = 0;
	CurrentCivilianShipCount = 0;

	// Compute the current count of all kinds of ships
	if (Sector)
	{
		TArray<IFlareSpacecraftInterface*>& ShipList = Sector->GetSectorShipInterfaces();
		for (int32 Index = 0; Index < ShipList.Num(); Index++)
		{
			IFlareSpacecraftInterface* Ship = ShipList[Index];
			check(Ship);

			if (Ship->GetCompany() != Company)
			{
				continue;
			}

			if (Ship->IsMilitary())
			{
				CurrentMilitaryShipCount++;
				if (Ship->GetDescription()->Size == EFlarePartSize::L)
				{
					CurrentCapitalShipCount++;
				}
				else
				{
					CurrentFighterCount++;
				}
			}
			else
			{
				CurrentCivilianShipCount++;
			}
		}
	}
}

void UFlareCompanyAI::ResetShipGroup(EFlareCombatTactic::Type Tactic)
{
	CurrentCombatTactics.Empty();
	for (int32 Index = EFlareCombatGroup::AllMilitary; Index <= EFlareCombatGroup::Civilan; Index++)
	{
		CurrentCombatTactics.Add(Tactic);
	}
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/


