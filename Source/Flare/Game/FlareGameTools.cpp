
#include "../Flare.h"

#include "FlareGameTools.h"
#include "FlareGame.h"

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

	GetGame()->ActivateSector(GetPC(), Sector);
}

void UFlareGameTools::ForceSectorDeactivation()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::ForceSectorDeactivation failed: no loaded world");
		return;
	}

	GetGame()->DeactivateSector(GetPC());
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

/*----------------------------------------------------
	World tools
----------------------------------------------------*/

int64 UFlareGameTools::GetWorldTime()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GetWorldTime failed: no loaded world");
		return 0;
	}
	FLOGV("World time: %lld", GetGameWorld()->GetTime());
	return GetGameWorld()->GetTime();
}

void UFlareGameTools::SetWorldTime(int64 Time)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::SetWorldTime failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::SetWorldTime failed: a sector is active");
		return;
	}

	GetGameWorld()->ForceTime(Time);
}


void UFlareGameTools::Simulate(int64 Duration)
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

	GetGameWorld()->Simulate(Duration);
}


/*----------------------------------------------------
	Company tools
----------------------------------------------------*/

void UFlareGameTools::DeclareWar(FName Company1ShortName, FName Company2ShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::DeclareWar failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::DeclareWar failed: a sector is active");
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
		FText WarString = LOCTEXT("War", "War has been declared");
		FText WarStringInfo = FText::FromString(Company1->GetCompanyName().ToString() + ", " + Company2->GetCompanyName().ToString() + " "
			+ LOCTEXT("WarInfo", "are now at war").ToString());
		PC->Notify(WarString, WarStringInfo, EFlareNotification::NT_Military);
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

	TArray<UFlareFleet*> CompanyFleets = Company->GetCompanyFleets();
	FLOGV("  > %d fleets", CompanyFleets.Num());
	for(int i = 0; i < CompanyFleets.Num(); i++)
	{
		UFlareFleet* Fleet = CompanyFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName());
	}

	TArray<UFlareSimulatedSpacecraft*> CompanySpacecrafts = Company->GetCompanySpacecrafts();
	FLOGV("  > %d spacecrafts (%d ships and %d stations)", CompanySpacecrafts.Num(), Company->GetCompanyShips().Num(), Company->GetCompanyStations().Num());
	for(int i = 0; i < CompanySpacecrafts.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[i];
		FLOGV("   %2d - %s", i,  *Spacecraft->GetImmatriculation().ToString());
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
	if(Index < 0 || Index > Companies.Num() -1)
	{
		FLOGV("AFlareGame::PrintCompanyByIndex failed: invalid index %d, with %d companies.", Index, Companies.Num());
		return;
	}

	PrintCompany(Companies[Index]->GetShortName());
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


	UFlareFleet* Fleet = FleetCompany->CreateFleet(FleetName, FleetSector);
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

	for(int i = 0; i < GetGameWorld()->GetTravels().Num(); i++)
	{
		UFlareTravel* Travel = GetGameWorld()->GetTravels()[i];
		FLOGV("%2d - %s to %s	", i, *Travel->GetFleet()->GetFleetName(), *Travel->GetDestinationSector()->GetSectorName());
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
	if(Index < 0 || Index > Travels.Num() -1)
	{
		FLOGV("AFlareGame::PrintTravelByIndex failed: invalid index %d, with %d travels.", Index, Travels.Num());
		return;
	}

	UFlareTravel* Travel = GetGameWorld()->GetTravels()[Index];
	FLOGV("> PrintTravel %s to %s", *Travel->GetFleet()->GetFleetName(), *Travel->GetDestinationSector()->GetSectorName());
	FLOGV("  - Departure time: %lld s", Travel->GetDepartureTime());
	FLOGV("  - Elapsed time: %lld s", Travel->GetElapsedTime());

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


	UFlareSimulatedSpacecraft* ShipPawn = NULL;
	AFlarePlayerController* PC = GetPC();

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateShipForMeInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	// TODO, avoid to spawn on a existing ship
	FVector TargetPosition = FVector::ZeroVector;

	ShipPawn = Sector->CreateShip(ShipClass, PC->GetCompany(), TargetPosition);

	return ShipPawn;
}




void UFlareGameTools::PrintSectorList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSectorList failed: no loaded world");
		return;
	}


	FLOGV("> PrintSectorList: %d sectors", GetGameWorld()->GetSectors().Num());

	for(int i = 0; i < GetGameWorld()->GetSectors().Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGameWorld()->GetSectors()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Sector->GetIdentifier().ToString(), *Sector->GetSectorName(), *Sector->GetSectorCode());
	}
}


