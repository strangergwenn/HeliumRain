
#include "../Flare.h"

#include "FlareGameTools.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"
#include "FlareCompany.h"
#include "FlareSectorHelper.h"

#define LOCTEXT_NAMESPACE "FlareGameTools"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameTools::UFlareGameTools(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}


/*----------------------------------------------------
	Game tools
----------------------------------------------------*/



void UFlareGameTools::ForceSectorActivation(FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::ForceSectorActivation failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::ForceSectorActivation failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	GetGame()->ActivateSector(Sector);
}

void UFlareGameTools::ForceSectorDeactivation()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::ForceSectorDeactivation failed: no loaded world");
		return;
	}

	GetGame()->DeactivateSector();
}

void UFlareGameTools::SetDefaultWeapon(FName NewDefaultWeaponIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = GetGame()->GetShipPartsCatalog()->Get(NewDefaultWeaponIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon)
	{
		GetGame()->SetDefaultWeaponIdentifier(NewDefaultWeaponIdentifier);
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultWeaponIdentifier.ToString())
	}
}

void UFlareGameTools::SetDefaultTurret(FName NewDefaultTurretIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = GetGame()->GetShipPartsCatalog()->Get(NewDefaultTurretIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
	{
		GetGame()->SetDefaultTurretIdentifier(NewDefaultTurretIdentifier);
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultTurretIdentifier.ToString())
	}
}

void UFlareGameTools::SetHudDistortionGrid(uint32 Value)
{
	AFlareHUD* Hud = Cast<AFlareHUD>(GetGame()->GetPC()->GetHUD());
	if (Hud)
	{
		Hud->SetDistortionGrid(Value);
	}
}

void UFlareGameTools::SetHudDistortion(uint32 Axis, uint32 X, uint32 Y, float Value)
{
	AFlareHUD* Hud = Cast<AFlareHUD>(GetGame()->GetPC()->GetHUD());
	if (Hud)
	{
		Hud->SetDistortion(Axis, X, Y, Value);
	}
}

/*----------------------------------------------------
	World tools
----------------------------------------------------*/

int64 UFlareGameTools::GetWorldDate()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GetWorldDate failed: no loaded world");
		return 0;
	}
	FLOGV("World date: %lld", GetGameWorld()->GetDate());
	return GetGameWorld()->GetDate();
}

void UFlareGameTools::SetWorldDate(int64 Date)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::SetWorldDate failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::SetWorldDate failed: a sector is active");
		return;
	}

	GetGameWorld()->ForceDate(Date);
}


void UFlareGameTools::Simulate()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::Simulate failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::Simulate failed: a sector is active");
		return;
	}

	GetGame()->DeactivateSector();
	GetGameWorld()->Simulate();
	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::SetPlanatariumTimeMultiplier(float Multiplier)
{
	GetGame()->GetPlanetarium()->SetTimeMultiplier(Multiplier);
}

void UFlareGameTools::RevealMap()
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::RevealMap failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("UFlareGameTools::RevealMap failed: a sector is active");
		return;
	}

	for (int i = 0; i < GetGameWorld()->GetSectors().Num(); i++)
	{

		UFlareSimulatedSector* Sector = GetGameWorld()->GetSectors()[i];
		AFlarePlayerController* PC = GetPC();
		PC->GetCompany()->VisitSector(Sector);
	}
}


/*----------------------------------------------------
	Company tools
----------------------------------------------------*/

void UFlareGameTools::DeclareWar(FName Company1ShortName, FName Company2ShortName)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::DeclareWar failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("UFlareGameTools::DeclareWar failed: a sector is active");
		return;
	}

	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2 && Company1 != Company2)
	{
		FLOGV("Declare war between %s and %s", *Company1->GetCompanyName().ToString(), *Company2->GetCompanyName().ToString());
		Company1->SetHostilityTo(Company2, true);
		Company2->SetHostilityTo(Company1, true);

		// Notify war
		AFlarePlayerController* PC = GetPC();
		FText WarText = LOCTEXT("War", "War has been declared");
		FText WarInfoText = FText::Format(LOCTEXT("WarStringInfoFormat", "{0}, {1} are now at war"), Company1->GetCompanyName(), Company2->GetCompanyName());
		PC->Notify(WarText, WarInfoText, NAME_None, EFlareNotification::NT_Military);
	}
}

