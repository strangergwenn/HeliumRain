
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSaveGame.h"
#include "FlareAsteroid.h"
#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareShell.h"


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
	DefaultWeaponIdentifer = FName("weapon-eradicator");
	DefaultTurretIdentifer = FName("weapon-artemis");

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

		FConstructorStatics()
			: SpacecraftCatalog(TEXT("/Game/Gameplay/Catalog/SpacecraftCatalog"))
			, ShipPartsCatalog(TEXT("/Game/Gameplay/Catalog/ShipPartsCatalog"))
			, CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog"))
			, AsteroidCatalog(TEXT("/Game/Gameplay/Catalog/AsteroidCatalog"))
			, CompanyCatalog(TEXT("/Game/Gameplay/Catalog/CompanyCatalog"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	SpacecraftCatalog = ConstructorStatics.SpacecraftCatalog.Object;
	ShipPartsCatalog = ConstructorStatics.ShipPartsCatalog.Object;
	CustomizationCatalog = ConstructorStatics.CustomizationCatalog.Object;
	AsteroidCatalog = ConstructorStatics.AsteroidCatalog.Object;
	CompanyCatalog = ConstructorStatics.CompanyCatalog.Object;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareGame::StartPlay()
{
	FLOG("AFlareGame::StartPlay");
	Super::StartPlay();

	// Create company emblems
	if (CompanyCatalog)
	{
		const TArray<FFlareCompanyDescription>& Companies = CompanyCatalog->Companies;
		UFlareCustomizationCatalog* Catalog = GetCustomizationCatalog();

		for (int32 Index = 0; Index < Companies.Num(); Index++)
		{
			// Create the parameter
			FVector2D EmblemSize = 128 * FVector2D::UnitVector;
			UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());
			UMaterialInstanceDynamic* Emblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
			
			// Setup the material
			Emblem->SetTextureParameterValue("Emblem", Companies[Index].Emblem);
			Emblem->SetVectorParameterValue("BasePaintColor", Catalog->GetColor(Companies[Index].CustomizationBasePaintColorIndex));
			Emblem->SetVectorParameterValue("PaintColor", Catalog->GetColor(Companies[Index].CustomizationPaintColorIndex));
			Emblem->SetVectorParameterValue("OverlayColor", Catalog->GetColor(Companies[Index].CustomizationOverlayColorIndex));
			Emblem->SetVectorParameterValue("GlowColor", Catalog->GetColor(Companies[Index].CustomizationLightColorIndex));
			CompanyEmblems.Add(Emblem);

			// Create the brush dynamically
			FSlateBrush EmblemBrush;
			EmblemBrush.ImageSize = EmblemSize;
			EmblemBrush.SetResourceObject(Emblem);
			CompanyEmblemBrushes.Add(EmblemBrush);
		}
	}

	// Spawn planet
	Planetarium = GetWorld()->SpawnActor<AFlarePlanetarium>(PlanetariumClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (Planetarium)
	{
		Planetarium->SetAltitude(10000);
		Planetarium->SetSunRotation(100);
	}
	else
	{
		FLOG("AFlareGame::StartPlay failed (no planetarium)");
	}
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
	SaveWorld(PC);
	PC->PrepareForExit();

	Super::Logout(Player);
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
			for (int32 ShipIndex = 0; ShipIndex < Save->ShipData.Num(); ShipIndex++)
			{
				const FFlareSpacecraftSave& Spacecraft = Save->ShipData[ShipIndex];
				if (Spacecraft.CompanyIdentifier == Save->PlayerData.CompanyIdentifier)
				{
					SaveSlotInfo.CompanyShipCount++;
				}
			}

			// Find company
			FFlareCompanySave* PlayerCompany = NULL;
			for (int32 CompanyIndex = 0; CompanyIndex < Save->CompanyData.Num(); CompanyIndex++)
			{
				const FFlareCompanySave& Company = Save->CompanyData[CompanyIndex];
				if (Company.Identifier == Save->PlayerData.CompanyIdentifier)
				{
					PlayerCompany = &(Save->CompanyData[CompanyIndex]);
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

void AFlareGame::CreateWorld(AFlarePlayerController* PC, FString CompanyName, int32 ScenarioIndex)
{
	FLOGV("CreateWorld ScenarioIndex %d", ScenarioIndex);
	FLOGV("CreateWorld CompanyName %s", *CompanyName);

	// Create companies
	for (int32 Index = 0; Index < GetCompanyCatalogCount(); Index++)
	{
		CreateCompany(Index);
	}

	// Manually setup the player company before creating it
	FFlareCompanyDescription CompanyData;
	CompanyData.Name = FText::FromString(CompanyName);
	CompanyData.ShortName = *FString("PLY");
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

	switch(ScenarioIndex)
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
	}

	// Load
	PC->Load(PlayerData);
	LoadedOrCreated = true;
	PC->OnLoadComplete();
}

void AFlareGame::InitEmptyScenario(FFlarePlayerSave* PlayerData)
{
	// Player ship
	AFlareSpacecraft* ShipPawn = CreateShipForMe(FName("ship-ghoul"));
	PlayerData->CurrentShipName = ShipPawn->GetImmatriculation();
}


void AFlareGame::InitPeacefulScenario(FFlarePlayerSave* PlayerData)
{
	// Player ship
	AFlareSpacecraft* ShipPawn = CreateShipForMe(FName("ship-ghoul"));
	PlayerData->CurrentShipName = ShipPawn->GetImmatriculation();


	CreateStation("station-hub", PlayerData->CompanyIdentifier, FVector(100000, 3000, 6000), FRotator(12, -166,36));
	CreateStation("station-outpost", PlayerData->CompanyIdentifier, FVector(150000, -10000, -4000), FRotator(93,-154 ,-45));
	CreateStation("station-outpost", PlayerData->CompanyIdentifier, FVector(-80000, -40000, -2000), FRotator(-98, -47,37));


	CreateShip("ship-omen", PlayerData->CompanyIdentifier , FVector(-202600, -65900, -64660));
	CreateShip("ship-omen", PlayerData->CompanyIdentifier , FVector(213890, 97140, -122440));
	CreateShip("ship-omen", PlayerData->CompanyIdentifier , FVector(281160, 20594, -31270));
	CreateShip("ship-omen", PlayerData->CompanyIdentifier , FVector(-195700, -93880, 271180));
	CreateShip("ship-omen", PlayerData->CompanyIdentifier , FVector(88900, -103630, 222380));

	CreateAsteroidAt(0, FVector(29040,7698,-3808));
	CreateAsteroidAt(1, FVector(64105,15792,-28780));
	CreateAsteroidAt(2, FVector(12845,25071,-10792));

	FVector BaseFleetLocation = FVector(-59940, 275780, 75350);

	SetDefaultTurret(FName("weapon-hades-heat"));
	CreateShip("ship-invader", PlayerData->CompanyIdentifier , BaseFleetLocation);

	SetDefaultTurret(FName("weapon-artemis"));
	CreateShip("ship-dragon", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(0, 20000, 0));
	CreateShip("ship-dragon", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(0, -20000, 0));

	SetDefaultWeapon(FName("weapon-wyrm"));
	CreateShip("ship-orca", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(10000, -15000, 0));
	CreateShip("ship-orca", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(10000, 15000, 0));
	CreateShip("ship-orca", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(-10000, -15000, 0));
	CreateShip("ship-orca", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(-10000, 15000, 0));

	SetDefaultWeapon(FName("weapon-eradicator"));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(20000, 0, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(18000, -10000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(18000, 10000, 0));

	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(-20000, -10000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseFleetLocation + FVector(-20000, 10000, 0));
}

void AFlareGame::InitThreatenedScenario(FFlarePlayerSave* PlayerData, UFlareCompany* PlayerCompany)
{

	CreateStation("station-hub", PlayerData->CompanyIdentifier, FVector(10000, -3000, -6000), FRotator(6, -16, 36));

	FVector BaseAllyFleetLocation = FVector(-50000, 0, 50);

	SetDefaultWeapon(FName("weapon-eradicator"));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, -10000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, -20000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, -30000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, -40000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, 10000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, 20000, 0));
	AFlareSpacecraft* ShipPawn = CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, 30000, 0));
	CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BaseAllyFleetLocation + FVector(0, 40000, 0));

	// Player ship
	PlayerData->CurrentShipName = ShipPawn->GetImmatriculation();


	// Enemy
	UFlareCompany* MiningSyndicate = Companies[0];

	FVector BaseEnemyFleetLocation = FVector(600000, 0, -50);

	SetDefaultWeapon(FName("weapon-eradicator"));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, 0, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, -10000, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, -20000, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, -30000, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, 10000, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, 20000, 0));
	CreateShip("ship-ghoul", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(0, 30000, 0));

	// Bombers
	float BomberDistance = 800000;

	SetDefaultWeapon(FName("weapon-wyrm"));
	CreateShip("ship-orca", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(BomberDistance, 0, 0));
	CreateShip("ship-orca", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(BomberDistance, -20000, 0));
	CreateShip("ship-orca", MiningSyndicate->GetIdentifier() , BaseEnemyFleetLocation + FVector(BomberDistance	, 20000, 0));

	DeclareWar(PlayerCompany->GetShortName(), MiningSyndicate->GetShortName());
}


