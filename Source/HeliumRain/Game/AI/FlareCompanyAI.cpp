
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

		if(TransportCapacityBalance > 0)
		{
			UnAssignShipsFromSector((uint32) TransportCapacityBalance);
		}
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

void UFlareCompanyAI::UnAssignShipsFromSector(uint32 Capacity)
{
	// TODO
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


