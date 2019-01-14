#include "WriteAchievementsListener.h"

#include "AchievementsInterfaceGOG.h"

#include "Online.h"

FWriteAchievementsListener::FWriteAchievementsListener(
	class FOnlineAchievementsGOG& InAchivementsInterface,
	const FUniqueNetIdGOG& InPlayerId,
	FOnlineAchievementsWriteRef& InWriteObject,
	const FOnAchievementsWrittenDelegate& InDelegate)
	: achivementsInterface{InAchivementsInterface}
	, playerId{InPlayerId}
	, achievementsWriteObject{InWriteObject}
	, achievementsWrittenDelegate{InDelegate}
{
}

void FWriteAchievementsListener::OnUserStatsAndAchievementsStoreSuccess()
{
	UE_LOG_ONLINE(Display, TEXT("OnUserStatsAndAchievementsStoreSuccess()"));

	TriggerOnAchievementsWrittenDelegate(true);
}

void FWriteAchievementsListener::OnUserStatsAndAchievementsStoreFailure(galaxy::api::IStatsAndAchievementsStoreListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("OnUserStatsAndAchievementsStoreFailure"));

	TriggerOnAchievementsWrittenDelegate(false);
}

void FWriteAchievementsListener::TriggerOnAchievementsWrittenDelegate(bool InResult)
{
	achievementsWriteObject->WriteState = InResult
		? EOnlineAsyncTaskState::Done
		: EOnlineAsyncTaskState::Failed;

	achievementsWrittenDelegate.ExecuteIfBound(playerId, InResult);
	achivementsInterface.FreeListener(MoveTemp(ListenerID));

	// Unlocked achivements are handled in FOnlineAchievementsGOG::OnAchievementUnlocked()
}