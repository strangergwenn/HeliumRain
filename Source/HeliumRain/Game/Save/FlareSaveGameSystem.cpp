
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
	return IFileManager::Get().FileSize(*GetSaveGamePath(SaveName, true)) >= 0 || IFileManager::Get().FileSize(*GetSaveGamePath(SaveName, false)) >= 0;
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

		bool Compress = true;
		if(Compress)
		{
			uint32 StrLength = FCStringAnsi::Strlen(TCHAR_TO_UTF8(*FileContents));

			uint8* CompressedDataRaw = new uint8[StrLength];
			int32 CompressedSize = StrLength;

			const bool bResult = FCompression::CompressMemory(NAME_Gzip, CompressedDataRaw, CompressedSize, TCHAR_TO_UTF8(*FileContents), StrLength);
			if (bResult)
			{
				ret = FFileHelper::SaveArrayToFile(TArrayView<const uint8>(CompressedDataRaw, CompressedSize), *GetSaveGamePath(SaveName, true));
			}

			delete[] CompressedDataRaw;
		}
		else
		{
			ret = FFileHelper::SaveStringToFile(FileContents, *GetSaveGamePath(SaveName, false));
		}
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
	bool SaveStringLoaded = false;

	TArray<uint8> Data;

	auto LoadCompressedFileToString = [&](FString& Result, const TCHAR* Filename)
	{
		TArray<uint8> DataCompressed;
		if(!FFileHelper::LoadFileToArray(DataCompressed, Filename))
		{
			FLOGV("Fail to read save '%s'", Filename);
			return false;
		}

		int b4 = DataCompressed[DataCompressed.Num()- 4];
		int b3 = DataCompressed[DataCompressed.Num()- 3];
		int b2 = DataCompressed[DataCompressed.Num()- 2];
		int b1 = DataCompressed[DataCompressed.Num()- 1];
		int UncompressedSize = (b1 << 24) | (b2 << 16) + (b3 << 8) + b4;

		Data.SetNum(UncompressedSize + 1);

		if(!FCompression::UncompressMemory(NAME_Gzip, Data.GetData(), UncompressedSize, DataCompressed.GetData(), DataCompressed.Num(), ECompressionFlags::COMPRESS_NoFlags, 31))
		{
			FLOGV("Fail to uncompress save '%s' with compressed size %d and uncompressed size %d", Filename, DataCompressed.Num(), UncompressedSize);
			return false;
		}

		Data[UncompressedSize] = 0; // end string

		Result = UTF8_TO_TCHAR(Data.GetData());

		return true;
	};

	if(LoadCompressedFileToString(SaveString, *GetSaveGamePath(SaveName, true)))
	{
		FLOGV("Save '%s' read", *GetSaveGamePath(SaveName, true));
		SaveStringLoaded  = true;
	}
	else if(FFileHelper::LoadFileToString(SaveString, *GetSaveGamePath(SaveName, false)))
	{
		FLOGV("Save '%s' read", *GetSaveGamePath(SaveName, false));
		SaveStringLoaded = true;
	}

	if(SaveStringLoaded)
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
			FLOGV("Fail to deserialize save '%s' (len: %d)", *GetSaveGamePath(SaveName, false), SaveString.Len());
		}
	}
	else
	{
		FLOGV("Fail to read save '%s' or '%s'", *GetSaveGamePath(SaveName, true), *GetSaveGamePath(SaveName, false));

	}

	return SaveGame;
}

bool UFlareSaveGameSystem::DeleteGame(const FString SaveName)
{
	bool Result = IFileManager::Get().Delete(*GetSaveGamePath(SaveName, false), true) | IFileManager::Get().Delete(*GetSaveGamePath(SaveName, true), true);
	return Result;
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


FString UFlareSaveGameSystem::GetSaveGamePath(const FString SaveName, bool compressed)
{
	if(compressed)
	{
		return FString::Printf(TEXT("%s/SaveGames/%s.json.gz"), *FPaths::ProjectSavedDir(), *SaveName);
	}
	else
	{
		return FString::Printf(TEXT("%s/SaveGames/%s.json"), *FPaths::ProjectSavedDir(), *SaveName);
	}
}