void UFlareGameTools::MakePeace(FName Company1ShortName, FName Company2ShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::MakePeace failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::MakePeace failed: a sector is active");
		return;
	}


	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2)
	{
		Company1->SetHostilityTo(Company2, false);
		Company2->SetHostilityTo(Company1, false);
	}
}

void UFlareGameTools::PrintCompany(FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompany failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::PrintCompany failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	FLOGV("> PrintCompany: %s - %s (%s)", *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	struct CompanyValue Value = Company->GetCompanyValue();
	FLOGV("  > %f $", Value.TotalValue / 100.);
	FLOGV("    - Money %f $", Value.MoneyValue / 100.);
	FLOGV("    - Stocks %f $", Value.StockValue/ 100.);
	FLOGV("    - Spacecrafts %f $", Value.SpacecraftsValue/ 100.);
	FLOGV("      - Ships %f $", Value.ShipsValue/ 100.);
	FLOGV("      - Stations %f $", Value.StationsValue/ 100.);
	TArray<UFlareFleet*> CompanyFleets = Company->GetCompanyFleets();
	FLOGV("  > %d fleets", CompanyFleets.Num());
	for (int i = 0; i < CompanyFleets.Num(); i++)
	{
		UFlareFleet* Fleet = CompanyFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName().ToString());
	}

	TArray<UFlareTradeRoute*> CompanyTradeRoutes = Company->GetCompanyTradeRoutes();
	FLOGV("  > %d trade routes", CompanyTradeRoutes.Num());
	for (int i = 0; i < CompanyTradeRoutes.Num(); i++)
	{
		UFlareTradeRoute* TradeRoute = CompanyTradeRoutes[i];
		FLOGV("   %2d - %s: %s", i,  *TradeRoute->GetIdentifier().ToString(), *TradeRoute->GetTradeRouteName().ToString());
	}

	TArray<UFlareSimulatedSpacecraft*> CompanySpacecrafts = Company->GetCompanySpacecrafts();
	FLOGV("  > %d spacecrafts (%d ships and %d stations)", CompanySpacecrafts.Num(), Company->GetCompanyShips().Num(), Company->GetCompanyStations().Num());
	for (int i = 0; i < CompanySpacecrafts.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[i];
		FLOGV("   %2d - %s", i,  *Spacecraft->GetImmatriculation().ToString());
	}

	TArray<UFlareCompany*> Companies = GetGame()->GetGameWorld()->GetCompanies();
	FLOG("  > Diplomacy");
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* OtherCompany = Companies[i];
		if(OtherCompany == Company)
		{
			continue;
		}
		FLOGV("   - %s want %s with %s (reputation: %f). %s want %s with %s (reputation: %f)",
			  *Company->GetCompanyName().ToString(),
			  (Company->GetHostility(OtherCompany) == EFlareHostility::Hostile ? *FString("war") : *FString("peace")),
			  *OtherCompany->GetCompanyName().ToString(),
			  Company->GetReputation(OtherCompany),
			  *OtherCompany->GetCompanyName().ToString(),
			  (OtherCompany->GetHostility(Company) == EFlareHostility::Hostile ? *FString("war") : *FString("peace")),
			  *Company->GetCompanyName().ToString(),
			  OtherCompany->GetReputation(Company));
	}
}

void UFlareGameTools::PrintCompanyByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompanyByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareCompany*> Companies = GetGameWorld()->GetCompanies();
	if (Index < 0 || Index > Companies.Num() -1)
	{
		FLOGV("AFlareGame::PrintCompanyByIndex failed: invalid index %d, with %d companies.", Index, Companies.Num());
		return;
	}

	PrintCompany(Companies[Index]->GetShortName());
}

void UFlareGameTools::GiveReputation(FName CompanyShortName1, FName CompanyShortName2, float Amount)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::GiveReputation failed: no loaded world");
		return;
	}

	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(CompanyShortName1);
	if (!Company1)
	{
		FLOGV("UFlareGameTools::GiveReputation failed: no company with short name '%s'", * CompanyShortName1.ToString());
		return;
	}

	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(CompanyShortName2);
	if (!Company2)
	{
		FLOGV("UFlareGameTools::GiveReputation failed: no company with short name '%s'", * CompanyShortName2.ToString());
		return;
	}

	Company1->GiveReputation(Company2, Amount, false);
}


