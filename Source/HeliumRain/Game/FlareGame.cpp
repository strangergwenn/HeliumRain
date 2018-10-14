
#include "FlareGame.h"
#include "../Flare.h"

#include "FlareSaveGame.h"
#include "FlareWorld.h"
#include "FlareAsteroid.h"
#include "FlareDebrisField.h"
#include "FlarePlanetarium.h"
#include "FlareGameTools.h"
#include "FlareScenarioTools.h"
#include "FlareSkirmishManager.h"

#include "Save/FlareSaveGameSystem.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"
#include "../Data/FlareMeteoriteCatalog.h"
#include "../Data/FlareAsteroidCatalog.h"
#include "../Data/FlareCompanyCatalog.h"
#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareTechnologyCatalog.h"
#include "../Data/FlareScannableCatalog.h"
#include "../Data/FlareOrbitalMap.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareSectorCatalogEntry.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlareHUD.h"
#include "../Player/FlareMenuPawn.h"
#include "../Player/FlareMenuManager.h"
#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareBomb.h"
#include "../Spacecrafts/FlareShell.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "../Quests/FlareQuestManager.h"

#include "AssetRegistryModule.h"
#include "Log/FlareLogWriter.h"

#include "Engine/PostProcessVolume.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "FlareGame"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareGame::AFlareGame(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CurrentImmatriculationIndex(0)
	, CurrentIdentifierIndex(0)
	, LoadedOrCreated(false)
	, SaveSlotCount(3)
	, CurrentStreamingLevelIndex(0)
	, AutoSave(true)
{
	// Game classes
	HUDClass = AFlareHUD::StaticClass();
	PlayerControllerClass = AFlarePlayerController::StaticClass();
	DefaultWeaponIdentifier = FName("weapon-eradicator");
	DefaultTurretIdentifier = FName("weapon-artemis");

	// Default asteroid
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultAsteroidObj(TEXT("/Game/Master/Default/SM_Asteroid_Default.SM_Asteroid_Default"));
	DefaultAsteroid = DefaultAsteroidObj.Object;
	
	// Data catalogs
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UFlareCustomizationCatalog> CustomizationCatalog;
		ConstructorHelpers::FObjectFinder<UFlareMeteoriteCatalog> MeteoriteCatalog;
		ConstructorHelpers::FObjectFinder<UFlareAsteroidCatalog> AsteroidCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCompanyCatalog> CompanyCatalog;
		ConstructorHelpers::FObjectFinder<UFlareOrbitalMap> OrbitalBodies;

		FConstructorStatics()
			: CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog.CustomizationCatalog"))
			, MeteoriteCatalog(TEXT("/Game/Gameplay/Catalog/MeteoriteCatalog.MeteoriteCatalog"))
			, AsteroidCatalog(TEXT("/Game/ThirdParty/Asteroids/AsteroidCatalog.AsteroidCatalog"))
			, CompanyCatalog(TEXT("/Game/Gameplay/Catalog/CompanyCatalog.CompanyCatalog"))
			, OrbitalBodies(TEXT("/Game/Gameplay/Catalog/OrbitalMap.OrbitalMap"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	CustomizationCatalog = ConstructorStatics.CustomizationCatalog.Object;
	MeteoriteCatalog = ConstructorStatics.MeteoriteCatalog.Object;
	AsteroidCatalog = ConstructorStatics.AsteroidCatalog.Object;
	CompanyCatalog = ConstructorStatics.CompanyCatalog.Object;
	OrbitalBodies = ConstructorStatics.OrbitalBodies.Object;

	// Create dynamic objects
	SaveGameSystem = NewObject<UFlareSaveGameSystem>(this, UFlareSaveGameSystem::StaticClass(), TEXT("SaveGameSystem"));
	SpacecraftCatalog = NewObject<UFlareSpacecraftCatalog>(this, UFlareSpacecraftCatalog::StaticClass(), TEXT("FlareSpacecraftCatalog"));
	ShipPartsCatalog = NewObject<UFlareSpacecraftComponentsCatalog>(this, UFlareSpacecraftComponentsCatalog::StaticClass(), TEXT("FlareSpacecraftComponentsCatalog"));
	QuestCatalog = NewObject<UFlareQuestCatalog>(this, UFlareQuestCatalog::StaticClass(), TEXT("FlareQuestCatalog"));
	ResourceCatalog = NewObject<UFlareResourceCatalog>(this, UFlareResourceCatalog::StaticClass(), TEXT("FlareResourceCatalog"));
	TechnologyCatalog = NewObject<UFlareTechnologyCatalog>(this, UFlareTechnologyCatalog::StaticClass(), TEXT("FlareTechnologyCatalog"));
	ScannableCatalog = NewObject<UFlareScannableCatalog>(this, UFlareScannableCatalog::StaticClass(), TEXT("FlareScannableCatalog"));

	// Look for sector assets
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareSectorCatalogEntry::StaticClass()->GetFName(), AssetList);

	// Do the orbital map setup
	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("AFlareGame::AFlareGame : Found '%s'", *AssetList[Index].GetFullName());
		UFlareSectorCatalogEntry* Sector = Cast<UFlareSectorCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Sector);
		SectorList.Add(Sector);
	}
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareGame::StartPlay()
{
	FLOG("AFlareGame::StartPlay");
	
	// Setup the post process 
	TArray<AActor*> PostProcessCandidates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), PostProcessCandidates);
	if (PostProcessCandidates.Num())
	{
		PostProcessVolume = Cast<APostProcessVolume>(PostProcessCandidates.Last());
		FCHECK(PostProcessVolume);
	}
	else
	{
		FLOG("AFlareGame::StartPlay : no post process found");
	}

	// Start the game
	Super::StartPlay();

	// Spawn planetarium if it's not in the level
	TArray<AActor*> PlanetariumCandidates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlarePlanetarium::StaticClass(), PlanetariumCandidates);
	if (PlanetariumCandidates.Num())
	{
		Planetarium = Cast<AFlarePlanetarium>(PlanetariumCandidates.Last());
	}
	FCHECK(Planetarium);

	// Spawn debris field system
	DebrisFieldSystem = NewObject<UFlareDebrisField>(this, UFlareDebrisField::StaticClass());

	// Spawn skirmish manager
	SkirmishManager = NewObject<UFlareSkirmishManager>(this, UFlareSkirmishManager::StaticClass());
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
	DeactivateSector();
	SaveGame(PC, false);
	PC->PrepareForExit();

	// Exit
	FFlareLogWriter::Shutdown();
	Super::Logout(Player);
}