void AFlareGame::InitAggresiveScenario(FFlarePlayerSave* PlayerData, UFlareCompany* PlayerCompany)
{

	// The goal is to attack an heavily defended enemy base

	// A third neutral company have few station and a fleet and will attack you after the  hostile fleet


	// The player army have this composition :

	// - 16 Ghoul/Eradicator to destroy enemy fighter un 2 wave (0km / 2 km)
	// - 10 bombers to destroy enemy invader quickly (3 km)
	// - 2 support invader (refill and repair) : HEAT/Hades (8 km)
	// - 2 attack dragon : 1 AA/Artemis and 1 Hades (4 km but slow)

	// The Alliance Shipbuiding base have :

	// - 3 Hub station
	// - 6 Outpost
	// - 8 asteroid
	// - 5 omen

	// - 10 Ghoul/Eradicator
	// - 3 Orca/Eradicator
	// - 1 Invader/Artemis
	// - 2 Dragon/Hades HEAT


	// The Helix have a small base at 8 km

	// - 1 Hub station
	// - 2 Outpost
	// - 1 asteroid
	// - 3 omen

	// But small but powerfull army
	// - 8 Orca/Eradicator
	// - 1 Invader/Hades HEAT
	// - 1 Dragon/Hades

	// The Helix team will attack when the Alliance is destroyed


	// Companies
	UFlareCompany* Helix = Companies[1];
	UFlareCompany* Sunwatch = Companies[2];


	// Sunwatch base
	FVector BaseSunwatchBaseLocation = FVector(0, 0, 0);
	CreateStation("station-hub", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation + FVector(0, 0, 0), FRotator(-92, 166, -11));
	CreateStation("station-hub", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation + FVector(110944, -1156, 159247), FRotator(12, 139, 119));
	CreateStation("station-hub", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation + FVector(104117, 171446, 153713), FRotator(63, 57, -68));
	CreateStation("station-outpost", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation + FVector(97886, -53237, -23485), FRotator(93,-154 ,-45));
	CreateStation("station-outpost", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation +  FVector(-122594, -22054, -74239), FRotator(-165, -111, 84));
	CreateStation("station-outpost", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation +  FVector(-79444, -36365, -73637), FRotator(36, -74, 14));
	CreateStation("station-outpost", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation +  FVector(40752, -91781, -158555), FRotator(97, -2, -152));
	CreateStation("station-outpost", Sunwatch->GetIdentifier(), BaseSunwatchBaseLocation +  FVector(163550, -139760, -25490), FRotator(134, 132, -153));

	CreateAsteroidAt(0, BaseSunwatchBaseLocation + FVector(73107, 74094, 97755));
	CreateAsteroidAt(1, BaseSunwatchBaseLocation + FVector(12946, 18884, 23809));
	CreateAsteroidAt(2, BaseSunwatchBaseLocation + FVector(-51672, 87149, -52379));
	CreateAsteroidAt(0, BaseSunwatchBaseLocation + FVector(93095, 92590, 32988));
	CreateAsteroidAt(1, BaseSunwatchBaseLocation + FVector(85846, 24798, -770));
	CreateAsteroidAt(2, BaseSunwatchBaseLocation + FVector(19864, -61543, 88115));
	CreateAsteroidAt(0, BaseSunwatchBaseLocation + FVector(128166, 76403, 149982));
	CreateAsteroidAt(1, BaseSunwatchBaseLocation + FVector(-148056, -145663, 126968));



	// Sunwatch fleet
	FVector BaseSunwatchFleetLocation = FVector(300000, 200000, 50000);

	SetDefaultWeapon(FName("weapon-eradicator"));
	for(int i = 0; i < 5; i++)
	{
		CreateShip("ship-ghoul", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(10000 * i , 0, 30000));
		CreateShip("ship-ghoul", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(10000 * i , 10000, -10000));
	}

	SetDefaultWeapon(FName("weapon-eradicator"));
	for(int i = 0; i < 3; i++)
	{
		CreateShip("ship-orca", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(10000 * i , -10000, 5000));
	}

	SetDefaultTurret(FName("weapon-artemis"));
	CreateShip("ship-invader", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(0, -30000, -200000));
	SetDefaultTurret(FName("weapon-hades-heat"));
	CreateShip("ship-dragon", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(20000, -30000, 15000));
	CreateShip("ship-dragon", Sunwatch->GetIdentifier() , BaseSunwatchFleetLocation + FVector(40000, -30000, 15000));



	// Helix base
	FVector BaseHelixBaseLocation = FVector(-300000, 100000, 600000);
	CreateStation("station-hub", Helix->GetIdentifier(), BaseHelixBaseLocation + FVector(0, 0, 0), FRotator(154, 142, 123));
	CreateStation("station-outpost", Helix->GetIdentifier(), BaseHelixBaseLocation + FVector(-12639, 47480, 3364), FRotator(42, -147, 13));
	CreateStation("station-outpost", Helix->GetIdentifier(), BaseHelixBaseLocation +  FVector(4274, 40997, 44388), FRotator(37, 27, -175));

	CreateAsteroidAt(0, BaseHelixBaseLocation	+ FVector(25491, -38851, -26195));

	// Helix fleet
	FVector BaseHelixFleetLocation = BaseHelixBaseLocation + FVector(100000, 0, 0);
	SetDefaultWeapon(FName("weapon-eradicator"));
	for(int i = 0; i < 8; i++)
	{
		CreateShip("ship-orca", Helix->GetIdentifier() , BaseHelixFleetLocation + FVector(10000 * i , -10000, 5000));
	}


	SetDefaultTurret(FName("weapon-hades-heat"));
	CreateShip("ship-invader", Helix->GetIdentifier() , BaseHelixFleetLocation + FVector(0, -30000, -200000));
	SetDefaultTurret(FName("weapon-hades"));
	CreateShip("ship-dragon", Helix->GetIdentifier() , BaseHelixFleetLocation + FVector(20000, -30000, 15000));


	// Player army
	FVector BasePlayerFleetLocation = FVector(-600000, -200000, -50000);

	SetDefaultWeapon(FName("weapon-eradicator"));

	for(int i = -4; i < 4; i++) // 8
	{
		CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(0 , 8000 * i, -5000));
	}

	for(int i = -4; i < 4; i++) // 8
	{
		AFlareSpacecraft* ShipPawn = CreateShip("ship-ghoul", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(-100000, 10000 * i , 5000));
		if( i == 0)
		{
			// Player ship
			PlayerData->CurrentShipName = ShipPawn->GetImmatriculation();
		}
	}
	SetDefaultWeapon(FName("weapon-wyrm"));
	for(int i = -5; i < 5; i++) // 8
	{
		CreateShip("ship-orca", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector( -300000 - 10000 * FMath::Abs(i), 30000 * i, 0));
	}


	SetDefaultTurret(FName("weapon-hades"));
	CreateShip("ship-dragon", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(-600000, -30000, 10000));

	SetDefaultTurret(FName("weapon-artemis"));
	CreateShip("ship-dragon", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(-600000, 30000, -9000));

	SetDefaultTurret(FName("weapon-hades-heat"));
	CreateShip("ship-invader", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(-800000, 0, 20000));
	CreateShip("ship-invader", PlayerData->CompanyIdentifier , BasePlayerFleetLocation + FVector(-700000, 200000, 20000));

	DeclareWar(PlayerCompany->GetShortName(), Sunwatch->GetShortName());
}


