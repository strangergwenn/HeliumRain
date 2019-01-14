#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"

class FLobbyStartListener
	: public IListenerGOG
	, public galaxy::api::ILobbyDataUpdateListener
{
public:

	FLobbyStartListener(class FOnlineSessionGOG& InSessionInterface, galaxy::api::GalaxyID InLobbyID, FName InSessionName, bool InAllowJoinInProgress);

private:

	void OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason) override;

	bool MarkSessionStarted(bool IsJoinable) const;

	void TriggerOnStartSessionCompleteDelegates(bool InResult);

	class FOnlineSessionGOG& sessionInterface;
	const galaxy::api::GalaxyID lobbyID;
	const FName sessionName;
	bool allowJoinInProgress;
};