void AFlareGame::ActivateSector(UFlareSimulatedSector* Sector)
{
	// No sector to activate
	if (!Sector)
	{
		return;
	}

	// Stop processing notifications
	GetPC()->SetBusy(true);

	// Check if we should really activate
	FLOGV("AFlareGame::ActivateSector : %s", *Sector->GetSectorName().ToString());
	if (ActiveSector)
	{
		// Sector to activate is already active
		FLOG("AFlareGame::ActivateSector : There is already an active sector");
		if (ActiveSector->GetSimulatedSector()->GetIdentifier() == Sector->GetIdentifier())
		{
			return;
		}

		// Deactivate the sector
		DeactivateSector();
	}

	// Sector to activate is already activating
	if (ActivatingSector == Sector)
	{
		return;
	}

	// Load the sector level - Will call OnLevelLoaded()
	ActivatingSector = Sector;
	if (LoadStreamingLevel(Sector->GetDescription()->LevelName))
	{
		OnLevelLoaded();
	}
}

void AFlareGame::ActivateCurrentSector()
{
	ActivateSector(GetPC()->GetPlayerShip()->GetCurrentSector());
}

UFlareSimulatedSector* AFlareGame::DeactivateSector()
{
	if (!ActiveSector)
	{
		return NULL;
	}

	UFlareSimulatedSector* Sector = ActiveSector->GetSimulatedSector();
	FLOGV("AFlareGame::DeactivateSector : %s", *Sector->GetSectorName().ToString());
	World->Save();

	// Set last flown ship
	UFlareSimulatedSpacecraft* PlayerShip = NULL;
	if (GetPC()->GetPlayerShip())
	{
		PlayerShip = GetPC()->GetPlayerShip();
	}

	// Destroy the active sector
	DebrisFieldSystem->Reset();
	UnloadStreamingLevel(ActiveSector->GetSimulatedSector()->GetDescription()->LevelName);
	ActiveSector->DestroySector();

	CombatLog::SectorDeactivated(Sector);

	ActiveSector = NULL;

	// Update the PC
	GetPC()->OnSectorDeactivated();

	return Sector;
}

void AFlareGame::Recovery()
{
	// Repair player ship
	FLOGV("AFlareGame::Recovery : player ship=%s", *GetPC()->GetPlayerShip()->GetImmatriculation().ToString());

	GetPC()->GetPlayerShip()->RecoveryRepair();
	GetPC()->GetPlayerShip()->ForceUndock();
	GetPC()->GetPlayerShip()->SetSpawnMode(EFlareSpawnMode::Travel);

	// Take a 5% of the player money
	UFlareCompany* PlayerCompany = GetPC()->GetCompany();
	int64 RecoveryFees = PlayerCompany->GetMoney() * 0.05;
	FLOGV("AFlareGame::Recovery : fees ship=%lld", RecoveryFees);
	GetPC()->GetCompany()->TakeMoney(RecoveryFees, false, FFlareTransactionLogEntry::LogRecoveryFees());
	ScenarioTools->BlueHeart->GetPeople()->Pay(RecoveryFees);

	// Force peace
	for (int32 CompanyIndex = 0; CompanyIndex < GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = GetGameWorld()->GetCompanies()[CompanyIndex];

		if (OtherCompany == PlayerCompany)
		{
			continue;
		}

		// Make peace
		if(OtherCompany->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
		{
			OtherCompany->SetHostilityTo(PlayerCompany, false);
			PlayerCompany->SetHostilityTo(OtherCompany, false);

			OtherCompany->GetAI()->GetData()->Pacifism = FMath::Max(50.f, OtherCompany->GetAI()->GetData()->Pacifism);
		}
	}
}

void AFlareGame::SetWorldPause(bool Pause)
{
	DebrisFieldSystem->SetWorldPause(Pause);
}

void AFlareGame::Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation)
{

	UFlareSimulatedSpacecraft* ShipToScrap = World->FindSpacecraft(ShipImmatriculation);
	UFlareSimulatedSpacecraft* ScrapingStation = World->FindSpacecraft(TargetStationImmatriculation);

	if(!ShipToScrap || !ScrapingStation)
	{
		FLOG("Scrap failed: ship to scrap or station not found");
		return;
	}

	if(ShipToScrap->GetCurrentSector() != ScrapingStation->GetCurrentSector())
	{
		FLOG("Scrap failed: ship and station not in the same sector");
		return;
	}

	if(ShipToScrap->GetCompany()->IsPlayerCompany())
	{
		DeactivateSector();
	}


	UFlareSimulatedSector* CurrentSector = ShipToScrap->GetCurrentSector();

	int64 ScrapRevenue = 0;

	for (int ResourceIndex = 0; ResourceIndex < ShipToScrap->GetDescription()->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &ShipToScrap->GetDescription()->CycleCost.InputResources[ResourceIndex];


		ScrapRevenue += Resource->Quantity * CurrentSector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
		int ResourceToGive = Resource->Quantity;

		ResourceToGive -= ScrapingStation->GetActiveCargoBay()->GiveResources(&Resource->Resource->Data, Resource->Quantity, ScrapingStation->GetCompany());
		// Remaining resources are lost
	}


	ScrapRevenue = FMath::Min(ScrapRevenue, ScrapingStation->GetCompany()->GetMoney());

	FLOGV("Scrap success for %d", ScrapRevenue);

	if (ScrapingStation->GetCompany() != ShipToScrap->GetCompany())
	{
		ScrapingStation->GetCompany()->TakeMoney(ScrapRevenue, false, FFlareTransactionLogEntry::LogScrapCost(ShipToScrap->GetCompany()));
		ShipToScrap->GetCompany()->GiveMoney(ScrapRevenue, FFlareTransactionLogEntry::LogScrapGain(ShipToScrap, ScrapingStation->GetCompany()));

		if(ShipToScrap->GetCompany()->IsPlayerCompany())
		{
			GetPC()->Notify(LOCTEXT("ShipSellScrap", "Ship scrap complete"),
				FText::Format(LOCTEXT("ShipSellScrapFormat", "Your ship {0} has been scrapped for {1} credits!"), UFlareGameTools::DisplaySpacecraftName(ShipToScrap), FText::AsNumber(UFlareGameTools::DisplayMoney(ScrapRevenue))),
				FName("ship-own-scraped"),
				EFlareNotification::NT_Economy);
		}
	}
	else
	{
		if(ShipToScrap->GetCompany()->IsPlayerCompany())
		{
			GetPC()->Notify(LOCTEXT("ShipOwnScrap", "Ship scrap complete"),
				FText::Format(LOCTEXT("ShipOwnScrapFormat", "Your ship {0} has been scrapped !"), UFlareGameTools::DisplaySpacecraftName(ShipToScrap)),
				FName("ship-own-scraped"),
				EFlareNotification::NT_Economy);
		}
	}

	ShipToScrap->GetCompany()->DestroySpacecraft(ShipToScrap);
	if(ShipToScrap->GetCompany()->IsPlayerCompany())
	{
		ActivateCurrentSector();
	}
}

