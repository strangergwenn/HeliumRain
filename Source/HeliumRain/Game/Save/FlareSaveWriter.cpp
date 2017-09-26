
#include "FlareSaveWriter.h"
#include "../../Flare.h"
#include "../FlareSaveGame.h"
#include "Game/FlareGameTools.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSaveWriter::UFlareSaveWriter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveGame(UFlareSaveGame* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// General stuff
	JsonObject->SetStringField("Game", "Helium Rain");
	JsonObject->SetStringField("SaveFormat", FormatInt32(1));
	JsonObject->SetBoolField("AutoSave", Data->AutoSave);

	// Game data
	JsonObject->SetObjectField("Player", SavePlayer(&Data->PlayerData));
	JsonObject->SetObjectField("PlayerCompanyDescription", SaveCompanyDescription(&Data->PlayerCompanyDescription));
	JsonObject->SetStringField("CurrentImmatriculationIndex", FormatInt32(Data->CurrentImmatriculationIndex));
	JsonObject->SetStringField("CurrentIdentifierIndex", FormatInt32(Data->CurrentIdentifierIndex));
	JsonObject->SetObjectField("World", SaveWorld(&Data->WorldData));


	return JsonObject;
}

/*----------------------------------------------------
	Generator
----------------------------------------------------*/

