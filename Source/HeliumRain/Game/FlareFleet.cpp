
#include "FlareFleet.h"
#include "../Flare.h"

#include "FlareCompany.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareSimulatedSector.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareFleet"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareFleet::UFlareFleet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareFleet::Load(const FFlareFleetSave& Data)
{
	FleetCompany = Cast<UFlareCompany>(GetOuter());
	Game = FleetCompany->GetGame();
	FleetData = Data;
	IsShipListLoaded = false;
}

FFlareFleetSave* UFlareFleet::Save()
{
	return &FleetData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

FText UFlareFleet::GetName()
{
	if (GetShips().Num() > 0)
	{
		return UFlareGameTools::DisplaySpacecraftName(GetShips()[0]);// TODO Clean with GetFleetName
	}
	else
	{
		return GetFleetName();
	}
}

FText  UFlareFleet::GetFleetName() const
{
	if(Game->GetPC()->GetPlayerFleet() == this)
	{
		return LOCTEXT("PlayerFleetName", "Player Fleet");
	}

	return FleetData.Name;
}

bool UFlareFleet::IsTraveling() const
{
	return CurrentTravel != NULL;
}

bool UFlareFleet::IsTrading() const
{
	return GetTradingShipCount() > 0;
}


bool UFlareFleet::CanTravel()
{
	if (IsTraveling() && !GetCurrentTravel()->CanChangeDestination())
	{
		return false;
	}

	if (GetImmobilizedShipCount() == FleetShips.Num())
	{
		// All ship are immobilized
		return false;
	}

	if(Game->GetPC()->GetPlayerFleet() == this && Game->GetPC()->GetPlayerShip()->GetDamageSystem()->IsStranded())
	{
		// The player ship is stranded
		return false;
	}

	return true;
}

bool UFlareFleet::CanTravel(FText& OutInfo)
{
	if (IsTraveling() && !GetCurrentTravel()->CanChangeDestination())
	{
		OutInfo = LOCTEXT("TravelingFormat", "Can't change destination");
		return false;
	}

	if (GetImmobilizedShipCount() == FleetShips.Num())
	{
		OutInfo = LOCTEXT("Traveling", "Trading, stranded or intercepted");
		return false;
	}

	if(Game->GetPC()->GetPlayerFleet() == this && Game->GetPC()->GetPlayerShip()->GetDamageSystem()->IsStranded())
	{
		OutInfo = LOCTEXT("PlayerShipStranded", "The ship you are piloting is stranded");
		return false;
	}

	return true;
}

uint32 UFlareFleet::GetImmobilizedShipCount()
{
	uint32 ImmobilizedShip = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (!FleetShips[ShipIndex]->CanTravel() && FleetShips[ShipIndex]->GetDamageSystem()->IsAlive())
		{
			ImmobilizedShip++;
		}
	}
	return ImmobilizedShip;
}

uint32 UFlareFleet::GetTradingShipCount() const
{
	uint32 TradingShip = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->IsTrading())
		{
			TradingShip++;
		}
	}
	return TradingShip;
}

int32 UFlareFleet::GetTransportCapacity()
{
	int32 CompanyCapacity = 0;

	for(UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(Ship->GetDamageSystem()->IsStranded())
		{
			continue;
		}

		CompanyCapacity += Ship->GetCargoBay()->GetCapacity();
	}

	return CompanyCapacity;
}

uint32 UFlareFleet::GetShipCount() const
{
	return FleetShips.Num();
}

uint32 UFlareFleet::GetMilitaryShipCountBySize(EFlarePartSize::Type Size) const
{
	uint32 Count = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->GetDescription()->Size == Size && FleetShips[ShipIndex]->IsMilitary())
		{
			Count++;
		}
	}

	return Count;
}

uint32 UFlareFleet::GetMaxShipCount()
{
	return 20;
}

FText UFlareFleet::GetStatusInfo() const
{
	bool Intersepted = false;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(Ship->IsIntercepted())
		{
			Intersepted = true;
			break;
		}
	}

	if (Intersepted)
	{
		return FText::Format(LOCTEXT("FleetIntercepted", "Intercepted in {0}"), GetCurrentSector()->GetSectorName());
	}
	else if (IsTraveling())
	{
		int64 RemainingDuration = CurrentTravel->GetRemainingTravelDuration();
		return FText::Format(LOCTEXT("TravelTextFormat", "Traveling to {0} ({1} left)"),
			CurrentTravel->GetDestinationSector()->GetSectorName(),
			FText::FromString(*UFlareGameTools::FormatDate(RemainingDuration, 1))); //FString needed here
	}
	else if (IsTrading())
	{
		if(GetTradingShipCount() == GetShipCount())
		{
			return FText::Format(LOCTEXT("FleetTrading", "Trading in {0}"), GetCurrentSector()->GetSectorName());
		}
		else
		{
			return FText::Format(LOCTEXT("FleetPartialTrading", "{0} of {1} ships are trading in {2}"), FText::AsNumber(GetTradingShipCount()), FText::AsNumber(GetShipCount()), GetCurrentSector()->GetSectorName());
		}
	}
	else
	{
		if (GetCurrentTradeRoute() && !GetCurrentTradeRoute()->IsPaused())
		{
			return FText::Format(LOCTEXT("FleetStartTrade", "Starting trade in {0}"), GetCurrentSector()->GetSectorName());
		}
		else
		{
			return FText::Format(LOCTEXT("FleetIdle", "Idle in {0}"), GetCurrentSector()->GetSectorName());
		}
	}

	return FText();
}