void UFlareGameTools::TakeCompanyControl(FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::TakeCompanyControl failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareGameTools::TakeCompanyControl failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	GetGame()->GetPC()->SetCompanyDescription(*Company->GetDescription());
	FFlarePlayerSave SavePlayerData;
	SavePlayerData.CompanyIdentifier = Company->GetIdentifier();
	SavePlayerData.LastFlownShipIdentifier = NAME_None;
	GetGame()->GetPC()->Load(SavePlayerData);
}


/*----------------------------------------------------
	Fleet tools
----------------------------------------------------*/

void UFlareGameTools::CreateFleet(FString FleetName, FName FirstShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateFleet failed: a sector is active");
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(FirstShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::CreateFleet failed: no Ship with immatriculation '%s'", *FirstShipImmatriculation.ToString());
		return;
	}


	UFlareCompany* FleetCompany = Ship->GetCompany();
	UFlareSimulatedSector* FleetSector = Ship->GetCurrentSector();


	UFlareFleet* Fleet = FleetCompany->CreateFleet(FText::FromString(FleetName), FleetSector);
	Fleet->AddShip(Ship);
}

void UFlareGameTools::DisbandFleet(FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::DisbandFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::DisbandFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::DisbandFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	Fleet->Disband();
}


void UFlareGameTools::AddToFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::AddToFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::AddToFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::AddToFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::AddToFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->AddShip(Ship);
}


void UFlareGameTools::RemoveFromFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::RemoveFromFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::RemoveFromFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->RemoveShip(Ship);
}

void UFlareGameTools::MergeFleets(FName Fleet1Identifier, FName Fleet2Identifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::MergeFleets failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::MergeFleets failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet1 = GetGameWorld()->FindFleet(Fleet1Identifier);
	if (!Fleet1)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet1Identifier.ToString());
		return;
	}

	UFlareFleet* Fleet2 = GetGameWorld()->FindFleet(Fleet2Identifier);
	if (!Fleet2)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet2Identifier.ToString());
		return;
	}

	Fleet1->Merge(Fleet2);
}

/*----------------------------------------------------
	Trade route tools
----------------------------------------------------*/

void UFlareGameTools::CreateTradeRoute(FString TradeRouteName, FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateTradeRoute failed: a sector is active");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::CreateTradeRoute failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->CreateTradeRoute(FText::FromString(TradeRouteName));
}

void UFlareGameTools::DissolveTradeRoute(FName TradeRouteIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::DisbandFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::DisbandFleet failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::DissolveTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	TradeRoute->Dissolve();
}


void UFlareGameTools::AddToTradeRoute(FName TradeRouteIdentifier, FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::AddToTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::AddToTradeRoute failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::AddToTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::AddToTradeRoute failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	TradeRoute->AssignFleet(Fleet);
}


void UFlareGameTools::RemoveFromTradeRoute(FName TradeRouteIdentifier, FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::RemoveFromTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::RemoveFromTradeRoute failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::RemoveFromTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::RemoveFromTradeRoute failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	TradeRoute->RemoveFleet(Fleet);
}

/*----------------------------------------------------
	Travel tools
----------------------------------------------------*/

void UFlareGameTools::StartTravel(FName FleetIdentifier, FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::StartTravel failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::StartTravel failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::StartTravel failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::StartTravel failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	GetGameWorld()->StartTravel(Fleet, Sector);
}


void UFlareGameTools::PrintTravelList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintTravelList failed: no loaded world");
		return;
	}


	FLOGV("> PrintTravelList: %d travels", GetGameWorld()->GetTravels().Num());

	for (int i = 0; i < GetGameWorld()->GetTravels().Num(); i++)
	{
		UFlareTravel* Travel = GetGameWorld()->GetTravels()[i];
		FLOGV("%2d - %s to %s	",
			i,
			*Travel->GetFleet()->GetFleetName().ToString(),
			*Travel->GetDestinationSector()->GetSectorName().ToString());
	}
}


