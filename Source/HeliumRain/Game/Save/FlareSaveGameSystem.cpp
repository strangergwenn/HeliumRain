
#include "FlareSaveGameSystem.h"
#include "../../Flare.h"

#include "FlareSaveWriter.h"
#include "FlareSaveReaderV1.h"
#include "../FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSaveGameSystem::UFlareSaveGameSystem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*----------------------------------------------------
	Interface
----------------------------------------------------*/


bool UFlareSaveGameSystem::DoesSaveGameExist(const FString SaveName)
{
	return IFileManager::Get().FileSize(*GetSaveGamePath(SaveName)) >= 0;
}

bool UFlareSaveGameSystem::SaveGame(const FString SaveName, UFlareSaveGame* SaveData)
{
	bool ret = false;
	SaveLock.Lock();
	FLOGV("UFlareSaveGameSystem::SaveGame SaveName=%s", *SaveName);

	UFlareSaveWriter* SaveWriter = NewObject<UFlareSaveWriter>(this, UFlareSaveWriter::StaticClass());
	TSharedRef<FJsonObject> JsonObject = SaveWriter->SaveGame(SaveData);

	// Save the json object
	FString FileContents;
	//TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&FileContents);
	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&FileContents);

	if (FJsonSerializer::Serialize(JsonObject, JsonWriter))
	{
		JsonWriter->Close();

		ret = FFileHelper::SaveStringToFile(FileContents, *GetSaveGamePath(SaveName));
		FLOG("UFlareSaveGameSystem::SaveGame : Save done");
	}
	else
	{
		FLOGV("Fail to serialize save %s", *SaveName);
		ret = false;
	}

	SaveLock.Unlock();

	SaveListLock.Lock();
	SaveList.Remove(SaveData);
	SaveListLock.Unlock();

	return ret;
}

UFlareSaveGame* UFlareSaveGameSystem::LoadGame(const FString SaveName)
{
	FLOGV("UFlareSaveGameSystem::LoadGame SaveName=%s", *SaveName);

	UFlareSaveGame *SaveGame = NULL;

	// Read the saveto a string
	FString SaveString;
	if(FFileHelper::LoadFileToString(SaveString, *GetSaveGamePath(SaveName)))
	{
		// Deserialize a JSON object from the string
		TSharedPtr< FJsonObject > Object;
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(SaveString);
		if(FJsonSerializer::Deserialize(Reader, Object) && Object.IsValid())
		{
			UFlareSaveReaderV1* SaveReader = NewObject<UFlareSaveReaderV1>(this, UFlareSaveReaderV1::StaticClass());
			SaveGame = SaveReader->LoadGame(Object);
		}
		else
		{
			FLOGV("Fail to deserialize save '%s'", *GetSaveGamePath(SaveName));
		}
	}
	else
	{
		FLOGV("Fail to read save '%s'", *GetSaveGamePath(SaveName));
	}

	return SaveGame;
}

bool UFlareSaveGameSystem::DeleteGame(const FString SaveName)
{
	return IFileManager::Get().Delete(*GetSaveGamePath(SaveName), true);
}


void UFlareSaveGameSystem::PushSaveData(UFlareSaveGame* SaveData)
{
	SaveListLock.Lock();
	SaveList.Add(SaveData);
	SaveListLock.Unlock();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


FString UFlareSaveGameSystem::GetSaveGamePath(const FString SaveName)
{
	return FString::Printf(TEXT("%s/SaveGames/%s.json"), *FPaths::GameSavedDir(), *SaveName);
}
