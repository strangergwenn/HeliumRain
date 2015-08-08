
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSaveGame.h"
#include "FlareAsteroid.h"

#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareShell.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareGame"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareGame::AFlareGame(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CurrentImmatriculationIndex(0)
	, LoadedOrCreated(false)
	, SaveSlotCount(3)
{
	// Game classes
	HUDClass = AFlareHUD::StaticClass();
	PlayerControllerClass = AFlarePlayerController::StaticClass();
	DefaultWeaponIdentifier = FName("weapon-eradicator");
	DefaultTurretIdentifier = FName("weapon-artemis");

	// Menu pawn
	static ConstructorHelpers::FObjectFinder<UBlueprint> MenuPawnBPClass(TEXT("/Game/Gameplay/Menu/BP_MenuPawn"));
	if (MenuPawnBPClass.Object != NULL)
	{
		MenuPawnClass = (UClass*)MenuPawnBPClass.Object->GeneratedClass;
	}

	// Planetary system
	static ConstructorHelpers::FObjectFinder<UBlueprint> PlanetariumBPClass(TEXT("/Game/Environment/BP_Planetarium"));
	if (PlanetariumBPClass.Object != NULL)
	{
		PlanetariumClass = (UClass*)PlanetariumBPClass.Object->GeneratedClass;
	}

	// Data catalogs
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UFlareSpacecraftCatalog> SpacecraftCatalog;
		ConstructorHelpers::FObjectFinder<UFlareSpacecraftComponentsCatalog> ShipPartsCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCustomizationCatalog> CustomizationCatalog;
		ConstructorHelpers::FObjectFinder<UFlareAsteroidCatalog> AsteroidCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCompanyCatalog> CompanyCatalog;
		ConstructorHelpers::FObjectFinder<UFlareSectorCatalog> SectorCatalog;

		FConstructorStatics()
			: SpacecraftCatalog(TEXT("/Game/Gameplay/Catalog/SpacecraftCatalog"))
			, ShipPartsCatalog(TEXT("/Game/Gameplay/Catalog/ShipPartsCatalog"))
			, CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog"))
			, AsteroidCatalog(TEXT("/Game/Gameplay/Catalog/AsteroidCatalog"))
			, CompanyCatalog(TEXT("/Game/Gameplay/Catalog/CompanyCatalog"))
			, SectorCatalog(TEXT("/Game/Gameplay/Catalog/SectorCatalog"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	SpacecraftCatalog = ConstructorStatics.SpacecraftCatalog.Object;
	ShipPartsCatalog = ConstructorStatics.ShipPartsCatalog.Object;
	CustomizationCatalog = ConstructorStatics.CustomizationCatalog.Object;
	AsteroidCatalog = ConstructorStatics.AsteroidCatalog.Object;
	CompanyCatalog = ConstructorStatics.CompanyCatalog.Object;
	SectorCatalog = ConstructorStatics.SectorCatalog.Object;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareGame::StartPlay()
{
	FLOG("AFlareGame::StartPlay");
	Super::StartPlay();

	// Add competitor's emblems
	if (CompanyCatalog)
	{
		const TArray<FFlareCompanyDescription>& Companies = CompanyCatalog->Companies;
		for (int32 Index = 0; Index < Companies.Num(); Index++)
		{
			AddEmblem(&Companies[Index]);
		}
	}

	// Spawn planet
	Planetarium = GetWorld()->SpawnActor<AFlarePlanetarium>(PlanetariumClass, FVector::ZeroVector, FRotator::ZeroRotator);
}

void AFlareGame::PostLogin(APlayerController* Player)
{
	FLOG("AFlareGame::PostLogin");
	Super::PostLogin(Player);
}

void AFlareGame::Logout(AController* Player)
{
	FLOG("AFlareGame::Logout");

	// Save the world, literally
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
	SaveGame(PC);
	PC->PrepareForExit();

	Super::Logout(Player);
}


void AFlareGame::ActivateSector(AController* Player, UFlareSimulatedSector* Sector)
{
	FLOGV("AFlareGame::ActivateSector %s", *Sector->GetSectorName());
	if (ActiveSector)
	{
		FLOG("AFlareGame::ActivateSector has active sector");
		if (ActiveSector->GetIdentifier() == Sector->GetIdentifier())
		{
			// Sector to activate is already active
			return;
		}

		DeactivateSector(Player);
	}


	FLOGV("AFlareGame::ActivateSector ship count %d", Sector->GetSectorShips().Num());

	bool PlayerHasShip = false;
	for(int ShipIndex = 0; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
		FLOGV("AFlareGame::ActivateSector ship %s", *Ship->GetImmatriculation().ToString());
		FLOGV(" %d", (Ship->GetCompany()->GetPlayerHostility() + 0));

		if (Ship->GetCompany()->GetPlayerHostility()  == EFlareHostility::Owned)
		{
			FLOG("  my ship");
			PlayerHasShip = true;
			break;
		}
	}

	FLOGV("PlayerHasShip %d", PlayerHasShip);
	if (PlayerHasShip)
	{
		// Create the new sector
		ActiveSector = NewObject<UFlareSector>(this, UFlareSector::StaticClass());
		ActiveSector->Load(*Sector->Save(), Sector);

		AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
		PC->OnSectorActivated();
	}
}

void AFlareGame::DeactivateSector(AController* Player)
{
	if (ActiveSector)
	{
		TArray<FFlareSpacecraftSave> SpacecraftData;
		FFlareSectorSave* SectorData = ActiveSector->Save(SpacecraftData);
		ActiveSector->Destroy();
		ActiveSector = NULL;

		//Reload  spacecrafts
		for(int i = 0 ; i < SpacecraftData.Num(); i++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = GetGameWorld()->FindSpacecraft(SpacecraftData[i].Immatriculation);
			Spacecraft->Load(SpacecraftData[i]);
		}

		// Reload sector
		UFlareSimulatedSector* Sector = World->FindSector(SectorData->Identifier);
		if (!Sector)
		{
			FLOGV("ERROR: no simulated sector match for active sector '%s'", *SectorData->Identifier.ToString());
		}

		Sector->Load(Sector->GetDescription(), *SectorData, *Sector->GetOrbitParameters());

		AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
		PC->OnSectorDeactivated();
	}
}

/*----------------------------------------------------
	Save slots
----------------------------------------------------*/

void AFlareGame::ReadAllSaveSlots()
{
	// Setup
	SaveSlots.Empty();
	FVector2D EmblemSize = 128 * FVector2D::UnitVector;
	UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());

	// Get all saves
	for (int32 Index = 1; Index <= SaveSlotCount; Index++)
	{
		FFlareSaveSlotInfo SaveSlotInfo;
		SaveSlotInfo.EmblemBrush.ImageSize = EmblemSize;
		UFlareSaveGame* Save = AFlareGame::ReadSaveSlot(Index);
		SaveSlotInfo.Save = Save;

		if (Save)
		{
			// Basic setup
			UFlareCustomizationCatalog* Catalog = GetCustomizationCatalog();
			FLOGV("AFlareGame::ReadAllSaveSlots : found valid save data in slot %d", Index);

			// Count player ships
			SaveSlotInfo.CompanyShipCount = 0;

			// Find player company and count ships
			FFlareCompanySave* PlayerCompany = NULL;
            for (int32 CompanyIndex = 0; CompanyIndex < Save->WorldData.CompanyData.Num(); CompanyIndex++)
			{
                const FFlareCompanySave& Company = Save->WorldData.CompanyData[CompanyIndex];
				if (Company.Identifier == Save->PlayerData.CompanyIdentifier)
				{
                    PlayerCompany = &(Save->WorldData.CompanyData[CompanyIndex]);
					SaveSlotInfo.CompanyShipCount = Save->WorldData.CompanyData[CompanyIndex].ShipData.Num();
				}
			}

			// Company info
			if (PlayerCompany)
			{
				const FFlareCompanyDescription* Desc = &Save->PlayerCompanyDescription;

				// Money and general infos
				SaveSlotInfo.CompanyMoney = PlayerCompany->Money;
				SaveSlotInfo.CompanyName = Desc->Name;

				// Emblem material
				SaveSlotInfo.Emblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
				SaveSlotInfo.Emblem->SetVectorParameterValue("BasePaintColor", Catalog->GetColor(Desc->CustomizationBasePaintColorIndex));
				SaveSlotInfo.Emblem->SetVectorParameterValue("PaintColor", Catalog->GetColor(Desc->CustomizationPaintColorIndex));
				SaveSlotInfo.Emblem->SetVectorParameterValue("OverlayColor", Catalog->GetColor(Desc->CustomizationOverlayColorIndex));
				SaveSlotInfo.Emblem->SetVectorParameterValue("GlowColor", Catalog->GetColor(Desc->CustomizationLightColorIndex));

				// Create the brush dynamically
				SaveSlotInfo.EmblemBrush.SetResourceObject(SaveSlotInfo.Emblem);
			}
		}
		else
		{
			SaveSlotInfo.Save = NULL;
			SaveSlotInfo.Emblem = NULL;
			SaveSlotInfo.EmblemBrush = FSlateNoResource();
			SaveSlotInfo.CompanyShipCount = 0;
			SaveSlotInfo.CompanyMoney = 0;
			SaveSlotInfo.CompanyName = FText::FromString("");
		}

		SaveSlots.Add(SaveSlotInfo);
	}

	FLOG("AFlareGame::ReadAllSaveSlots : all slots found");
}

int32 AFlareGame::GetSaveSlotCount() const
{
	return SaveSlotCount;
}

int32 AFlareGame::GetCurrentSaveSlot() const
{
	return CurrentSaveIndex;
}

void AFlareGame::SetCurrentSlot(int32 Index)
{
	FLOGV("AFlareGame::SetCurrentSlot : now using slot %d", Index);
	CurrentSaveIndex = Index;
}

bool AFlareGame::DoesSaveSlotExist(int32 Index) const
{
	int32 RealIndex = Index - 1;
	return RealIndex < SaveSlots.Num() && SaveSlots[RealIndex].Save;
}

const FFlareSaveSlotInfo& AFlareGame::GetSaveSlotInfo(int32 Index)
{
	int32 RealIndex = Index - 1;
	return SaveSlots[RealIndex];
}

UFlareSaveGame* AFlareGame::ReadSaveSlot(int32 Index)
{
	FString SaveFile = "SaveSlot" + FString::FromInt(Index);
	if (UGameplayStatics::DoesSaveGameExist(SaveFile, 0))
	{
		UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));
		Save = Cast<UFlareSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveFile, 0));
		return Save;
	}
	else
	{
		return NULL;
	}
}

