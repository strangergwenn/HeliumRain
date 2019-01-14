#pragma once

#include "OnlineStats.h"

namespace OnlineLeaderboardConverter
{

	FString ToJsonString(const FStatPropertyArray& InStasMap);

	FStatsColumnArray FromJsonString(const FString& JsonString);

}
