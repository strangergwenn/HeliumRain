#include "ReadLeaderboardForUsersListener.h"

#include "Types/UniqueNetIdGOG.h"

FReadLeaderboardForUsersListener::FReadLeaderboardForUsersListener(
	class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
	TArray<TSharedRef<const FUniqueNetId>> InPlayers,
	FOnlineLeaderboardReadRef InOutReadLeaderboard)
	: FLeaderboardRetriever{InLeaderboardsInterface, MoveTemp(InOutReadLeaderboard)}
	, players{MoveTemp(InPlayers)}
{
}

void FReadLeaderboardForUsersListener::RequestLeaderboardEntries()
{
	TArray<galaxy::api::GalaxyID> galaxyUsers;
	for (const auto& player : players)
		galaxyUsers.Emplace(FUniqueNetIdGOG{*player});

	galaxy::api::Stats()->RequestLeaderboardEntriesForUsers(TCHAR_TO_UTF8(*readLeaderboard->LeaderboardName.ToString()), galaxyUsers.GetData(), galaxyUsers.Num(), this);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to request leaderboard entries for users: leaderboardName='%s', playerCount='%d'; %s; %s"),
			*readLeaderboard->LeaderboardName.ToString(), players.Num(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		TriggerOnLeaderboardReadCompleteDelegates(false);
		return;
	}
}
