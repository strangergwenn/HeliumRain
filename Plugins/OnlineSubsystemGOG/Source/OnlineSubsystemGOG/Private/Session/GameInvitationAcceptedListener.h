#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"

#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

class FGameInvitationAcceptedListener
	: public IListenerGOG
	, public galaxy::api::ILobbyDataRetrieveListener
{
public:

	FGameInvitationAcceptedListener(
		class FOnlineSessionGOG& InSessionInterface,
		FString InAppID,
		TSharedRef<const FUniqueNetId> InOwnUserID,
		FUniqueNetIdGOG InInviterUserID,
		FUniqueNetIdGOG InSessionID);

private:

	void OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason) override;

	void TriggerOnSessionUserInviteAcceptedDelegates(bool InWasSuccessful, FOnlineSessionSearchResult InInvitedSession = {});

	class FOnlineSessionGOG& sessionInterface;
	FString appID;
	TSharedRef<const FUniqueNetId> ownUserID;
	FUniqueNetIdGOG inviterUserID;
	FUniqueNetIdGOG sessionID;
	FString sessionName;
};
