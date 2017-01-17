
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSaveGame.h"
#include "FlareAsteroid.h"
#include "FlareDebrisField.h"
#include "FlareGameTools.h"
#include "FlareScenarioTools.h"

#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareShell.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../Quests/FlareQuestManager.h"
#include "../Data/FlareQuestCatalog.h"
#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSectorCatalogEntry.h"
#include "Save/FlareSaveGameSystem.h"
#include "AssetRegistryModule.h"
#include "Log/FlareLogWriter.h"

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
{
	// Game classes
	HUDClass = AFlareHUD::StaticClass();
	PlayerControllerClass = AFlarePlayerController::StaticClass();
	DefaultWeaponIdentifier = FName("weapon-eradicator");
	DefaultTurretIdentifier = FName("weapon-artemis");

	// Default asteroid
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultAsteroidObj(TEXT("/Game/Master/Default/SM_Asteroid_Default"));
	DefaultAsteroid = DefaultAsteroidObj.Object;

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
		ConstructorHelpers::FObjectFinder<UFlareCustomizationCatalog> CustomizationCatalog;
		ConstructorHelpers::FObjectFinder<UFlareAsteroidCatalog> AsteroidCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCompanyCatalog> CompanyCatalog;
		ConstructorHelpers::FObjectFinder<UFlareOrbitalMap> OrbitalBodies;

		FConstructorStatics()
			: CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog"))
			, AsteroidCatalog(TEXT("/Game/ThirdParty/Asteroids/AsteroidCatalog"))
			, CompanyCatalog(TEXT("/Game/Gameplay/Catalog/CompanyCatalog"))
			, OrbitalBodies(TEXT("/Game/Gameplay/Catalog/OrbitalMap"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	CustomizationCatalog = ConstructorStatics.CustomizationCatalog.Object;
	AsteroidCatalog = ConstructorStatics.AsteroidCatalog.Object;
	CompanyCatalog = ConstructorStatics.CompanyCatalog.Object;
	OrbitalBodies = ConstructorStatics.OrbitalBodies.Object;

	// Create dynamic objects
	SaveGameSystem = NewObject<UFlareSaveGameSystem>(this, UFlareSaveGameSystem::StaticClass(), TEXT("SaveGameSystem"));
	SpacecraftCatalog = NewObject<UFlareSpacecraftCatalog>(this, UFlareSpacecraftCatalog::StaticClass(), TEXT("FlareSpacecraftCatalog"));
	ShipPartsCatalog = NewObject<UFlareSpacecraftComponentsCatalog>(this, UFlareSpacecraftComponentsCatalog::StaticClass(), TEXT("FlareSpacecraftComponentsCatalog"));
	QuestCatalog = NewObject<UFlareQuestCatalog>(this, UFlareQuestCatalog::StaticClass(), TEXT("FlareQuestCatalog"));
	ResourceCatalog = NewObject<UFlareResourceCatalog>(this, UFlareResourceCatalog::StaticClass(), TEXT("FlareResourceCatalog"));

	// Look for sector assets
	TArray<FAssetData> AssetList;
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
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

		FWeightedBlendable Blendable = PostProcessVolume->Settings.WeightedBlendables.Array.Last();
		UMaterial* MasterMaterial = Cast<UMaterial>(Blendable.Object);
		if (MasterMaterial)
		{
			BlurMaterial = UMaterialInstanceDynamic::Create(MasterMaterial, GetWorld());
			FCHECK(BlurMaterial);
			PostProcessVolume->Settings.RemoveBlendable(MasterMaterial);
			FLOG("AFlareGame::StartPlay : blur material ready");
		}
		else
		{
			FLOG("AFlareGame::StartPlay : no usable material found for blur");
		}
	}
	else
	{
		FLOG("AFlareGame::StartPlay : no post process found");
	}

	// Start the game
	Super::StartPlay();

	// Spawn planetarium if it's not in the level
	TArray<AActor*> PlanetariumCandidates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), PlanetariumClass, PlanetariumCandidates);
	if (PlanetariumCandidates.Num())
	{
		Planetarium = Cast<AFlarePlanetarium>(PlanetariumCandidates.Last());
	}
	else
	{
		Planetarium = GetWorld()->SpawnActor<AFlarePlanetarium>(PlanetariumClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	// Spawn debris field system
	DebrisFieldSystem = NewObject<UFlareDebrisField>(this, UFlareDebrisField::StaticClass());
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
	if (!Sector)
	{
		// No sector to activate
		return;
	}

	// Check if we should really activate
	FLOGV("AFlareGame::ActivateSector : %s", *Sector->GetSectorName().ToString());
	if (ActiveSector)
	{
		FLOG("AFlareGame::ActivateSector : There is already an active sector");
		if (ActiveSector->GetSimulatedSector()->GetIdentifier() == Sector->GetIdentifier())
		{
			// Sector to activate is already active
			return;
		}

		// Deactivate the sector
		DeactivateSector();
	}

	if (ActivatingSector == Sector)
	{
		// Sector to activate is already activating
		return;
	}

	// Load the sector level - Will call OnLevelLoaded()
	ActivatingSector = Sector;
	if(LoadStreamingLevel(Sector->GetDescription()->LevelName))
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
	// No player fleet, create recovery ship
	UFlareCompany* PlayerCompany = GetPC()->GetCompany();

	ScenarioTools->Init(PlayerCompany,GetPC()->GetPlayerData());
	GetPC()->SetPlayerShip(ScenarioTools->CreateRecoveryPlayerShip());
	GetPC()->Load(*GetPC()->GetPlayerData());

	for (int32 CompanyIndex = 0; CompanyIndex < GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* OtherCompany = GetGameWorld()->GetCompanies()[CompanyIndex];

		if (OtherCompany == PlayerCompany)
		{
			continue;
		}

		// Make peace
		OtherCompany->SetHostilityTo(PlayerCompany, false);
		PlayerCompany->SetHostilityTo(OtherCompany, false);

		if (OtherCompany->GetReputation(PlayerCompany) < 0)
		{
			OtherCompany->ForceReputation(PlayerCompany, 0);
		}
	}

	GetPC()->Notify(LOCTEXT("ShipRecovery", "Recovery ship"),
		FText::Format(LOCTEXT("ShipRecoveryFormat", "Your fleet was destroyed. You wake up in a recovery ship in {0}..."),
					  GetPC()->GetPlayerShip()->GetCurrentSector()->GetSectorName()),
		FName("ship-recovery"),
		EFlareNotification::NT_Info);
}

void AFlareGame::SetWorldPause(bool Pause)
{
	DebrisFieldSystem->SetWorldPause(Pause);
}

void AFlareGame::Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation)
{
	DeactivateSector();

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
	UFlareSimulatedSector* CurrentSector = ShipToScrap->GetCurrentSector();

	int64 ScrapRevenue = 0;

	for (int ResourceIndex = 0; ResourceIndex < ShipToScrap->GetDescription()->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &ShipToScrap->GetDescription()->CycleCost.InputResources[ResourceIndex];


		ScrapRevenue += Resource->Quantity * CurrentSector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
		int ResourceToGive = Resource->Quantity;

		ResourceToGive -= ScrapingStation->GetCargoBay()->GiveResources(&Resource->Resource->Data, Resource->Quantity, ScrapingStation->GetCompany());
		// Remaining resources are lost
	}


	ScrapRevenue = FMath::Min(ScrapRevenue, ScrapingStation->GetCompany()->GetMoney());

	FLOGV("Scrap success for %d", ScrapRevenue);

	if (ScrapingStation->GetCompany() != ShipToScrap->GetCompany())
	{
		ScrapingStation->GetCompany()->TakeMoney(ScrapRevenue);
		ShipToScrap->GetCompany()->GiveMoney(ScrapRevenue);
		GetPC()->Notify(LOCTEXT("ShipSellScrap", "Ship scrap complete"),
			FText::Format(LOCTEXT("ShipSellScrapFormat", "Your ship {0} has been scrapped for {1} credits!"), FText::FromString(ShipToScrap->GetImmatriculation().ToString()), FText::AsNumber(UFlareGameTools::DisplayMoney(ScrapRevenue))),
			FName("ship-own-scraped"),
			EFlareNotification::NT_Economy);
	}
	else
	{
		GetPC()->Notify(LOCTEXT("ShipOwnScrap", "Ship scrap complete"),
			FText::Format(LOCTEXT("ShipOwnScrapFormat", "Your ship {0} has been scrapped !"), FText::FromString(ShipToScrap->GetImmatriculation().ToString())),
			FName("ship-own-scraped"),
			EFlareNotification::NT_Economy);
	}

	ShipToScrap->GetCompany()->DestroySpacecraft(ShipToScrap);
}

