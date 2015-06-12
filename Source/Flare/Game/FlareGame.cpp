
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
		ConstructorHelpers::FObjectFinder<UFlareSpacecraftCatalog> SpacecraftCatalog;
		ConstructorHelpers::FObjectFinder<UFlareSpacecraftComponentsCatalog> ShipPartsCatalog;
		ConstructorHelpers::FObjectFinder<UFlareCustomizationCatalog> CustomizationCatalog;

		FConstructorStatics()
			: SpacecraftCatalog(TEXT("/Game/Gameplay/Catalog/SpacecraftCatalog"))
			, ShipPartsCatalog(TEXT("/Game/Gameplay/Catalog/ShipPartsCatalog"))
			, CustomizationCatalog(TEXT("/Game/Gameplay/Catalog/CustomizationCatalog"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Push catalog data into storage
	SpacecraftCatalog = ConstructorStatics.SpacecraftCatalog.Object;
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
			LoadShip(Save->StationData[i]);
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

AFlareSpacecraft* AFlareGame::LoadShip(const FFlareSpacecraftSave& ShipData)
{
	AFlareSpacecraft* Ship = NULL;
	FLOGV("AFlareGame::LoadShip ('%s')", *ShipData.Name.ToString());

	if (SpacecraftCatalog)
	{
		FFlareSpacecraftDescription* Desc = SpacecraftCatalog->Get(ShipData.Identifier);
		if (Desc)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.Name = ShipData.Name;
			Params.bNoFail = true;

			// Create and configure the ship
			Ship = GetWorld()->SpawnActor<AFlareSpacecraft>(Desc->Template->GeneratedClass, ShipData.Location, ShipData.Rotation, Params);
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
			AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(*ActorItr);

			// Ship
			if (Ship && Ship->GetDescription() && !Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentShip()))
			{
				FLOGV("AFlareGame::SaveWorld : saving ship ('%s')", *Ship->GetName());
				FFlareSpacecraftSave* TempData = Ship->Save();
				Save->ShipData.Add(*TempData);
			}

			// Station
			else if (Ship && Ship->GetDescription() && Ship->IsStation() && (MenuPawn == NULL || Ship != MenuPawn->GetCurrentStation()))
			{
				FLOGV("AFlareGame::SaveWorld : saving station ('%s')", *Ship->GetName());
				FFlareSpacecraftSave* TempData = Ship->Save();
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
	AFlareSpacecraft* ShipPawn = CreateShipForMe(FName("ship-ghoul"));
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
	CompanyData.CustomizationLightColorIndex = CompanyData.CustomizationOverlayColorIndex;
	CompanyData.CustomizationPatternIndex = 0;

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
	for (TObjectIterator<UFlareCompany> ObjectItr; ObjectItr; ++ObjectItr)
	{
		UFlareCompany* Company = Cast<UFlareCompany>(*ObjectItr);
		if (Company && Company->GetShortName() == CompanyShortName)
		{
			for(int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
			{
				FVector Shift = (BaseShift * (ShipIndex + 1) / 2) * (ShipIndex % 2 == 0 ? 1:-1);
				CreateShip(ShipClass, Company->GetIdentifier(), TargetPosition + Shift);
			}

			break;
		}
	}
}


AFlareSpacecraft* AFlareGame::CreateShip(FName ShipClass, FName CompanyIdentifier, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(ShipClass);

	if(!Desc) {
		Desc = GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if(Desc) {
		return CreateShip(Desc, CompanyIdentifier, TargetPosition);
	}
	return NULL;
}

AFlareSpacecraft* AFlareGame::CreateStation(FName StationClass, FName CompanyIdentifier, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = GetSpacecraftCatalog()->Get(StationClass);

	if(!Desc) {
		Desc = GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if(Desc) {
		return CreateShip(Desc, CompanyIdentifier, TargetPosition);
	}
	return NULL;
}

AFlareSpacecraft* AFlareGame::CreateShip(FFlareSpacecraftDescription* ShipDescription, FName CompanyIdentifier, FVector TargetPosition)
{
	AFlareSpacecraft* ShipPawn = NULL;
	UFlareCompany* Company = FindCompany(CompanyIdentifier);

	if (ShipDescription && Company)
	{
		// Default data
		FFlareSpacecraftSave ShipData;
		ShipData.Location = TargetPosition;
		ShipData.Rotation = FRotator::ZeroRotator;
		ShipData.LinearVelocity = FVector::ZeroVector;
		ShipData.AngularVelocity = FVector::ZeroVector; 
		ShipData.Name = Immatriculate(Company->GetShortName(), ShipDescription->Identifier);
		ShipData.Identifier = ShipDescription->Identifier;
		ShipData.Heat = 600 * ShipDescription->HeatCapacity;
		ShipData.PowerOutageDelay = 0;

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
			ComponentData.ComponentIdentifier = FName("weapon-eradicator");
			ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
			ComponentData.Damage = 0.f;
			ComponentData.Weapon.FiredAmmo = 0;
			ShipData.Components.Add(ComponentData);
		}
		
		for (int32 i = 0; i < ShipDescription->TurretSlots.Num(); i++)
		{
			FFlareSpacecraftComponentSave ComponentData;
			ComponentData.ComponentIdentifier = FName("weapon-hades");
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
		FLOGV("AFlareGame::CreateShip : Created ship '%s' at %s", *ShipPawn->GetName(), *TargetPosition.ToString());
	}

	return ShipPawn;
}

FName AFlareGame::Immatriculate(FName Company, FName TargetClass)
{
	FString Immatriculation;
	FFlareSpacecraftDescription* SpacecraftDesc = SpacecraftCatalog->Get(TargetClass);

	// Spacecraft
	if (SpacecraftDesc)
	{
		if (IFlareSpacecraftInterface::IsStation(SpacecraftDesc))
		{
			Immatriculation += "ST";
		}
		else if (IFlareSpacecraftInterface::IsMilitary(SpacecraftDesc))
		{
			Immatriculation += "M";
		}
		else
		{
			Immatriculation += "C";
		}
		Immatriculation += EFlarePartSize::ToString(SpacecraftDesc->Size) + "-" + SpacecraftDesc->ImmatriculationCode.ToString();
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
		AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
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
