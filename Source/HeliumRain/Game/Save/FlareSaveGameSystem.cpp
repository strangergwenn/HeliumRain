
#include "../../Flare.h"

#include "FlareSaveGameSystem.h"
#include "FlareSaveWriter.h"
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

	FLOGV("UFlareSaveGameSystem::SaveGame SaveName=%s", *SaveName);

	UFlareSaveWriter* SaveWriter = NewObject<UFlareSaveWriter>(this, UFlareSaveWriter::StaticClass());
	TSharedRef<FJsonObject> JsonObject = SaveWriter->SaveGame(SaveData);

	// Save the json object
	FString FileContents;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&FileContents);
	//TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&FileContents);

	if (FJsonSerializer::Serialize(JsonObject, JsonWriter))
	{
		JsonWriter->Close();
		return FFileHelper::SaveStringToFile(FileContents, *GetSaveGamePath(SaveName));
	}
	else
	{
		FLOGV("Fail to serialize save %s", *SaveName);
		return false;
	}
}

UFlareSaveGame* UFlareSaveGameSystem::LoadGame(const FString SaveName)
{
	// TODO Load stuff

	FString JsonRaw = "{ \"exampleString\": \"Hello World\" }";
		TSharedPtr<FJsonObject> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonRaw);
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
		{
			FString ExampleString = JsonParsed->GetStringField("exampleString");
		}

	//return FFileHelper::LoadFileToArray(Data, *GetSaveGamePath(SaveName));
	return NULL;
}

bool UFlareSaveGameSystem::DeleteGame(const FString SaveName)
{
	return IFileManager::Get().Delete(*GetSaveGamePath(SaveName), true);
}



/*----------------------------------------------------
	Getters
----------------------------------------------------*/


FString UFlareSaveGameSystem::GetSaveGamePath(const FString SaveName)
{
	return FString::Printf(TEXT("%s/SaveGames/%s.json"), *FPaths::GameSavedDir(), *SaveName);
}
