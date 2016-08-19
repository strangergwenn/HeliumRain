
#include "../../Flare.h"
#include "../FlareSaveGame.h"
#include "FlareSaveReaderV1.h"
#include "FlareSaveWriter.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSaveReaderV1::UFlareSaveReaderV1(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareSaveGame* UFlareSaveReaderV1::LoadGame(TSharedPtr< FJsonObject > GameObject)
{

	FString Game;
	FString SaveFormat;
	if(!GameObject->TryGetStringField(TEXT("Game"), Game))
	{
		FLOG("WARNING: Fail to read game name. Save corrupted");
		return NULL;
	}

	if(!GameObject->TryGetStringField(TEXT("SaveFormat"), SaveFormat))
	{
		FLOG("WARNING: Fail to read save format. Save corrupted");
		return NULL;
	}

	if(Game != "Helium Rain" || SaveFormat != UFlareSaveWriter::FormatInt32(1))
	{
		FLOGV("WARNING: Invalid save version. Game is '%s' ('%s' excepted). Save format is '%s' ('%s' excepted)",
			  *Game, "Helium Rain",
			  *SaveFormat, *UFlareSaveWriter::FormatInt32(1));
	}

	// Ok, create Save

	UFlareSaveGame* SaveGame = NewObject<UFlareSaveGame>(this, UFlareSaveGame::StaticClass());

	const TSharedPtr< FJsonObject >* Player;
	if(GameObject->TryGetObjectField(TEXT("Player"), Player))
	{
		LoadPlayer(*Player, &SaveGame->PlayerData);
	}

	const TSharedPtr< FJsonObject >* PlayerCompanyDescription;
	if(GameObject->TryGetObjectField(TEXT("PlayerCompanyDescription"), PlayerCompanyDescription))
	{
		LoadCompanyDescription(*PlayerCompanyDescription, &SaveGame->PlayerCompanyDescription);
	}

	LoadInt32(GameObject, "CurrentImmatriculationIndex", &SaveGame->CurrentImmatriculationIndex);

	const TSharedPtr< FJsonObject >* World;
	if(GameObject->TryGetObjectField(TEXT("World"), World))
	{
		LoadWorld(*World, &SaveGame->WorldData);
	}

	return SaveGame;
}

void UFlareSaveReaderV1::LoadPlayer(const TSharedPtr<FJsonObject> Object, FFlarePlayerSave* Data)
{
	LoadInt32(Object, "ScenarioId", &Data->ScenarioId);
	LoadFName(Object, "CompanyIdentifier", &Data->CompanyIdentifier);
	LoadFName(Object, "LastFlownShipIdentifier", &Data->LastFlownShipIdentifier);

	const TSharedPtr< FJsonObject >* Quest;
	if(Object->TryGetObjectField(TEXT("Quest"), Quest))
	{
		LoadQuest(*Quest, &Data->QuestData);
	}
}


void UFlareSaveReaderV1::LoadQuest(const TSharedPtr<FJsonObject> Object, FFlareQuestSave* Data)
{

	LoadFName(Object, "SelectedQuest", &Data->SelectedQuest);
	Object->TryGetBoolField(TEXT("PlayTutorial"), Data->PlayTutorial);


	const TArray<TSharedPtr<FJsonValue>>* QuestProgresses;
	if(Object->TryGetArrayField("QuestProgresses", QuestProgresses))
	{
		for (TSharedPtr<FJsonValue> Item : *QuestProgresses)
		{
			FFlareQuestProgressSave ChildData;
			LoadQuestProgress(Item->AsObject(), &ChildData);
			Data->QuestProgresses.Add(ChildData);
		}
	}

	LoadFNameArray(Object, "SuccessfulQuests", &Data->SuccessfulQuests);
	LoadFNameArray(Object, "AbandonnedQuests", &Data->AbandonnedQuests);
	LoadFNameArray(Object, "FailedQuests", &Data->FailedQuests);
}


void UFlareSaveReaderV1::LoadQuestProgress(const TSharedPtr<FJsonObject> Object, FFlareQuestProgressSave* Data)
{
	LoadFName(Object, "QuestIdentifier", &Data->QuestIdentifier);

	LoadFNameArray(Object, "SuccessfullSteps", &Data->SuccessfullSteps);


	const TArray<TSharedPtr<FJsonValue>>* CurrentStepProgress;
	if(Object->TryGetArrayField("CurrentStepProgress", CurrentStepProgress))
	{
		for (TSharedPtr<FJsonValue> Item : *CurrentStepProgress)
		{
			FFlareQuestStepProgressSave ChildData;
			LoadQuestStepProgress(Item->AsObject(), &ChildData);
			Data->CurrentStepProgress.Add(ChildData);
		}
	}
}


