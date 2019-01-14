#pragma once

#include "LeaderboardRetriever.h"

#include "OnlineStats.h"

class FReadLeaderboardForUsersListener
	: public FLeaderboardRetriever
{
PACKAGE_SCOPE:

	FReadLeaderboardForUsersListener(
		class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
		TArray<TSharedRef<const FUniqueNetId>> InPlayers,
		FOnlineLeaderboardReadRef InInOutReadLeaderboard);

	void RequestLeaderboardEntries() override;

private:

	const TArray<TSharedRef<const FUniqueNetId>> players;
};