void UFlareGameTools::PrintSector(FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompany failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);
	if (!Sector)
	{
		FLOGV("AFlareGame::PrintSector failed: no sector with identifier '%s'", * SectorIdentifier.ToString());
		return;
	}

	FLOGV("> PrintSector: %s - %s (%s)", *Sector->GetIdentifier().ToString(), *Sector->GetSectorName(), *Sector->GetSectorCode());

	TArray<UFlareFleet*> SectorFleets = Sector->GetSectorFleets();
	FLOGV("  > %d fleets", SectorFleets.Num());
	for(int i = 0; i < SectorFleets.Num(); i++)
	{
		UFlareFleet* Fleet = SectorFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorShips = Sector->GetSectorShips();
	FLOGV("  > %d ships", SectorShips.Num());
	for(int i = 0; i < SectorShips.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorShips[i];
		FLOGV("   %2d - %s", i,  *Spacecraft->GetImmatriculation().ToString());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorStations = Sector->GetSectorStations();
	FLOGV("  > %d stations", SectorStations.Num());
	for(int i = 0; i < SectorStations.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorStations[i];
		FLOGV("   %2d - %s", i,  *Spacecraft->GetImmatriculation().ToString());
	}
}


void UFlareGameTools::PrintSectorByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSectorByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= GetGameWorld()->GetSectors();
	if(Index < 0 || Index > Sectors.Num() -1)
	{
		FLOGV("AFlareGame::PrintSectorByIndex failed: invalid index %d, with %d sectors.", Index, Sectors.Num());
		return;
	}

	PrintSector(Sectors[Index]->GetIdentifier());
}

/*----------------------------------------------------
	Active Sector tools
----------------------------------------------------*/

AFlareSpacecraft* UFlareGameTools::CreateStationForMe(FName StationClass)
{

	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationForMe failed: no active sector");
		return NULL;
	}

	// TODO
	/*
	AFlareSpacecraft* StationPawn = NULL;
	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
	}

	StationPawn = GetActiveSector()->CreateStation(StationClass, PC->GetCompany(), TargetPosition);

	return StationPawn;*/
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationInCompany failed: no active sector");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if(!Company)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	UFlareSimulatedSector* ActiveSector = GetActiveSector()->GetSimulatedSector();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
	}

	GetGame()->DeactivateSector(PC);

	UFlareSimulatedSpacecraft* NewStation= NULL;

	NewStation = ActiveSector->CreateStation(StationClass, Company, TargetPosition);

	GetGame()->ActivateSector(PC, ActiveSector);

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
	UFlareSimulatedSector* ActiveSector = GetActiveSector()->GetSimulatedSector();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
	}

	GetGame()->DeactivateSector(PC);

	UFlareSimulatedSpacecraft* NewShip = NULL;

	NewShip = ActiveSector->CreateShip(ShipClass, PC->GetCompany(), TargetPosition);
	GetGame()->ActivateSector(PC, ActiveSector);

	return NewShip;
}


AFlareSpacecraft* UFlareGameTools::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
	FLOG("AFlareGame::CreateShipInCompany");
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipInCompany failed: no active sector");
		return NULL;
	}

	//TODO
	/*
	AFlareSpacecraft* ShipPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;

	// Get target position
	AFlareSpacecraft* ExistingShipPawn = GetPC()->GetShipPawn();
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
	}


	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (Company)
	{
		FLOG("UFlareSector::CreateShipInCompany 2");
		ShipPawn = GetActiveSector()->CreateShip(ShipClass, Company, TargetPosition);
	}
	else
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
	}
	return ShipPawn;
	*/
	return NULL;
}