void UFlareSaveReaderV1::LoadQuestStepProgress(const TSharedPtr<FJsonObject> Object, FFlareQuestStepProgressSave* Data)
{
	LoadFName(Object, "ConditionIdentifier", &Data->ConditionIdentifier);
	LoadInt32(Object, "CurrentProgression", &Data->CurrentProgression);
	LoadTransform(Object, "InitialTransform", &Data->InitialTransform);
	LoadFloat(Object, "InitialVelocity", &Data->InitialVelocity);
}


void UFlareSaveReaderV1::LoadCompanyDescription(const TSharedPtr<FJsonObject> Object, FFlareCompanyDescription* Data)
{
	LoadFText(Object, "Name", &Data->Name);
	LoadFName(Object, "ShortName", &Data->ShortName);
	LoadFText(Object, "Description", &Data->Description);
	LoadInt32(Object, "CustomizationBasePaintColorIndex", &Data->CustomizationBasePaintColorIndex);
	LoadInt32(Object, "CustomizationPaintColorIndex", &Data->CustomizationPaintColorIndex);
	LoadInt32(Object, "CustomizationOverlayColorIndex", &Data->CustomizationOverlayColorIndex);
	LoadInt32(Object, "CustomizationLightColorIndex", &Data->CustomizationLightColorIndex);
	LoadInt32(Object, "CustomizationPatternIndex", &Data->CustomizationPatternIndex);
}


