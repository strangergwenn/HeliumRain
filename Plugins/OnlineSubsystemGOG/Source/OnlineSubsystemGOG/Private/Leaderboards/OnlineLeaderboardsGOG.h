#pragma once

#include "CommonGOG.h"
#include "CachedLeaderboardsDetails.h"
#include "Types/IListenerGOG.h"
#include "ListenerManager.h"

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineStats.h"

class FOnlineLeaderboardsGOG
	: public IOnlineLeaderboards
	, public FListenerManager
{
public:

	bool ReadLeaderboards(const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, FOnlineLeaderboardReadRef& InOutReadLeaderboard) override;

	bool ReadLeaderboardsForFriends(int32 InLocalUserNum, FOnlineLeaderboardReadRef& InInOutReadLeaderboard) override;

	bool ReadLeaderboardsAroundRank(int32 InRank, uint32 InRange, FOnlineLeaderboardReadRef& InInOutReadLeaderboard) override;

	bool ReadLeaderboardsAroundUser(TSharedRef<const FUniqueNetId> InPlayerID, uint32 InRange, FOnlineLeaderboardReadRef& InInOutReadLeaderboard) override;

	void FreeStats(FOnlineLeaderboardRead& InInOutReadLeaderboard) override;

	bool WriteLeaderboards(const FName& InSessionName, const FUniqueNetId& InPlayer, FOnlineLeaderboardWrite& InWriteLeaderboard) override;

	bool FlushLeaderboards(const FName& InSessionName) override;

	bool WriteOnlinePlayerRatings(const FName& InSessionName, int32 leaderboardId, const TArray<FOnlinePlayerScore>& InPlayerScores) override;

PACKAGE_SCOPE:

	FOnlineLeaderboardsGOG(const class FOnlineSubsystemGOG& InOnlineSubsystemGOG, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount);

	void RemoveCachedLeaderboard(FName InSessionName);

private:

	bool MarkLeaderboardStarted(FOnlineLeaderboardReadRef& InOutReadLeaderboard, IOnlineLeaderboards* onlineLeaderboardsInterface) const;

	bool UpdateWriteCache(const FName& InSessionName, FOnlineLeaderboardWrite &InWriteLeaderboard);

	TSet<TUniquePtr<IListenerGOG>> listenerRegistry;

	using SessionName = FName;
	using LeaderboardName = FName;
	TMap<const SessionName, TMap<const LeaderboardName, CachedLeaderboardDetails>> writeLeaderboardCache;

	const class FOnlineSubsystemGOG& onlineSubsystemGOG;
	TSharedRef<class FUserOnlineAccountGOG> ownUserOnlineAccount;
};