void AFlareGame::ScrapStation(UFlareSimulatedSpacecraft* StationToScrap)
{
	if(!StationToScrap)
	{
		FLOG("Station scrap failed: station to scrap not found");
		return;
	}

	if(!StationToScrap->CanScrapStation())
	{
		FLOG("Station scrap failed: station cannot be to scrapped");
		return;
	}

	DeactivateSector();


	UFlareSimulatedSector* CurrentSector = StationToScrap->GetCurrentSector();
	TMap<FFlareResourceDescription*, int32> ScrapResources = StationToScrap->ComputeScrapResources();

	CurrentSector->DistributeResources(ScrapResources, StationToScrap, StationToScrap->GetCompany(), false);


	FLOG("Station scrap success");


	GetPC()->Notify(LOCTEXT("StationOwnScrap", "Station scrap complete"),
		FText::Format(LOCTEXT("StationOwnScrapFormat", "Your station {0} has been scrapped !"), UFlareGameTools::DisplaySpacecraftName(StationToScrap)),
		FName("station-own-scraped"),
		EFlareNotification::NT_Economy);

	if(StationToScrap->IsComplexElement())
	{
		StationToScrap->GetComplexMaster()->UnregisterComplexElement(StationToScrap);
		StationToScrap->GetComplexMaster()->Reload();
	}

	StationToScrap->GetCompany()->DestroySpacecraft(StationToScrap);

	ActivateCurrentSector();
}

void AFlareGame::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (QuestManager)
	{
		QuestManager->OnTick(DeltaSeconds);
	}

	if(SkirmishManager)
	{
		SkirmishManager->Update(DeltaSeconds);
	}

	if (GetActiveSector() != NULL)
	{
		for (int CompanyIndex = 0; CompanyIndex < GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			GetGameWorld()->GetCompanies()[CompanyIndex]->TickAI();
		}
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
			SaveSlotInfo.UUID = Save->PlayerData.UUID;

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
				SaveSlotInfo.CompanyValue = PlayerCompany->CompanyValue;
				SaveSlotInfo.CompanyName = Desc->Name;

				// Emblem material
				SaveSlotInfo.Emblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
				SaveSlotInfo.Emblem->SetTextureParameterValue("Emblem", GetCustomizationCatalog()->GetEmblem(Save->PlayerData.PlayerEmblemIndex));
				SaveSlotInfo.Emblem->SetVectorParameterValue("BasePaintColor", Desc->CustomizationBasePaintColor);
				SaveSlotInfo.Emblem->SetVectorParameterValue("PaintColor", Desc->CustomizationPaintColor);
				SaveSlotInfo.Emblem->SetVectorParameterValue("OverlayColor", Desc->CustomizationOverlayColor);
				SaveSlotInfo.Emblem->SetVectorParameterValue("GlowColor", Desc->CustomizationLightColor);

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
			SaveSlotInfo.CompanyValue = 0;
			SaveSlotInfo.CompanyName = FText();
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

FString AFlareGame::GetSaveFileName(int32 Index) const
{
	return "SaveSlot" + FString::FromInt(Index);
}

UFlareSaveGame* AFlareGame::ReadSaveSlot(int32 Index)
{
	FString SaveFile = GetSaveFileName(Index);
	UFlareSaveGame* Save = NULL;

	// Prototype load
	if (SaveGameSystem->DoesSaveGameExist(SaveFile))
	{
		FLOG("AFlareGame::ReadSaveSlot : using JSON");
		Save = SaveGameSystem->LoadGame(SaveFile);
	}
	
	if (Save == NULL && UGameplayStatics::DoesSaveGameExist(SaveFile, 0))
	{
		// Try legacy load
		FLOG("AFlareGame::ReadSaveSlot : using legacy");
		Save = Cast<UFlareSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveFile, 0));
	}

	return Save;
}

