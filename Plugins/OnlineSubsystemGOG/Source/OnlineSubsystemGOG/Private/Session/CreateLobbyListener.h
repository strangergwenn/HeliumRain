#pragma once

#include "Types/IListenerGOG.h"

#include "OnlineSubsystem.h"

#include "OnlineSessionSettings.h"

class FCreateLobbyListener
	: public IListenerGOG
	, public galaxy::api::ILobbyCreatedListener
	, public galaxy::api::ILobbyEnteredListener
	, public galaxy::api::ILobbyDataUpdateListener
	, public galaxy::api::IRichPresenceChangeListener
{
public:

	FCreateLobbyListener(
		class FOnlineSessionGOG& InSessionInterface,
		FName InSessionName,
		TSharedRef<const FUniqueNetId> InSessionOwnerID,
		FString InSessionOwnerName,
		FOnlineSessionSettings InSettings);

private:

	void OnLobbyCreated(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyCreateResult InResult) override;

	void OnLobbyEntered(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyEnterResult InResult)  override;

	void OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID) override;

	void OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason) override;

	bool AdvertiseToFriends();

	void OnRichPresenceChangeSuccess() override;

	void OnRichPresenceChangeFailure(galaxy::api::IRichPresenceChangeListener::FailureReason InFailureReason) override;

	void TriggerOnCreateSessionCompleteDelegates(bool InIsSuccessful);

	class FOnlineSessionGOG& sessionInterface;
	const FName sessionName;
	FOnlineSessionSettings sessionSettings;
	TSharedRef<const FUniqueNetId> sessionOwnerID;
	FString sessionOwnerName;

	galaxy::api::GalaxyID newLobbyID;
};