void UFlareGameTools::PrintTravelByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintTravelByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareTravel*>Travels= GetGameWorld()->GetTravels();
	if (Index < 0 || Index > Travels.Num() -1)
	{
		FLOGV("AFlareGame::PrintTravelByIndex failed: invalid index %d, with %d travels.", Index, Travels.Num());
		return;
	}

	UFlareTravel* Travel = GetGameWorld()->GetTravels()[Index];
	FLOGV("> PrintTravel %s to %s",
		*Travel->GetFleet()->GetFleetName().ToString(),
		*Travel->GetDestinationSector()->GetSectorName().ToString());
	FLOGV("  - Departure date: day %lld ", Travel->GetDepartureDate());
	FLOGV("  - Elapsed time: %lld days", Travel->GetElapsedTime());

}

/*----------------------------------------------------
	Sector tools
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipForMeInSector(FName ShipClass, FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateShipForMeInSector failed: no world");
		return NULL;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationForMe failed: a sector is active");
		return NULL;
	}



	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateShipForMeInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();

	FVector TargetPosition = FVector::ZeroVector;
	UFlareSimulatedSpacecraft* ShipPawn = Sector->CreateStation(ShipClass, PC->GetCompany(), TargetPosition);

	return ShipPawn;
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationInCompanyAttachedInSector(FName StationClass, FName CompanyShortName, FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateStationInCompanyAttachedInSector failed: no world");
		return NULL;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationInCompanyAttachedInSector failed: a sector is active");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::CreateStationInCompanyAttachedInSector failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateStationInCompanyAttachedInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	UFlareSimulatedSpacecraft* NewStation = Sector->CreateStation(StationClass, Company, FVector::ZeroVector, FRotator::ZeroRotator);
	return NewStation;
}

void UFlareGameTools::PrintSectorList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSectorList failed: no loaded world");
		return;
	}


	FLOGV("> PrintSectorList: %d sectors", GetGameWorld()->GetSectors().Num());

	for (int i = 0; i < GetGameWorld()->GetSectors().Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGameWorld()->GetSectors()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Sector->GetIdentifier().ToString(), *Sector->GetSectorName().ToString(), *Sector->GetSectorCode());
	}
}


void UFlareGameTools::PrintSector(FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSector failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);
	if (!Sector)
	{
		FLOGV("AFlareGame::PrintSector failed: no sector with identifier '%s'", * SectorIdentifier.ToString());
		return;
	}

	FLOGV("> PrintSector: %s - %s (%s)", *Sector->GetIdentifier().ToString(), *Sector->GetSectorName().ToString(), *Sector->GetSectorCode());

	TArray<UFlareFleet*> SectorFleets = Sector->GetSectorFleets();
	FLOGV("  > %d fleets", SectorFleets.Num());
	for (int i = 0; i < SectorFleets.Num(); i++)
	{
		UFlareFleet* Fleet = SectorFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName().ToString());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorShips = Sector->GetSectorShips();
	FLOGV("  > %d ships", SectorShips.Num());
	for (int i = 0; i < SectorShips.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorShips[i];
		FLOGV("   %2d - %s (%d)", i,  *Spacecraft->GetImmatriculation().ToString(), Spacecraft->IsStation());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorStations = Sector->GetSectorStations();
	FLOGV("  > %d stations", SectorStations.Num());
	for (int i = 0; i < SectorStations.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorStations[i];
		FLOGV("   %2d - %s (%d)", i,  *Spacecraft->GetImmatriculation().ToString(), Spacecraft->IsStation());
	}

	//Prices
	FLOG("  > prices");
	for(int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		FLOGV("   - %s : %f credits", *Resource->Name.ToString(), Sector->GetPreciseResourcePrice(Resource) / 100.);
	}

	//People
	FLOG("  > people");
	Sector->GetPeople()->PrintInfo();
}


void UFlareGameTools::PrintSectorByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::PrintSectorByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= GetGameWorld()->GetSectors();
	if (Index < 0 || Index > Sectors.Num() -1)
	{
		FLOGV("UFlareGameTools::PrintSectorByIndex failed: invalid index %d, with %d sectors.", Index, Sectors.Num());
		return;
	}

	PrintSector(Sectors[Index]->GetIdentifier());
}


void UFlareGameTools::GiveBirth(int32 SectorIndex, uint32 Population)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::GiveBirth failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= GetGameWorld()->GetSectors();
	if (SectorIndex < 0 || SectorIndex > Sectors.Num() -1)
	{
		FLOGV("UFlareGameTools::GiveBirth failed: invalid index %d, with %d sectors.", SectorIndex, Sectors.Num());
		return;
	}

	FName SectorIdentifier = Sectors[SectorIndex]->GetIdentifier();

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);
	if (!Sector)
	{
		FLOGV("AFlareGame::GiveBirth failed: no sector with identifier '%s'", * SectorIdentifier.ToString());
		return;
	}

	Sector->GetPeople()->GiveBirth(Population);
}

void UFlareGameTools::Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::Scrap failed: no loaded world");
		return;
	}

	GetGame()->Scrap(ShipImmatriculation, TargetStationImmatriculation);
}



/*----------------------------------------------------
	Trade tools
----------------------------------------------------*/