bool AFlareGame::DeleteSaveSlot(int32 Index)
{
	FString SaveFile = "SaveSlot" + FString::FromInt(Index);

	FLOGV("AFlareGame::DeleteSaveSlot %d", Index);

	if (DoesSaveSlotExist(Index))
	{
		const FFlareSaveSlotInfo& SaveSlotInfo = GetSaveSlotInfo(Index);
		FString FileName1 = FString::Printf(TEXT("%s/SaveGames/Combat-%s.log"), *FPaths::ProjectSavedDir(), *SaveSlotInfo.UUID.ToString());
		FLOGV("Delete %s", *FileName1);

		IFileManager::Get().Delete(*FileName1, true);
		FString FileName2 = FString::Printf(TEXT("%s/SaveGames/Game-%s.log"), *FPaths::ProjectSavedDir(), *SaveSlotInfo.UUID.ToString());
		FLOGV("Delete %s", *FileName2);
		IFileManager::Get().Delete(*FileName2, true);
	}

	bool Deleted = false;
	if (UGameplayStatics::DoesSaveGameExist(SaveFile, 0))
	{
		Deleted |= UGameplayStatics::DeleteGameInSlot(SaveFile, 0);
	}

	if (SaveGameSystem->DoesSaveGameExist(SaveFile))
	{
		Deleted |= SaveGameSystem->DeleteGame(SaveFile);
	}

	return Deleted;
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

void AFlareGame::CreateGame(FFlareCompanyDescription CompanyData, int32 ScenarioIndex, int32 PlayerEmblemIndex, bool PlayTutorial)
{
	// Clean up
	PlayerController = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	Clean();
	PlayerController->Clean();

	// Create the new world
	World = NewObject<UFlareWorld>(this, UFlareWorld::StaticClass());
	FFlareWorldSave WorldData;
	WorldData.Date = 0;
	World->Load(WorldData);
	
	// Create companies
	for (int32 Index = 0; Index < GetCompanyCatalogCount(); Index++)
	{
		CreateCompany(Index);
	}

	// Setup the player company
	PlayerController->SetCompanyDescription(CompanyData);
	FFlarePlayerSave PlayerData;
	UFlareCompany* PlayerCompany = CreateCompany(-1);
	PlayerData.CompanyIdentifier = PlayerCompany->GetIdentifier();
	PlayerData.UUID = FName(*FGuid::NewGuid().ToString());
	PlayerData.ScenarioId = ScenarioIndex;
	PlayerData.PlayerEmblemIndex = PlayerEmblemIndex;
	PlayerData.QuestData.PlayTutorial = PlayTutorial;
	PlayerData.QuestData.NextGeneratedQuestIndex = 0;
	PlayerController->SetCompany(PlayerCompany);
	
	// Create world tools
	ScenarioTools = NewObject<UFlareScenarioTools>(this, UFlareScenarioTools::StaticClass());
	ScenarioTools->Init(PlayerCompany, &PlayerData);
	World->PostLoad();

	// Create scenario
	switch (ScenarioIndex)
	{
		case -1: // Empty
			ScenarioTools->GenerateEmptyScenario();
		break;
		case 0: // Freighter
			ScenarioTools->GenerateFreighterScenario();
		break;
		case 1: // Fighter
			ScenarioTools->GenerateFighterScenario();
		break;
		case 2: // Debug
			ScenarioTools->GenerateDebugScenario();
		break;
	}

	// Load
	PlayerController->Load(PlayerData);

	// Init the quest manager
	QuestManager = NewObject<UFlareQuestManager>(this, UFlareQuestManager::StaticClass());
	QuestManager->Load(PlayerData.QuestData);

	// End loading
	LoadedOrCreated = true;
	PlayerController->OnLoadComplete();
	FFlareLogWriter::InitWriter(PlayerData.UUID);
}

void AFlareGame::CreateSkirmishGame(UFlareSkirmishManager* Skirmish)
{
	// Clean up
	PlayerController = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	Clean();
	PlayerController->Clean();

	// Create the new world
	World = NewObject<UFlareWorld>(this, UFlareWorld::StaticClass());
	FFlareWorldSave WorldData;
	WorldData.Date = 0;
	World->Load(WorldData);

	// Create companies
	for (int32 Index = 0; Index < GetCompanyCatalogCount(); Index++)
	{
		CreateCompany(Index);
	}
	
	// Setup the player company
	PlayerController->SetCompanyDescription(Skirmish->GetData().PlayerCompanyData);
	FFlarePlayerSave PlayerData;
	UFlareCompany* PlayerCompany = CreateCompany(-1);
	PlayerData.CompanyIdentifier = PlayerCompany->GetIdentifier();
	PlayerData.UUID = FName(*FGuid::NewGuid().ToString());
	PlayerData.ScenarioId = -1;
	PlayerData.PlayerEmblemIndex = 0;
	PlayerData.QuestData.PlayTutorial = false;
	PlayerData.QuestData.NextGeneratedQuestIndex = 0;
	PlayerController->SetCompany(PlayerCompany);

	// Create the universe	
	ScenarioTools = NewObject<UFlareScenarioTools>(this, UFlareScenarioTools::StaticClass());
	ScenarioTools->Init(PlayerCompany, &PlayerData);
	
	// Get sector data
	FFlareSectorDescription* SectorDescription = &Skirmish->GetData().SectorDescription;
	FFlareSectorSave SectorSave;
	FFlareSectorOrbitParameters SectorParameters;

	// Copy sector data around
	SectorSave.Identifier = SectorDescription->Identifier;
	SectorSave.GivenName = SectorDescription->Name;
	SectorSave.IsTravelSector = false;
	SectorParameters.CelestialBodyIdentifier = SectorDescription->CelestialBodyIdentifier;
	SectorParameters.Phase = SectorDescription->Phase;

	// Compute altitude
	for (auto Body : GetOrbitalBodies()->OrbitalBodies)
	{
		if (SectorDescription->CelestialBodyIdentifier == Body.CelestialBodyIdentifier)
		{
			float AltitudeRange = Body.MaximalOrbitAltitude - Body.MinimalOrbitAltitude;
			SectorParameters.Altitude = Body.MinimalOrbitAltitude + Skirmish->GetData().SectorAltitude * AltitudeRange;
		}
	}

	// Load sector
	UFlareSimulatedSector* Sector = World->LoadSector(SectorDescription, SectorSave, SectorParameters);
	FCHECK(Sector);
	ScenarioTools->CreateAsteroids(Sector, Skirmish->GetData().AsteroidCount, FVector(75, 15, 15));

	// Setup enemy
	UFlareCompany* EnemyCompany = World->FindCompanyByShortName(Skirmish->GetData().EnemyCompanyName);
	FCHECK(EnemyCompany);

	// Spawn distance between fleets
	float Distance = 5000;
	FVector TargetPosition1 = Distance * FVector(-50, 0, 0);
	FVector TargetPosition2 = Distance * FVector(50, 0, 0);

	// Create player fleet
	UFlareFleet* Fleet = NULL;
	for (auto Order : Skirmish->GetData().Player.OrderedSpacecrafts)
	{
		UFlareSimulatedSpacecraft* Ship = CreateSkirmishSpacecraft(Sector, PlayerCompany, Order, TargetPosition1);

		// Set fleet
		if (Fleet == NULL)
		{
			Fleet = Ship->GetCurrentFleet();
			PlayerData.LastFlownShipIdentifier = Ship->GetImmatriculation();
			PlayerData.PlayerFleetIdentifier = Fleet->GetIdentifier();
		}
		else
		{
			Fleet->AddShip(Ship);
		}
	}

	// Create enemy fleet
	Fleet = NULL;
	for (auto Order : Skirmish->GetData().Enemy.OrderedSpacecrafts)
	{
		UFlareSimulatedSpacecraft* Ship = CreateSkirmishSpacecraft(Sector, EnemyCompany, Order, TargetPosition2);

		// Set fleet
		if (Fleet == NULL)
		{
			Fleet = Ship->GetCurrentFleet();
		}
		else
		{
			Ship->GetCurrentFleet()->Merge(Fleet);
		}
	}

	// Load player
	PlayerController->Load(PlayerData);
	PlayerCompany->SetHostilityTo(EnemyCompany, true);
	EnemyCompany->SetHostilityTo(PlayerCompany, true);

	// End loading
	LoadedOrCreated = true;
	PlayerController->OnLoadComplete();
	FFlareLogWriter::InitWriter(PlayerData.UUID);
}

UFlareSimulatedSpacecraft* AFlareGame::CreateSkirmishSpacecraft(UFlareSimulatedSector* Sector, UFlareCompany* Company, FFlareSkirmishSpacecraftOrder Order, FVector TargetPosition)
{
	const FFlareSpacecraftDescription* ShipDesc = Order.Description;
	UFlareSimulatedSpacecraft* Ship = Sector->CreateSpacecraft(ShipDesc->Identifier, Company, TargetPosition);

	for (auto& Component : Ship->GetData().Components)
	{
		FFlareSpacecraftComponentDescription* ComponentDescription = GetShipPartsCatalog()->Get(Component.ComponentIdentifier);

		if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
		{
			Component.ComponentIdentifier = Order.EngineType;
		}
		else if (ComponentDescription->Type == EFlarePartType::RCS)
		{
			Component.ComponentIdentifier = Order.RCSType;
		}
		if (ComponentDescription->Type == EFlarePartType::Weapon)
		{
			FName SlotName = Component.ShipSlotIdentifier;
			int32 SlotGroupIndex = UFlareSimulatedSpacecraftWeaponsSystem::GetGroupIndexFromSlotIdentifier(Ship->GetDescription(), SlotName);
			FCHECK(SlotGroupIndex >= 0 && SlotGroupIndex < Order.WeaponTypes.Num());

			Component.ComponentIdentifier = Order.WeaponTypes[SlotGroupIndex];
		}
	}

	return Ship;
}

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
	CompanyData.Money = 0;
	CompanyData.FleetImmatriculationIndex = 0;
	CompanyData.TradeRouteImmatriculationIndex = 0;
	CompanyData.PlayerLastPeaceDate = 0;
	CompanyData.PlayerLastTributeDate = 0;
	CompanyData.PlayerLastWarDate = 0;
	CompanyData.ResearchRatio = 1.0;
	CompanyData.ResearchAmount = 0;
	CompanyData.ResearchSpent = 0;
	CompanyData.Retaliation = 0.f;
	CompanyData.AI.BudgetMilitary = 0;
	CompanyData.AI.BudgetStation = 0;
	CompanyData.AI.BudgetTechnology = 0;
	CompanyData.AI.BudgetTrade = 0;
	CompanyData.AI.Caution = 0;
	CompanyData.AI.Pacifism = 50;
	CompanyData.PlayerReputation = 50;
	// Create company
	Company = World->LoadCompany(CompanyData);
	FLOGV("AFlareGame::CreateCompany : Created company '%s'", *Company->GetName());

	Company->PostLoad();

	return Company;
}