bool AFlareGame::DeleteSaveSlot(int32 Index)
{
	FString SaveFile = "SaveSlot" + FString::FromInt(Index);
	if (UGameplayStatics::DoesSaveGameExist(SaveFile, 0))
	{
		return UGameplayStatics::DeleteGameInSlot(SaveFile, 0);
	}
	else
	{
		return false;
	}
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

// TODO Rename as CreateGame
void AFlareGame::CreateGame(AFlarePlayerController* PC, FString CompanyName, int32 ScenarioIndex, bool PlayTutorial)
{
	FLOGV("CreateGame ScenarioIndex %d", ScenarioIndex);
	FLOGV("CreateGame CompanyName %s", *CompanyName);

	// Create the new world
	World = NewObject<UFlareWorld>(this, UFlareWorld::StaticClass());
	FFlareWorldSave WorldData;
	WorldData.Time = 475856;
	/*{
		FFlareSectorSave SectorData;
		SectorData.Identifier = "start";
		SectorData.Name = "Nema 1";
		SectorData.Orbit.CelestialBodyIdentifier = "planet-nema";
		SectorData.Orbit.Altitude = 10000;
		SectorData.Orbit.Phase = 0;
		WorldData.SectorData.Add(SectorData);

	}

	{
		FFlareSectorSave SectorData;
		SectorData.Identifier = "nema2";
		SectorData.Name = "Nema 2";
		SectorData.Orbit.CelestialBodyIdentifier = "planet-nema";
		SectorData.Orbit.Altitude = 10000;
		SectorData.Orbit.Phase = 30;
		WorldData.SectorData.Add(SectorData);
	}

	{
		FFlareSectorSave SectorData;
		SectorData.Identifier = "nema3";
		SectorData.Name = "Nema 3";
		SectorData.Orbit.CelestialBodyIdentifier = "planet-nema";
		SectorData.Orbit.Altitude = 100000;
		SectorData.Orbit.Phase = 20;
		WorldData.SectorData.Add(SectorData);
	}

	{
		FFlareSectorSave SectorData;
		SectorData.Identifier = "nema4";
		SectorData.Name = "Nema 4";
		SectorData.Orbit.CelestialBodyIdentifier = "planet-nema";
		SectorData.Orbit.Altitude = 100000;
		SectorData.Orbit.Phase = 60;
		WorldData.SectorData.Add(SectorData);
	}

	{
		FFlareSectorSave SectorData;
		SectorData.Identifier = "nema-moon1-1";
		SectorData.Name = "Nema Moon 1 - 1";
		SectorData.Orbit.CelestialBodyIdentifier = "moon-nema1";
		SectorData.Orbit.Altitude = 1000;
		SectorData.Orbit.Phase = 00;
		WorldData.SectorData.Add(SectorData);
	}*/

	World->Load(WorldData);

	// Create companies
	for (int32 Index = 0; Index < GetCompanyCatalogCount(); Index++)
	{
		CreateCompany(Index);
	}

	// Manually setup the player company before creating it
	FFlareCompanyDescription CompanyData;
	CompanyData.Name = FText::FromString(CompanyName);
	CompanyData.ShortName = *FString("PLY"); // TODO : Extract better short name
	CompanyData.Emblem = NULL; // TODO
	CompanyData.CustomizationBasePaintColorIndex = 0;
	CompanyData.CustomizationPaintColorIndex = 3;
	CompanyData.CustomizationOverlayColorIndex = 6;
	CompanyData.CustomizationLightColorIndex = 13;
	CompanyData.CustomizationPatternIndex = 1;
	PC->SetCompanyDescription(CompanyData);

	// Player company
	FFlarePlayerSave PlayerData;
	UFlareCompany* Company = CreateCompany(-1);
	PlayerData.CompanyIdentifier = Company->GetIdentifier();
	PlayerData.ScenarioId = ScenarioIndex;
	PC->SetCompany(Company);

	// TODO Later with world init
	/*switch(ScenarioIndex)
	{
		case -1: // Empty
			InitEmptyScenario(&PlayerData);
		break;
		case 0: // Peaceful
			InitPeacefulScenario(&PlayerData);
		break;
		case 1: // Threatened
			InitThreatenedScenario(&PlayerData, Company);
		break;
		case 2: // Aggressive
			InitAggresiveScenario(&PlayerData, Company);
		break;
	}*/

	FLOG("CreateGame create initial ship");
	World->FindSector("nema-50k-10")->CreateShip("ship-ghoul", Company, FVector::ZeroVector);
	World->FindSector("nema-10k-30")->CreateShip("ship-atlas", Company, FVector::ZeroVector);
	World->FindSector("nema-50k-0")->CreateShip("ship-dragon", Company, FVector::ZeroVector);
	World->FindSector("moon1-1k-0")->CreateShip("ship-invader", Company, FVector::ZeroVector);



	// Load
	PC->Load(PlayerData);


	LoadedOrCreated = true;
	PC->OnLoadComplete();
}

bool AFlareGame::LoadGame(AFlarePlayerController* PC)
{
	FLOGV("AFlareGame::LoadGame : loading from slot %d", CurrentSaveIndex);
	UFlareSaveGame* Save = ReadSaveSlot(CurrentSaveIndex);

	// Load from save
	if (PC && Save)
	{
		PC->SetCompanyDescription(Save->PlayerCompanyDescription);

        // Create the new world
        World = NewObject<UFlareWorld>(this, UFlareWorld::StaticClass());

		FLOGV("AFlareGame::LoadGame time=%lld", Save->WorldData.Time);

        World->Load(Save->WorldData);
		CurrentImmatriculationIndex = Save->CurrentImmatriculationIndex;
		
        // TODO check if load is ok for ship event before the PC load

		// Load the player
		PC->Load(Save->PlayerData);
		AddEmblem(PC->GetCompanyDescription());
		LoadedOrCreated = true;
		PC->OnLoadComplete();
		return true;
	}

	// No file existing
	else
	{
		FLOGV("AFlareGame::LoadWorld : could lot load slot %d", CurrentSaveIndex);
		return false;
	}
}

bool AFlareGame::SaveGame(AFlarePlayerController* PC)
{
	if (!IsLoadedOrCreated())
	{
		FLOG("AFlareGame::SaveGame : no game loaded, aborting");
		return false;
	}

	FLOGV("AFlareGame::SaveGame : saving to slot %d", CurrentSaveIndex);
	UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));

	// Save process
	if (PC && Save)
	{
		// Save the player
		PC->Save(Save->PlayerData, Save->PlayerCompanyDescription);
		Save->WorldData = *World->Save(ActiveSector);
		Save->CurrentImmatriculationIndex = CurrentImmatriculationIndex;


		FLOGV("AFlareGame::SaveGame time=%lld", Save->WorldData.Time);
		// Save
		UGameplayStatics::SaveGameToSlot(Save, "SaveSlot" + FString::FromInt(CurrentSaveIndex), 0);
		return true;
	}

	// No PC
	else
	{
		FLOG("AFlareGame::SaveGame failed");
		return false;
	}
}

