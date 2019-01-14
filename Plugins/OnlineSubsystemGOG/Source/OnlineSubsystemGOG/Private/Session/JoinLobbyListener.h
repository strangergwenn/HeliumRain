#pragma once

#include "Types/IListenerGOG.h"

#include "Types/UniqueNetIdGOG.h"

#include "OnlineSessionInterface.h"

#include "OnlineSessionSettings.h"

class FJoinLobbyListener
	: public IListenerGOG
	, public galaxy::api::ILobbyEnteredListener
{
public:

	FJoinLobbyListener(class FOnlineSessionGOG& InSessionInterface, FName InSessionName, FOnlineSession InJoiningSession);

private:

	void OnLobbyEntered(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyEnterResult InResult) override;

	void TriggerOnJoinSessionCompleteDelegates(EOnJoinSessionCompleteResult::Type InResult);

	class FOnlineSessionGOG& sessionInterface;
	const FName sessionName;
	const FOnlineSession joiningSession;
};
