#include "ReadLeaderboardAroundUserListener.h"

FReadLeaderboardAroundUserListener::FReadLeaderboardAroundUserListener(
	class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
	TSharedRef<const FUniqueNetIdGOG> InPlayer,
	uint32 InRange,
	FOnlineLeaderboardReadRef& InOutReadLeaderboard)
	: FLeaderboardRetriever{InLeaderboardsInterface, MoveTemp(InOutReadLeaderboard)}
	, player{InPlayer}
	, range{InRange}
{
}

void FReadLeaderboardAroundUserListener::RequestLeaderboardEntries()
{
	galaxy::api::Stats()->RequestLeaderboardEntriesAroundUser(TCHAR_TO_UTF8(*readLeaderboard->LeaderboardName.ToString()), range, range, *player, this);

	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to request leaderboard entries around user: leaderboardName='%s', playerID='%s', range=%u; %s; %s"),
			*readLeaderboard->LeaderboardName.ToString(), *player->ToString(), range, UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		TriggerOnLeaderboardReadCompleteDelegates(false);
		return;
	}
}
