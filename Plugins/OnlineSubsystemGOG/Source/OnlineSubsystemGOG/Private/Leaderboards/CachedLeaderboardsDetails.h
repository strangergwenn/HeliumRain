#pragma once

#include "OnlineSubsystemTypes.h"

struct CachedLeaderboardDetails
{
	int32 MainScore;
	FString SerializedDetails;
	ELeaderboardUpdateMethod::Type UpdateMethod;

	ELeaderboardSort::Type SortMethod;
	ELeaderboardFormat::Type DisplayFormat;
};
