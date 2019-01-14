#pragma once

#include "CommonGOG.h"
#include "Types/IListenerGOG.h"

#include "OnlineStats.h"

class FLeaderboardRetriever
	: public IListenerGOG
	, public galaxy::api::ILeaderboardRetrieveListener
	, protected galaxy::api::ILeaderboardEntriesRetrieveListener
{
PACKAGE_SCOPE:

	FLeaderboardRetriever(class FOnlineLeaderboardsGOG& InLeaderboardsInterface, FOnlineLeaderboardReadRef InInOutReadLeaderboard);

protected:

	virtual void RequestLeaderboardEntries() = 0;

	void OnLeaderboardRetrieveSuccess(const char* InName) override;

	void OnLeaderboardRetrieveFailure(const char* InName, galaxy::api::ILeaderboardRetrieveListener::FailureReason InFailureReason) override;

	void OnLeaderboardEntriesRetrieveSuccess(const char* InName, uint32_t InEntryCount) override;

	void OnLeaderboardEntriesRetrieveFailure(const char* InName, galaxy::api::ILeaderboardEntriesRetrieveListener::FailureReason InFailureReason) override;

	void TriggerOnLeaderboardReadCompleteDelegates(bool InResult);

	class FOnlineLeaderboardsGOG& leaderboardsInterface;
	FOnlineLeaderboardReadRef readLeaderboard;
};