void AFlareGame::UnloadGame()
{
	FLOG("AFlareGame::UnloadGame");

	if (ActiveSector)
	{
		ActiveSector->Destroy();
		ActiveSector = NULL;
	}
	World = NULL;
	LoadedOrCreated = false;
}


/*----------------------------------------------------
	Creation tools
----------------------------------------------------*/

UFlareCompany* AFlareGame::CreateCompany(int32 CatalogIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::CreateCompany failed: no loaded world");
		return NULL;
	}

	UFlareCompany* Company = NULL;
	FFlareCompanySave CompanyData;

	// Generate identifier
	CurrentImmatriculationIndex++;
	FString Immatriculation = FString::Printf(TEXT("CPNY-%06d"), CurrentImmatriculationIndex);
	CompanyData.Identifier = *Immatriculation;

	// Generate arbitrary save data
	CompanyData.CatalogIdentifier = CatalogIdentifier;
	CompanyData.Money = FMath::RandRange(5, 10) * 10000;
	CompanyData.FleetImmatriculationIndex = 0;
	// Create company
	Company = World->LoadCompany(CompanyData);
	FLOGV("AFlareGame::CreateCompany : Created company '%s'", *Company->GetName());

	return Company;
}

UFlareSimulatedSpacecraft* AFlareGame::CreateShipForMeInSector(FName ShipClass, FName SectorIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::CreateShipForMeInSector failed: no world");
		return NULL;
	}

	UFlareSimulatedSpacecraft* ShipPawn = NULL;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	UFlareSimulatedSector* Sector = World->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateShipForMeInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	// Parent company
	if (PC && PC->GetCompany())
	{
		// TODO, avoid to spawn on a existing ship
		FVector TargetPosition = FVector::ZeroVector;

		ShipPawn = Sector->CreateShip(ShipClass, PC->GetCompany(), TargetPosition);
	}
	return ShipPawn;
}


