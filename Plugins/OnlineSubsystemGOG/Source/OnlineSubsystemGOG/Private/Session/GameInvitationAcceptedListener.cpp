#include "GameInvitationAcceptedListener.h"

#include "OnlineSessionGOG.h"
#include "OnlineSessionUtils.h"

FGameInvitationAcceptedListener::FGameInvitationAcceptedListener(
	FOnlineSessionGOG& InSessionInterface,
	FString InAppID,
	TSharedRef<const FUniqueNetId> InOwnUserID,
	FUniqueNetIdGOG InInviterUserID, FUniqueNetIdGOG InSessionID)
	: sessionInterface{InSessionInterface}
	, appID{InAppID}
	, ownUserID{InOwnUserID}
	, inviterUserID{InInviterUserID}
	, sessionID{InSessionID}
{
}

void FGameInvitationAcceptedListener::OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("FGameInvitationAcceptedListener::OnLobbyDataRetrieveSuccess: lobbyID=%llu"), InLobbyID.ToUint64());

	FOnlineSessionSearchResult invitedSession;
	if (!OnlineSessionUtils::Fill(InLobbyID, invitedSession) || !invitedSession.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to fetch important information for incoming game invitation: lobbyID=%llu"), InLobbyID.ToUint64());
		TriggerOnSessionUserInviteAcceptedDelegates(false);
		return;
	}

	TriggerOnSessionUserInviteAcceptedDelegates(true, MoveTemp(invitedSession));
}

void FGameInvitationAcceptedListener::OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Warning, TEXT("FGameInvitationAcceptedListener::OnLobbyDataRetrieveFailure: lobbyID=%llu"), InLobbyID.ToUint64());

	switch (InFailureReason)
	{
		case galaxy::api::ILobbyDataRetrieveListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST:
		{
			UE_LOG_ONLINE(Warning, TEXT("Receieved game invitation for session that is does not exist anymore: lobbyID=%llu"), InLobbyID.ToUint64());
			break;
		}

		default:
			UE_LOG_ONLINE(Warning, TEXT("Undefined error"));
	}

	TriggerOnSessionUserInviteAcceptedDelegates(false);
}

void FGameInvitationAcceptedListener::TriggerOnSessionUserInviteAcceptedDelegates(bool InWasSuccessful, FOnlineSessionSearchResult InInvitedSession)
{
	sessionInterface.TriggerOnSessionUserInviteAcceptedDelegates(InWasSuccessful, LOCAL_USER_NUM, ownUserID, InInvitedSession);
	sessionInterface.FreeListener(MoveTemp(ListenerID));
}