AFlareSpacecraft* AFlareGame::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(StationClass);

	if (!Desc)
	{
		Desc = GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition, TargetRotation);
	}
	return NULL;
}

AFlareSpacecraft* AFlareGame::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, CompanyIdentifier, TargetPosition);
	}
	return NULL;
}

AFlareSpacecraft* AFlareGame::CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition, FRotator TargetRotation)
{
	AFlareSpacecraft* ShipPawn = NULL;
	UFlareCompany* Company = FindCompany(CompanyIdentifier);

	if (ShipDescription && Company)
	{
		// Default data
		FFlareSpacecraftSave ShipData;
		ShipData.Location = TargetPosition;
		ShipData.Rotation = TargetRotation;
		ShipData.LinearVelocity = FVector::ZeroVector;
		ShipData.AngularVelocity = FVector::ZeroVector;
		Immatriculate(Company, ShipDescription->Identifier, &ShipData);
		ShipData.Identifier = ShipDescription->Identifier;
		ShipData.Heat = 600 * ShipDescription->HeatCapacity;
		ShipData.PowerOutageDelay = 0;
		ShipData.PowerOutageAcculumator = 0;

		FName RCSIdentifier;
		FName OrbitalEngineIdentifier;

		// Size selector
		if (ShipDescription->Size == EFlarePartSize::S)
		{
			RCSIdentifier = FName("rcs-piranha");
			OrbitalEngineIdentifier = FName("engine-octopus");
		}
		else if (ShipDescription->Size == EFlarePartSize::L)
		{
			RCSIdentifier = FName("rcs-rift");
			OrbitalEngineIdentifier = FName("pod-surtsey");
		}
		else
		{
			// TODO
		}

		for (int32 i = 0; i < ShipDescription->RCSCount; i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = RCSIdentifier;
			ComponentData.ShipSlotIdentifier = FName(*("rcs-" + FString::FromInt(i)));
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}

		for (int32 i = 0; i < ShipDescription->OrbitalEngineCount; i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = OrbitalEngineIdentifier;
			ComponentData.ShipSlotIdentifier = FName(*("engine-" + FString::FromInt(i)));
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}

		for (int32 i = 0; i < ShipDescription->GunSlots.Num(); i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = DefaultWeaponIdentifer;
			ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
			ComponentData.Damage = 0.f;
			ComponentData.Weapon.FiredAmmo = 0;
			ShipData.Components.Add(ComponentData);
		}

		for (int32 i = 0; i < ShipDescription->TurretSlots.Num(); i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = DefaultTurretIdentifer;
			ComponentData.ShipSlotIdentifier = ShipDescription->TurretSlots[i].SlotIdentifier;
			ComponentData.Turret.BarrelsAngle = 0;
			ComponentData.Turret.TurretAngle = 0;
			ComponentData.Weapon.FiredAmmo = 0;
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}

		for (int32 i = 0; i < ShipDescription->InternalComponentSlots.Num(); i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = ShipDescription->InternalComponentSlots[i].ComponentIdentifier;
			ComponentData.ShipSlotIdentifier = ShipDescription->InternalComponentSlots[i].SlotIdentifier;
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}

		// Init pilot
		ShipData.Pilot.Identifier = "chewie";
		ShipData.Pilot.Name = "Chewbaca";

		// Init company
		ShipData.CompanyIdentifier = CompanyIdentifier;

		// Create the ship
		ShipPawn = LoadShip(ShipData);
		FLOGV("AFlareGame::CreateShip : Created ship '%s' at %s", *ShipPawn->GetImmatriculation(), *TargetPosition.ToString());
	}

	return ShipPawn;
}

