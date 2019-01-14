#pragma once

#include "LeaderboardRetriever.h"

#include "OnlineStats.h"

class FReadLeaderboardAroundRankListener
	: public FLeaderboardRetriever
{
PACKAGE_SCOPE:

	FReadLeaderboardAroundRankListener(class FOnlineLeaderboardsGOG& InLeaderboardsInterface, int32 InRank, uint32 InRange, FOnlineLeaderboardReadRef InOutReadLeaderboard);

	void RequestLeaderboardEntries() override;

private:

	int32 rank;
	uint32 range;
};
