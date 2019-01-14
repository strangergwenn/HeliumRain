#pragma once

#include "JsonObject.h"

namespace JsonConverter
{

	FString ToJsonString(const TSharedRef<FJsonObject>& InJsonObject);

	TSharedPtr<FJsonObject> FromJsonString(const FString& InJsonString);

}