bool AFlareGame::LoadWorld(AFlarePlayerController* PC)
{
	FLOGV("AFlareGame::LoadWorld : loading from slot %d", CurrentSaveIndex);
	UFlareSaveGame* Save = ReadSaveSlot(CurrentSaveIndex);

	// Load from save
	if (PC && Save)
	{
		// Load
		CurrentImmatriculationIndex = Save->CurrentImmatriculationIndex;

		// Load all companies
		PC->SetCompanyDescription(Save->PlayerCompanyDescription);
		for (int32 i = 0; i < Save->CompanyData.Num(); i++)
		{
			LoadCompany(Save->CompanyData[i]);
		}

		// Load the player
		PC->Load(Save->PlayerData);

		// Load all stations
		for (int32 i = 0; i < Save->StationData.Num(); i++)
		{
			LoadShip(Save->StationData[i]);
		}

		// Load all ships
		for (int32 i = 0; i < Save->ShipData.Num(); i++)
		{
			LoadShip(Save->ShipData[i]);
		}

		// Load all bombs
		for (int32 i = 0; i < Save->BombData.Num(); i++)
		{
			LoadBomb(Save->BombData[i]);
		}

		// Load all asteroids
		for (int32 i = 0; i < Save->AsteroidData.Num(); i++)
		{
			LoadAsteroid(Save->AsteroidData[i]);
		}

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

UFlareCompany* AFlareGame::LoadCompany(const FFlareCompanySave& CompanyData)
{
	UFlareCompany* Company = NULL;

	// Create the new company
	Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
	Company->Load(CompanyData);
	Companies.AddUnique(Company);
	FLOGV("AFlareGame::LoadCompany : loaded '%s'", *Company->GetCompanyName().ToString());

	return Company;
}

AFlareAsteroid* AFlareGame::LoadAsteroid(const FFlareAsteroidSave& AsteroidData)
{
	FActorSpawnParameters Params;
	Params.bNoFail = true;

	AFlareAsteroid* Asteroid = GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), AsteroidData.Location, AsteroidData.Rotation, Params);
	Asteroid->Load(AsteroidData);
	return Asteroid;
}

