#include "GameInvitationReceivedListener.h"

#include "OnlineSessionGOG.h"
#include "OnlineSessionUtils.h"

FGameInvitationReceivedListener::FGameInvitationReceivedListener(
	class FOnlineSessionGOG& InSessionInterface,
	FString InAppID,
	FUniqueNetIdGOG InOwnUserID,
	FUniqueNetIdGOG InInviterUserID,
	FUniqueNetIdGOG InSessionID)
	: sessionInterface{InSessionInterface}
	, appID{InAppID}
	, ownUserID{InOwnUserID}
	, inviterUserID{MoveTemp(InInviterUserID)}
	, sessionID{MoveTemp(InSessionID)}
{
}

void FGameInvitationReceivedListener::OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("FGameInvitationReceivedListener::OnLobbyDataRetrieveSuccess: lobbyID=%llu"), InLobbyID.ToUint64());

	FOnlineSessionSearchResult invitedSession;
	if (!OnlineSessionUtils::Fill(InLobbyID, invitedSession))
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to fetch important information for incoming game invitation: lobbyID=%llu"), InLobbyID.ToUint64());
		Finalize();
		return;
	}

	sessionInterface.TriggerOnSessionInviteReceivedDelegates(ownUserID, inviterUserID, appID, invitedSession);
	Finalize();
}

void FGameInvitationReceivedListener::OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Warning, TEXT("FGameInvitationReceivedListener::OnLobbyDataRetrieveFailure: lobbyID=%llu"), InLobbyID.ToUint64());

	switch (InFailureReason)
	{
		case galaxy::api::ILobbyDataRetrieveListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST:
		{
			UE_LOG_ONLINE(Warning, TEXT("Receieved game invitation for session that is does not exist anymore: lobbyID=%llu"), InLobbyID.ToUint64());
			break;
		}

		default:
			UE_LOG_ONLINE(Warning, TEXT("Undefined error"));
			break;
	}

	Finalize();
}

void FGameInvitationReceivedListener::Finalize()
{
	sessionInterface.FreeListener(MoveTemp(ListenerID));
}
