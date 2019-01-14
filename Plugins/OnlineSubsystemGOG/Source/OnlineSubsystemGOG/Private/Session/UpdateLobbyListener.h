#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

class FUpdateLobbyListener
	: public IListenerGOG
	, public galaxy::api::ILobbyDataUpdateListener
{
public:

	FUpdateLobbyListener(class FOnlineSessionGOG& InSessionInterface, FName InSessionName);

private:

	void OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason) override;

	void TriggerOnUpdateSessionCompleteDelegates(bool InWasSuccessful);

	class FOnlineSessionGOG& sessionInterface;
	FName sessionName;
};