TSharedRef<FJsonObject> UFlareSaveWriter::SavePlayer(FFlarePlayerSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("UUID", Data->UUID.ToString());
	JsonObject->SetStringField("ScenarioId", FormatInt32(Data->ScenarioId));
	JsonObject->SetStringField("PlayerEmblemIndex", FormatInt32(Data->PlayerEmblemIndex));
	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	JsonObject->SetStringField("PlayerFleetIdentifier", Data->PlayerFleetIdentifier.ToString());
	JsonObject->SetStringField("LastFlownShipIdentifier", Data->LastFlownShipIdentifier.ToString());
	JsonObject->SetObjectField("Quest", SaveQuest(&Data->QuestData));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuest(FFlareQuestSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SelectedQuest", Data->SelectedQuest.ToString());
	JsonObject->SetBoolField("PlayTutorial", Data->PlayTutorial);
	JsonObject->SetStringField("NextGeneratedQuestIndex", FormatInt64(Data->NextGeneratedQuestIndex));


	TArray< TSharedPtr<FJsonValue> > QuestProgresses;
	for(int i = 0; i < Data->QuestProgresses.Num(); i++)
	{
		QuestProgresses.Add(MakeShareable(new FJsonValueObject(SaveQuestProgress(&Data->QuestProgresses[i]))));
	}
	JsonObject->SetArrayField("QuestProgresses", QuestProgresses);

	TArray< TSharedPtr<FJsonValue> > SuccessfulQuests;
	for(int i = 0; i < Data->SuccessfulQuests.Num(); i++)
	{
		SuccessfulQuests.Add(MakeShareable(new FJsonValueString(Data->SuccessfulQuests[i].ToString())));
	}
	JsonObject->SetArrayField("SuccessfulQuests", SuccessfulQuests);

	TArray< TSharedPtr<FJsonValue> > AbandonedQuests;
	for(int i = 0; i < Data->AbandonedQuests.Num(); i++)
	{
		AbandonedQuests.Add(MakeShareable(new FJsonValueString(Data->AbandonedQuests[i].ToString())));
	}
	JsonObject->SetArrayField("AbandonedQuests", AbandonedQuests);

	TArray< TSharedPtr<FJsonValue> > FailedQuests;
	for(int i = 0; i < Data->FailedQuests.Num(); i++)
	{
		FailedQuests.Add(MakeShareable(new FJsonValueString(Data->FailedQuests[i].ToString())));
	}
	JsonObject->SetArrayField("FailedQuests", FailedQuests);

	TArray< TSharedPtr<FJsonValue> > GeneratedQuests;
	for(int i = 0; i < Data->GeneratedQuests.Num(); i++)
	{
		GeneratedQuests.Add(MakeShareable(new FJsonValueObject(SaveGeneratedQuest(&Data->GeneratedQuests[i]))));
	}
	JsonObject->SetArrayField("GeneratedQuests", GeneratedQuests);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuestProgress(FFlareQuestProgressSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("QuestIdentifier", Data->QuestIdentifier.ToString());
	JsonObject->SetStringField("Status", FormatEnum<EFlareQuestStatus::Type>("EFlareQuestStatus",Data->Status));

	JsonObject->SetStringField("AvailableDate", FormatInt64(Data->AvailableDate));
	JsonObject->SetStringField("AcceptationDate", FormatInt64(Data->AcceptationDate));

	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	TArray< TSharedPtr<FJsonValue> > SuccessfullSteps;
	for(int i = 0; i < Data->SuccessfullSteps.Num(); i++)
	{
		SuccessfullSteps.Add(MakeShareable(new FJsonValueString(Data->SuccessfullSteps[i].ToString())));
	}
	JsonObject->SetArrayField("SuccessfullSteps", SuccessfullSteps);

	TArray< TSharedPtr<FJsonValue> > CurrentStepProgress;
	for(int i = 0; i < Data->CurrentStepProgress.Num(); i++)
	{
		CurrentStepProgress.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->CurrentStepProgress[i]))));
	}
	JsonObject->SetArrayField("CurrentStepProgress", CurrentStepProgress);

	TArray< TSharedPtr<FJsonValue> > TriggerConditionsSave;
	for(int i = 0; i < Data->TriggerConditionsSave.Num(); i++)
	{
		TriggerConditionsSave.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->TriggerConditionsSave[i]))));
	}
	JsonObject->SetArrayField("TriggerConditionsSave", TriggerConditionsSave);

	TArray< TSharedPtr<FJsonValue> > ExpirationConditionsSave;
	for(int i = 0; i < Data->ExpirationConditionsSave.Num(); i++)
	{
		ExpirationConditionsSave.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->ExpirationConditionsSave[i]))));
	}
	JsonObject->SetArrayField("ExpirationConditionsSave", ExpirationConditionsSave);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveGeneratedQuest(FFlareGeneratedQuestSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("QuestClass", Data->QuestClass.ToString());
	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuestStepProgress(FFlareQuestConditionSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ConditionIdentifier", Data->ConditionIdentifier.ToString());
	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyDescription(FFlareCompanyDescription* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("ShortName", Data->ShortName.ToString());
	JsonObject->SetStringField("Description", Data->Description.ToString());

	JsonObject->SetStringField("CustomizationBasePaintColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationBasePaintColor)));
	JsonObject->SetStringField("CustomizationPaintColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationPaintColor)));
	JsonObject->SetStringField("CustomizationOverlayColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationOverlayColor)));
	JsonObject->SetStringField("CustomizationLightColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationLightColor)));
	JsonObject->SetStringField("CustomizationPatternIndex", FormatInt32(Data->CustomizationPatternIndex));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveWorld(FFlareWorldSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Date", FormatInt64(Data->Date));

	TArray< TSharedPtr<FJsonValue> > Companies;
	for(int i = 0; i < Data->CompanyData.Num(); i++)
	{
		Companies.Add(MakeShareable(new FJsonValueObject(SaveCompany(&Data->CompanyData[i]))));
	}
	JsonObject->SetArrayField("Companies", Companies);

	TArray< TSharedPtr<FJsonValue> > Sectors;
	for(int i = 0; i < Data->SectorData.Num(); i++)
	{
		Sectors.Add(MakeShareable(new FJsonValueObject(SaveSector(&Data->SectorData[i]))));
	}
	JsonObject->SetArrayField("Sectors", Sectors);

	TArray< TSharedPtr<FJsonValue> > Travels;
	for(int i = 0; i < Data->TravelData.Num(); i++)
	{
		Travels.Add(MakeShareable(new FJsonValueObject(SaveTravel(&Data->TravelData[i]))));
	}
	JsonObject->SetArrayField("Travels", Travels);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompany(FFlareCompanySave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("CatalogIdentifier", FormatInt32(Data->CatalogIdentifier));
	JsonObject->SetStringField("Money", FormatInt64(Data->Money));
	JsonObject->SetStringField("CompanyValue", FormatInt64(Data->CompanyValue));
	JsonObject->SetStringField("PlayerLastPeaceDate", FormatInt64(Data->PlayerLastPeaceDate));
	JsonObject->SetStringField("PlayerLastWarDate", FormatInt64(Data->PlayerLastWarDate));
	JsonObject->SetStringField("PlayerLastTributeDate", FormatInt64(Data->PlayerLastTributeDate));
	JsonObject->SetStringField("FleetImmatriculationIndex", FormatInt32(Data->FleetImmatriculationIndex));
	JsonObject->SetStringField("TradeRouteImmatriculationIndex", FormatInt32(Data->TradeRouteImmatriculationIndex));
	JsonObject->SetStringField("ResearchAmount", FormatInt32(Data->ResearchAmount));
	JsonObject->SetStringField("ResearchSpent", FormatInt32(Data->ResearchSpent));
	JsonObject->SetObjectField("AI", SaveCompanyAI(&Data->AI));
	SaveFloat(JsonObject,"Shame", Data->Shame);
	SaveFloat(JsonObject,"ResearchRatio", Data->ResearchRatio);

	TArray< TSharedPtr<FJsonValue> > UnlockedTechnologies;
	for (int i = 0; i < Data->UnlockedTechnologies.Num(); i++)
	{
		UnlockedTechnologies.Add(MakeShareable(new FJsonValueString(Data->UnlockedTechnologies[i].ToString())));
	}
	JsonObject->SetArrayField("UnlockedTechnologies", UnlockedTechnologies);

	TArray< TSharedPtr<FJsonValue> > CaptureOrders;
	for (int i = 0; i < Data->CaptureOrders.Num(); i++)
	{
		CaptureOrders.Add(MakeShareable(new FJsonValueString(Data->CaptureOrders[i].ToString())));
	}
	JsonObject->SetArrayField("CaptureOrders", CaptureOrders);


	TArray< TSharedPtr<FJsonValue> > HostileCompanies;
	for(int i = 0; i < Data->HostileCompanies.Num(); i++)
	{
		HostileCompanies.Add(MakeShareable(new FJsonValueString(Data->HostileCompanies[i].ToString())));
	}
	JsonObject->SetArrayField("HostileCompanies", HostileCompanies);

	TArray< TSharedPtr<FJsonValue> > Ships;
	for(int i = 0; i < Data->ShipData.Num(); i++)
	{
		Ships.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->ShipData[i]))));
	}
	JsonObject->SetArrayField("Ships", Ships);

	TArray< TSharedPtr<FJsonValue> > Stations;
	for(int i = 0; i < Data->StationData.Num(); i++)
	{
		Stations.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->StationData[i]))));
	}
	JsonObject->SetArrayField("Stations", Stations);

	TArray< TSharedPtr<FJsonValue> > DestroyedSpacecrafts;
	for(int i = 0; i < Data->DestroyedSpacecraftData.Num(); i++)
	{
		DestroyedSpacecrafts.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->DestroyedSpacecraftData[i]))));
	}
	JsonObject->SetArrayField("DestroyedSpacecrafts", DestroyedSpacecrafts);

	TArray< TSharedPtr<FJsonValue> > Fleets;
	for(int i = 0; i < Data->Fleets.Num(); i++)
	{
		Fleets.Add(MakeShareable(new FJsonValueObject(SaveFleet(&Data->Fleets[i]))));
	}
	JsonObject->SetArrayField("Fleets", Fleets);

	TArray< TSharedPtr<FJsonValue> > TradeRoutes;
	for(int i = 0; i < Data->TradeRoutes.Num(); i++)
	{
		TradeRoutes.Add(MakeShareable(new FJsonValueObject(SaveTradeRoute(&Data->TradeRoutes[i]))));
	}
	JsonObject->SetArrayField("TradeRoutes", TradeRoutes);


	TArray< TSharedPtr<FJsonValue> > SectorsKnowledge;
	for(int i = 0; i < Data->SectorsKnowledge.Num(); i++)
	{
		SectorsKnowledge.Add(MakeShareable(new FJsonValueObject(SaveSectorKnowledge(&Data->SectorsKnowledge[i]))));
	}
	JsonObject->SetArrayField("SectorsKnowledge", SectorsKnowledge);

	SaveFloat(JsonObject,"PlayerReputation", Data->PlayerReputation);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraft(FFlareSpacecraftSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// TODO light save if destroyed
	JsonObject->SetBoolField("IsDestroyed", Data->IsDestroyed);
	JsonObject->SetBoolField("IsUnderConstruction", Data->IsUnderConstruction);
	JsonObject->SetStringField("Immatriculation", Data->Immatriculation.ToString());
	JsonObject->SetStringField("NickName", Data->NickName.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("SpawnMode", FormatEnum<EFlareSpawnMode::Type>("EFlareSpawnMode",Data->SpawnMode));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("DockedTo", Data->DockedTo.ToString());
	JsonObject->SetStringField("DockedAt", FormatInt32(Data->DockedAt));
	SaveFloat(JsonObject,"DockedAngle", Data->DockedAngle);
	SaveFloat(JsonObject,"Heat", Data->Heat);
	SaveFloat(JsonObject,"PowerOutageDelay", Data->PowerOutageDelay);
	SaveFloat(JsonObject,"PowerOutageAcculumator", Data->PowerOutageAcculumator);
	JsonObject->SetStringField("DynamicComponentStateIdentifier", Data->DynamicComponentStateIdentifier.ToString());
	SaveFloat(JsonObject,"DynamicComponentStateProgress", Data->DynamicComponentStateProgress);
	JsonObject->SetStringField("Level", FormatInt32(Data->Level));
	JsonObject->SetBoolField("IsTrading", Data->IsTrading);
	JsonObject->SetBoolField("IsIntercepted", Data->IsIntercepted);
	SaveFloat(JsonObject,"RefillStock", Data->RefillStock);
	SaveFloat(JsonObject,"RepairStock", Data->RepairStock);
	JsonObject->SetBoolField("IsReserve", Data->IsReserve);
	JsonObject->SetBoolField("AllowExternalOrder", Data->AllowExternalOrder);
	JsonObject->SetObjectField("Pilot", SavePilot(&Data->Pilot));
	JsonObject->SetObjectField("Asteroid", SaveAsteroid(&Data->AsteroidData));
	JsonObject->SetStringField("HarpoonCompany", Data->HarpoonCompany.ToString());
	JsonObject->SetStringField("AttachActorName", Data->AttachActorName.ToString());
	JsonObject->SetStringField("AttachComplexStationName", Data->AttachComplexStationName.ToString());
	JsonObject->SetStringField("AttachComplexConnectorName", Data->AttachComplexConnectorName.ToString());

	TArray< TSharedPtr<FJsonValue> > Components;
	for(int i = 0; i < Data->Components.Num(); i++)
	{
		Components.Add(MakeShareable(new FJsonValueObject(SaveSpacecraftComponent(&Data->Components[i]))));
	}
	JsonObject->SetArrayField("Components", Components);

	TArray< TSharedPtr<FJsonValue> > Cargo;
	for(int i = 0; i < Data->Cargo.Num(); i++)
	{
		Cargo.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->Cargo[i]))));
	}
	JsonObject->SetArrayField("Cargo", Cargo);

	TArray< TSharedPtr<FJsonValue> > CargoBackup;
	for(int i = 0; i < Data->CargoBackup.Num(); i++)
	{
		CargoBackup.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->CargoBackup[i]))));
	}
	JsonObject->SetArrayField("CargoBackup", CargoBackup);

	TArray< TSharedPtr<FJsonValue> > FactoryStates;
	for(int i = 0; i < Data->FactoryStates.Num(); i++)
	{
		FactoryStates.Add(MakeShareable(new FJsonValueObject(SaveFactory(&Data->FactoryStates[i]))));
	}
	JsonObject->SetArrayField("FactoryStates", FactoryStates);

	TArray< TSharedPtr<FJsonValue> > ShipyardOrderQueue;
	for(int i = 0; i < Data->ShipyardOrderQueue.Num(); i++)
	{
		ShipyardOrderQueue.Add(MakeShareable(new FJsonValueObject(SaveShipyardOrderQueue(&Data->ShipyardOrderQueue[i]))));
	}
	JsonObject->SetArrayField("ShipyardOrderQueue", ShipyardOrderQueue);

	TArray< TSharedPtr<FJsonValue> > SalesExcludedResources;
	for(int i = 0; i < Data->SalesExcludedResources.Num(); i++)
	{
		SalesExcludedResources.Add(MakeShareable(new FJsonValueString(Data->SalesExcludedResources[i].ToString())));
	}
	JsonObject->SetArrayField("SalesExcludedResources", SalesExcludedResources);

	TArray< TSharedPtr<FJsonValue> > ConnectedStations;
	for (int i = 0; i < Data->ConnectedStations.Num(); i++)
	{
		ConnectedStations.Add(MakeShareable(new FJsonValueObject(SaveStationConnection(&Data->ConnectedStations[i]))));
	}
	JsonObject->SetArrayField("ConnectedStations", ConnectedStations);

	TArray<FName> CapturePointCompanies;
	Data->CapturePoints.GetKeys(CapturePointCompanies);
	TArray< TSharedPtr<FJsonValue> > CapturePoints;
	for(int i = 0; i < Data->CapturePoints.Num(); i++)
	{
		FName Company = CapturePointCompanies[i];
		int32 Points = Data->CapturePoints[Company];

		TSharedRef<FJsonObject> JsonChildObject = MakeShareable(new FJsonObject());

		JsonChildObject->SetStringField("Company", Company.ToString());
		JsonChildObject->SetStringField("Points", FormatInt32(Points));

		CapturePoints.Add(MakeShareable(new FJsonValueObject(JsonChildObject)));
	}
	JsonObject->SetArrayField("CapturePoints", CapturePoints);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SavePilot(FFlareShipPilotSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Name", Data->Name);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveAsteroid(FFlareAsteroidSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("Scale", FormatVector(Data->Scale));
	JsonObject->SetStringField("AsteroidMeshID", FormatInt32(Data->AsteroidMeshID));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveMeteorite(FFlareMeteoriteSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("TargetOffset", FormatVector(Data->TargetOffset));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("MeteoriteMeshID", FormatInt32(Data->MeteoriteMeshID));
	JsonObject->SetStringField("IsMetal", FormatInt32(Data->IsMetal));
	SaveFloat(JsonObject,"Damage", Data->Damage);
	SaveFloat(JsonObject,"BrokenDamage", Data->BrokenDamage);
	JsonObject->SetStringField("TargetStation", Data->TargetStation.ToString());
	JsonObject->SetStringField("HasMissed", FormatInt32(Data->HasMissed));
	JsonObject->SetStringField("DaysBeforeImpact", FormatInt32(Data->DaysBeforeImpact));



	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponent(FFlareSpacecraftComponentSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ComponentIdentifier", Data->ComponentIdentifier.ToString());
	JsonObject->SetStringField("ShipSlotIdentifier", Data->ShipSlotIdentifier.ToString());
	SaveFloat(JsonObject,"Damage", Data->Damage);
	JsonObject->SetObjectField("Turret", SaveSpacecraftComponentTurret(&Data->Turret));
	JsonObject->SetObjectField("Weapon", SaveSpacecraftComponentWeapon(&Data->Weapon));
	JsonObject->SetObjectField("Pilot", SaveTurretPilot(&Data->Pilot));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponentTurret(FFlareSpacecraftComponentTurretSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	SaveFloat(JsonObject,"TurretAngle", Data->TurretAngle);
	SaveFloat(JsonObject,"BarrelsAngle", Data->BarrelsAngle);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponentWeapon(FFlareSpacecraftComponentWeaponSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("FiredAmmo", FormatInt32(Data->FiredAmmo));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveTurretPilot(FFlareTurretPilotSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Name", Data->Name);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeOperation(FFlareTradeRouteSectorOperationSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	JsonObject->SetStringField("MaxQuantity", FormatInt32(Data->MaxQuantity));
	JsonObject->SetStringField("MaxWait", FormatInt32(Data->MaxWait));
	JsonObject->SetStringField("Type", FormatEnum<EFlareTradeRouteOperation::Type>("EFlareTradeRouteOperation",Data->Type));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCargo(FFlareCargoSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	JsonObject->SetStringField("Quantity", FormatInt32(Data->Quantity));
	JsonObject->SetStringField("Lock", FormatEnum<EFlareResourceLock::Type>("EFlareResourceLock",Data->Lock));
	JsonObject->SetStringField("Restriction", FormatEnum<EFlareResourceRestriction::Type>("EFlareResourceRestriction",Data->Restriction));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveShipyardOrderQueue(FFlareShipyardOrderSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Company", Data->Company.ToString());
	JsonObject->SetStringField("ShipClass", Data->ShipClass.ToString());
	JsonObject->SetStringField("AdvancePayment", FormatInt32(Data->AdvancePayment));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveStationConnection(FFlareConnectionSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ConnectorName", Data->ConnectorName.ToString());
	JsonObject->SetStringField("StationIdentifier", Data->StationIdentifier.ToString());

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFactory(FFlareFactorySave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetBoolField("Active", Data->Active);
	JsonObject->SetStringField("CostReserved", FormatInt32(Data->CostReserved));
	JsonObject->SetStringField("ProductedDuration", FormatInt64(Data->ProductedDuration));
	JsonObject->SetBoolField("InfiniteCycle", Data->InfiniteCycle);
	JsonObject->SetStringField("CycleCount", FormatInt32(Data->CycleCount));
	JsonObject->SetStringField("TargetShipClass", Data->TargetShipClass.ToString());
	JsonObject->SetStringField("TargetShipCompany", Data->TargetShipCompany.ToString());

	TArray< TSharedPtr<FJsonValue> > ResourceReserved;
	for(int i = 0; i < Data->ResourceReserved.Num(); i++)
	{
		ResourceReserved.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->ResourceReserved[i]))));
	}
	JsonObject->SetArrayField("ResourceReserved", ResourceReserved);

	TArray< TSharedPtr<FJsonValue> > OutputCargoLimit;
	for(int i = 0; i < Data->OutputCargoLimit.Num(); i++)
	{
		OutputCargoLimit.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->OutputCargoLimit[i]))));
	}
	JsonObject->SetArrayField("OutputCargoLimit", OutputCargoLimit);

	return JsonObject;
}

//////////////////

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFleet(FFlareFleetSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());

	TArray< TSharedPtr<FJsonValue> > ShipImmatriculations;
	for(int i = 0; i < Data->ShipImmatriculations.Num(); i++)
	{
		ShipImmatriculations.Add(MakeShareable(new FJsonValueString(Data->ShipImmatriculations[i].ToString())));
	}
	JsonObject->SetArrayField("ShipImmatriculations", ShipImmatriculations);
	JsonObject->SetStringField("FleetColor", FormatVector(UFlareGameTools::ColorToVector(Data->FleetColor)));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeRoute(FFlareTradeRouteSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("FleetIdentifier", Data->FleetIdentifier.ToString());
	JsonObject->SetStringField("TargetSectorIdentifier", Data->TargetSectorIdentifier.ToString());
	JsonObject->SetStringField("CurrentOperationIndex", FormatInt32(Data->CurrentOperationIndex));
	JsonObject->SetStringField("CurrentOperationProgress", FormatInt32(Data->CurrentOperationProgress));
	JsonObject->SetStringField("CurrentOperationDuration", FormatInt32(Data->CurrentOperationDuration));
	JsonObject->SetBoolField("IsPaused", Data->IsPaused);


	// Stats
	JsonObject->SetStringField("StatsDays", FormatInt32(Data->StatsDays));
	JsonObject->SetStringField("StatsLoadResources", FormatInt32(Data->StatsLoadResources));
	JsonObject->SetStringField("StatsUnloadResources", FormatInt32(Data->StatsUnloadResources));
	JsonObject->SetStringField("StatsMoneySell", FormatInt64(Data->StatsMoneySell));
	JsonObject->SetStringField("StatsMoneyBuy", FormatInt64(Data->StatsMoneyBuy));
	JsonObject->SetStringField("StatsOperationSuccessCount", FormatInt32(Data->StatsOperationSuccessCount));
	JsonObject->SetStringField("StatsOperationFailCount", FormatInt32(Data->StatsOperationFailCount));

	TArray< TSharedPtr<FJsonValue> > Sectors;
	for(int i = 0; i < Data->Sectors.Num(); i++)
	{
		Sectors.Add(MakeShareable(new FJsonValueObject(SaveTradeRouteSector(&Data->Sectors[i]))));
	}
	JsonObject->SetArrayField("Sectors", Sectors);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeRouteSector(FFlareTradeRouteSectorSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SectorIdentifier", Data->SectorIdentifier.ToString());


	TArray< TSharedPtr<FJsonValue> > Operations;
	for(int i = 0; i < Data->Operations.Num(); i++)
	{
		Operations.Add(MakeShareable(new FJsonValueObject(SaveTradeOperation(&Data->Operations[i]))));
	}
	JsonObject->SetArrayField("Operations", Operations);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveSectorKnowledge(FFlareCompanySectorKnowledge* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SectorIdentifier", Data->SectorIdentifier.ToString());
	JsonObject->SetStringField("Knowledge", FormatEnum<EFlareSectorKnowledge::Type>("EFlareSectorKnowledge",Data->Knowledge));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyAI(FFlareCompanyAISave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("BudgetMilitary", FormatInt64(Data->BudgetMilitary));
	JsonObject->SetStringField("BudgetStation", FormatInt64(Data->BudgetStation));
	JsonObject->SetStringField("BudgetTechnology", FormatInt64(Data->BudgetTechnology));
	JsonObject->SetStringField("BudgetTrade", FormatInt64(Data->BudgetTrade));
	SaveFloat(JsonObject,"Caution", Data->Caution);
	SaveFloat(JsonObject,"Pacifism", Data->Pacifism);
	JsonObject->SetStringField("ResearchProject", Data->ResearchProject.ToString());

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyReputation(FFlareCompanyReputationSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	SaveFloat(JsonObject,"Reputation", Data->Reputation);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSector(FFlareSectorSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("GivenName", Data->GivenName.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("LocalTime", FormatInt64(Data->LocalTime));
	JsonObject->SetObjectField("People", SavePeople(&Data->PeopleData));


	TArray< TSharedPtr<FJsonValue> > Bombs;
	for(int i = 0; i < Data->BombData.Num(); i++)
	{
		Bombs.Add(MakeShareable(new FJsonValueObject(SaveBomb(&Data->BombData[i]))));
	}
	JsonObject->SetArrayField("Bombs", Bombs);


	TArray< TSharedPtr<FJsonValue> > Asteroids;
	for(int i = 0; i < Data->AsteroidData.Num(); i++)
	{
		Asteroids.Add(MakeShareable(new FJsonValueObject(SaveAsteroid(&Data->AsteroidData[i]))));
	}
	JsonObject->SetArrayField("Asteroids", Asteroids);

	TArray< TSharedPtr<FJsonValue> > Meteorites;
	for(int i = 0; i < Data->MeteoriteData.Num(); i++)
	{
		Meteorites.Add(MakeShareable(new FJsonValueObject(SaveMeteorite(&Data->MeteoriteData[i]))));
	}
	JsonObject->SetArrayField("Meteorites", Meteorites);

	TArray< TSharedPtr<FJsonValue> > FleetIdentifiers;
	for(int i = 0; i < Data->FleetIdentifiers.Num(); i++)
	{
		FleetIdentifiers.Add(MakeShareable(new FJsonValueString(Data->FleetIdentifiers[i].ToString())));
	}
	JsonObject->SetArrayField("FleetIdentifiers", FleetIdentifiers);

	TArray< TSharedPtr<FJsonValue> > SpacecraftIdentifiers;
	for(int i = 0; i < Data->SpacecraftIdentifiers.Num(); i++)
	{
		SpacecraftIdentifiers.Add(MakeShareable(new FJsonValueString(Data->SpacecraftIdentifiers[i].ToString())));
	}
	JsonObject->SetArrayField("SpacecraftIdentifiers", SpacecraftIdentifiers);


	TArray< TSharedPtr<FJsonValue> > ResourcePrices;
	for(int i = 0; i < Data->ResourcePrices.Num(); i++)
	{
		ResourcePrices.Add(MakeShareable(new FJsonValueObject(SaveResourcePrice(&Data->ResourcePrices[i]))));
	}
	JsonObject->SetArrayField("ResourcePrices", ResourcePrices);

	JsonObject->SetBoolField("IsTravelSector", Data->IsTravelSector);

	JsonObject->SetObjectField("FleetSupplyConsumptionStats", SaveFloatBuffer(&Data->FleetSupplyConsumptionStats));
	JsonObject->SetStringField("DailyFleetSupplyConsumption", FormatInt32(Data->DailyFleetSupplyConsumption));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SavePeople(FFlarePeopleSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Population", FormatInt32(Data->Population));
	JsonObject->SetStringField("FoodStock", FormatInt32(Data->FoodStock));
	JsonObject->SetStringField("FuelStock", FormatInt32(Data->FuelStock));
	JsonObject->SetStringField("ToolStock", FormatInt32(Data->ToolStock));
	JsonObject->SetStringField("TechStock", FormatInt32(Data->TechStock));
	SaveFloat(JsonObject,"FoodConsumption", Data->FoodConsumption);
	SaveFloat(JsonObject,"FuelConsumption", Data->FuelConsumption);
	SaveFloat(JsonObject,"ToolConsumption", Data->ToolConsumption);
	SaveFloat(JsonObject,"TechConsumption", Data->TechConsumption);
	JsonObject->SetStringField("Money", FormatInt32(Data->Money));
	JsonObject->SetStringField("Dept", FormatInt32(Data->Dept));
	JsonObject->SetStringField("BirthPoint", FormatInt32(Data->BirthPoint));
	JsonObject->SetStringField("DeathPoint", FormatInt32(Data->DeathPoint));
	JsonObject->SetStringField("HungerPoint", FormatInt32(Data->HungerPoint));
	JsonObject->SetStringField("HappinessPoint", FormatInt32(Data->HappinessPoint));

	TArray< TSharedPtr<FJsonValue> > CompanyReputations;
	for(int i = 0; i < Data->CompanyReputations.Num(); i++)
	{
		CompanyReputations.Add(MakeShareable(new FJsonValueObject(SaveCompanyReputation(&Data->CompanyReputations[i]))));
	}
	JsonObject->SetArrayField("CompanyReputations", CompanyReputations);


	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveBomb(FFlareBombSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("WeaponSlotIdentifier", Data->WeaponSlotIdentifier.ToString());
	JsonObject->SetStringField("AimTargetSpacecraft", Data->AimTargetSpacecraft.ToString());
	JsonObject->SetStringField("ParentSpacecraft", Data->ParentSpacecraft.ToString());
	JsonObject->SetStringField("AttachTarget", Data->AttachTarget.ToString());
	JsonObject->SetBoolField("Activated", Data->Activated);
	JsonObject->SetBoolField("Dropped", Data->Dropped);
	JsonObject->SetBoolField("Locked", Data->Locked);
	SaveFloat(JsonObject,"DropParentDistance", Data->DropParentDistance);
	SaveFloat(JsonObject,"LifeTime", Data->LifeTime);
	SaveFloat(JsonObject,"BurnDuration", Data->BurnDuration);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveResourcePrice(FFFlareResourcePrice* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	SaveFloat(JsonObject,"Price", Data->Price);
	JsonObject->SetObjectField("Prices", SaveFloatBuffer(&Data->Prices));


	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFloatBuffer(FFlareFloatBuffer* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("MaxSize", FormatInt32(Data->MaxSize));
	JsonObject->SetStringField("WriteIndex", FormatInt32(Data->WriteIndex));



	TArray< TSharedPtr<FJsonValue> > Values;
	for(int i = 0; i < Data->Values.Num(); i++)
	{
		Values.Add(MakeShareable(new FJsonValueNumber(Data->Values[i])));
	}
	JsonObject->SetArrayField("Values", Values);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveBundle(FFlareBundle* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if(Data->FloatValues.Num() > 0)
	{
		TSharedRef<FJsonObject> FloatObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->FloatValues)
		{
			FloatObject->SetNumberField(Pair.Key.ToString(), FixFloat(Pair.Value));
		}
		JsonObject->SetObjectField("FloatValues", FloatObject);
	}

	if(Data->Int32Values.Num() > 0)
	{
		TSharedRef<FJsonObject> Int32Object = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->Int32Values)
		{
			Int32Object->SetStringField(Pair.Key.ToString(), FormatInt32(Pair.Value));
		}
		JsonObject->SetObjectField("Int32Values", Int32Object);
	}

	if(Data->TransformValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->TransformValues)
		{
			TransformObject->SetStringField(Pair.Key.ToString(), FormatTransform(Pair.Value));
		}
		JsonObject->SetObjectField("TransformValues", TransformObject);
	}

	if(Data->VectorArrayValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->VectorArrayValues)
		{
			TArray< TSharedPtr<FJsonValue> > VectorArray;
			for(FVector Vector: Pair.Value.Entries)
			{
				VectorArray.Add(MakeShareable(new FJsonValueString(FormatVector(Vector))));
			}

			TransformObject->SetArrayField(Pair.Key.ToString(), VectorArray);
		}
		JsonObject->SetObjectField("VectorArrayValues", TransformObject);
	}

	if(Data->NameValues.Num() > 0)
	{
		TSharedRef<FJsonObject> Int32Object = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->NameValues)
		{
			Int32Object->SetStringField(Pair.Key.ToString(), Pair.Value.ToString());
		}
		JsonObject->SetObjectField("NameValues", Int32Object);
	}
	
	if(Data->NameArrayValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->NameArrayValues)
		{
			TArray< TSharedPtr<FJsonValue> > NameArray;
			for(FName Name: Pair.Value.Entries)
			{
				NameArray.Add(MakeShareable(new FJsonValueString(Name.ToString())));
			}

			TransformObject->SetArrayField(Pair.Key.ToString(), NameArray);
		}
		JsonObject->SetObjectField("NameArrayValues", TransformObject);
	}

	if (Data->StringValues.Num() > 0)
	{
		TSharedRef<FJsonObject> StringObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->StringValues)
		{
			StringObject->SetStringField(Pair.Key.ToString(), Pair.Value);
		}
		JsonObject->SetObjectField("StringValues", StringObject);
	}

	if (Data->Tags.Num() > 0)
	{
		TArray< TSharedPtr<FJsonValue> > Tags;
		for(int i = 0; i < Data->Tags.Num(); i++)
		{
			Tags.Add(MakeShareable(new FJsonValueString(Data->Tags[i].ToString())));
		}
		JsonObject->SetArrayField("Tags", Tags);
	}

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTravel(FFlareTravelSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("FleetIdentifier", Data->FleetIdentifier.ToString());
	JsonObject->SetStringField("OriginSectorIdentifier", Data->OriginSectorIdentifier.ToString());
	JsonObject->SetStringField("DestinationSectorIdentifier", Data->DestinationSectorIdentifier.ToString());
	JsonObject->SetStringField("DepartureDate", FormatInt64(Data->DepartureDate));

	JsonObject->SetObjectField("SectorData", SaveSector(&Data->SectorData));

	return JsonObject;
}

void UFlareSaveWriter::SaveFloat(TSharedPtr< FJsonObject > Object, FString Key, float Data)
{
	Object->SetNumberField(Key, FixFloat(Data));
}