void UFlareGameTools::PrintCargoBay(FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCargoBay failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::PrintCargoBay failed: a sector is active");
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::PrintCargoBay failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	UFlareCargoBay* CargoBay = Ship->GetCargoBay();

	FLOGV("Cargo bay for '%s' : ", *ShipImmatriculation.ToString());
	for (uint32 CargoIndex = 0; CargoIndex < CargoBay->GetSlotCount(); CargoIndex++)
	{
		FFlareCargo* Cargo = CargoBay->GetSlot(CargoIndex);
		FLOGV("  - %s : %u / %u ", (Cargo->Resource ? *Cargo->Resource->Name.ToString() : TEXT("[Empty]")), Cargo->Quantity, Cargo->Capacity);
	}
}

void UFlareGameTools::GiveResources(FName ShipImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GiveResources failed: no loaded world");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("AFlareGame::GiveResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::GiveResources failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Ship->GetCargoBay()->GiveResources(Resource, Quantity);
}

void UFlareGameTools::TakeResources(FName ShipImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::TakeResources failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::TakeResources failed: a sector is active");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("AFlareGame::TakeResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::TakeResources failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Ship->GetCargoBay()->TakeResources(Resource, Quantity);
}

void UFlareGameTools::TakeMoney(FName CompanyShortName, int64 Amount)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::TakeMoney failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::TakeMoney failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->TakeMoney(Amount);
}

void UFlareGameTools::GiveMoney(FName CompanyShortName, int64 Amount)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GiveMoney failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::GiveMoney failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->GiveMoney(Amount);
}

void UFlareGameTools::TransferResources(FName SourceImmatriculation, FName DestinationImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::TransferResources failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("UFlareGameTools::TransferResources failed: a sector is active");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* SourceSpacecraft = GetGameWorld()->FindSpacecraft(SourceImmatriculation);
	if (!SourceSpacecraft)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no source spacecraft with immatriculation '%s'", *SourceImmatriculation.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* DestinationSpacecraft = GetGameWorld()->FindSpacecraft(DestinationImmatriculation);
	if (!DestinationSpacecraft)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no destination spacecraft with immatriculation '%s'", *DestinationImmatriculation.ToString());
		return;
	}


	SectorHelper::Trade(SourceSpacecraft, DestinationSpacecraft, Resource, Quantity);
}