bool AFlareGame::LoadGame(AFlarePlayerController* PC)
{
	FLOGV("AFlareGame::LoadGame : loading from slot %d", CurrentSaveIndex);
	PlayerController = PC;
	Clean();
	PC->Clean();

	UFlareSaveGame* Save = ReadSaveSlot(CurrentSaveIndex);

	// Load from save
	if (PC && Save)
	{
		PC->SetCompanyDescription(Save->PlayerCompanyDescription);

        // Create the new world
        World = NewObject<UFlareWorld>(this, UFlareWorld::StaticClass());
		FLOGV("AFlareGame::LoadGame date=%lld", Save->WorldData.Date);
        World->Load(Save->WorldData);
		CurrentImmatriculationIndex = Save->CurrentImmatriculationIndex;
		CurrentIdentifierIndex = Save->CurrentIdentifierIndex;
		AutoSave = Save->AutoSave;
				
        // TODO check if load is ok for ship event before the PC load

		// Load the player
		PC->Load(Save->PlayerData);
		PC->GetCompany()->SetupEmblem();

		// Create world tools
		ScenarioTools = NewObject<UFlareScenarioTools>(this, UFlareScenarioTools::StaticClass());
		ScenarioTools->Init(PC->GetCompany(), &Save->PlayerData);
		World->PostLoad();
		World->CheckIntegrity();
		ScenarioTools->PostLoad();

		// Init the quest manager
		QuestManager = NewObject<UFlareQuestManager>(this, UFlareQuestManager::StaticClass());
		QuestManager->Load(Save->PlayerData.QuestData);

		LoadedOrCreated = true;
		PC->OnLoadComplete();
		FFlareLogWriter::InitWriter(Save->PlayerData.UUID);

		World->ProcessIncomingPlayerEnemy();

		return true;
	}

	// No file existing
	else
	{
		FLOGV("AFlareGame::LoadWorld : could lot load slot %d", CurrentSaveIndex);
		return false;
	}
}


class FAsyncSave : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FAsyncSave>;
public:
	FAsyncSave(UFlareSaveGameSystem* SaveSystemParam, const FString SaveNameParam, UFlareSaveGame *SaveDataParam) :
		SaveName(SaveNameParam),
		SaveData(SaveDataParam),
		SaveSystem(SaveSystemParam)
	{}

protected:
	FString SaveName;
	UFlareSaveGame *SaveData;
	UFlareSaveGameSystem* SaveSystem;

	void DoWork()
	{
		FLOG("Async save start");
		SaveSystem->SaveGame(SaveName, SaveData);
		FLOG("Async save end");
	}

	// This next section of code needs to be here.  Not important as to why.

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncSave, STATGROUP_ThreadPoolAsyncTasks);
	}
};

bool AFlareGame::SaveGame(AFlarePlayerController* PC, bool Async, bool Force)
{
	if (!IsLoadedOrCreated())
	{
		FLOG("AFlareGame::SaveGame : no game loaded, aborting");
		return false;
	}

	if (IsSkirmish())
	{
		FLOG("AFlareGame::SaveGame : skirmish, aborting");
		return false;
	}

	if(!AutoSave && !Force)
	{
		FLOG("AFlareGame::SaveGame : skip save because autosave is saved");
		return true;
	}

	FLOGV("AFlareGame::SaveGame : saving to slot %d", CurrentSaveIndex);
	UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));
	
	// Save process
	if (PC && Save) 
	{
		// Save the player
		PC->Save(Save->PlayerData, Save->PlayerCompanyDescription);
		Save->WorldData = *World->Save();
		Save->CurrentImmatriculationIndex = CurrentImmatriculationIndex;
		Save->CurrentIdentifierIndex = CurrentIdentifierIndex;
		Save->PlayerData.QuestData = *QuestManager->Save();
		Save->AutoSave = AutoSave;

		FLOGV("AFlareGame::SaveGame date=%lld", Save->WorldData.Date);
		// Save
		FString SaveName = "SaveSlot" + FString::FromInt(CurrentSaveIndex);

		// Save prototype

		SaveGameSystem->PushSaveData(Save);

		if(Async)
		{
			(new FAutoDeleteAsyncTask<FAsyncSave>(SaveGameSystem, SaveName, Save))->StartBackgroundTask();
		}
		else
		{
			SaveGameSystem->SaveGame(SaveName, Save);
		}

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

	// Deactivate current sector
	if (ActiveSector)
	{
		UnloadStreamingLevel(ActiveSector->GetSimulatedSector()->GetDescription()->LevelName);
		ActiveSector->DestroySector();
		ActiveSector = NULL;
	}
	DebrisFieldSystem->Reset();

	// Cleanup stuff
	Clean();
	if (GetPC())
	{
		GetPC()->Clean();
	}

	FFlareLogWriter::Shutdown();

	// Consistency check
	int32 ActorCount = 0;
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), ActorList);
	for (int32 Index = 0; Index < ActorList.Num(); Index++)
	{
		if (ActorList[Index]->IsA(AFlareBomb::StaticClass())
		 || ActorList[Index]->IsA(AFlareShell::StaticClass())
		 || ActorList[Index]->IsA(AFlareSpacecraft::StaticClass()))
		{
			ActorCount++;
			FLOGV("AFlareGame::UnloadGame : spurious remaining actor '%s' of class '%s'",
				*ActorList[Index]->GetName(),
				*ActorList[Index]->GetClass()->GetName());
		}
	}
	FCHECK(ActorCount == 0);

	// Force GC
	GEngine->ForceGarbageCollection(true);
}

void AFlareGame::Clean()
{
	World = NULL;
	QuestManager = NULL;
	ActiveSector = NULL;

	LoadedOrCreated = false;

	CurrentImmatriculationIndex = 0;
	CurrentIdentifierIndex = 0;
}


/*----------------------------------------------------
	Level streaming
----------------------------------------------------*/