AFlareSpacecraft* AFlareGame::CreateStationForMe(FName StationClass)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateStationForMe failed: no active sector");
		return NULL;
	}

	AFlareSpacecraft* StationPawn = NULL;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	// Parent company
	if (PC && PC->GetCompany())
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		FVector TargetPosition = FVector::ZeroVector;
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}

		StationPawn = ActiveSector->CreateStation(StationClass, PC->GetCompany(), TargetPosition);
	}
	return StationPawn;
}

AFlareSpacecraft* AFlareGame::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateStationInCompany failed: no active sector");
		return NULL;
	}

	AFlareSpacecraft* StationPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
		}
	}

	UFlareCompany* Company = World->FindCompanyByShortName(CompanyShortName);
	if (Company)
	{
		StationPawn = ActiveSector->CreateStation(StationClass, Company, TargetPosition);
	}

	return StationPawn;
} 

AFlareSpacecraft* AFlareGame::CreateShipForMe(FName ShipClass)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateShipForMe failed: no active sector");
		return NULL;
	}

	AFlareSpacecraft* ShipPawn = NULL;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	// Parent company
	if (PC && PC->GetCompany())
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		FVector TargetPosition = FVector::ZeroVector;
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}

		ShipPawn = ActiveSector->CreateShip(ShipClass, PC->GetCompany(), TargetPosition);
	}
	return ShipPawn;
}