void UFlareGameTools::CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipsInCompany failed: no active sector");
		return;
	}

	//TODO
	/*
	FVector TargetPosition = FVector::ZeroVector;
	FVector BaseShift = FVector::ZeroVector;


	AFlareSpacecraft* ExistingShipPawn = GetPC()->GetShipPawn();
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
		BaseShift = ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(0, 1, 0)); // 100m
	}


	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (Company)
	{
			for (int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
			{
				FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
				GetActiveSector()->CreateShip(ShipClass, Company, TargetPosition + Shift);
			}
	}
	*/
}

void UFlareGameTools::CreateQuickBattle(float Distance, FName Company1Name, FName Company2Name, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateQuickBattle failed: no active sector");
		return;
	}

	//TODO
	/*
	FVector BasePosition = FVector::ZeroVector;
	FVector BaseOffset = FVector(1.f, 0.f, 0.f) * Distance / 50.f; // Half the distance in cm
	FVector BaseShift = FVector(0.f, 30000.f, 0.f);  // 100 m
	FVector BaseDeep = FVector(30000.f, 0.f, 0.f); // 100 m

	UFlareCompany* Company1 = NULL;
	UFlareCompany* Company2 = NULL;

	Company1 = GetGameWorld()->FindCompanyByShortName(Company1Name);
	if (!Company1)
	{
		FLOGV("AFlareGame::CreateQuickBattle failed: no company named '%s'", *Company1Name.ToString());
		return;
	}

	Company2 = GetGameWorld()->FindCompanyByShortName(Company2Name);
	if (!Company2)
	{
		FLOGV("AFlareGame::CreateQuickBattle failed: no company named '%s'", *Company2Name.ToString());
		return;
	}

	// Get target position
	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	if (ExistingShipPawn)
	{
		BasePosition = ExistingShipPawn->GetActorLocation();
		BaseOffset = ExistingShipPawn->GetActorRotation().RotateVector(Distance * 50.f * FVector(1, 0, 0)); // Half the distance in cm
		BaseDeep = ExistingShipPawn->GetActorRotation().RotateVector(FVector(30000.f, 0, 0)); // 100 m in cm
		BaseShift = ExistingShipPawn->GetActorRotation().RotateVector(FVector(0, 30000.f, 0)); // 300 m in cm
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass1Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1 : -1);
		GetActiveSector()->CreateShip(ShipClass1, Company1, BasePosition + BaseOffset + Shift);
		GetActiveSector()->CreateShip(ShipClass1, Company2, BasePosition - BaseOffset - Shift);
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1 : -1);
		GetActiveSector()->CreateShip(ShipClass2, Company1, BasePosition + BaseOffset + Shift + BaseDeep);
		GetActiveSector()->CreateShip(ShipClass2, Company2, BasePosition - BaseOffset - Shift - BaseDeep);
	}
	*/
}



void UFlareGameTools::CreateAsteroid(int32 ID)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateAsteroid failed: no active sector");
		return;
	}

	//TODO
	/*
	AFlarePlayerController* PC = GetPC();

	// Location
	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(20000 * FVector(1, 0, 0));
	}

	GetActiveSector()->CreateAsteroidAt(ID, TargetPosition);
	*/
}

void UFlareGameTools::EmptySector()
{
	FLOG("AFlareGame::EmptySector");
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::EmptySector failed: no active sector");
		return;
	}

	//TODO
	/*
	GetActiveSector()->EmptySector();
	*/
}

void UFlareGameTools::PrintCompanyList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompanyList failed: no loaded world");
		return;
	}


	FLOGV("> PrintCompanyList: %d companies", GetGameWorld()->GetCompanies().Num());

	for(int i = 0; i < GetGameWorld()->GetCompanies().Num(); i++)
	{
		UFlareCompany* Company = GetGameWorld()->GetCompanies()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	}
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
