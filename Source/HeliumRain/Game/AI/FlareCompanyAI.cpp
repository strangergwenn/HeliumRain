
#include "../../Flare.h"
#include "FlareCompanyAI.h"
#include "../FlareCompany.h"
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

	FLOGV("Simulate AI for %s", *Company->GetCompanyName().ToString());

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



	/*
	TArray<UFlareSimulatedSpacecraft*> CompanyShips = Company->GetCompanyShips();

	// Assign ships to current sector
	for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = CompanyShips[ShipIndex];
		Ship->AssignToSector(true);
	}*/
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
	Getters
----------------------------------------------------*/