AFlareSpacecraft* AFlareGame::LoadShip(const FFlareSpacecraftSave& ShipData)
{
	AFlareSpacecraft* Ship = NULL;
	FLOGV("AFlareGame::LoadShip ('%s')", *ShipData.Immatriculation.ToString());

	if (SpacecraftCatalog)
	{
		FFlareSpacecraftDescription* Desc = SpacecraftCatalog->Get(ShipData.Identifier);
		if (Desc)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.bNoFail = true;

			// Create and configure the ship
			Ship = GetWorld()->SpawnActor<AFlareSpacecraft>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
			if (Ship)
			{
				Ship->Load(ShipData);
				UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
				RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
				RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);
			}
			else
			{
				FLOG("AFlareGame::LoadShip fail to create AFlareSpacecraft");
			}
		}
		else
		{
			FLOG("AFlareGame::LoadShip failed (no description available)");
		}
	}
	else
	{
		FLOG("AFlareGame::LoadShip failed (no catalog data)");
	}

	return Ship;
}

AFlareBomb* AFlareGame::LoadBomb(const FFlareBombSave& BombData)
{
	AFlareBomb* Bomb = NULL;
	FLOG("AFlareGame::LoadBomb");

	AFlareSpacecraft* ParentSpacecraft = NULL;

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (SpacecraftCandidate && SpacecraftCandidate->GetImmatriculation() == BombData.ParentSpacecraft)
		{
			ParentSpacecraft = SpacecraftCandidate;
			break;
		}
	}

	if (ParentSpacecraft)
	{
		UFlareWeapon* ParentWeapon = NULL;
		TArray<UActorComponent*> Components = ParentSpacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UFlareWeapon* WeaponCandidate = Cast<UFlareWeapon>(Components[ComponentIndex]);
			if (WeaponCandidate && WeaponCandidate->SlotIdentifier == BombData.WeaponSlotIdentifier)
			{

				ParentWeapon = WeaponCandidate;
				break;
			}
		}

		if (ParentWeapon)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.bNoFail = true;

			// Create and configure the ship
			Bomb = GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombData.Location, BombData.Rotation, Params);
			if (Bomb)
			{
				Bomb->Initialize(&BombData, ParentWeapon);

				UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());

				RootComponent->SetPhysicsLinearVelocity(BombData.LinearVelocity, false);
				RootComponent->SetPhysicsAngularVelocity(BombData.AngularVelocity, false);
			}
			else
			{
				FLOG("AFlareGame::LoadBomb fail to create AFlareBom");
			}
		}
		else
		{
			FLOG("AFlareGame::LoadBomb failed (no parent weapon)");
		}
	}
	else
	{
		FLOG("AFlareGame::LoadBomb failed (no parent ship)");
	}

	return Bomb;
}