void AFlareGame::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// This should never fail
	for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
	{
		APawn* Pawn = Iterator->Get();
		FCHECK(Pawn);
	}

	if (QuestManager)
	{
		QuestManager->OnTick(DeltaSeconds);
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

UFlareSaveGame* AFlareGame::ReadSaveSlot(int32 Index)
{
	FString SaveFile = "SaveSlot" + FString::FromInt(Index);
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

void AFlareGame::CreateGame(AFlarePlayerController* PC, FText CompanyName, int32 ScenarioIndex, bool PlayTutorial)
{
	FLOGV("AFlareGame::CreateGame ScenarioIndex %d", ScenarioIndex);
	FLOGV("AFlareGame::CreateGame CompanyName %s", *CompanyName.ToString());

	PlayerController = PC;
	Clean();
	PC->Clean();

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

	// Manually setup the player company before creating it
	FFlareCompanyDescription CompanyData;
	CompanyData.Name = CompanyName;
	CompanyData.ShortName = *FString("PLY");
	CompanyData.Emblem = NULL;
	CompanyData.CustomizationBasePaintColorIndex = 0;
	CompanyData.CustomizationPaintColorIndex = 8;
	CompanyData.CustomizationOverlayColorIndex = 4;
	CompanyData.CustomizationLightColorIndex = 13;
	CompanyData.CustomizationPatternIndex = 0;
	PC->SetCompanyDescription(CompanyData);

	// Player company
	FFlarePlayerSave PlayerData;
	UFlareCompany* PlayerCompany = CreateCompany(-1);
	PlayerCompany->GiveShame(-0.1);
	PlayerData.CompanyIdentifier = PlayerCompany->GetIdentifier();
	PlayerData.UUID = FName(*FGuid::NewGuid().ToString());
	PlayerData.ScenarioId = ScenarioIndex;
	PlayerData.QuestData.PlayTutorial = PlayTutorial;
	PlayerData.QuestData.NextGeneratedQuestIndex = 0;
	PC->SetCompany(PlayerCompany);
	
	// Create world tools
	ScenarioTools = NewObject<UFlareScenarioTools>(this, UFlareScenarioTools::StaticClass());
	ScenarioTools->Init(PlayerCompany, &PlayerData);
	World->PostLoad();

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
	PC->Load(PlayerData);

	// Init the quest manager
	QuestManager = NewObject<UFlareQuestManager>(this, UFlareQuestManager::StaticClass());
	QuestManager->Load(PlayerData.QuestData);

	// End loading
	LoadedOrCreated = true;
	PC->OnLoadComplete();
	FFlareLogWriter::InitWriter(PlayerData.UUID);
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
	CompanyData.Shame = 0.0;
	CompanyData.AI.ConstructionProjectNeedCapacity = 0;
	CompanyData.AI.ConstructionProjectSectorIdentifier = NAME_None;
	CompanyData.AI.ConstructionProjectStationDescriptionIdentifier = NAME_None;
	CompanyData.AI.ConstructionProjectStationIdentifier = NAME_None;
	CompanyData.AI.BudgetMilitary = 0;
	CompanyData.AI.BudgetStation = 0;
	CompanyData.AI.BudgetTechnology = 0;
	CompanyData.AI.BudgetTrade = 0;
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
				
        // TODO check if load is ok for ship event before the PC load

		// Load the player
		PC->Load(Save->PlayerData);
		PC->GetCompany()->SetupEmblem();

		// Create world tools
		ScenarioTools = NewObject<UFlareScenarioTools>(this, UFlareScenarioTools::StaticClass());
		ScenarioTools->Init(PC->GetCompany(), &Save->PlayerData);
		World->PostLoad();
		World->CheckIntegrity();

		// Init the quest manager
		QuestManager = NewObject<UFlareQuestManager>(this, UFlareQuestManager::StaticClass());
		QuestManager->Load(Save->PlayerData.QuestData);

		LoadedOrCreated = true;
		PC->OnLoadComplete();
		FFlareLogWriter::InitWriter(Save->PlayerData.UUID);

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

bool AFlareGame::SaveGame(AFlarePlayerController* PC, bool Async)
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
		Save->WorldData = *World->Save();
		Save->CurrentImmatriculationIndex = CurrentImmatriculationIndex;
		Save->CurrentIdentifierIndex = CurrentIdentifierIndex;
		Save->PlayerData.QuestData = *QuestManager->Save();

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
	GetWorld()->ForceGarbageCollection(true);
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

		UGameplayStatics::LoadStreamLevel(this, SectorLevel, true, true, Info);
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

		UGameplayStatics::UnloadStreamLevel(this, SectorLevel, Info);
		CurrentStreamingLevelIndex++;
		IsLoadingStreamingLevel = true;
	}
}

void AFlareGame::OnLevelLoaded()
{
	IsLoadingStreamingLevel = false;

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
	GetQuestManager()->OnSectorActivation(ActivatingSector);

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

void AFlareGame::Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave)
{
	FString Immatriculation;
	FString NickName;
	CurrentImmatriculationIndex++;
	FFlareSpacecraftDescription* SpacecraftDesc = SpacecraftCatalog->Get(TargetClass);
	bool IsStation = SpacecraftDesc->IsStation();

	// Company name
	Immatriculation += Company->GetShortName().ToString().ToUpper();

	// Class
	Immatriculation += SpacecraftDesc->ImmatriculationCode.ToString().ToUpper();

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

FText AFlareGame::PickCapitalShipName()
{
	if (BaseImmatriculationNameList.Num() == 0)
	{
		InitCapitalShipNameDatabase();
	}
	int32 PickIndex = FMath::RandRange(0,BaseImmatriculationNameList.Num()-1);

	FText BaseName = BaseImmatriculationNameList[PickIndex];

	// Check unicity
	bool Unique;
	int32 NameIncrement = 1;
	FText CandidateName;
	do
	{
		Unique = true;
		FString Suffix;
		if (NameIncrement > 1)
		{
			FString Roman = ConvertToRoman(NameIncrement);
			Suffix = FString(" ") + Roman;
		}
		else
		{
			Suffix = FString("");
		}

		CandidateName = FText::FromString(BaseName.ToString() + Suffix);

		// Browse all existing ships the check if the name is unique
		for (int i = 0; i < GetGameWorld()->GetCompanies().Num(); i++)
		{
			UFlareCompany* Company = GetGameWorld()->GetCompanies()[i];

			// Ships
			for (int32 ShipIndex = 0 ; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
			{
				UFlareSimulatedSpacecraft* SpacecraftCandidate = Company->GetCompanyShips()[ShipIndex];
				if (SpacecraftCandidate && SpacecraftCandidate->GetNickName().ToString() == CandidateName.ToString())
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
	BaseImmatriculationNameList.Add(FText::FromString("Arrow"));
	BaseImmatriculationNameList.Add(FText::FromString("Atom"));
	BaseImmatriculationNameList.Add(FText::FromString("BinaryStar"));
	BaseImmatriculationNameList.Add(FText::FromString("Blackout"));
	BaseImmatriculationNameList.Add(FText::FromString("Crescent"));
	BaseImmatriculationNameList.Add(FText::FromString("Comet"));
	BaseImmatriculationNameList.Add(FText::FromString("Coronation"));
	BaseImmatriculationNameList.Add(FText::FromString("Destiny"));
	BaseImmatriculationNameList.Add(FText::FromString("Duty"));
	BaseImmatriculationNameList.Add(FText::FromString("Enterprise"));
	BaseImmatriculationNameList.Add(FText::FromString("Giant"));
	BaseImmatriculationNameList.Add(FText::FromString("Goliath"));
	BaseImmatriculationNameList.Add(FText::FromString("Hammer"));
	BaseImmatriculationNameList.Add(FText::FromString("Honor"));
	BaseImmatriculationNameList.Add(FText::FromString("Intruder"));
	BaseImmatriculationNameList.Add(FText::FromString("Explorer"));
	BaseImmatriculationNameList.Add(FText::FromString("Kerman"));
	BaseImmatriculationNameList.Add(FText::FromString("Lance"));
	BaseImmatriculationNameList.Add(FText::FromString("Meteor"));
	BaseImmatriculationNameList.Add(FText::FromString("Mammoth"));
	BaseImmatriculationNameList.Add(FText::FromString("Photon"));
	BaseImmatriculationNameList.Add(FText::FromString("Repulse"));
	BaseImmatriculationNameList.Add(FText::FromString("Resolve"));
	BaseImmatriculationNameList.Add(FText::FromString("Revenge"));
	BaseImmatriculationNameList.Add(FText::FromString("Sahara"));
	BaseImmatriculationNameList.Add(FText::FromString("Sovereign"));
	BaseImmatriculationNameList.Add(FText::FromString("Shrike"));
	BaseImmatriculationNameList.Add(FText::FromString("Spearhead"));
	BaseImmatriculationNameList.Add(FText::FromString("Stalker"));
	BaseImmatriculationNameList.Add(FText::FromString("Thunder"));
	BaseImmatriculationNameList.Add(FText::FromString("Unity"));
	BaseImmatriculationNameList.Add(FText::FromString("Valor"));
	BaseImmatriculationNameList.Add(FText::FromString("Winter"));
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

inline const FFlareCompanyDescription* AFlareGame::GetPlayerCompanyDescription() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	return PC->GetCompanyDescription();
}


#undef LOCTEXT_NAMESPACE
