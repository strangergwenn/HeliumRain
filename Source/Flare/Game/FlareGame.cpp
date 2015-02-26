
#include "../Flare.h"
#include "FlareGame.h"
#include "FlareSaveGame.h"
#include "../Player/FlareHUD.h"
#include "../Player/FlarePlayerController.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareGame::AFlareGame(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CurrentImmatriculationIndex(0)
{
	// Game classes
	HUDClass = AFlareHUD::StaticClass();
	PlayerControllerClass = AFlarePlayerController::StaticClass();

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
		ConstructorHelpers::FObjectFinder<UFlareShipCatalog> ShipCatalog;
		ConstructorHelpers::FObjectFinder<UFlareStationCatalog> StationCatalog;
		ConstructorHelpers::FObjectFinder<UFlareShipPartsCatalog> ShipPartsCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCustomizationCatalog> CustomizationCatalog;

		FConstructorStatics()
			: ShipCatalog(TEXT("/Game/Gameplay/Catalog/ShipCatalog"))
			, StationCatalog(TEXT("/Game/Gameplay/Catalog/StationCatalog"))
			, ShipPartsCatalog(TEXT("/Game/Gameplay/Catalog/ShipPartsCatalog"))
			, CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	ShipCatalog = ConstructorStatics.ShipCatalog.Object;
	StationCatalog = ConstructorStatics.StationCatalog.Object;
	ShipPartsCatalog = ConstructorStatics.ShipPartsCatalog.Object;
	CustomizationCatalog = ConstructorStatics.CustomizationCatalog.Object;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareGame::StartPlay()
{
	FLOG("AFlareGame::StartPlay");
	Super::StartPlay();

	// Spawn planet
	Planetarium = GetWorld()->SpawnActor<AFlarePlanetarium>(PlanetariumClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (Planetarium)
	{
		Planetarium->SetAltitude(8000);
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

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
	LoadWorld(PC, "DefaultSave");
}

void AFlareGame::Logout(AController* Player)
{
	FLOG("AFlareGame::Logout");

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
	SaveWorld(PC, "DefaultSave");

	PC->PrepareForExit();

	Super::Logout(Player);
}

AFlareShip* AFlareGame::LoadShip(const FFlareShipSave& ShipData)
{
	AFlareShip* Ship = NULL;
	FLOGV("AFlareGame::LoadShip ('%s')", *ShipData.Name.ToString());

	if (ShipCatalog)
	{
		FFlareShipDescription* Desc = ShipCatalog->Get(ShipData.Identifier);
		if (Desc)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.Name = ShipData.Name;
			Params.bNoFail = true;

			// Create and configure the ship
			Ship = GetWorld()->SpawnActor<AFlareShip>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
			if (Ship)
			{
				Ship->Load(ShipData);
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

AFlareStation* AFlareGame::LoadStation(const FFlareStationSave& StationData)
{
	AFlareStation* Station = NULL;
	FLOGV("AFlareGame::LoadStation ('%s')", *StationData.Name.ToString());

	if (StationCatalog)
	{
		FFlareStationDescription* Desc = StationCatalog->Get(StationData.Identifier);
		if (Desc)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.Name = StationData.Name;
			Params.bNoFail = true;

			// Create and configure the station
			Station = GetWorld()->SpawnActor<AFlareStation>(Desc->Template->GeneratedClass, StationData.Location, StationData.Rotation, Params);
			if (Station)
			{
				Station->Initialize();
				Station->Load(StationData);
			}
		}
		else
		{
			FLOG("AFlareGame::LoadStation failed (no description available)");
		}
	}
	else
	{
		FLOG("AFlareGame::LoadStation failed (no catalog data)");
	}

	return Station;
}


/*----------------------------------------------------
	Creation tools
----------------------------------------------------*/

AFlareShip* AFlareGame::CreateShip(FName ShipClass)
{
	AFlareShip* ShipPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;
	FFlareShipDescription* Desc = GetShipCatalog()->Get(ShipClass);

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}
	}

	if (Desc)
	{
		// Default data
		FFlareShipSave ShipData;
		ShipData.Location = TargetPosition;
		ShipData.Rotation = FRotator::ZeroRotator;
		ShipData.Name = Immatriculate("GWE", ShipClass);
		ShipData.Identifier = ShipClass;

		// Size selector
		if (Desc->Size == EFlarePartSize::S)
		{
			ShipData.OrbitalEngineIdentifier = FName("engine-octopus");
			ShipData.RCSIdentifier = FName("rcs-piranha");
			for (int32 i = 0; i < Desc->GunCount; i++)
			{
				ShipData.WeaponIdentifiers.Add("weapon-eradicator");
			}
		}
		else if (Desc->Size == EFlarePartSize::L)
		{
			ShipData.OrbitalEngineIdentifier = FName("pod-surtsey");
			ShipData.RCSIdentifier = FName("rcs-rift");
			for (int32 i = 0; i < Desc->TurretCount; i++)
			{
				// TODO TURRETS
			}
		}

		// Create the ship
		ShipPawn = LoadShip(ShipData);
		FLOGV("AFlareGame::CreateShip : Created ship '%s'", *ShipPawn->GetName());
	}

	return ShipPawn;
}

AFlareStation* AFlareGame::CreateStation(FName StationClass)
{
	AFlareStation* StationPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;
	FFlareStationDescription* Desc = GetStationCatalog()->Get(StationClass);

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}
	}

	if (Desc)
	{
		// Default data
		FFlareStationSave StationData;
		StationData.Location = TargetPosition;
		StationData.Rotation = FRotator::ZeroRotator;
		StationData.Name = Immatriculate("GWE", StationClass);
		StationData.Identifier = StationClass;

		// Create the station
		StationPawn = LoadStation(StationData);
		FLOGV("AFlareGame::CreateStation : Created station '%s'", *StationPawn->GetName());
	}

	return StationPawn;
}

FName AFlareGame::Immatriculate(FName Company, FName TargetClass)
{
	FString Immatriculation;
	FFlareShipDescription* ShipDesc = ShipCatalog->Get(TargetClass);
	FFlareStationDescription* StationDesc = StationCatalog->Get(TargetClass);
	
	// Ship
	if (ShipDesc)
	{
		if (ShipDesc->Military)
		{
			Immatriculation += "M";
		}
		else
		{
			Immatriculation += "C";
		}
		Immatriculation += EFlarePartSize::ToString(ShipDesc->Size);
	}

	// Station
	else if (StationDesc)
	{
		Immatriculation += "ST";
	}

	// Company
	Immatriculation += "-";
	Immatriculation += Company.ToString();

	// ID
	CurrentImmatriculationIndex++;
	Immatriculation += FString::Printf(TEXT("-%06d"), CurrentImmatriculationIndex);

	// Return
	FLOGV("AFlareGame::Immatriculate (%s) : %s", *TargetClass.ToString(), *Immatriculation);
	return FName(*Immatriculation);
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

bool AFlareGame::LoadWorld(AFlarePlayerController* PC, FString SaveFile)
{
	FLOGV("AFlareGame::LoadWorld : loading from %s", *SaveFile);
	UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));

	// Load from save
	if (PC && Save && UGameplayStatics::DoesSaveGameExist(SaveFile, 0))
	{
		// Load
		Save = Cast<UFlareSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveFile, 0));
		CurrentImmatriculationIndex = Save->CurrentImmatriculationIndex;

		// Load the player
		PC->Load(Save->PlayerData);

		// Load all stations
		for (int32 i = 0; i < Save->StationData.Num(); i++)
		{
			LoadStation(Save->StationData[i]);
		}

		// Load all ships
		for (int32 i = 0; i < Save->ShipData.Num(); i++)
		{
			LoadShip(Save->ShipData[i]);
		}

		return true;
	}

	// No file existing
	else
	{
		FLOGV("AFlareGame::LoadWorld : could lot load %s", *SaveFile);
		return false;
	}
}