bool AFlareGame::SaveWorld(AFlarePlayerController* PC)
{
	if (!IsLoadedOrCreated())
	{
		FLOG("AFlareGame::SaveWorld : no game loaded, aborting");
		return false;
	}

	FLOGV("AFlareGame::SaveWorld : saving to slot %d", CurrentSaveIndex);
	UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));

	// Save process
	if (PC && Save)
	{
		// Save the player
		PC->Save(Save->PlayerData, Save->PlayerCompanyDescription);
		Save->ShipData.Empty();
		Save->CurrentImmatriculationIndex = CurrentImmatriculationIndex;

		// Save all physical ships
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Tentative casts
			AFlareMenuPawn* MenuPawn = PC->GetMenuPawn();
			AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(*ActorItr);
			AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(*ActorItr);

			// Ship
			if (Ship && Ship->GetDescription() && !Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentSpacecraft()))
			{
				FLOGV("AFlareGame::SaveWorld : saving ship ('%s')", *Ship->GetImmatriculation());
				FFlareSpacecraftSave* TempData = Ship->Save();
				Save->ShipData.Add(*TempData);
			}

			// Station
			else if (Ship && Ship->GetDescription() && Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentSpacecraft()))
			{
				FLOGV("AFlareGame::SaveWorld : saving station ('%s')", *Ship->GetImmatriculation());
				FFlareSpacecraftSave* TempData = Ship->Save();
				Save->StationData.Add(*TempData);
			}

			// Asteroid
			else if (Asteroid)
			{
				FLOGV("AFlareGame::SaveWorld : saving asteroid ('%s')", *Asteroid->GetName());
				FFlareAsteroidSave* TempData = Asteroid->Save();
				Save->AsteroidData.Add(*TempData);
			}
		}

		// Companies
		for (int i = 0; i < Companies.Num(); i++)
		{
			UFlareCompany* Company = Companies[i];
			if (Company)
			{
				FLOGV("AFlareGame::SaveWorld : saving company ('%s')", *Company->GetName());
				FFlareCompanySave* TempData = Company->Save();
				Save->CompanyData.Add(*TempData);
			}
		}

		// Bombs
		for (TObjectIterator<AFlareBomb> ObjectItr; ObjectItr; ++ObjectItr)
		{
			AFlareBomb* Bomb = Cast<AFlareBomb>(*ObjectItr);
			if (Bomb && Bomb->IsDropped())
			{
				FLOGV("AFlareGame::SaveWorld : saving bomb ('%s')", *Bomb->GetName());
				FFlareBombSave* TempData = Bomb->Save();
				Save->BombData.Add(*TempData);
			}
		}

		// Save
		UGameplayStatics::SaveGameToSlot(Save, "SaveSlot" + FString::FromInt(CurrentSaveIndex), 0);
		return true;
	}

	// No PC
	else
	{
		FLOG("AFlareGame::SaveWorld failed");
		return false;
	}
}

