#include "LobbyStartListener.h"

#include "OnlineSessionGOG.h"

#include "Online.h"

FLobbyStartListener::FLobbyStartListener(class FOnlineSessionGOG& InSessionInterface, galaxy::api::GalaxyID InLobbyID, FName InSessionName, bool InAllowJoinInProgress)
	: sessionInterface{InSessionInterface}
	, lobbyID{MoveTemp(InLobbyID)}
	, sessionName{MoveTemp(InSessionName)}
	, allowJoinInProgress{InAllowJoinInProgress}
{
}

void FLobbyStartListener::OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("OnLobbyDataUpdated: lobbyID=%llu"), InLobbyID.ToUint64());

	checkf(lobbyID == InLobbyID, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	auto isLobbyJoinable = galaxy::api::Matchmaking()->IsLobbyJoinable(InLobbyID);
	auto err = galaxy::api::GetError();
	if (err)
		UE_LOG_ONLINE(Error, TEXT("Failed to check lobby joinability: %s, %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

	if (isLobbyJoinable != allowJoinInProgress)
		UE_LOG_ONLINE(Error, TEXT("Failed to set Lobby as %s"), allowJoinInProgress ? TEXT("joinable") : TEXT("non-joinable"));

	TriggerOnStartSessionCompleteDelegates(isLobbyJoinable == allowJoinInProgress);
}

void FLobbyStartListener::OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("FLobbyStartListener::OnLobbyDataUpdateFailure: lobbyID=%llu"), InLobbyID.ToUint64());

	checkf(lobbyID == InLobbyID, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	UE_LOG_ONLINE(Error, InFailureReason == galaxy::api::ILobbyDataUpdateListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST
		? TEXT("Specified lobby does not exists")
		: TEXT("Unknown error"));

	TriggerOnStartSessionCompleteDelegates(false);
}

bool FLobbyStartListener::MarkSessionStarted(bool IsJoinable) const
{
	auto storedSession = sessionInterface.GetNamedSession(sessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to finalize session stating as OnlineSession interface is invalid: sessionName='%s'"), *sessionName.ToString());
		return false;
	}

	storedSession->SessionState = IsJoinable ? EOnlineSessionState::InProgress : EOnlineSessionState::Pending;
	return IsJoinable;
}

void FLobbyStartListener::TriggerOnStartSessionCompleteDelegates(bool InResult)
{
	sessionInterface.TriggerOnStartSessionCompleteDelegates(sessionName, MarkSessionStarted(InResult));
	sessionInterface.FreeListener(MoveTemp(ListenerID));
}
