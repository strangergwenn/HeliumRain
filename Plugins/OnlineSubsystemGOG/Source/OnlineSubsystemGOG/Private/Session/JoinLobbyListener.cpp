#include "JoinLobbyListener.h"

#include "OnlineSessionGOG.h"
#include "LobbyData.h"

#include "Online.h"

FJoinLobbyListener::FJoinLobbyListener(class FOnlineSessionGOG& InSessionInterface, FName InSessionName, FOnlineSession InJoiningSession)
	: sessionInterface{InSessionInterface}
	, sessionName{MoveTemp(InSessionName)}
	, joiningSession{MoveTemp(InJoiningSession)}
{
}

void FJoinLobbyListener::TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::Type InResult)
{
	// Save local copy of the session
	if(InResult == EOnJoinSessionCompleteResult::Success)
		sessionInterface.AddNamedSession(sessionName, joiningSession);

	sessionInterface.TriggerOnJoinSessionCompleteDelegates(sessionName, InResult);

	sessionInterface.FreeListener(MoveTemp(ListenerID));
}

void FJoinLobbyListener::OnLobbyEntered(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyEnterResult InResult)
{
	UE_LOG_ONLINE(Display, TEXT("OnLobbyEntered: lobbyID=%llu"), InLobbyID.ToUint64());

	if (InResult != galaxy::api::LOBBY_ENTER_RESULT_SUCCESS)
	{
		switch (InResult)
		{
			case galaxy::api::LOBBY_ENTER_RESULT_LOBBY_DOES_NOT_EXIST:
			{
				UE_LOG_ONLINE(Error, TEXT("Lobby does not exists: lobbyID=%llu"), InLobbyID.ToUint64());

				TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::SessionDoesNotExist);
				return;
			}

			case galaxy::api::LOBBY_ENTER_RESULT_LOBBY_IS_FULL:
			{
				UE_LOG_ONLINE(Display, TEXT("Lobby is full: lobbyID=%llu"), InLobbyID.ToUint64());

				TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::SessionIsFull);
				return;
			}


			case galaxy::api::LOBBY_ENTER_RESULT_ERROR:
			{
				UE_LOG_ONLINE(Error, TEXT("Unknown error when entering lobby: lobbyID=%llu"), InLobbyID.ToUint64());

				TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::UnknownError);
				return;
			}
		}
	}

	TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::Success);
}