bool AFlareGame::SaveWorld(AFlarePlayerController* PC, FString SaveFile)
{
	FLOGV("AFlareGame::SaveWorld : saving to '%s'", *SaveFile);
	UFlareSaveGame* Save = Cast<UFlareSaveGame>(UGameplayStatics::CreateSaveGameObject(UFlareSaveGame::StaticClass()));

	// Save process
	if (PC && Save)
	{
		// Save the player
		PC->Save(Save->PlayerData);
		Save->ShipData.Empty();
		Save->CurrentImmatriculationIndex = CurrentImmatriculationIndex;

		// Save all physical ships
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Tentative casts
			AFlareMenuPawn* MenuPawn = PC->GetMenuPawn();
			AFlareShip* Ship = Cast<AFlareShip>(*ActorItr);
			AFlareStation* Station = Cast<AFlareStation>(*ActorItr);

			// Ship
			if (Ship && Ship->GetDescription() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentShip()))
			{
				FLOGV("AFlareGame::SaveWorld : saving ship ('%s')", *Ship->GetName());
				FFlareShipSave* TempData = Ship->Save();
				Save->ShipData.Add(*TempData);
			}

			// Station
			else if (Station && Station->GetDescription() && (MenuPawn == NULL || Station != MenuPawn->GetCurrentStation()))
			{
				FLOGV("AFlareGame::SaveWorld : saving station ('%s')", *Station->GetName());
				FFlareStationSave* TempData = Station->Save();
				Save->StationData.Add(*TempData);
			}
		}

		// Save
		UGameplayStatics::SaveGameToSlot(Save, SaveFile, 0);
		return true;
	}

	// No PC
	else
	{
		FLOG("AFlareGame::SaveWorld failed");
		return false;
	}
}