int32 UFlareFleet::GetFleetCapacity() const
{
	int32 FreeCargoSpace = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FreeCargoSpace += FleetShips[ShipIndex]->GetCargoBay()->GetCapacity();
	}
	return FreeCargoSpace;
}

int32 UFlareFleet::GetFleetFreeCargoSpace() const
{
	int32 FreeCargoSpace = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FreeCargoSpace += FleetShips[ShipIndex]->GetCargoBay()->GetFreeCargoSpace();
	}
	return FreeCargoSpace;
}

int32 UFlareFleet::GetCombatPoints(bool ReduceByDamage) const
{
	int32 CombatPoints = 0;

	for(UFlareSimulatedSpacecraft* Ship: FleetShips)
	{
		CombatPoints += Ship->GetCombatPoints(ReduceByDamage);
	}
	return CombatPoints;
}

void UFlareFleet::RemoveImmobilizedShips()
{
	TArray<UFlareSimulatedSpacecraft*> ShipToRemove;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (!FleetShips[ShipIndex]->CanTravel() && FleetShips[ShipIndex] != Game->GetPC()->GetPlayerShip())
		{
			ShipToRemove.Add(FleetShips[ShipIndex]);
		}
	}

	for (int ShipIndex = 0; ShipIndex < ShipToRemove.Num(); ShipIndex++)
	{
		RemoveShip(ShipToRemove[ShipIndex]);
	}
}

void UFlareFleet::SetFleetColor(FLinearColor Color)
{
	FleetData.FleetColor = Color;
}

FLinearColor UFlareFleet::GetFleetColor() const
{
	return FleetData.FleetColor;
}


int32 UFlareFleet::GetRepairDuration() const
{
	int32 RepairDuration = 0;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		int32 ShipRepairDuration = Ship->GetRepairDuration();

		if (ShipRepairDuration > RepairDuration)
		{
			RepairDuration = ShipRepairDuration;
		}
	}

	return RepairDuration;
}

int32 UFlareFleet::GetRefillDuration() const
{
	int32 RefillDuration = 0;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		int32 ShipRefillDuration = Ship->GetRefillDuration();

		if (ShipRefillDuration > RefillDuration)
		{
			RefillDuration = ShipRefillDuration;
		}
	}

	return RefillDuration;
}

int32 UFlareFleet::InterceptShips()
{
	// Intercept half of ships at maximum and min 1
	int32 MaxInterseptedShipCount = FMath::Max(1,FleetShips.Num() / 2);
	int32 InterseptedShipCount = 0;
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(InterseptedShipCount >=MaxInterseptedShipCount)
		{
			break;
		}

		if (Ship == Game->GetPC()->GetPlayerShip())
		{
			// Never intercept the player ship
			continue;
		}

		if(FMath::FRand() < 0.1)
		{
			Ship->SetIntercepted(true);
			InterseptedShipCount++;
		}
	}
	return InterseptedShipCount;
}

