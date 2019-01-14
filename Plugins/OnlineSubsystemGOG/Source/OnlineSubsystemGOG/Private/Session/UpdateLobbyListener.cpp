#include "UpdateLobbyListener.h"

#include "OnlineSessionGOG.h"

#include "Online.h"

FUpdateLobbyListener::FUpdateLobbyListener(class FOnlineSessionGOG& InSessionInterface, FName InSessionName)
	: sessionInterface{InSessionInterface}
	, sessionName{MoveTemp(InSessionName)}
{
}

void FUpdateLobbyListener::OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("FUpdateLobbyListener::OnLobbyDataUpdateSuccess: lobbyID=%llu"), InLobbyID.ToUint64());

	TriggerOnUpdateSessionCompleteDelegates(true);
}

void FUpdateLobbyListener::OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Warning, TEXT("FUpdateLobbyListener::OnLobbyDataUpdateFailure: lobbyID=%llu"), InLobbyID.ToUint64());

	switch (InFailureReason)
	{
		case galaxy::api::ILobbyDataUpdateListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST:
		{
			UE_LOG_ONLINE(Warning, TEXT("Specified lobby does not exists anymore: lobbyID=%llu"), InLobbyID.ToUint64());
			break;
		}
		case galaxy::api::ILobbyDataUpdateListener::FAILURE_REASON_UNDEFINED:
		default:
			UE_LOG_ONLINE(Warning, TEXT("Unknown faliure during lobby data update: lobbyID=%llu"), InLobbyID.ToUint64());
			break;
	}

	TriggerOnUpdateSessionCompleteDelegates(false);
}

void FUpdateLobbyListener::TriggerOnUpdateSessionCompleteDelegates(bool InWasSuccessful)
{
	sessionInterface.TriggerOnUpdateSessionCompleteDelegates(MoveTemp(sessionName), InWasSuccessful);

	sessionInterface.FreeListener(MoveTemp(ListenerID));
}