void AFlareGame::DeleteWorld()
{
	FLOG("AFlareGame::DeleteWorld");
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode())
		{
			SpacecraftCandidate->Destroy();
		}

		AFlareBomb* BombCandidate = Cast<AFlareBomb>(*ActorItr);
		if (BombCandidate)
		{
			BombCandidate->Destroy();
		}

		AFlareShell* ShellCandidate = Cast<AFlareShell>(*ActorItr);
		if (ShellCandidate)
		{
			ShellCandidate->Destroy();
		}

		AFlareAsteroid* AsteroidCandidate = Cast<AFlareAsteroid>(*ActorItr);
		if (AsteroidCandidate)
		{
			AsteroidCandidate->Destroy();
		}
	}

	Companies.Empty();
	LoadedOrCreated = false;
}


/*----------------------------------------------------
	Creation tools
----------------------------------------------------*/

UFlareCompany* AFlareGame::CreateCompany(int32 CatalogIdentifier)
{
	UFlareCompany* Company = NULL;
	FFlareCompanySave CompanyData;

	// Generate identifier
	CurrentImmatriculationIndex++;
	FString Immatriculation = FString::Printf(TEXT("CPNY-%06d"), CurrentImmatriculationIndex);
	CompanyData.Identifier = *Immatriculation;

	// Generate arbitrary save data
	CompanyData.CatalogIdentifier = CatalogIdentifier;
	CompanyData.Money = 100000;

	// Create company
	Company = LoadCompany(CompanyData);
	FLOGV("AFlareGame::CreateCompany : Created company '%s'", *Company->GetName());

	return Company;
}

AFlareSpacecraft* AFlareGame::CreateStationForMe(FName StationClass)
{
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

		StationPawn = CreateStation(StationClass, PC->GetCompany()->GetIdentifier(), TargetPosition);
	}
	return StationPawn;
}

AFlareSpacecraft* AFlareGame::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance)
{
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

	// Find company
	for(int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			StationPawn = CreateStation(StationClass, Company->GetIdentifier(), TargetPosition);
			break;
		}
	}
	return StationPawn;
} 

AFlareSpacecraft* AFlareGame::CreateShipForMe(FName ShipClass)
{
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

		ShipPawn = CreateShip(ShipClass, PC->GetCompany()->GetIdentifier(), TargetPosition);
	}
	return ShipPawn;
}


AFlareSpacecraft* AFlareGame::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
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

	// Find company
	for(int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			ShipPawn = CreateShip(ShipClass, Company->GetIdentifier(), TargetPosition);
			break;
		}
	}
	return ShipPawn;
}

void AFlareGame::CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count)
{
	AFlareSpacecraft* ShipPawn = NULL;
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

	// Find company
	for(int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			for (int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
			{
				FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
				CreateShip(ShipClass, Company->GetIdentifier(), TargetPosition + Shift);
			}

			break;
		}
	}
}

void AFlareGame::CreateQuickBattle(float Distance, FName Company1, FName Company2, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	FVector BasePosition = FVector::ZeroVector;
	FVector BaseOffset = FVector(1.f, 0.f, 0.f) * Distance / 50.f; // Half the distance in cm
	FVector BaseShift = FVector(0.f, 30000.f, 0.f);  // 100 m
	FVector BaseDeep = FVector(30000.f, 0.f, 0.f); // 100 m

	FName Company1Identifier;
	FName Company2Identifier;

	for(int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == Company1)
		{
			Company1Identifier = Company->GetIdentifier();
		}

		if (Company && Company->GetShortName() == Company2)
		{
			Company2Identifier = Company->GetIdentifier();
		}
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
		CreateShip(ShipClass1, Company1Identifier, BasePosition + BaseOffset + Shift);
		CreateShip(ShipClass1, Company2Identifier, BasePosition - BaseOffset - Shift);
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1 : -1);
		CreateShip(ShipClass2, Company1Identifier, BasePosition + BaseOffset + Shift + BaseDeep);
		CreateShip(ShipClass2, Company2Identifier, BasePosition - BaseOffset - Shift - BaseDeep);
	}
}

