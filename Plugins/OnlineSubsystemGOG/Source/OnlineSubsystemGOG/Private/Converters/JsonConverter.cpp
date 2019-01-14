#include "JsonConverter.h"

#include "JsonSerializer.h"
#include "OnlineSubsystem.h"

namespace JsonConverter
{

	FString ToJsonString(const TSharedRef<FJsonObject>& InJsonObject)
	{
		FString serializedJsonStr;
		auto JsonWriter = TJsonWriterFactory<>::Create(&serializedJsonStr);
		if (!FJsonSerializer::Serialize(InJsonObject, JsonWriter, true))
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to serialize Json object to FString"));
			return{};
		}

		return serializedJsonStr;
	}

	TSharedPtr<FJsonObject> FromJsonString(const FString& InJsonString)
	{
		if (InJsonString.IsEmpty())
		{
			UE_LOG_ONLINE(VeryVerbose, TEXT("De-serializing empty object"));
			return{};
		}

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InJsonString);
		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to serialize Json object to FString"));
		}

		return JsonObject;
	}

}