AFlareSpacecraft* AFlareGame::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
	FLOG("AFlareGame::CreateShipInCompany");
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateShipInCompany failed: no active sector");
		return NULL;
	}

	AFlareSpacecraft* ShipPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
		}
	}
	else
	{
		FLOG("UFlareSector::CreateShipInCompany failed : No player controller");
	}

	UFlareCompany* Company = World->FindCompanyByShortName(CompanyShortName);
	if (Company)
	{
		FLOG("UFlareSector::CreateShipInCompany 2");
		ShipPawn = ActiveSector->CreateShip(ShipClass, Company, TargetPosition);
	}
	else
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
	}
	return ShipPawn;
}

void AFlareGame::CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateShipsInCompany failed: no active sector");
		return;
	}

	FVector TargetPosition = FVector::ZeroVector;
	FVector BaseShift = FVector::ZeroVector;

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
			BaseShift = ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(0, 1, 0)); // 100m
		}
	}

	UFlareCompany* Company = World->FindCompanyByShortName(CompanyShortName);
	if (Company)
	{
			for (int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
			{
				FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
				ActiveSector->CreateShip(ShipClass, Company, TargetPosition + Shift);
			}
	}
}

void AFlareGame::CreateQuickBattle(float Distance, FName Company1Name, FName Company2Name, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateQuickBattle failed: no active sector");
		return;
	}

	FVector BasePosition = FVector::ZeroVector;
	FVector BaseOffset = FVector(1.f, 0.f, 0.f) * Distance / 50.f; // Half the distance in cm
	FVector BaseShift = FVector(0.f, 30000.f, 0.f);  // 100 m
	FVector BaseDeep = FVector(30000.f, 0.f, 0.f); // 100 m

	UFlareCompany* Company1 = NULL;
	UFlareCompany* Company2 = NULL;

	Company1 = World->FindCompanyByShortName(Company1Name);
	if (!Company1)
	{
		FLOGV("AFlareGame::CreateQuickBattle failed: no company named '%s'", *Company1Name.ToString());
		return;
	}

	Company2 = World->FindCompanyByShortName(Company2Name);
	if (!Company2)
	{
		FLOGV("AFlareGame::CreateQuickBattle failed: no company named '%s'", *Company2Name.ToString());
		return;
	}

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			BasePosition = ExistingShipPawn->GetActorLocation();
			BaseOffset = ExistingShipPawn->GetActorRotation().RotateVector(Distance * 50.f * FVector(1, 0, 0)); // Half the distance in cm
			BaseDeep = ExistingShipPawn->GetActorRotation().RotateVector(FVector(30000.f, 0, 0)); // 100 m in cm
			BaseShift = ExistingShipPawn->GetActorRotation().RotateVector(FVector(0, 30000.f, 0)); // 300 m in cm
		}
	}


	for (int32 ShipIndex = 0; ShipIndex < ShipClass1Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1 : -1);
		ActiveSector->CreateShip(ShipClass1, Company1, BasePosition + BaseOffset + Shift);
		ActiveSector->CreateShip(ShipClass1, Company2, BasePosition - BaseOffset - Shift);
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1 : -1);
		ActiveSector->CreateShip(ShipClass2, Company1, BasePosition + BaseOffset + Shift + BaseDeep);
		ActiveSector->CreateShip(ShipClass2, Company2, BasePosition - BaseOffset - Shift - BaseDeep);
	}
}

void AFlareGame::SetDefaultWeapon(FName NewDefaultWeaponIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = ShipPartsCatalog->Get(NewDefaultWeaponIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon)
	{
		DefaultWeaponIdentifier = NewDefaultWeaponIdentifier;
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultWeaponIdentifier.ToString())
	}
}

void AFlareGame::SetDefaultTurret(FName NewDefaultTurretIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = ShipPartsCatalog->Get(NewDefaultTurretIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
	{
		DefaultTurretIdentifier = NewDefaultTurretIdentifier;
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultTurretIdentifier.ToString())
	}
}