void AFlareGame::SetDefaultWeapon(FName NewDefaultWeaponIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = ShipPartsCatalog->Get(NewDefaultWeaponIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon)
	{
		DefaultWeaponIdentifer = NewDefaultWeaponIdentifier;
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
		DefaultTurretIdentifer = NewDefaultTurretIdentifier;
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultTurretIdentifier.ToString())
	}
}

void AFlareGame::CreateAsteroid(int32 ID)
{
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

		CreateAsteroidAt(ID, TargetPosition);
	}
}

void AFlareGame::EmptyWorld()
{
	FLOG("AFlareGame::EmptyWorld");

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	AFlareSpacecraft* CurrentPlayedShip = NULL;

	if (PC)
	{
		// Current played ship
		CurrentPlayedShip = PC->GetShipPawn();
	}

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
		if (SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode() && SpacecraftCandidate != CurrentPlayedShip)
		{
			SpacecraftCandidate->Destroy();
		}

		AFlareBomb* BombCandidate = Cast<AFlareBomb>(*ActorItr);
		if (BombCandidate)
		{
			BombCandidate->Destroy();
		}

		AFlareShell* ShellCandidate = Cast<AFlareShell>(*ActorItr);
		if (ShellCandidate)
		{
			ShellCandidate->Destroy();
		}

		AFlareAsteroid* AsteroidCandidate = Cast<AFlareAsteroid>(*ActorItr);
		if (AsteroidCandidate)
		{
			AsteroidCandidate->Destroy();
		}
	}

	UFlareCompany* CurrentShipCompany = NULL;

	if(CurrentPlayedShip)
	{
		CurrentShipCompany = CurrentPlayedShip->GetCompany();
	}

	Companies.Empty();
	Companies.Add(CurrentShipCompany);
}

void AFlareGame::CreateAsteroidAt(int32 ID, FVector Location)
{
	if(ID >= GetAsteroidCatalog()->Asteroids.Num())
	{
		FLOGV("Astroid create fail : Asteroid max ID is %d", GetAsteroidCatalog()->Asteroids.Num() -1);
		return;
	}

	// Spawn parameters
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	FFlareAsteroidSave Data;
	Data.AsteroidMeshID = ID;
	Data.LinearVelocity = FVector::ZeroVector;
	Data.AngularVelocity = FMath::VRand() * FMath::FRandRange(-1.f,1.f);
	Data.Scale = FVector(1,1,1) * FMath::FRandRange(0.9,1.1);
	FRotator Rotation = FRotator(FMath::FRandRange(0,360), FMath::FRandRange(0,360), FMath::FRandRange(0,360));

	// Spawn and setup
	AFlareAsteroid* Asteroid = GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), Location, Rotation, Params);
	Asteroid->Load(Data);

}

void AFlareGame::DeclareWar(FName Company1ShortName, FName Company2ShortName)
{
	UFlareCompany* Company1 = NULL;
	UFlareCompany* Company2 = NULL;

	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == Company1ShortName)
		{
			Company1 = Company;
		}

		if (Company && Company->GetShortName() == Company2ShortName)
		{
			Company2 = Company;
		}
	}

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
	UFlareCompany* Company1 = NULL;
	UFlareCompany* Company2 = NULL;

	for(int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company && Company->GetShortName() == Company1ShortName)
		{
			Company1 = Company;
		}

		if (Company && Company->GetShortName() == Company2ShortName)
		{
			Company2 = Company;
		}
	}

	if(Company1 && Company2)
	{
		Company1->SetHostilityTo(Company2, false);
		Company2->SetHostilityTo(Company1, false);
	}
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
		FLOGV("Pass %d with %s", NameIncrement, *CandidateName.ToString());
		FString Suffix;
		if (NameIncrement > 1)
		{
			FString Roman = ConvertToRoman(NameIncrement);
			FLOGV("ConvertToRoman %s", *Roman);
			Suffix = FString("-") + Roman;
		}
		else
		{
			Suffix = FString("");
		}
		FLOGV("Suffix %s", *Suffix);

		CandidateName = FName(*(BaseName.ToString()+Suffix));
		FLOGV("CandidateName %s", *CandidateName.ToString());

		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if (SpacecraftCandidate && SpacecraftCandidate->GetNickName() == CandidateName)
			{
				FLOGV("Not unique %s", *CandidateName.ToString());
				Unique = false;
				break;
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
		Index = Companies.Find(PC->GetCompany());
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