/*----------------------------------------------------
	Active Sector tools
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationForMe(FName StationClass)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationForMe failed: no active sector");
		return NULL;
	}

	return CreateStationInCompany(StationClass, GetPC()->GetCompany()->GetShortName(), 100);
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationInCompany failed: no active sector");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateStationInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();
	UFlareSimulatedSpacecraft* NewStation = ActiveSector->CreateStation(StationClass, Company, TargetPosition);
	GetGame()->ActivateCurrentSector();
	
	FFlareMenuParameterData Data;
	Data.Spacecraft = ExistingShipPawn->GetParent();
	PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Data);

	return NewStation;
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipForMe(FName ShipClass)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipForMe failed: no active sector");
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	return CreateShipInCompany(ShipClass, PC->GetCompany()->GetShortName(), 100);
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
	FLOG("AFlareGame::CreateShipInCompany");
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipInCompany failed: no active sector");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();
	UFlareSimulatedSpacecraft* NewShip = ActiveSector->CreateShip(ShipClass, Company, TargetPosition);
	GetGame()->ActivateCurrentSector();

	FFlareMenuParameterData Data;
	Data.Spacecraft = NewShip;
	PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Data);

	return NewShip;
}

void UFlareGameTools::CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipsInCompany failed: no active sector");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return;
	}

	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	for (int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
	{
		ActiveSector->CreateShip(ShipClass, Company, TargetPosition);
	}

	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::CreateQuickBattle(float Distance, FName Company1Name, FName Company2Name, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateQuickBattle failed: no active sector");
		return;
	}

	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1Name);
	if (!Company1)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *Company1Name.ToString());
		return;
	}

	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2Name);
	if (!Company2)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *Company2Name.ToString());
		return;
	}

	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition1 = FVector::ZeroVector;
	FVector TargetPosition2 = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition1 = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance / 2.f * FVector(100, 0, 0));
		TargetPosition2 = ExistingShipPawn->GetActorLocation() - ExistingShipPawn->GetActorRotation().RotateVector(Distance / 2.f * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	for (int32 ShipIndex = 0; ShipIndex < ShipClass1Count; ShipIndex++)
	{
		ActiveSector->CreateShip(ShipClass1, Company1, TargetPosition1);
		ActiveSector->CreateShip(ShipClass1, Company2, TargetPosition2);
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		ActiveSector->CreateShip(ShipClass2, Company1, TargetPosition1);
		ActiveSector->CreateShip(ShipClass2, Company2, TargetPosition2);
	}

	GetGame()->ActivateCurrentSector();
}


void UFlareGameTools::CreateAsteroid(int32 ID, FName Name)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateAsteroid failed: no active sector");
		return;
	}


	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(200 * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	ActiveSector->CreateAsteroid(ID, Name, TargetPosition);

	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::PrintCompanyList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompanyList failed: no loaded world");
		return;
	}


	FLOGV("> PrintCompanyList: %d companies", GetGameWorld()->GetCompanies().Num());

	for (int i = 0; i < GetGameWorld()->GetCompanies().Num(); i++)
	{
		UFlareCompany* Company = GetGameWorld()->GetCompanies()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	}
}

/*----------------------------------------------------
	Helper
----------------------------------------------------*/

FString UFlareGameTools::FormatTime(int64 Time, int Deep)
{
	if (Time < SECONDS_IN_MINUTE)
	{
		return FString::FromInt(Time) + (Time > 2 ? FString(LOCTEXT("seconds", " seconds").ToString()) : FString(LOCTEXT("second", " second").ToString()));
	}
	else if (Time < SECONDS_IN_HOUR)
	{
		int64 Minutes = Time / SECONDS_IN_MINUTE;
		int64 RemainingSeconds = Time % SECONDS_IN_MINUTE;

		FString MinutesString;

		if (Minutes > 0)
		{
			MinutesString += FString::FromInt(Minutes) + (Minutes > 2 ? FString(LOCTEXT("minutes", " minutes").ToString()) : FString(LOCTEXT("minute", " minute").ToString()));
		}

		if (Deep > 0 && RemainingSeconds > 0)
		{
			MinutesString += FString(" ") + FormatTime(RemainingSeconds, Deep - 1);
		}

		return MinutesString;
	}
	else if (Time < SECONDS_IN_DAY)
	{
		int64 Hours = Time / SECONDS_IN_HOUR;
		int64 RemainingSeconds = Time % SECONDS_IN_HOUR;
		int64 RemainingMinutes = RemainingSeconds / SECONDS_IN_MINUTE;

		FString HoursString;

		if (Hours > 0)
		{
			HoursString += FString::FromInt(Hours) + (Hours > 2 ? FString(LOCTEXT("hours", " hours").ToString()) : FString(LOCTEXT("hour", " hour").ToString()));
		}

		if (Deep > 0 && RemainingMinutes > 0)
		{
			HoursString += FString(" ") + FormatTime(RemainingSeconds, Deep - 1);
		}

		return HoursString;
	}
	else if (Time < SECONDS_IN_YEAR)
	{
		int64 Days = Time / SECONDS_IN_DAY;
		int64 RemainingSeconds = Time % SECONDS_IN_DAY;
		int64 RemainingHours = RemainingSeconds / SECONDS_IN_HOUR;

		FString DaysString;

		if (Days > 0)
		{
			DaysString += FString::FromInt(Days) + (Days > 2 ? FString(LOCTEXT("days", " days").ToString()) : FString(LOCTEXT("day", " day").ToString()));
		}

		if (Deep > 0 && RemainingHours > 0)
		{
			DaysString += FString(" ") + FormatTime(RemainingSeconds, Deep - 1);
		}

		return DaysString;
	}
	else
	{
		int64 Years = Time / SECONDS_IN_YEAR;
		int64 RemainingSeconds = Time % SECONDS_IN_YEAR;
		int64 RemainingDays = RemainingSeconds / SECONDS_IN_DAY;

		FString YearsString;

		if (Years > 0)
		{
			YearsString += FString::FromInt(Years) + (Years > 2 ? FString(LOCTEXT("years", " years").ToString()) : FString(LOCTEXT("year", " year").ToString()));
		}

		if (Deep > 0 && RemainingDays > 0)
		{
			YearsString += FString(" ") + FormatTime(RemainingSeconds, Deep - 1);
		}

		return YearsString;
	}
}