void AFlareGame::CreateAsteroid(int32 ID)
{
	if (!ActiveSector)
	{
		FLOG("AFlareGame::CreateAsteroid failed: no active sector");
		return;
	}

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	if (PC)
	{

		// Location
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
		FVector TargetPosition = FVector::ZeroVector;
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(20000 * FVector(1, 0, 0));
		}

		ActiveSector->CreateAsteroidAt(ID, TargetPosition);
	}
}

void AFlareGame::EmptySector()
{
	FLOG("AFlareGame::EmptySector");
	if (!ActiveSector)
	{
		FLOG("AFlareGame::EmptySector failed: no active sector");
		return;
	}

	ActiveSector->EmptySector();
}

void AFlareGame::DeclareWar(FName Company1ShortName, FName Company2ShortName)
{
	if (!World)
	{
		FLOG("AFlareGame::DeclareWar failed: no loaded world");
		return;
	}

	UFlareCompany* Company1 = World->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = World->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2 && Company1 != Company2)
	{
		FLOGV("Declare war between %s and %s", *Company1->GetCompanyName().ToString(), *Company2->GetCompanyName().ToString());
		Company1->SetHostilityTo(Company2, true);
		Company2->SetHostilityTo(Company1, true);
		
		// Notify war 
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		FText WarString = LOCTEXT("War", "War has been declared");
		FText WarStringInfo = FText::FromString(Company1->GetCompanyName().ToString() + ", " + Company2->GetCompanyName().ToString() + " "
			+ LOCTEXT("WarInfo", "are now at war").ToString());
		PC->Notify(WarString, WarStringInfo, EFlareNotification::NT_Military);
	}
}

void AFlareGame::MakePeace(FName Company1ShortName, FName Company2ShortName)
{
	if (!World)
	{
		FLOG("AFlareGame::MakePeace failed: no loaded world");
		return;
	}

	UFlareCompany* Company1 = World->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = World->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2)
	{
		Company1->SetHostilityTo(Company2, false);
		Company2->SetHostilityTo(Company1, false);
	}
}


void AFlareGame::ForceSectorActivation(FName SectorIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::ForceSectorActivation failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = World->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::ForceSectorActivation failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		ActivateSector(PC, Sector);
	}
}

void AFlareGame::ForceSectorDeactivation()
{
	if (!World)
	{
		FLOG("AFlareGame::ForceSectorDeactivation failed: no loaded world");
		return;
	}

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		DeactivateSector(PC);
	}
}


int64 AFlareGame::GetWorldTime()
{
	if (!World)
	{
		FLOG("AFlareGame::GetWorldTime failed: no loaded world");
		return 0;
	}
	FLOGV("World time: %lld", World->GetTime());
	return World->GetTime();
}

void AFlareGame::SetWorldTime(int64 Time)
{
	if (!World)
	{
		FLOG("AFlareGame::SetWorldTime failed: no loaded world");
		return;
	}
	World->ForceTime(Time);
}


void AFlareGame::Simulate(int64 Duration)
{
	if (!World)
	{
		FLOG("AFlareGame::Simulate failed: no loaded world");
		return;
	}
	World->Simulate(Duration);
}

void AFlareGame::StartTravel(FName FleetIdentifier, FName SectorIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::StartTravel failed: no loaded world");
		return;
	}

	UFlareFleet* Fleet = World->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::StartTravel failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSector* Sector = World->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::StartTravel failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	World->StartTravel(Fleet, Sector);
}

void AFlareGame::CreateFleet(FString FleetName, FName FirstShipImmatriculation)
{
	if (!World)
	{
		FLOG("AFlareGame::CreateFleet failed: no loaded world");
		return;
	}

	UFlareSimulatedSpacecraft* Ship = World->FindSpacecraft(FirstShipImmatriculation);
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

void AFlareGame::DisbandFleet(FName FleetIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::DisbandFleet failed: no loaded world");
		return;
	}

	UFlareFleet* Fleet = World->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::DisbandFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	Fleet->Disband();
}


void AFlareGame::AddToFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!World)
	{
		FLOG("AFlareGame::AddToFleet failed: no loaded world");
		return;
	}

	UFlareFleet* Fleet = World->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::AddToFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = World->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::AddToFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->AddShip(Ship);
}


void AFlareGame::RemoveFromFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!World)
	{
		FLOG("AFlareGame::RemoveFromFleet failed: no loaded world");
		return;
	}

	UFlareFleet* Fleet = World->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = World->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->RemoveShip(Ship);
}

void AFlareGame::MergeFleets(FName Fleet1Identifier, FName Fleet2Identifier)
{
	if (!World)
	{
		FLOG("AFlareGame::MergeFleets failed: no loaded world");
		return;
	}

	UFlareFleet* Fleet1 = World->FindFleet(Fleet1Identifier);
	if (!Fleet1)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet1Identifier.ToString());
		return;
	}

	UFlareFleet* Fleet2 = World->FindFleet(Fleet2Identifier);
	if (!Fleet2)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet2Identifier.ToString());
		return;
	}

	Fleet1->Merge(Fleet2);
}


