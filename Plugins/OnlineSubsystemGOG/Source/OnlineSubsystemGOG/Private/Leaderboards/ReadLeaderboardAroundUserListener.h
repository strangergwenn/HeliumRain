#pragma once

#include "LeaderboardRetriever.h"
#include "Types/UniqueNetIdGOG.h"

#include "OnlineStats.h"

class FReadLeaderboardAroundUserListener
	: public FLeaderboardRetriever
{
PACKAGE_SCOPE:

	FReadLeaderboardAroundUserListener(
		class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
		TSharedRef<const FUniqueNetIdGOG> InPlayer,
		uint32 InRange,
		FOnlineLeaderboardReadRef& InOutReadLeaderboard);

	void RequestLeaderboardEntries() override;

private:

	TSharedRef<const FUniqueNetIdGOG> player;
	const uint32 range;
};