bool AFlareGame::LoadStreamingLevel(FName SectorLevel)
{
	if (SectorLevel != NAME_None)
	{
		FLOGV("AFlareGame::LoadStreamingLevel : Loading streaming level '%s'", *SectorLevel.ToString());

		FLatentActionInfo Info;
		Info.CallbackTarget = this;
		Info.ExecutionFunction = "OnLevelLoaded";
		Info.UUID = CurrentStreamingLevelIndex;
		Info.Linkage = 0;

		UGameplayStatics::LoadStreamLevel(this, SectorLevel, true, false, Info);
		CurrentStreamingLevelIndex++;
		IsLoadingStreamingLevel = true;
		return false;
	}
	return true;
}

void AFlareGame::UnloadStreamingLevel(FName SectorLevel)
{
	if (SectorLevel != NAME_None)
	{
		FLOGV("AFlareGame::UnloadStreamingLevel : Unloading streaming level '%s'", *SectorLevel.ToString());

		FLatentActionInfo Info;
		Info.CallbackTarget = this;
		Info.ExecutionFunction = "OnLevelUnloaded";
		Info.UUID = CurrentStreamingLevelIndex;
		Info.Linkage = 0;

		UGameplayStatics::UnloadStreamLevel(this, SectorLevel, Info, false);
		CurrentStreamingLevelIndex++;
		IsLoadingStreamingLevel = true;
	}
}

void AFlareGame::OnLevelLoaded()
{
	IsLoadingStreamingLevel = false;
	GetPC()->SetBusy(false);

	// Ensure the current state is correct
	if (ActivatingSector == NULL || ActivatingSector->GetGame()->GetGameWorld() == NULL)
	{
		FLOG("AFlareGame::OnLevelLoaded : no sector");
		return;
	}

	// Ships
	bool PlayerHasShip = false;
	for (int ShipIndex = 0; ShipIndex < ActivatingSector->GetSectorShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = ActivatingSector->GetSectorShips()[ShipIndex];
		if (Ship->GetCompany()->GetPlayerHostility()  == EFlareHostility::Owned)
		{
			PlayerHasShip = true;
			break;
		}
	}

	// Planetarium & sector setup
	FLOGV("AFlareGame::OnLevelLoaded : PlayerHasShip = %d", PlayerHasShip);
	if (PlayerHasShip)
	{
		CombatLog::SectorActivated(ActivatingSector);

		// Create the new sector
		ActiveSector = NewObject<UFlareSector>(this, UFlareSector::StaticClass());
		FFlareSectorSave* SectorData = ActivatingSector->Save();
		if ((SectorData->LocalTime / UFlareGameTools::SECONDS_IN_DAY)  < GetGameWorld()->GetDate())
		{
			// TODO Find time with light
			SectorData->LocalTime = GetGameWorld()->GetDate() * UFlareGameTools::SECONDS_IN_DAY;
		}

		// Load and setup the sector
		Planetarium->ResetTime();
		Planetarium->SkipNight(UFlareGameTools::SECONDS_IN_DAY);
		ActiveSector->Load(ActivatingSector);
		DebrisFieldSystem->Setup(this, ActivatingSector);

		GetPC()->OnSectorActivated(ActiveSector);
	}

	// Quests
	if (GetQuestManager())
	{
		GetQuestManager()->OnSectorActivation(ActivatingSector);
	}

	ActivatingSector = NULL;
}

void AFlareGame::OnLevelUnLoaded()
{
	IsLoadingStreamingLevel = false;
	FLOG("AFlareGame::OnLevelUnLoaded");
}


/*----------------------------------------------------
	Immatriculations
----------------------------------------------------*/

void AFlareGame::Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave, bool IsChildStation)
{
	FString Immatriculation;
	FString NickName;
	FFlareSpacecraftDescription* SpacecraftDesc = SpacecraftCatalog->Get(TargetClass);

	// Create prefix code (company name & class)
	Immatriculation += Company->GetShortName().ToString().ToUpper(); // FString needed here
	Immatriculation += SpacecraftDesc->ImmatriculationCode.ToString().ToUpper(); // FString needed here

	// Generate name
	if (SpacecraftDesc->IsStation() && !IsChildStation)
	{
		FString StationName = SpacecraftDesc->ShortName.ToString();
		if (StationName.Len() == 0)
		{
			StationName = SpacecraftDesc->Name.ToString();
		}
		NickName = PickSpacecraftName(Company, true, "-" + StationName).ToString();
	}
	else if (SpacecraftDesc->Size == EFlarePartSize::L && !IsChildStation)
	{
		NickName = PickSpacecraftName(Company, false, "").ToString();
	}
	else
	{
		NickName = FString::Printf(TEXT("%04d"), CurrentImmatriculationIndex);
		CurrentImmatriculationIndex++;
	}

	// Update data
	Immatriculation += FString::Printf(TEXT("-%s"), *NickName);
	SpacecraftSave->Immatriculation = FName(*Immatriculation);
	SpacecraftSave->NickName = FText::FromString(NickName);
}

FName AFlareGame::GenerateIdentifier(FName BaseName)
{
	FString Identifier;

	Identifier = FString::Printf(TEXT("%s-%d"), *BaseName.ToString(), CurrentIdentifierIndex);

	CurrentIdentifierIndex++;
	return FName(*Identifier);
}

// convertToRoman:
//   In:  val: value to convert.
//        res: buffer to hold result.
//   Out: n/a
//   Cav: caller responsible for buffer size.

FString AFlareGame::ConvertToRoman(uint32 val)
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