void UFlareSaveReaderV1::LoadWorld(const TSharedPtr<FJsonObject> Object, FFlareWorldSave* Data)
{
	LoadInt64(Object, "Date", &Data->Date);

	const TArray<TSharedPtr<FJsonValue>>* Companies;
	if(Object->TryGetArrayField("Companies", Companies))
	{
		for (TSharedPtr<FJsonValue> Item : *Companies)
		{
			FFlareCompanySave ChildData;
			LoadCompany(Item->AsObject(), &ChildData);
			Data->CompanyData.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Sectors;
	if(Object->TryGetArrayField("Sectors", Sectors))
	{
		for (TSharedPtr<FJsonValue> Item : *Sectors)
		{
			FFlareSectorSave ChildData;
			LoadSector(Item->AsObject(), &ChildData);
			Data->SectorData.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Travels;
	if(Object->TryGetArrayField("Travels", Travels))
	{
		for (TSharedPtr<FJsonValue> Item : *Travels)
		{
			FFlareTravelSave ChildData;
			LoadTravel(Item->AsObject(), &ChildData);
			Data->TravelData.Add(ChildData);
		}
	}
}



void UFlareSaveReaderV1::LoadCompany(const TSharedPtr<FJsonObject> Object, FFlareCompanySave* Data)
{
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadInt32(Object, "CatalogIdentifier", &Data->CatalogIdentifier);
	LoadInt64(Object, "Money", &Data->Money);
	LoadInt64(Object, "CompanyValue", &Data->CompanyValue);
	LoadInt32(Object, "FleetImmatriculationIndex", &Data->FleetImmatriculationIndex);
	LoadInt32(Object, "TradeRouteImmatriculationIndex", &Data->TradeRouteImmatriculationIndex);


	const TSharedPtr< FJsonObject >* AI;
	if(Object->TryGetObjectField(TEXT("AI"), AI))
	{
		LoadCompanyAI(*AI, &Data->AI);
	}

	LoadFNameArray(Object, "HostileCompanies", &Data->HostileCompanies);

	const TArray<TSharedPtr<FJsonValue>>* Ships;
	if(Object->TryGetArrayField("Ships", Ships))
	{
		for (TSharedPtr<FJsonValue> Item : *Ships)
		{
			FFlareSpacecraftSave ChildData;
			LoadSpacecraft(Item->AsObject(), &ChildData);
			Data->ShipData.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Stations;
	if(Object->TryGetArrayField("Stations", Stations))
	{
		for (TSharedPtr<FJsonValue> Item : *Stations)
		{
			FFlareSpacecraftSave ChildData;
			LoadSpacecraft(Item->AsObject(), &ChildData);
			Data->StationData.Add(ChildData);
		}
	}


	const TArray<TSharedPtr<FJsonValue>>* Fleets;
	if(Object->TryGetArrayField("Fleets", Fleets))
	{
		for (TSharedPtr<FJsonValue> Item : *Fleets)
		{
			FFlareFleetSave ChildData;
			LoadFleet(Item->AsObject(), &ChildData);
			Data->Fleets.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* TradeRoutes;
	if(Object->TryGetArrayField("TradeRoutes", TradeRoutes))
	{
		for (TSharedPtr<FJsonValue> Item : *TradeRoutes)
		{
			FFlareTradeRouteSave ChildData;
			LoadTradeRoute(Item->AsObject(), &ChildData);
			Data->TradeRoutes.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* SectorsKnowledge;
	if(Object->TryGetArrayField("SectorsKnowledge", SectorsKnowledge))
	{
		for (TSharedPtr<FJsonValue> Item : *SectorsKnowledge)
		{
			FFlareCompanySectorKnowledge ChildData;
			LoadSectorKnowledge(Item->AsObject(), &ChildData);
			Data->SectorsKnowledge.Add(ChildData);
		}
	}


	const TArray<TSharedPtr<FJsonValue>>* CompaniesReputation;
	if(Object->TryGetArrayField("CompaniesReputation", CompaniesReputation))
	{
		for (TSharedPtr<FJsonValue> Item : *CompaniesReputation)
		{
			FFlareCompanyReputationSave ChildData;
			LoadCompanyReputation(Item->AsObject(), &ChildData);
			Data->CompaniesReputation.Add(ChildData);
		}
	}
}



void UFlareSaveReaderV1::LoadSpacecraft(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftSave* Data)
{
	LoadFName(Object, "Immatriculation", &Data->Immatriculation);
	LoadFText(Object, "NickName", &Data->NickName);
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadFName(Object, "CompanyIdentifier", &Data->CompanyIdentifier);
	LoadVector(Object, "Location", &Data->Location);
	LoadRotator(Object, "Rotation", &Data->Rotation);
	Data->SpawnMode = LoadEnum<EFlareSpawnMode::Type>(Object, "SpawnMode", "EFlareSpawnMode");
	LoadVector(Object, "LinearVelocity", &Data->LinearVelocity);
	LoadVector(Object, "AngularVelocity", &Data->AngularVelocity);
	LoadFName(Object, "DockedTo", &Data->DockedTo);
	LoadInt32(Object, "DockedAt", &Data->DockedAt);
	LoadFloat(Object, "Heat", &Data->Heat);
	LoadFloat(Object, "PowerOutageDelay", &Data->PowerOutageDelay);
	LoadFloat(Object, "PowerOutageAcculumator", &Data->PowerOutageAcculumator);
	LoadFName(Object, "DynamicComponentStateIdentifier", &Data->DynamicComponentStateIdentifier);
	LoadFloat(Object, "DynamicComponentStateProgress", &Data->DynamicComponentStateProgress);

	// LEGACY alpha 3
	Data->IsTrading = false;

	Object->TryGetBoolField(TEXT("IsTrading"), Data->IsTrading);

	LoadInt32(Object, "Level", &Data->Level);
	if (Data->Level == 0)
	{
		Data->Level = 1;
	}

	const TSharedPtr< FJsonObject >* Pilot;
	if(Object->TryGetObjectField(TEXT("Pilot"), Pilot))
	{
		LoadPilot(*Pilot, &Data->Pilot);
	}

	const TSharedPtr< FJsonObject >* Asteroid;
	if(Object->TryGetObjectField(TEXT("Asteroid"), Asteroid))
	{
		LoadAsteroid(*Asteroid, &Data->AsteroidData);
	}

	const TArray<TSharedPtr<FJsonValue>>* Components;
	if(Object->TryGetArrayField("Components", Components))
	{
		for (TSharedPtr<FJsonValue> Item : *Components)
		{
			FFlareSpacecraftComponentSave ChildData;
			LoadSpacecraftComponent(Item->AsObject(), &ChildData);
			Data->Components.Add(ChildData);
		}
	}


	const TArray<TSharedPtr<FJsonValue>>* Cargo;
	if(Object->TryGetArrayField("Cargo", Cargo))
	{
		for (TSharedPtr<FJsonValue> Item : *Cargo)
		{
			FFlareCargoSave ChildData;
			LoadCargo(Item->AsObject(), &ChildData);
			Data->Cargo.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* FactoryStates;
	if(Object->TryGetArrayField("FactoryStates", FactoryStates))
	{
		for (TSharedPtr<FJsonValue> Item : *FactoryStates)
		{
			FFlareFactorySave ChildData;
			LoadFactory(Item->AsObject(), &ChildData);
			Data->FactoryStates.Add(ChildData);
		}
	}

	LoadFNameArray(Object, "SalesExcludedResources", &Data->SalesExcludedResources);
}


void UFlareSaveReaderV1::LoadPilot(const TSharedPtr<FJsonObject> Object, FFlareShipPilotSave* Data)
{
	LoadFName(Object, "Identifier", &Data->Identifier);
	Object->TryGetStringField(TEXT("Name"), Data->Name);
}


void UFlareSaveReaderV1::LoadAsteroid(const TSharedPtr<FJsonObject> Object, FFlareAsteroidSave* Data)
{
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadVector(Object, "Location", &Data->Location);
	LoadRotator(Object, "Rotation", &Data->Rotation);
	LoadVector(Object, "LinearVelocity", &Data->LinearVelocity);
	LoadVector(Object, "AngularVelocity", &Data->AngularVelocity);
	LoadVector(Object, "Scale", &Data->Scale);
	LoadInt32(Object, "AsteroidMeshID", &Data->AsteroidMeshID);

}


void UFlareSaveReaderV1::LoadSpacecraftComponent(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentSave* Data)
{
	LoadFName(Object, "ComponentIdentifier", &Data->ComponentIdentifier);

	LoadFName(Object, "ShipSlotIdentifier", &Data->ShipSlotIdentifier);
	LoadFloat(Object, "Damage", &Data->Damage);

	const TSharedPtr< FJsonObject >* Turret;
	if(Object->TryGetObjectField(TEXT("Turret"), Turret))
	{
		LoadSpacecraftComponentTurret(*Turret, &Data->Turret);
	}

	const TSharedPtr< FJsonObject >* Weapon;
	if(Object->TryGetObjectField(TEXT("Weapon"), Weapon))
	{
		LoadSpacecraftComponentWeapon(*Weapon, &Data->Weapon);
	}

	const TSharedPtr< FJsonObject >* Pilot;
	if(Object->TryGetObjectField(TEXT("Pilot"), Pilot))
	{
		LoadTurretPilot(*Pilot, &Data->Pilot);
	}
}


void UFlareSaveReaderV1::LoadSpacecraftComponentTurret(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentTurretSave* Data)
{
	LoadFloat(Object, "TurretAngle", &Data->TurretAngle);
	LoadFloat(Object, "BarrelsAngle", &Data->BarrelsAngle);
}


void UFlareSaveReaderV1::LoadSpacecraftComponentWeapon(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentWeaponSave* Data)
{
	LoadInt32(Object, "FiredAmmo", &Data->FiredAmmo);
}


void UFlareSaveReaderV1::LoadTurretPilot(const TSharedPtr<FJsonObject> Object, FFlareTurretPilotSave* Data)
{
	LoadFName(Object, "Identifier", &Data->Identifier);
	Object->TryGetStringField(TEXT("Name"), Data->Name);
}


void UFlareSaveReaderV1::LoadTradeOperation(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSectorOperationSave* Data)
{
	LoadFName(Object, "ResourceIdentifier", &Data->ResourceIdentifier);
	LoadInt32(Object, "MaxQuantity", (int32*) &Data->MaxQuantity);
	LoadInt32(Object, "MaxWait", (int32*) &Data->MaxWait);
	Data->Type = LoadEnum<EFlareTradeRouteOperation::Type>(Object, "Type", "EFlareTradeRouteOperation");
}

void UFlareSaveReaderV1::LoadCargo(const TSharedPtr<FJsonObject> Object, FFlareCargoSave* Data)
{
	LoadFName(Object, "ResourceIdentifier", &Data->ResourceIdentifier);
	LoadInt32(Object, "Quantity", (int32*) &Data->Quantity); // TODO clean after conversion
	Data->Lock = LoadEnum<EFlareResourceLock::Type>(Object, "Lock", "EFlareResourceLock");
}


void UFlareSaveReaderV1::LoadFactory(const TSharedPtr<FJsonObject> Object, FFlareFactorySave* Data)
{
	Object->TryGetBoolField(TEXT("Active"), Data->Active);
	LoadInt32(Object, "CostReserved", (int32*) &Data->CostReserved); // TODO clean after conversion
	LoadInt64(Object, "ProductedDuration", &Data->ProductedDuration);
	Object->TryGetBoolField(TEXT("InfiniteCycle"), Data->InfiniteCycle);
	LoadInt32(Object, "CycleCount", (int32*) &Data->CycleCount); // TODO clean after conversion
	LoadFName(Object, "TargetShipClass", &Data->TargetShipClass);
	LoadFName(Object, "TargetShipCompany", &Data->TargetShipCompany);
	LoadFName(Object, "OrderShipClass", &Data->OrderShipClass);
	LoadFName(Object, "OrderShipCompany", &Data->OrderShipCompany);
	LoadInt32(Object, "OrderShipAdvancePayment", &Data->OrderShipAdvancePayment);


	const TArray<TSharedPtr<FJsonValue>>* ResourceReserved;
	if(Object->TryGetArrayField("ResourceReserved", ResourceReserved))
	{
		for (TSharedPtr<FJsonValue> Item : *ResourceReserved)
		{
			FFlareCargoSave ChildData;
			LoadCargo(Item->AsObject(), &ChildData);
			Data->ResourceReserved.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* OutputCargoLimit;
	if(Object->TryGetArrayField("OutputCargoLimit", OutputCargoLimit))
	{
		for (TSharedPtr<FJsonValue> Item : *OutputCargoLimit)
		{
			FFlareCargoSave ChildData;
			LoadCargo(Item->AsObject(), &ChildData);
			Data->OutputCargoLimit.Add(ChildData);
		}
	}
}



void UFlareSaveReaderV1::LoadFleet(const TSharedPtr<FJsonObject> Object, FFlareFleetSave* Data)
{
	LoadFText(Object, "Name", &Data->Name);
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadFNameArray(Object, "ShipImmatriculations", &Data->ShipImmatriculations);
}


void UFlareSaveReaderV1::LoadTradeRoute(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSave* Data)
{
	LoadFText(Object, "Name", &Data->Name);
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadFName(Object, "FleetIdentifier", &Data->FleetIdentifier);
	LoadFName(Object, "TargetSectorIdentifier", &Data->TargetSectorIdentifier);
	LoadInt32(Object, "CurrentOperationIndex", &Data->CurrentOperationIndex);
	LoadInt32(Object, "CurrentOperationProgress", &Data->CurrentOperationProgress);
	LoadInt32(Object, "CurrentOperationDuration", &Data->CurrentOperationDuration);

	if(!Object->TryGetBoolField(TEXT("IsPaused"), Data->IsPaused))
	{
		Data->IsPaused = false;
	}

	// LEGACY alpha 3
	TArray<FName> FleetIdentifiers;
	LoadFNameArray(Object, "FleetIdentifiers", &FleetIdentifiers);
	if(FleetIdentifiers.Num() > 0)
	{
		Data->FleetIdentifier = FleetIdentifiers[0];
	}



	const TArray<TSharedPtr<FJsonValue>>* Sectors;
	if(Object->TryGetArrayField("Sectors", Sectors))
	{
		for (TSharedPtr<FJsonValue> Item : *Sectors)
		{
			FFlareTradeRouteSectorSave ChildData;
			LoadTradeRouteSector(Item->AsObject(), &ChildData);
			Data->Sectors.Add(ChildData);
		}
	}
}


void UFlareSaveReaderV1::LoadTradeRouteSector(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSectorSave* Data)
{

	LoadFName(Object, "SectorIdentifier", &Data->SectorIdentifier);


	// LEGACY alpha 3
	const TArray<TSharedPtr<FJsonValue>>* ResourcesToUnload;
	if(Object->TryGetArrayField("ResourcesToUnload", ResourcesToUnload))
	{
		for (TSharedPtr<FJsonValue> Item : *ResourcesToUnload)
		{
			FFlareCargoSave ChildData;
			LoadCargo(Item->AsObject(), &ChildData);

			FFlareTradeRouteSectorOperationSave Operation;
			Operation.Type = EFlareTradeRouteOperation::Unload;
			Operation.ResourceIdentifier = ChildData.ResourceIdentifier;
			Operation.MaxQuantity = (ChildData.Quantity == 0 ? -1: ChildData.Quantity);
			Operation.MaxWait = -1;

			Data->Operations.Add(Operation);
		}
	}

	// LEGACY alpha 3
	const TArray<TSharedPtr<FJsonValue>>* ResourcesToLoad;
	if(Object->TryGetArrayField("ResourcesToLoad", ResourcesToLoad))
	{
		for (TSharedPtr<FJsonValue> Item : *ResourcesToLoad)
		{
			FFlareCargoSave ChildData;
			LoadCargo(Item->AsObject(), &ChildData);

			FFlareTradeRouteSectorOperationSave Operation;
			Operation.Type = EFlareTradeRouteOperation::Load;
			Operation.ResourceIdentifier = ChildData.ResourceIdentifier;
			Operation.MaxQuantity = (ChildData.Quantity == 0 ? -1: ChildData.Quantity);
			Operation.MaxWait = -1;

			Data->Operations.Add(Operation);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Operations;
	if(Object->TryGetArrayField("Operations", Operations))
	{
		for (TSharedPtr<FJsonValue> Item : *Operations)
		{
			FFlareTradeRouteSectorOperationSave ChildData;
			LoadTradeOperation(Item->AsObject(), &ChildData);

			Data->Operations.Add(ChildData);
		}
	}
}


void UFlareSaveReaderV1::LoadSectorKnowledge(const TSharedPtr<FJsonObject> Object, FFlareCompanySectorKnowledge* Data)
{
	LoadFName(Object, "SectorIdentifier", &Data->SectorIdentifier);
	Data->Knowledge = LoadEnum<EFlareSectorKnowledge::Type>(Object, "Knowledge", "EFlareSectorKnowledge");
}


void UFlareSaveReaderV1::LoadCompanyAI(const TSharedPtr<FJsonObject> Object, FFlareCompanyAISave* Data)
{
}


void UFlareSaveReaderV1::LoadCompanyReputation(const TSharedPtr<FJsonObject> Object, FFlareCompanyReputationSave* Data)
{
	LoadFName(Object, "CompanyIdentifier", &Data->CompanyIdentifier);
	LoadFloat(Object, "Reputation", &Data->Reputation);
}




void UFlareSaveReaderV1::LoadSector(const TSharedPtr<FJsonObject> Object, FFlareSectorSave* Data)
{
	LoadFText(Object, "GivenName", &Data->GivenName);
	LoadFName(Object, "Identifier", &Data->Identifier);
	LoadInt64(Object, "LocalTime", &Data->LocalTime);

	const TSharedPtr< FJsonObject >* People;
	if(Object->TryGetObjectField(TEXT("People"), People))
	{
		LoadPeople(*People, &Data->PeopleData);
	}

	const TArray<TSharedPtr<FJsonValue>>* Bombs;
	if(Object->TryGetArrayField("Bombs", Bombs))
	{
		for (TSharedPtr<FJsonValue> Item : *Bombs)
		{
			FFlareBombSave ChildData;
			LoadBomb(Item->AsObject(), &ChildData);
			Data->BombData.Add(ChildData);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Asteroids;
	if(Object->TryGetArrayField("Asteroids", Asteroids))
	{
		for (TSharedPtr<FJsonValue> Item : *Asteroids)
		{
			FFlareAsteroidSave ChildData;
			LoadAsteroid(Item->AsObject(), &ChildData);
			Data->AsteroidData.Add(ChildData);
		}
	}

	LoadFNameArray(Object, "FleetIdentifiers", &Data->FleetIdentifiers);
	LoadFNameArray(Object, "SpacecraftIdentifiers", &Data->SpacecraftIdentifiers);

	const TArray<TSharedPtr<FJsonValue>>* ResourcePrices;
	if(Object->TryGetArrayField("ResourcePrices", ResourcePrices))
	{
		for (TSharedPtr<FJsonValue> Item : *ResourcePrices)
		{
			FFFlareResourcePrice ChildData;
			LoadResourcePrice(Item->AsObject(), &ChildData);
			Data->ResourcePrices.Add(ChildData);
		}
	}

	if(!Object->TryGetBoolField(TEXT("IsTravelSector"), Data->IsTravelSector))
	{
		Data->IsTravelSector = false;
	}

}


void UFlareSaveReaderV1::LoadPeople(const TSharedPtr<FJsonObject> Object, FFlarePeopleSave* Data)
{
	LoadInt32(Object, "Population", (int32*) &Data->Population); // TODO clean after conversion
	LoadInt32(Object, "FoodStock", (int32*) &Data->FoodStock);
	LoadInt32(Object, "FuelStock", (int32*) &Data->FuelStock);
	LoadInt32(Object, "ToolStock", (int32*) &Data->ToolStock);
	LoadInt32(Object, "TechStock", (int32*) &Data->TechStock);
	LoadFloat(Object, "FoodConsumption", &Data->FoodConsumption);
	LoadFloat(Object, "FuelConsumption", &Data->FuelConsumption);
	LoadFloat(Object, "ToolConsumption", &Data->ToolConsumption);
	LoadFloat(Object, "TechConsumption", &Data->TechConsumption);
	LoadInt32(Object, "Money", (int32*) &Data->Money);
	LoadInt32(Object, "Dept", (int32*) &Data->Dept);
	LoadInt32(Object, "BirthPoint", (int32*) &Data->BirthPoint);
	LoadInt32(Object, "DeathPoint", (int32*) &Data->DeathPoint);
	LoadInt32(Object, "HungerPoint", (int32*) &Data->HungerPoint);
	LoadInt32(Object, "HappinessPoint", (int32*) &Data->HappinessPoint);

	const TArray<TSharedPtr<FJsonValue>>* CompanyReputations;
	if(Object->TryGetArrayField("CompanyReputations", CompanyReputations))
	{
		for (TSharedPtr<FJsonValue> Item : *CompanyReputations)
		{
			FFlareCompanyReputationSave ChildData;
			LoadCompanyReputation(Item->AsObject(), &ChildData);
			Data->CompanyReputations.Add(ChildData);
		}
	}
}


void UFlareSaveReaderV1::LoadBomb(const TSharedPtr<FJsonObject> Object, FFlareBombSave* Data)
{
	LoadVector(Object, "Location", &Data->Location);
	LoadRotator(Object, "Rotation", &Data->Rotation);
	LoadVector(Object, "LinearVelocity", &Data->LinearVelocity);
	LoadVector(Object, "AngularVelocity", &Data->AngularVelocity);
	LoadFName(Object, "WeaponSlotIdentifier", &Data->WeaponSlotIdentifier);
	LoadFName(Object, "ParentSpacecraft", &Data->ParentSpacecraft);
	Object->TryGetBoolField(TEXT("Activated"), Data->Activated);
	Object->TryGetBoolField(TEXT("Dropped"), Data->Dropped);
	LoadFloat(Object, "DropParentDistance", &Data->DropParentDistance);
	LoadFloat(Object, "LifeTime", &Data->LifeTime);
}


void UFlareSaveReaderV1::LoadResourcePrice(const TSharedPtr<FJsonObject> Object, FFFlareResourcePrice* Data)
{
	LoadFName(Object, "ResourceIdentifier", &Data->ResourceIdentifier);
	LoadFloat(Object, "Price", &Data->Price);
	LoadFloatBuffer(Object, "Prices", &Data->Prices);
}


void UFlareSaveReaderV1::LoadTravel(const TSharedPtr<FJsonObject> Object, FFlareTravelSave* Data)
{
	LoadFName(Object, "FleetIdentifier", &Data->FleetIdentifier);
	LoadFName(Object, "OriginSectorIdentifier", &Data->OriginSectorIdentifier);
	LoadFName(Object, "DestinationSectorIdentifier", &Data->DestinationSectorIdentifier);
	LoadInt64(Object, "DepartureDate", &Data->DepartureDate);

	const TSharedPtr< FJsonObject >* Sector;
	if(Object->TryGetObjectField(TEXT("SectorData"), Sector))
	{
		LoadSector(*Sector, &Data->SectorData);
	}
	else
	{
		UFlareTravel::InitTravelSector(Data->SectorData);
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

void UFlareSaveReaderV1::LoadInt32(TSharedPtr< FJsonObject > Object, FString Key, int32* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		*Data = FCString::Atoi(*DataString);
	}
	else
	{
		FLOGV("WARNING: Fail to load int32 key '%s'. Save corrupted", *Key);
		*Data = 0;
	}
}

void UFlareSaveReaderV1::LoadInt64(TSharedPtr< FJsonObject > Object, FString Key, int64* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		*Data = FCString::Atoi64(*DataString);
	}
	else
	{
		FLOGV("WARNING: Fail to load int64 key '%s'. Save corrupted", *Key);
		*Data = 0;
	}
}


void UFlareSaveReaderV1::LoadFloat(TSharedPtr< FJsonObject > Object, FString Key, float* Data)
{
	double DataDouble;
	if(Object->TryGetNumberField(Key, DataDouble))
	{
		*Data = DataDouble;
	}
	else
	{
		FLOGV("WARNING: Fail to load float key '%s'. Save corrupted", *Key);
		*Data = 0;
	}
}

void UFlareSaveReaderV1::LoadFName(TSharedPtr< FJsonObject > Object, FString Key, FName* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		*Data = FName(*DataString);
	}
	else
	{
		FLOGV("WARNING: Fail to load FName key '%s'. Save corrupted", *Key);
		*Data = NAME_None;
	}
}

void UFlareSaveReaderV1::LoadFText(TSharedPtr< FJsonObject > Object, FString Key, FText* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		*Data = FText::FromString(DataString);
	}
	else
	{
		FLOGV("WARNING: Fail to load FText key '%s'. Save corrupted", *Key);
	}
}

void UFlareSaveReaderV1::LoadFNameArray(TSharedPtr< FJsonObject > Object, FString Key, TArray<FName>* Data)
{
	const TArray<TSharedPtr<FJsonValue>>* Array;
	if(Object->TryGetArrayField(Key, Array))
	{
		for (TSharedPtr<FJsonValue> Item : *Array)
		{
			(*Data).Add(FName(*Item->AsString()));
		}
	}
}

void UFlareSaveReaderV1::LoadFloatArray(TSharedPtr< FJsonObject > Object, FString Key, TArray<float>* Data)
{
	const TArray<TSharedPtr<FJsonValue>>* Array;
	if(Object->TryGetArrayField(Key, Array))
	{
		for (TSharedPtr<FJsonValue> Item : *Array)
		{
			float Value = Item->AsNumber();
			Data->Add(Value);
		}
	}
}


void UFlareSaveReaderV1::LoadTransform(TSharedPtr< FJsonObject > Object, FString Key, FTransform* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		TArray<FString> Values;
		if (DataString.ParseIntoArray(Values, TEXT(",")) == 10)
		{
			*Data = FTransform(
						FQuat(
					FCString::Atof(*Values[0]),
					FCString::Atof(*Values[1]),
					FCString::Atof(*Values[2]),
					FCString::Atof(*Values[3])),
						FVector(
					FCString::Atof(*Values[4]),
					FCString::Atof(*Values[5]),
					FCString::Atof(*Values[6])),
						FVector(
					FCString::Atof(*Values[7]),
					FCString::Atof(*Values[8]),
					FCString::Atof(*Values[9])));
		}
		else
		{
			FLOGV("WARNING: Fail to load FTransform key '%s'. No 10 values in '%s'. Save corrupted", *Key, *DataString);
		}
	}
	else
	{
		FLOGV("WARNING: Fail to load FTransform key '%s'. Save corrupted", *Key);
	}
}


void UFlareSaveReaderV1::LoadVector(TSharedPtr< FJsonObject > Object, FString Key, FVector* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		TArray<FString> Values;
		if (DataString.ParseIntoArray(Values, TEXT(",")) == 3)
		{
			*Data = FVector(
					FCString::Atof(*Values[0]),
					FCString::Atof(*Values[1]),
					FCString::Atof(*Values[2]));
		}
		else
		{
			FLOGV("WARNING: Fail to load FVector key '%s'. No 3 values in '%s'. Save corrupted", *Key, *DataString);
		}
	}
	else
	{
		FLOGV("WARNING: Fail to load FVector key '%s'. Save corrupted", *Key);
	}
}

void UFlareSaveReaderV1::LoadRotator(TSharedPtr< FJsonObject > Object, FString Key, FRotator* Data)
{
	FString DataString;
	if(Object->TryGetStringField(Key, DataString))
	{
		TArray<FString> Values;
		if (DataString.ParseIntoArray(Values, TEXT(",")) == 3)
		{
			*Data = FRotator(
					FCString::Atof(*Values[0]),
					FCString::Atof(*Values[1]),
					FCString::Atof(*Values[2]));
		}
		else
		{
			FLOGV("WARNING: Fail to load FRotator key '%s'. No 3 values in '%s'. Save corrupted", *Key, *DataString);
		}
	}
	else
	{
		FLOGV("WARNING: Fail to load FRotator key '%s'. Save corrupted", *Key);
	}
}

void UFlareSaveReaderV1::LoadFloatBuffer(TSharedPtr< FJsonObject > Object, FString Key, FFlareFloatBuffer* Data)
{
	const TSharedPtr< FJsonObject >* FloatBuffer;
	if(Object->TryGetObjectField(Key, FloatBuffer))
	{
		LoadInt32(*FloatBuffer, "MaxSize", &Data->MaxSize);
		LoadInt32(*FloatBuffer, "WriteIndex", &Data->WriteIndex);

		LoadFloatArray(*FloatBuffer, "Values", &Data->Values);
	}
	else
	{
		FLOGV("WARNING: Fail to load float key '%s'. Save corrupted", *Key);
		Data->Init(1);
	}
}
