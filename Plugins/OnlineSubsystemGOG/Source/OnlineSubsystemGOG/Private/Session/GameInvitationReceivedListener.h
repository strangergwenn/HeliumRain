#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"

#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

class FGameInvitationReceivedListener
	: public IListenerGOG
	, public galaxy::api::ILobbyDataRetrieveListener
{
public:

	FGameInvitationReceivedListener(
		class FOnlineSessionGOG& InSessionInterface,
		FString InAppID,
		FUniqueNetIdGOG InOwnUserID,
		FUniqueNetIdGOG InInviterUserID,
		FUniqueNetIdGOG InSessionID);

private:

	void OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason) override;

	void Finalize();

	class FOnlineSessionGOG& sessionInterface;
	FString appID;
	FUniqueNetIdGOG ownUserID;
	FUniqueNetIdGOG inviterUserID;
	FUniqueNetIdGOG sessionID;
	FString sessionName;
};