FText AFlareGame::PickSpacecraftName(UFlareCompany* OwnerCompany, bool IsStation, FString BaseSuffix)
{
	if (CapitalShipNameList.Num() == 0 || StationNameList.Num() == 0)
	{
		InitSpacecraftNameDatabase();
	}

	// Get a base name
	int32 PickIndex = FMath::RandRange(0, (IsStation ? StationNameList.Num() : CapitalShipNameList.Num()) -1);
	FText BaseName = IsStation ? StationNameList[PickIndex] : CapitalShipNameList[PickIndex];

	// TODO : only take a name that no other company uses

	// Check unicity
	bool Unique = false;
	int32 NameIncrement = 1;
	FString Suffix;
	FString CandidateName;
	do
	{
		Unique = true;

		// Generate suffix text
		if (NameIncrement > 1)
		{
			FString Roman = ConvertToRoman(NameIncrement);
			Suffix = FString("-") + Roman;
		}
		else
		{
			Suffix = FString("");
		}
		CandidateName = BaseName.ToString() + Suffix;

		// Browse all existing ships the check if the name is unique
		for (int i = 0; i < GetGameWorld()->GetCompanies().Num(); i++)
		{
			// List stations
			TArray<UFlareSimulatedSpacecraft*> CandidateShips;
			for (auto& Candidate : GetGameWorld()->GetCompanies()[i]->GetCompanyStations())
			{
				CandidateShips.Add(Candidate);
			}
			for (auto& Candidate : GetGameWorld()->GetCompanies()[i]->GetCompanyChildStations())
			{
				CandidateShips.Add(Candidate);
			}
			for (auto& Candidate : GetGameWorld()->GetCompanies()[i]->GetCompanyShips())
			{
				CandidateShips.Add(Candidate);
			}

			// Check unicity
			for (auto& OtherSpacecraft : CandidateShips)
			{
				// Break up other name to transform "<name>-<type><number>" into "<name>-number>"
				TArray<FString> NickNameParts;
				OtherSpacecraft->GetNickName().ToString().ParseIntoArray(NickNameParts, TEXT("-"));
				FString OtherName;

				if (NickNameParts.Num())
				{
					OtherName = NickNameParts[0];

					// Extract index suffix from the candidate
					if (OtherSpacecraft->IsStation() && NickNameParts.Num() == 3)
					{
						OtherName += "-" + NickNameParts.Last();
					}
					else if (!OtherSpacecraft->IsStation() && NickNameParts.Num() == 2)
					{
						OtherName += "-" + NickNameParts.Last();
					}
				
				}
				else
				{
					OtherName = OtherSpacecraft->GetNickName().ToString();
				}

				if (OtherName == CandidateName)
				{
					Unique = false;
					break;
				}
			}
		}

		NameIncrement++;

	} while(!Unique);

	// Got it !
	CandidateName = BaseName.ToString() + BaseSuffix + Suffix;
	return FText::FromString(CandidateName);
}

