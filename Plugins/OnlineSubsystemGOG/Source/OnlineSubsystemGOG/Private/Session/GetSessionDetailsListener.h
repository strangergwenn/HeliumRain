#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "ListenerManager.h"

#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

class FGetSessionDetailsListener
	: public IListenerGOG
	, public galaxy::api::ILobbyDataRetrieveListener
{
public:

	FGetSessionDetailsListener(
		FListenerManager& InListenerManager,
		FUniqueNetIdGOG InSessionID,
		FUniqueNetIdGOG InFriendID,
		FOnSingleSessionResultCompleteDelegate InCompletionDelegate);

private:

	void OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason) override;

	void TriggerOnSessionDetailsCompleteDelegate(FOnlineSessionSearchResult InOnlineSessionSearchResult = {});

	FListenerManager& listenerManager;
	FUniqueNetIdGOG sessionID;
	FUniqueNetIdGOG friendID;
	FOnSingleSessionResultCompleteDelegate completionDelegate;
};