void AFlareGame::PrintCompanyList()
{
	if (!World)
	{
		FLOG("AFlareGame::PrintCompanyList failed: no loaded world");
		return;
	}


	FLOGV("> PrintCompanyList: %d companies", World->GetCompanies().Num());

	for(int i = 0; i < World->GetCompanies().Num(); i++)
	{
		UFlareCompany* Company = World->GetCompanies()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	}
}


void AFlareGame::PrintCompany(FName CompanyShortName)
{
	if (!World)
	{
		FLOG("AFlareGame::PrintCompany failed: no loaded world");
		return;
	}

	UFlareCompany* Company = World->FindCompanyByShortName(CompanyShortName);
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

void AFlareGame::PrintCompanyByIndex(int32 Index)
{
	if (!World)
	{
		FLOG("AFlareGame::PrintCompanyByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareCompany*> Companies = World->GetCompanies();
	if(Index < 0 || Index > Companies.Num() -1)
	{
		FLOGV("AFlareGame::PrintCompanyByIndex failed: invalid index %d, with %d companies.", Index, Companies.Num());
		return;
	}

	PrintCompany(Companies[Index]->GetShortName());
}



void AFlareGame::PrintSectorList()
{
	if (!World)
	{
		FLOG("AFlareGame::PrintSectorList failed: no loaded world");
		return;
	}


	FLOGV("> PrintSectorList: %d sectors", World->GetSectors().Num());

	for(int i = 0; i < World->GetSectors().Num(); i++)
	{
		UFlareSimulatedSector* Sector = World->GetSectors()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Sector->GetIdentifier().ToString(), *Sector->GetSectorName(), *Sector->GetSectorCode());
	}
}


void AFlareGame::PrintSector(FName SectorIdentifier)
{
	if (!World)
	{
		FLOG("AFlareGame::PrintCompany failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = World->FindSector(SectorIdentifier);
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


void AFlareGame::PrintSectorByIndex(int32 Index)
{
	if (!World)
	{
		FLOG("AFlareGame::PrintSectorByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= World->GetSectors();
	if(Index < 0 || Index > Sectors.Num() -1)
	{
		FLOGV("AFlareGame::PrintSectorByIndex failed: invalid index %d, with %d sectors.", Index, Sectors.Num());
		return;
	}

	PrintSector(Sectors[Index]->GetIdentifier());
}


void AFlareGame::PrintTravelList()
{
	if (!World)
	{
		FLOG("AFlareGame::PrintTravelList failed: no loaded world");
		return;
	}


	FLOGV("> PrintTravelList: %d travels", World->GetTravels().Num());

	for(int i = 0; i < World->GetTravels().Num(); i++)
	{
		UFlareTravel* Travel = World->GetTravels()[i];
		FLOGV("%2d - %s to %s	", i, *Travel->GetFleet()->GetFleetName(), *Travel->GetDestinationSector()->GetSectorName());
	}
}


void AFlareGame::PrintTravelByIndex(int32 Index)
{
	if (!World)
	{
		FLOG("AFlareGame::PrintTravelByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareTravel*>Travels= World->GetTravels();
	if(Index < 0 || Index > Travels.Num() -1)
	{
		FLOGV("AFlareGame::PrintTravelByIndex failed: invalid index %d, with %d travels.", Index, Travels.Num());
		return;
	}

	UFlareTravel* Travel = World->GetTravels()[Index];
	FLOGV("> PrintTravel %s to %s", *Travel->GetFleet()->GetFleetName(), *Travel->GetDestinationSector()->GetSectorName());
	FLOGV("  - Departure time: %lld s", Travel->GetDepartureTime());
	FLOGV("  - Elapsed time: %lld s", Travel->GetElapsedTime());

}


/*----------------------------------------------------
	Immatriculations
----------------------------------------------------*/

void AFlareGame::Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave)
{
	FString Immatriculation;
	FString NickName;
	CurrentImmatriculationIndex++;
	FFlareSpacecraftDescription* SpacecraftDesc = SpacecraftCatalog->Get(TargetClass);
	bool IsStation = IFlareSpacecraftInterface::IsStation(SpacecraftDesc);

	// Company name
	Immatriculation += Company->GetShortName().ToString();
	Immatriculation += "-";

	// Class
	if (IsStation)
	{
		Immatriculation += SpacecraftDesc->Name.ToString();
	}
	else
	{
		Immatriculation += SpacecraftDesc->ImmatriculationCode.ToString();
	}

	// Name
	if (SpacecraftDesc->Size == EFlarePartSize::L && !IsStation)
	{
		NickName = PickCapitalShipName().ToString();
	}
	else
	{
		NickName = FString::Printf(TEXT("%04d"), CurrentImmatriculationIndex);
	}
	Immatriculation += FString::Printf(TEXT("-%s"), *NickName);

	// Update data
	FLOGV("AFlareGame::Immatriculate (%s) : %s", *TargetClass.ToString(), *Immatriculation);
	SpacecraftSave->Immatriculation = FName(*Immatriculation);
	SpacecraftSave->NickName = FName(*NickName);
}

// convertToRoman:
//   In:  val: value to convert.
//        res: buffer to hold result.
//   Out: n/a
//   Cav: caller responsible for buffer size.

static FString ConvertToRoman(unsigned int val)
{
	FString Roman;

	const char *huns[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
	const char *tens[] = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
	const char *ones[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
	int size []  = { 0,   1,    2,    3,     2,   1,    2,     3,      4,    2};

	//  Add 'M' until we drop below 1000.
	while (val >= 1000) {
		Roman += FString("M");
		val -= 1000;
	}

	// Add each of the correct elements, adjusting as we go.

	Roman += FString(huns[val/100]);
	val = val % 100;
	Roman += FString(tens[val/10]);
	val = val % 10;
	Roman += FString(ones[val]);
	return Roman;
}

FName AFlareGame::PickCapitalShipName()
{
	if (BaseImmatriculationNameList.Num() == 0)
	{
		InitCapitalShipNameDatabase();
	}
	int32 PickIndex = FMath::RandRange(0,BaseImmatriculationNameList.Num()-1);

	FName BaseName = BaseImmatriculationNameList[PickIndex];

	// Check unicity
	bool Unique;
	int32 NameIncrement = 1;
	FName CandidateName;
	do
	{
		Unique = true;
		FString Suffix;
		if (NameIncrement > 1)
		{
			FString Roman = ConvertToRoman(NameIncrement);
			Suffix = FString("-") + Roman;
		}
		else
		{
			Suffix = FString("");
		}

		CandidateName = FName(*(BaseName.ToString()+Suffix));

		// Browse all existing ships the check if the name is unique
		// TODO check ship in travel (not in a sector with travels will be implemented)
		for(int SectorIndex = 0; SectorIndex < World->GetSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = World->GetSectors()[SectorIndex];

			for(int ShipIndex = 0; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
			{
				UFlareSimulatedSpacecraft* SpacecraftCandidate = Sector->GetSectorShips()[ShipIndex];
				if (SpacecraftCandidate && SpacecraftCandidate->GetNickName() == CandidateName)
				{
					FLOGV("Not unique %s", *CandidateName.ToString());
					Unique = false;
					break;
				}
			}
		}
		NameIncrement++;
	} while(!Unique);

	FLOGV("OK for %s", *CandidateName.ToString());

	return CandidateName;
}

void AFlareGame::InitCapitalShipNameDatabase()
{
	BaseImmatriculationNameList.Empty();
	BaseImmatriculationNameList.Add("Revenge");
	BaseImmatriculationNameList.Add("Sovereign");
	BaseImmatriculationNameList.Add("Stalker");
	BaseImmatriculationNameList.Add("Leviathan");
	BaseImmatriculationNameList.Add("Resolve");
	BaseImmatriculationNameList.Add("Explorer");
	BaseImmatriculationNameList.Add("Arrow");
	BaseImmatriculationNameList.Add("Intruder");
	BaseImmatriculationNameList.Add("Goliath");
	BaseImmatriculationNameList.Add("Shrike");
	BaseImmatriculationNameList.Add("Thunder");
	BaseImmatriculationNameList.Add("Enterprise");
	BaseImmatriculationNameList.Add("Sahara");
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlareGame::AddEmblem(const FFlareCompanyDescription* Company)
{
	// Create the parameter
	FVector2D EmblemSize = 128 * FVector2D::UnitVector;
	UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());
	UMaterialInstanceDynamic* Emblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
	UFlareCustomizationCatalog* Catalog = GetCustomizationCatalog();

	// Setup the material
	Emblem->SetTextureParameterValue("Emblem", Company->Emblem);
	Emblem->SetVectorParameterValue("BasePaintColor", Catalog->GetColor(Company->CustomizationBasePaintColorIndex));
	Emblem->SetVectorParameterValue("PaintColor", Catalog->GetColor(Company->CustomizationPaintColorIndex));
	Emblem->SetVectorParameterValue("OverlayColor", Catalog->GetColor(Company->CustomizationOverlayColorIndex));
	Emblem->SetVectorParameterValue("GlowColor", Catalog->GetColor(Company->CustomizationLightColorIndex));
	CompanyEmblems.Add(Emblem);

	// Create the brush dynamically
	FSlateBrush EmblemBrush;
	EmblemBrush.ImageSize = EmblemSize;
	EmblemBrush.SetResourceObject(Emblem);
	CompanyEmblemBrushes.Add(EmblemBrush);
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

inline const FFlareCompanyDescription* AFlareGame::GetPlayerCompanyDescription() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	return PC->GetCompanyDescription();
}

inline const FSlateBrush* AFlareGame::GetCompanyEmblem(int32 Index) const
{
	// Player company
	if (Index == -1)
	{
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		Index = World->GetCompanies().Find(PC->GetCompany());
	}

	// General case
	if (Index >= 0 && Index < CompanyEmblemBrushes.Num())
	{
		return &CompanyEmblemBrushes[Index];
	}
	else
	{
		return NULL;
	}
}

#undef LOCTEXT_NAMESPACE
