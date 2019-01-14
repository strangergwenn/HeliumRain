#include "OnlineLeaderboardConverter.h"

#include "JsonConverter.h"
#include "OnlineSubsystem.h"

#define TEXT_JSON_LEADERBOARD_NAME TEXT("Extra")
#define TEXT_LEADERBOARD_COLUMN_NAME TEXT("Name")

namespace OnlineLeaderboardConverter
{

	using JsonLeaderboardMap = TArray<TSharedPtr<FJsonValue>>;

	FString ToJsonString(const FStatPropertyArray& InLeaderboard)
	{
		JsonLeaderboardMap jsonLeaderboard;
		for (const auto& leaderboardRow : InLeaderboard)
		{
			auto jsonLeaderboardRow = leaderboardRow.Value.ToJson();
			jsonLeaderboardRow->SetStringField(TEXT_LEADERBOARD_COLUMN_NAME, leaderboardRow.Key.ToString());
			jsonLeaderboard.Add(MakeShared<FJsonValueObject>(jsonLeaderboardRow));
		}

		auto resultObject = MakeShared<FJsonObject>();
		resultObject->SetArrayField(TEXT_JSON_LEADERBOARD_NAME, jsonLeaderboard);

		return JsonConverter::ToJsonString(resultObject);
	}

	FStatsColumnArray FromJsonString(const FString& InJsonStr)
	{
		if (InJsonStr.IsEmpty())
		{
			UE_LOG_ONLINE(VeryVerbose, TEXT("Deserializing empty json object"));
			return{};
		}

		auto leaderboardJsonObject = JsonConverter::FromJsonString(InJsonStr);
		if (!leaderboardJsonObject.IsValid() || !leaderboardJsonObject->HasTypedField<EJson::Array>(TEXT_JSON_LEADERBOARD_NAME))
		{
			UE_LOG_ONLINE(Error, TEXT("Leaderboard object '%s' not found in json"), TEXT_JSON_LEADERBOARD_NAME);
			return{};
		}

		FStatsColumnArray convertedLeaderboard;
		for (auto leaderboardStatJson : leaderboardJsonObject->GetArrayField(TEXT_JSON_LEADERBOARD_NAME))
		{
			auto leaderboardRowObject = leaderboardStatJson->AsObject();
			if (!leaderboardRowObject.IsValid())
			{
				UE_LOG_ONLINE(Error, TEXT("Failed to get leaderboard. Corrupted json"));
				return{};
			}

			FString leaderboardStatName;
			if (!leaderboardRowObject->TryGetStringField(TEXT_LEADERBOARD_COLUMN_NAME, leaderboardStatName)
				|| leaderboardStatName.IsEmpty())
			{
				UE_LOG_ONLINE(Error, TEXT("Failed to get leaderboard stat name. Corrupted json"));
				return{};
			}

			FVariantData leaderboardEntryValue;
			if (!leaderboardEntryValue.FromJson(leaderboardRowObject.ToSharedRef()))
			{
				UE_LOG_ONLINE(Error, TEXT("Failed to get leaderboard stat value. Corrupted json: statName='%s'"), *leaderboardStatName);
				return{};
			}

			convertedLeaderboard.Emplace(*leaderboardStatName, MoveTemp(leaderboardEntryValue));
		}

		return convertedLeaderboard;
	}
}
