
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

	// Load the world from save, create a new one if no save was found
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
	if (PC)
	{
		bool WorldLoaded = LoadWorld(PC, "DefaultSave");
		if (!WorldLoaded)
		{
			CreateWorld(PC);
		}
	}
}

void AFlareGame::Logout(AController* Player)
{
	FLOG("AFlareGame::Logout");

	// Save the world, literally
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Player);
	SaveWorld(PC, "DefaultSave");
	PC->PrepareForExit();

	Super::Logout(Player);
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

		// Load all companies
		for (int32 i = 0; i < Save->CompanyData.Num(); i++)
		{
			LoadCompany(Save->CompanyData[i]);
		}

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

UFlareCompany* AFlareGame::LoadCompany(const FFlareCompanySave& CompanyData)
{
	UFlareCompany* Company = NULL;
	FLOGV("AFlareGame::LoadCompany ('%s')", *CompanyData.Name);

	// Create the new company
	Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
	Company->Load(CompanyData);
	Companies.AddUnique(Company);

	return Company;
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
			 UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
			
			RootComponent->SetPhysicsLinearVelocity(ShipData.LinearVelocity, false);
			RootComponent->SetPhysicsAngularVelocity(ShipData.AngularVelocity, false);
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

		// Save all UObjects
		for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
		{
			UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);

			// Company
			if (Company)
			{
				FLOGV("AFlareGame::SaveWorld : saving company ('%s')", *Company->GetName());
				FFlareCompanySave* TempData = Company->Save();
				Save->CompanyData.Add(*TempData);
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


/*----------------------------------------------------
	Creation tools
----------------------------------------------------*/

void AFlareGame::CreateWorld(AFlarePlayerController* PC)
{
	FFlarePlayerSave PlayerData;

	// Player company
	UFlareCompany* Company = CreateCompany("Player Inc");
	PlayerData.CompanyIdentifier = Company->GetIdentifier();
	PC->SetCompany(Company);

	// Enemy
	CreateCompany("Evil Corp");

	// Player ship
	AFlareShip* ShipPawn = CreateShipForMe(FName("ship-ghoul"));
	PlayerData.CurrentShipName = ShipPawn->GetName();

	// Load
	PC->Load(PlayerData);
}

UFlareCompany* AFlareGame::CreateCompany(FString CompanyName)
{
	UFlareCompany* Company = NULL;
	FFlareCompanySave CompanyData;

	// Generate identifier
	CurrentImmatriculationIndex++;
	FString Immatriculation = FString::Printf(TEXT("CPNY-%06d"), CurrentImmatriculationIndex);

	// Generate arbitrary save data
	CompanyData.Name = CompanyName.ToUpper();
	CompanyData.ShortName = *CompanyName.Left(3).ToUpper();
	CompanyData.Identifier = *Immatriculation;
	CompanyData.Money = 100000;
	CompanyData.CustomizationBasePaintColorIndex = 0;
	CompanyData.CustomizationPaintColorIndex = 4;
	CompanyData.CustomizationOverlayColorIndex = FMath::RandRange(1, 15);
	CompanyData.CustomizationPatternIndex = 0;

	// Create company
	Company = LoadCompany(CompanyData);
	FLOGV("AFlareGame::CreateCompany : Created company '%s'", *Company->GetName());

	return Company;
}

AFlareStation* AFlareGame::CreateStationForMe(FName StationClass)
{
	AFlareStation* StationPawn = NULL;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	// Parent company
	if (PC && PC->GetCompany())
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		FVector TargetPosition = FVector::ZeroVector;
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}

		StationPawn = CreateStation(StationClass, PC->GetCompany()->GetIdentifier(), TargetPosition);
	}
	return StationPawn;
}

AFlareStation* AFlareGame::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance)
{
	AFlareStation* StationPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
		}
	}

	// Find company
	for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
	{
		UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			StationPawn = CreateStation(StationClass, Company->GetIdentifier(), TargetPosition);
			break;
		}
	}
	return StationPawn;
} 

AFlareStation* AFlareGame::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition)
{
	AFlareStation* StationPawn = NULL;
	FFlareStationDescription* Desc = GetStationCatalog()->Get(StationClass);
	UFlareCompany* Company = FindCompany(CompanyIdentifier);

	if (Desc && Company)
	{
		// Default data
		FFlareStationSave StationData;
		StationData.Location = TargetPosition;
		StationData.Rotation = FRotator::ZeroRotator;
		StationData.Name = Immatriculate(Company->GetShortName(), StationClass);
		StationData.Identifier = StationClass;
		StationData.CompanyIdentifier = CompanyIdentifier;

		// Create the station
		StationPawn = LoadStation(StationData);
		FLOGV("AFlareGame::CreateStation : Created station '%s'", *StationPawn->GetName());
	}

	return StationPawn;
}

AFlareShip* AFlareGame::CreateShipForMe(FName ShipClass)
{
	AFlareShip* ShipPawn = NULL;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	// Parent company
	if (PC && PC->GetCompany())
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		FVector TargetPosition = FVector::ZeroVector;
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(10000 * FVector(1, 0, 0));
		}

		ShipPawn = CreateShip(ShipClass, PC->GetCompany()->GetIdentifier(), TargetPosition);
	}
	return ShipPawn;
}