FString UFlareGameTools::FormatDate(int64 Days, int Deep)
{
	if (Days < DAYS_IN_YEAR)
	{
		if (Days < 1)
		{
			Days = 1;
		}
		return FString::FromInt(Days) + (Days > 1 ? FString(LOCTEXT("days", " days").ToString()) : FString(LOCTEXT("day", " day").ToString()));
	}
	else
	{
		int64 Years = Days / DAYS_IN_YEAR;
		int64 RemainingDays = Days % DAYS_IN_YEAR;

		FString YearsString;

		if (Years > 0)
		{
			YearsString += FString::FromInt(Years) + (Years > 2 ? FString(LOCTEXT("years", " years").ToString()) : FString(LOCTEXT("year", " year").ToString()));
		}

		if (Deep > 0 && RemainingDays > 0)
		{
			YearsString += FString(" ") + FormatDate(RemainingDays, Deep - 1);
		}

		return YearsString;
	}
}

FText UFlareGameTools::GetDisplayDate(int64 Days)
{
	int64 Years = START_YEAR + (Days / DAYS_IN_YEAR);
	int64 RemainingDays = Days % DAYS_IN_YEAR;
	FString YearsString;

	return FText::Format(LOCTEXT("DateFormat", "Year {0} - Day {1}"),
		FText::FromString(FString::FromInt(Years)),
		FText::AsNumber(RemainingDays));
}

int64 UFlareGameTools::ComputeShipPrice(FName ShipClass, UFlareSimulatedSector *Sector, bool WithMargin)
{
	FFlareSpacecraftDescription* Desc = Sector->GetGame()->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		FLOGV("ComputeShipPrice failed: Unkwnon ship %s", *ShipClass.ToString());
		return 0;
	}

	int64 Cost = 0;
	Cost += Desc->CycleCost.ProductionCost;

	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.InputResources[ResourceIndex];
		Cost += Resource->Quantity * Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
	}

	// Substract output resource
	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.OutputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.OutputResources[ResourceIndex];
		Cost -= Resource->Quantity * Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
	}

	return FMath::Max((int64) 0, Cost) * (WithMargin ? 1.2f : 1.0f);
}


uint32 UFlareGameTools::ComputeConstructionCapacity(FName ShipClass, AFlareGame *Game)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		FLOGV("ComputeConstructionCapacity failed: Unkwnon ship %s", *ShipClass.ToString());
		return 0;
	}

	int32 Capacity = 0;

	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.InputResources[ResourceIndex];
		Capacity += Resource->Quantity;
	}

	// Substract output resource
	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.OutputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.OutputResources[ResourceIndex];
		Capacity += Resource->Quantity;
	}

	return Capacity;
}

/*----------------------------------------------------
	Getter
----------------------------------------------------*/

AFlarePlayerController* UFlareGameTools::GetPC() const
{
	return Cast<AFlarePlayerController>(GetOuter());
}

AFlareGame* UFlareGameTools::GetGame() const
{
	return GetPC()->GetGame();
}


UFlareWorld* UFlareGameTools::GetGameWorld() const
{
	return GetGame()->GetGameWorld();
}

UFlareSector* UFlareGameTools::GetActiveSector() const
{
	return GetGame()->GetActiveSector();
}

#undef LOCTEXT_NAMESPACE
