#include "QueryAchievementsListener.h"

#include "AchievementsInterfaceGOG.h"

#include "Online.h"

FQueryAchievementsListener::FQueryAchievementsListener(
	FOnlineAchievementsGOG& InAchivementsInterface,
	const FUniqueNetIdGOG& InPlayerId,
	const FOnQueryAchievementsCompleteDelegate& InDelegate)
	: achivementsInterface{InAchivementsInterface}
	, playerId{InPlayerId}
	, queryAchievementsCompleteDelegate{InDelegate}
{
}

void FQueryAchievementsListener::OnUserStatsAndAchievementsRetrieveSuccess(galaxy::api::GalaxyID InUserID)
{
	UE_LOG_ONLINE(Display, TEXT("OnUserStatsAndAchievementsRetrieveSuccess: userID=%llu"), InUserID.ToUint64());

	check(InUserID == playerId && "Achievements retrieved for unknown user. This shall not be happening with Galaxy specific listeners");

	achivementsInterface.OnAchievementsRetrieved(InUserID);

	TriggerOnQueryAchievementsCompleteDelegate(true);
}

void FQueryAchievementsListener::OnUserStatsAndAchievementsRetrieveFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IUserStatsAndAchievementsRetrieveListener::FailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("OnUserStatsAndAchievementsRetrieveFailure: userID=%llu"), InUserID.ToUint64());

	check(InUserID == playerId && "Achievements retrieved for unknown user. This shall not be happening with Galaxy specific listeners");

	TriggerOnQueryAchievementsCompleteDelegate(false);
}

void FQueryAchievementsListener::TriggerOnQueryAchievementsCompleteDelegate(bool InResult)
{
	queryAchievementsCompleteDelegate.ExecuteIfBound(playerId, InResult);

	achivementsInterface.FreeListener(MoveTemp(ListenerID));
}
