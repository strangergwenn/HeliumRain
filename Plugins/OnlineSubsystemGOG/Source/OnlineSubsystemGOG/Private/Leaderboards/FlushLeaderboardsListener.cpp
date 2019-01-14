#include "FlushLeaderboardsListener.h"
#include "Converters/OnlineLeaderboardConverter.h"
#include "OnlineLeaderboardsGOG.h"

#include "OnlineSubsystemUtils.h"

FFlushLeaderboardsListener::FFlushLeaderboardsListener(
	class FOnlineLeaderboardsGOG& InLeaderboardsInterface,
	FName InSessionName,
	TMap<const FName, CachedLeaderboardDetails> InLeaderboardsDetails)
	: leaderboardsInterface{InLeaderboardsInterface}
	, sessionName{MoveTemp(InSessionName)}
	, leaderboardsDetails{MoveTemp(InLeaderboardsDetails)}
{
}

void FFlushLeaderboardsListener::OnLeaderboardRetrieveSuccess(const char* InName)
{
	UE_LOG_ONLINE(Display, TEXT("OnLeaderboardRetrieveSuccess: leaderboardName='%s'"), UTF8_TO_TCHAR(InName));

	auto& leaderboardDetails = leaderboardsDetails[UTF8_TO_TCHAR(InName)];

	galaxy::api::Stats()->SetLeaderboardScoreWithDetails(
		InName,
		leaderboardDetails.MainScore,
		TCHAR_TO_UTF8(*leaderboardDetails.SerializedDetails),
		CharLen(leaderboardDetails.SerializedDetails),
		leaderboardDetails.UpdateMethod == ELeaderboardUpdateMethod::Force,
		this);

	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to update score: sessionName='%s', leaderboardName='%s', newScore=%d, detailsSize=%d, updateMethods='%s'; %s; %s"),
			*sessionName.ToString(),
			UTF8_TO_TCHAR(InName),
			leaderboardDetails.MainScore,
			CharLen(leaderboardDetails.SerializedDetails),
			ELeaderboardUpdateMethod::ToString(leaderboardDetails.UpdateMethod),
			UTF8_TO_TCHAR(err->GetName()),
			UTF8_TO_TCHAR(err->GetMsg()));

		TriggerOnLeaderboardFlushComplete(false);
		return;
	}

	// Wait for LeaderboardScoreUpdate
}

void FFlushLeaderboardsListener::OnLeaderboardRetrieveFailure(const char* InName, galaxy::api::ILeaderboardRetrieveListener::FailureReason)
{
	UE_LOG_ONLINE(Error, TEXT("OnLeaderboardRetrieveFailure: sessionName='%s', leaderboardName='%s'"), *sessionName.ToString(), UTF8_TO_TCHAR(InName));

	TriggerOnLeaderboardFlushComplete(false);
}

void FFlushLeaderboardsListener::OnLeaderboardScoreUpdateSuccess(const char* InName, int32_t, uint32_t, uint32_t)
{
	UE_LOG_ONLINE(Display, TEXT("OnLeaderboardScoreUpdateSuccess: sessionName='%s', leaderboardName='%s'"), *sessionName.ToString(), UTF8_TO_TCHAR(InName));

	if (!leaderboardsDetails.Remove(UTF8_TO_TCHAR(InName)))
	{
		UE_LOG_ONLINE(Error, TEXT("Updated scores for unknown leaderboard"));
		check(false && "Updated scores for unknown leaderboard. This shall never happen");
		TriggerOnLeaderboardFlushComplete(false);
	}

	if (leaderboardsDetails.Num() == 0)
		TriggerOnLeaderboardFlushComplete(true);
}

void FFlushLeaderboardsListener::OnLeaderboardScoreUpdateFailure(const char* InName, int32_t InScore, galaxy::api::ILeaderboardScoreUpdateListener::FailureReason InFailureReason)
{
	if (InFailureReason == galaxy::api::ILeaderboardScoreUpdateListener::FAILURE_REASON_NO_IMPROVEMENT)
	{
		UE_LOG_ONLINE(Display, TEXT("OnLeaderboardScoreUpdate: no improvement in score update: sessionName='%s', leaderboardName='%s'"), *sessionName.ToString(), UTF8_TO_TCHAR(InName));
		OnLeaderboardScoreUpdateSuccess(InName, InScore, -1, -1);
		return;
	}

	UE_LOG_ONLINE(Error, TEXT("OnLeaderboardScoreUpdateFailure: sessionName='%s', leaderboardName='%s'"), *sessionName.ToString(), UTF8_TO_TCHAR(InName));
	TriggerOnLeaderboardFlushComplete(false);
}

void FFlushLeaderboardsListener::TriggerOnLeaderboardFlushComplete(bool InResult)
{
	if (InResult)
		// TBD: should we keep it in case of failure?
		leaderboardsInterface.RemoveCachedLeaderboard(sessionName);

	leaderboardsInterface.TriggerOnLeaderboardFlushCompleteDelegates(sessionName, InResult);

	leaderboardsInterface.FreeListener(MoveTemp(ListenerID));
}