AFlareShip* AFlareGame::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
	AFlareShip* ShipPawn = NULL;
	FVector TargetPosition = FVector::ZeroVector;

	// Get target position
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * 100 * FVector(1, 0, 0));
		}
	}

	// Find company
	for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
	{
		UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			ShipPawn = CreateShip(ShipClass, Company->GetIdentifier(), TargetPosition);
			break;
		}
	}
	return ShipPawn;
}

AFlareShip* AFlareGame::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
{
	AFlareShip* ShipPawn = NULL;
	FFlareShipDescription* Desc = GetShipCatalog()->Get(ShipClass);

	UFlareCompany* Company = FindCompany(CompanyIdentifier);

	if (Desc && Company)
	{
		// Default data
		FFlareShipSave ShipData;
		ShipData.Location = TargetPosition;
		ShipData.Rotation = FRotator::ZeroRotator;
		ShipData.LinearVelocity = FVector::ZeroVector;
		ShipData.AngularVelocity = FVector::ZeroVector; 
		ShipData.Name = Immatriculate(Company->GetShortName(), ShipClass);
		ShipData.Identifier = ShipClass;
		ShipData.Heat = 600 * Desc->HeatCapacity;
		ShipData.PowerOutageDelay = 0;

		FName RCSIdentifier;
		FName OrbitalEngineIdentifier;
		
		// Size selector
		if (Desc->Size == EFlarePartSize::S)
		{
			RCSIdentifier = FName("rcs-piranha");
			OrbitalEngineIdentifier = FName("engine-octopus");
		}
		else if (Desc->Size == EFlarePartSize::L)
		{
			RCSIdentifier = FName("rcs-rift");
			OrbitalEngineIdentifier = FName("pod-surtsey");
		}
		else
		{
			// TODO
		}
		
		for (int32 i = 0; i < Desc->RCSCount; i++)
		{
			FFlareShipComponentSave ComponentData;
			ComponentData.ComponentIdentifier = RCSIdentifier;
			ComponentData.ShipSlotIdentifier = FName(*("rcs-" + FString::FromInt(i)));
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}
		
		for (int32 i = 0; i < Desc->OrbitalEngineCount; i++)
		{
			FFlareShipComponentSave ComponentData;
			ComponentData.ComponentIdentifier = OrbitalEngineIdentifier;
			ComponentData.ShipSlotIdentifier = FName(*("engine-" + FString::FromInt(i)));
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}

		for (int32 i = 0; i < Desc->GunSlots.Num(); i++)
		{
			FFlareShipComponentSave ComponentData;
			ComponentData.ComponentIdentifier = FName("weapon-eradicator");
			ComponentData.ShipSlotIdentifier = Desc->GunSlots[i].SlotIdentifier;
			ComponentData.Damage = 0.f;
			ShipData.Components.Add(ComponentData);
		}
		
		for (int32 i = 0; i < Desc->TurretSlots.Num(); i++)
		{
			// TODO TURRETS
		}
		
		for (int32 i = 0; i < Desc->InternalComponentSlots.Num(); i++)
		{
			FFlareShipComponentSave ComponentData;
			ComponentData.ComponentIdentifier = Desc->InternalComponentSlots[i].ComponentIdentifier;
			ComponentData.ShipSlotIdentifier = Desc->InternalComponentSlots[i].SlotIdentifier;
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
		FLOGV("AFlareGame::CreateShip : Created ship '%s' at %s", *ShipPawn->GetName(), *TargetPosition.ToString());
	}

	return ShipPawn;
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

void AFlareGame::CreateQuickBattle(float Distance, FName Company1, FName Company2, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	FVector BasePosition = FVector::ZeroVector;
	FVector BaseOffset = FVector(1.f, 0.f, 0.f) * Distance / 50.f; // Half the distance in cm
	FVector BaseShift =  FVector(0.f, 3000.f, 0.f) ;  // 30 m
	FVector BaseDeep = FVector(10000.f, 0.f, 0.f); // 100 m

	FName Company1Identifier;
	FName Company2Identifier;

	for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
	{
		UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
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
		AFlareShip* ExistingShipPawn = PC->GetShipPawn();
		if (ExistingShipPawn)
		{
			BasePosition = ExistingShipPawn->GetActorLocation();
			BaseOffset = ExistingShipPawn->GetActorRotation().RotateVector(Distance * 50.f * FVector(1, 0, 0)); // Half the distance in cm
			BaseDeep = ExistingShipPawn->GetActorRotation().RotateVector(FVector(10000.f, 0, 0)); // 100 m in cm
			BaseShift = ExistingShipPawn->GetActorRotation().RotateVector(FVector(0, 3000.f, 0)); // 30 m in cm
		}
	}


	for(int32 ShipIndex = 0; ShipIndex < ShipClass1Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
		CreateShip(ShipClass1, Company1Identifier, BasePosition + BaseOffset + Shift);
		CreateShip(ShipClass1, Company2Identifier, BasePosition - BaseOffset - Shift);
	}

	for(int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
		CreateShip(ShipClass2, Company1Identifier, BasePosition + BaseOffset + Shift + BaseDeep);
		CreateShip(ShipClass2, Company2Identifier, BasePosition - BaseOffset - Shift - BaseDeep);
	}
}