void AFlareGame::InitSpacecraftNameDatabase()
{
	StationNameList.Empty();
	StationNameList.Add(FText::FromString("Adrastea"));
	StationNameList.Add(FText::FromString("Aegaeon"));
	StationNameList.Add(FText::FromString("Aegir"));
	StationNameList.Add(FText::FromString("Aitne"));
	StationNameList.Add(FText::FromString("Albiorix"));
	StationNameList.Add(FText::FromString("Amalthea"));
	StationNameList.Add(FText::FromString("Ananke"));
	StationNameList.Add(FText::FromString("Anthe"));
	StationNameList.Add(FText::FromString("Ariel"));
	StationNameList.Add(FText::FromString("Aoede"));
	StationNameList.Add(FText::FromString("Autonoe"));
	StationNameList.Add(FText::FromString("Bebhionn"));
	StationNameList.Add(FText::FromString("Belinda"));
	StationNameList.Add(FText::FromString("Bianca"));
	StationNameList.Add(FText::FromString("Bergelmir"));
	StationNameList.Add(FText::FromString("Bestla"));
	StationNameList.Add(FText::FromString("Bestla"));
	StationNameList.Add(FText::FromString("Caliban"));
	StationNameList.Add(FText::FromString("Callirrhoe"));
	StationNameList.Add(FText::FromString("Callisto"));
	StationNameList.Add(FText::FromString("Calypso"));
	StationNameList.Add(FText::FromString("Carme"));
	StationNameList.Add(FText::FromString("Carpo"));
	StationNameList.Add(FText::FromString("Chaldene"));
	StationNameList.Add(FText::FromString("Cordelia"));
	StationNameList.Add(FText::FromString("Cressida"));
	StationNameList.Add(FText::FromString("Cupid"));
	StationNameList.Add(FText::FromString("Cyllene"));
	StationNameList.Add(FText::FromString("Daphnis"));
	StationNameList.Add(FText::FromString("Deimos"));
	StationNameList.Add(FText::FromString("Desdemona"));
	StationNameList.Add(FText::FromString("Despina"));
	StationNameList.Add(FText::FromString("Dia"));
	StationNameList.Add(FText::FromString("Dione"));
	StationNameList.Add(FText::FromString("Elara"));
	StationNameList.Add(FText::FromString("Erinone"));
	StationNameList.Add(FText::FromString("Euanthe"));
	StationNameList.Add(FText::FromString("Eukelade"));
	StationNameList.Add(FText::FromString("Euporie"));
	StationNameList.Add(FText::FromString("Europa"));
	StationNameList.Add(FText::FromString("Eurydome"));
	StationNameList.Add(FText::FromString("Epimetheus"));
	StationNameList.Add(FText::FromString("Erriapus"));
	StationNameList.Add(FText::FromString("Farbauti"));
	StationNameList.Add(FText::FromString("Fenrir"));
	StationNameList.Add(FText::FromString("Ferdinand"));
	StationNameList.Add(FText::FromString("Francisco"));
	StationNameList.Add(FText::FromString("Fornjot"));
	StationNameList.Add(FText::FromString("Galatea"));
	StationNameList.Add(FText::FromString("Ganymede"));
	StationNameList.Add(FText::FromString("Greip"));
	StationNameList.Add(FText::FromString("Halimede"));
	StationNameList.Add(FText::FromString("Hati"));
	StationNameList.Add(FText::FromString("Harpalyke"));
	StationNameList.Add(FText::FromString("Hegemone"));
	StationNameList.Add(FText::FromString("Helene"));
	StationNameList.Add(FText::FromString("Helike"));
	StationNameList.Add(FText::FromString("Hermippe"));
	StationNameList.Add(FText::FromString("Herse"));
	StationNameList.Add(FText::FromString("Himalia"));
	StationNameList.Add(FText::FromString("Hyperion"));
	StationNameList.Add(FText::FromString("Hyrrokkin"));
	StationNameList.Add(FText::FromString("Io"));
	StationNameList.Add(FText::FromString("Iocast"));
	StationNameList.Add(FText::FromString("Isonoe"));
	StationNameList.Add(FText::FromString("Janus"));
	StationNameList.Add(FText::FromString("Jarnsaxa"));
	StationNameList.Add(FText::FromString("Juliet"));
	StationNameList.Add(FText::FromString("Kale"));
	StationNameList.Add(FText::FromString("Kallichore"));
	StationNameList.Add(FText::FromString("Kalyke"));
	StationNameList.Add(FText::FromString("Kari"));
	StationNameList.Add(FText::FromString("Kiviuq"));
	StationNameList.Add(FText::FromString("Kore"));
	StationNameList.Add(FText::FromString("Laomedeia"));
	StationNameList.Add(FText::FromString("Lapetus"));
	StationNameList.Add(FText::FromString("Larissa"));
	StationNameList.Add(FText::FromString("Leda"));
	StationNameList.Add(FText::FromString("Ljiraq"));
	StationNameList.Add(FText::FromString("Loge"));
	StationNameList.Add(FText::FromString("Lysithea"));
	StationNameList.Add(FText::FromString("Margaret"));
	StationNameList.Add(FText::FromString("Megaclite"));
	StationNameList.Add(FText::FromString("Methone"));
	StationNameList.Add(FText::FromString("Metis"));
	StationNameList.Add(FText::FromString("Mimas"));
	StationNameList.Add(FText::FromString("Miranda"));
	StationNameList.Add(FText::FromString("Mneme"));
	StationNameList.Add(FText::FromString("Mundilfari"));
	StationNameList.Add(FText::FromString("Naiad"));
	StationNameList.Add(FText::FromString("Narvi"));
	StationNameList.Add(FText::FromString("Nereid"));
	StationNameList.Add(FText::FromString("Neso"));
	StationNameList.Add(FText::FromString("Oberon"));
	StationNameList.Add(FText::FromString("Ophelia"));
	StationNameList.Add(FText::FromString("Orthosie"));
	StationNameList.Add(FText::FromString("Paaliaq"));
	StationNameList.Add(FText::FromString("Pallene"));
	StationNameList.Add(FText::FromString("Pan"));
	StationNameList.Add(FText::FromString("Pandora"));
	StationNameList.Add(FText::FromString("Pasiphae"));
	StationNameList.Add(FText::FromString("Pasithee"));
	StationNameList.Add(FText::FromString("Perdita"));
	StationNameList.Add(FText::FromString("Phobos"));
	StationNameList.Add(FText::FromString("Phoebe"));
	StationNameList.Add(FText::FromString("Polydeuces"));
	StationNameList.Add(FText::FromString("Portia"));
	StationNameList.Add(FText::FromString("Praxidike"));
	StationNameList.Add(FText::FromString("Prometheus"));
	StationNameList.Add(FText::FromString("Prospero"));
	StationNameList.Add(FText::FromString("Proteus"));
	StationNameList.Add(FText::FromString("Puck"));
	StationNameList.Add(FText::FromString("Qian"));
	StationNameList.Add(FText::FromString("Rhea"));
	StationNameList.Add(FText::FromString("Rosalind"));
	StationNameList.Add(FText::FromString("Setebos"));
	StationNameList.Add(FText::FromString("Shiarnaq"));
	StationNameList.Add(FText::FromString("Sinope"));
	StationNameList.Add(FText::FromString("Sponde"));
	StationNameList.Add(FText::FromString("Skathi"));
	StationNameList.Add(FText::FromString("Skoll"));
	StationNameList.Add(FText::FromString("Stephano"));
	StationNameList.Add(FText::FromString("Styx"));
	StationNameList.Add(FText::FromString("Suttungr"));
	StationNameList.Add(FText::FromString("Surtur"));
	StationNameList.Add(FText::FromString("Sycorax"));
	StationNameList.Add(FText::FromString("Tarqeq"));
	StationNameList.Add(FText::FromString("Tarvos"));
	StationNameList.Add(FText::FromString("Taygete"));
	StationNameList.Add(FText::FromString("Thebe"));
	StationNameList.Add(FText::FromString("Tethys"));
	StationNameList.Add(FText::FromString("Telesto"));
	StationNameList.Add(FText::FromString("Thalassa"));
	StationNameList.Add(FText::FromString("Thebe"));
	StationNameList.Add(FText::FromString("Thelxinoe"));
	StationNameList.Add(FText::FromString("Themisto"));
	StationNameList.Add(FText::FromString("Thrymr"));
	StationNameList.Add(FText::FromString("Thyone"));
	StationNameList.Add(FText::FromString("Titania"));
	StationNameList.Add(FText::FromString("Umbriel"));
	StationNameList.Add(FText::FromString("Valeska"));
	StationNameList.Add(FText::FromString("Wanda"));
	StationNameList.Add(FText::FromString("Xerxes"));
	StationNameList.Add(FText::FromString("Ymir"));
	StationNameList.Add(FText::FromString("Zeus"));
	
	CapitalShipNameList.Empty();
	CapitalShipNameList.Add(FText::FromString("Arrow"));
	CapitalShipNameList.Add(FText::FromString("Atom"));
	CapitalShipNameList.Add(FText::FromString("Binary_Star"));
	CapitalShipNameList.Add(FText::FromString("Blackout"));
	CapitalShipNameList.Add(FText::FromString("Crescent"));
	CapitalShipNameList.Add(FText::FromString("Comet"));
	CapitalShipNameList.Add(FText::FromString("Coronation"));
	CapitalShipNameList.Add(FText::FromString("Destiny"));
	CapitalShipNameList.Add(FText::FromString("Duty"));
	CapitalShipNameList.Add(FText::FromString("Enterprise"));
	CapitalShipNameList.Add(FText::FromString("Giant"));
	CapitalShipNameList.Add(FText::FromString("Goliath"));
	CapitalShipNameList.Add(FText::FromString("Hammer"));
	CapitalShipNameList.Add(FText::FromString("Honor"));
	CapitalShipNameList.Add(FText::FromString("Intruder"));
	CapitalShipNameList.Add(FText::FromString("Explorer"));
	CapitalShipNameList.Add(FText::FromString("Kerman"));
	CapitalShipNameList.Add(FText::FromString("Lance"));
	CapitalShipNameList.Add(FText::FromString("Meteor"));
	CapitalShipNameList.Add(FText::FromString("Mammoth"));
	CapitalShipNameList.Add(FText::FromString("Photon"));
	CapitalShipNameList.Add(FText::FromString("Repulse"));
	CapitalShipNameList.Add(FText::FromString("Resolve"));
	CapitalShipNameList.Add(FText::FromString("Revenge"));
	CapitalShipNameList.Add(FText::FromString("Sahara"));
	CapitalShipNameList.Add(FText::FromString("Sovereign"));
	CapitalShipNameList.Add(FText::FromString("Shrike"));
	CapitalShipNameList.Add(FText::FromString("Spearhead"));
	CapitalShipNameList.Add(FText::FromString("Stalker"));
	CapitalShipNameList.Add(FText::FromString("Thunder"));
	CapitalShipNameList.Add(FText::FromString("Titan"));
	CapitalShipNameList.Add(FText::FromString("Triton"));
	CapitalShipNameList.Add(FText::FromString("Unity"));
	CapitalShipNameList.Add(FText::FromString("Valor"));
	CapitalShipNameList.Add(FText::FromString("Winter"));
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareCompanyDescription*  AFlareGame::GetCompanyDescription(int32 Index) const
{
	return (CompanyCatalog ? &CompanyCatalog->Companies[Index] : NULL);
}

const FFlareCompanyDescription* AFlareGame::GetPlayerCompanyDescription() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	return PC->GetCompanyDescription();
}

const int32 AFlareGame::GetCompanyCatalogCount() const
{
	return (CompanyCatalog ? CompanyCatalog->Companies.Num() : 0);
}

FText AFlareGame::GetBuildDate() const
{
	return FText::FromString(__DATE__);
}

bool AFlareGame::IsSkirmish() const
{
	return SkirmishManager->IsPlaying();
}

#undef LOCTEXT_NAMESPACE