void UFlareFleet::AddShip(UFlareSimulatedSpacecraft* Ship)
{
	if (IsTraveling())
	{
		FLOGV("Fleet Disband fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	if (GetCurrentSector() != Ship->GetCurrentSector())
	{
		FLOGV("Fleet Merge fail: '%s' is the sector '%s' but '%s' is the sector '%s'",
			  *GetFleetName().ToString(),
			  *GetCurrentSector()->GetSectorName().ToString(),
			  *Ship->GetImmatriculation().ToString(),
			  *Ship->GetCurrentSector()->GetSectorName().ToString());
		return;
	}

	UFlareFleet* OldFleet = Ship->GetCurrentFleet();
	if (OldFleet)
	{
		OldFleet->RemoveShip(Ship);
	}

	FleetData.ShipImmatriculations.Add(Ship->GetImmatriculation());
	FleetShips.AddUnique(Ship);
	Ship->SetCurrentFleet(this);

	if (FleetCompany == GetGame()->GetPC()->GetCompany() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("add-ship-to-fleet"));
	}
}

void UFlareFleet::RemoveShip(UFlareSimulatedSpacecraft* Ship, bool destroyed)
{
	if (IsTraveling())
	{
		FLOGV("Fleet RemoveShip fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	FleetData.ShipImmatriculations.Remove(Ship->GetImmatriculation());
	FleetShips.Remove(Ship);
	Ship->SetCurrentFleet(NULL);

	if (!destroyed)
	{
		Ship->GetCompany()->CreateAutomaticFleet(Ship);
	}

	if(FleetShips.Num() == 0)
	{
		Disband();
	}
}

/** Remove all ship from the fleet and delete it. Not possible during travel */
void UFlareFleet::Disband()
{
	if (IsTraveling())
	{
		FLOGV("Fleet Disband fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FleetShips[ShipIndex]->SetCurrentFleet(NULL);
	}

	if (GetCurrentTradeRoute())
	{
		GetCurrentTradeRoute()->RemoveFleet(this);
	}
	GetCurrentSector()->DisbandFleet(this);
	FleetCompany->RemoveFleet(this);
}

bool UFlareFleet::CanMerge(UFlareFleet* OtherFleet, FText& OutInfo)
{
	if (GetShipCount() + OtherFleet->GetShipCount() > GetMaxShipCount())
	{
		OutInfo = LOCTEXT("MergeFleetMaxShips", "Can't merge, max ships reached");
		return false;
	}

	if (IsTraveling())
	{
		OutInfo = LOCTEXT("MergeFleetTravel", "Can't merge during travel");
		return false;
	}

	if (OtherFleet == Game->GetPC()->GetPlayerFleet())
	{
		OutInfo = LOCTEXT("MergeFleetPlayer", "Can't merge the player fleet into another");
		return false;
	}

	if (OtherFleet->IsTraveling())
	{
		OutInfo = LOCTEXT("MergeOtherFleetTravel", "Can't merge traveling ships");
		return false;
	}

	if (GetCurrentSector() != OtherFleet->GetCurrentSector())
	{
		OutInfo = LOCTEXT("MergeFleetDifferenSector", "Can't merge from a different sector");
		return false;
	}

	return true;
}

void UFlareFleet::Merge(UFlareFleet* Fleet)
{
	FText Unused;
	if (!CanMerge(Fleet, Unused))
	{
		FLOGV("Fleet Merge fail: '%s' is not mergeable", *Fleet->GetFleetName().ToString());
		return;
	}

	TArray<UFlareSimulatedSpacecraft*> Ships = Fleet->GetShips();
	Fleet->Disband();
	for (int ShipIndex = 0; ShipIndex < Ships.Num(); ShipIndex++)
	{
		AddShip(Ships[ShipIndex]);
	}
}

void UFlareFleet::SetCurrentSector(UFlareSimulatedSector* Sector)
{
	if (!Sector->IsTravelSector())
	{
		CurrentTravel = NULL;
	}

	CurrentSector = Sector;
	InitShipList();
}

void UFlareFleet::SetCurrentTravel(UFlareTravel* Travel)
{
	CurrentSector = Travel->GetTravelSector();
	CurrentTravel = Travel;
	InitShipList();
	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FleetShips[ShipIndex]->SetSpawnMode(EFlareSpawnMode::Travel);
	}
}

void UFlareFleet::InitShipList()
{
	if (!IsShipListLoaded)
	{
		IsShipListLoaded = true;
		FleetShips.Empty();
		for (int ShipIndex = 0; ShipIndex < FleetData.ShipImmatriculations.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = FleetCompany->FindSpacecraft(FleetData.ShipImmatriculations[ShipIndex]);
			if (!Ship)
			{
				FLOGV("WARNING: Fail to find ship with id %s in company %s for fleet %s (%d ships)",
						*FleetData.ShipImmatriculations[ShipIndex].ToString(),
						*FleetCompany->GetCompanyName().ToString(),
						*GetFleetName().ToString(),
						FleetData.ShipImmatriculations.Num());
				continue;
			}
			Ship->SetCurrentFleet(this);
			FleetShips.Add(Ship);
		}
	}
}

bool UFlareFleet::IsRepairing() const
{
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if (Ship->IsRepairing())
		{
			return true;
		}
	}

	return false;
}

bool UFlareFleet::IsRefilling() const
{
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if (Ship->IsRefilling())
		{
			return true;
		}
	}
	return false;
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<UFlareSimulatedSpacecraft*>& UFlareFleet::GetShips()
{
	InitShipList();

	return FleetShips;
}

#undef LOCTEXT_NAMESPACE
