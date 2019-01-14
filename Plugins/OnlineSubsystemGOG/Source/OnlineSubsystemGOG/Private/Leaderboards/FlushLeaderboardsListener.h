#pragma once

#include "Types/IListenerGOG.h"
#include "CachedLeaderboardsDetails.h"

#include "OnlineStats.h"

class FFlushLeaderboardsListener
	: public IListenerGOG
	, public galaxy::api::ILeaderboardRetrieveListener
	, private galaxy::api::ILeaderboardScoreUpdateListener
{
PACKAGE_SCOPE:

	FFlushLeaderboardsListener(
		class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
		FName InSessionName,
		TMap<const FName,
		CachedLeaderboardDetails> InLeaderboardsDetails);

private:

	void OnLeaderboardRetrieveSuccess(const char* InName) override;

	void OnLeaderboardRetrieveFailure(const char* InName, galaxy::api::ILeaderboardRetrieveListener::FailureReason InFailureReason) override;

	void OnLeaderboardScoreUpdateSuccess(const char* InName, int32_t InScore, uint32_t InOldRank, uint32_t InNewRank) override;

	void OnLeaderboardScoreUpdateFailure(const char* InName, int32_t InScore, galaxy::api::ILeaderboardScoreUpdateListener::FailureReason InFailureReason) override;

	void TriggerOnLeaderboardFlushComplete(bool InResult);

private:

	class FOnlineLeaderboardsGOG& leaderboardsInterface;
	const FName sessionName;
	TMap<const FName, CachedLeaderboardDetails> leaderboardsDetails;
};
