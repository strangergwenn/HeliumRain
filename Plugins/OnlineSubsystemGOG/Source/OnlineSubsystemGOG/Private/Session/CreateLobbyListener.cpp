#include "CreateLobbyListener.h"

#include "LobbyData.h"
#include "OnlineSessionUtils.h"
#include "Session/OnlineSessionGOG.h"
#include "Converters/OnlineSessionSettingsConverter.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"

namespace
{

	bool AddLocalSession(
		FOnlineSessionGOG& InSessionInterface,
		const galaxy::api::GalaxyID& InLobbyID,
		const FName& InSessionName,
		const TSharedRef<const FUniqueNetId>& InSessionOwnerID,
		const FString& InSessionOwnerName,
		const FOnlineSessionSettings& InSessionSettings)
	{
		auto newSession = InSessionInterface.AddNamedSession(InSessionName, InSessionSettings);
		if (!newSession)
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to create new local session (NULL)"));
			return false;
		}

		newSession->SessionState = EOnlineSessionState::Pending;
		newSession->SessionInfo = MakeShared<FOnlineSessionInfoGOG>(InLobbyID);
		newSession->OwningUserId = InSessionOwnerID;
		newSession->OwningUserName = InSessionOwnerName;
		newSession->NumOpenPublicConnections = InSessionSettings.NumPublicConnections;

		return true;
	}

	bool MakeSessionJoinable(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener* const InListener)
	{
		galaxy::api::Matchmaking()->SetLobbyJoinable(InLobbyID, true, InListener);
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to make session joinable: lobbyID=%llu; %s: %s"), InLobbyID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			return false;
		}

		return true;
	}

}

FCreateLobbyListener::FCreateLobbyListener(
	class FOnlineSessionGOG& InSessionInterface,
	FName InSessionName,
	TSharedRef<const FUniqueNetId> InSessionOwnerID,
	FString InSessionOwnerName,
	FOnlineSessionSettings InSettings)
	: sessionInterface{InSessionInterface}
	, sessionName{MoveTemp(InSessionName)}
	, sessionOwnerID{MoveTemp(InSessionOwnerID)}
	, sessionOwnerName{MoveTemp(InSessionOwnerName)}
	, sessionSettings{MoveTemp(InSettings)}
{
	checkf(!sessionOwnerName.IsEmpty() && sessionOwnerID->IsValid(), TEXT("Invalid session owner information"));
}

void FCreateLobbyListener::OnLobbyCreated(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyCreateResult InResult)
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnLobbyCreated: lobbyID=%llu, result=%d"), InLobbyID.ToUint64(), static_cast<int>(InResult));

	if (InResult != galaxy::api::LOBBY_CREATE_RESULT_SUCCESS)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to create a lobby: lobbyID=%llu"), newLobbyID.ToUint64());

		TriggerOnCreateSessionCompleteDelegates(false);
		return;
	}

	check(InLobbyID.IsValid());
	newLobbyID = InLobbyID;

	// Wait till OnLobbyEntered
}

void FCreateLobbyListener::OnLobbyEntered(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::LobbyEnterResult InResult)
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnLobbyEntered: lobbyID=%llu, result=%d"), InLobbyID.ToUint64(), static_cast<int>(InResult));

	checkf(newLobbyID == InLobbyID, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	if (InResult != galaxy::api::LOBBY_ENTER_RESULT_SUCCESS)
	{

		switch (InResult)
		{
			case galaxy::api::LOBBY_ENTER_RESULT_LOBBY_DOES_NOT_EXIST:
			{
				UE_LOG_ONLINE(Error, TEXT("Lobby does not exists: lobbyID=%llu"), InLobbyID.ToUint64());
				break;
			}

			case galaxy::api::LOBBY_ENTER_RESULT_LOBBY_IS_FULL:
			{
				UE_LOG_ONLINE(Display, TEXT("Lobby is full: lobbyID=%llu"), InLobbyID.ToUint64());
				break;
			}

			case galaxy::api::LOBBY_ENTER_RESULT_ERROR:
			{
				UE_LOG_ONLINE(Error, TEXT("Unknown error when entering lobby: lobbyID=%llu"), InLobbyID.ToUint64());
				break;
			}
		}

		TriggerOnCreateSessionCompleteDelegates(false);
		return;
	}

	sessionSettings.Settings.Emplace(lobby_data::SESSION_OWNER_NAME, FOnlineSessionSetting{sessionOwnerName, EOnlineDataAdvertisementType::ViaOnlineService});
	sessionSettings.Settings.Emplace(lobby_data::SESSION_OWNER_ID, FOnlineSessionSetting{sessionOwnerID->ToString(), EOnlineDataAdvertisementType::ViaOnlineService});

	if (!OnlineSessionUtils::SetLobbyData(newLobbyID, sessionSettings)
		|| !MakeSessionJoinable(newLobbyID, this))
	{
		TriggerOnCreateSessionCompleteDelegates(false);
		return;
	}

	// Wait until LobbyDataUpdateSuccess as a confirmation that LobbyData is set in the backend and Lobby is joinable
}

void FCreateLobbyListener::OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnLobbyDataUpdateSuccess: lobbyID=%llu"), InLobbyID.ToUint64());

	checkf(newLobbyID == InLobbyID, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	auto isLobbyJoinable = galaxy::api::Matchmaking()->IsLobbyJoinable(InLobbyID);
	auto err = galaxy::api::GetError();
	if (err)
		UE_LOG_ONLINE(Error, TEXT("Failed to check lobby joinability: %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

	if (!isLobbyJoinable)
	{
		UE_LOG_ONLINE(Error, TEXT("Cannot mark lobby as joinable"));
		TriggerOnCreateSessionCompleteDelegates(false);
	}

	if (!AddLocalSession(sessionInterface, newLobbyID, sessionName, sessionOwnerID, sessionOwnerName, sessionSettings))
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to add local session: sessionName=%s"), *sessionName.ToString());
		TriggerOnCreateSessionCompleteDelegates(false);
	}

	if (!OnlineSessionUtils::ShouldAdvertiseViaPresence(sessionSettings))
	{
		TriggerOnCreateSessionCompleteDelegates(true);
		return;
	}

	if (!AdvertiseToFriends())
		TriggerOnCreateSessionCompleteDelegates(false);
}

bool FCreateLobbyListener::AdvertiseToFriends()
{
	FString connectString;
	if (!sessionInterface.GetResolvedConnectString(sessionName, connectString))
		return false;

	galaxy::api::Friends()->SetRichPresence("connect", TCHAR_TO_UTF8(*connectString), this);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to set rich presence connect string: connectString='%s'; %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		return false;
	}

	return true;
}

void FCreateLobbyListener::OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnLobbyDataUpdateFailure: lobbyID=%llu"), InLobbyID.ToUint64());

	checkf(newLobbyID == InLobbyID, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	UE_LOG_ONLINE(Error, InFailureReason == galaxy::api::ILobbyDataUpdateListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST
		? TEXT("Specified lobby does not exists")
		: TEXT("Unknown error"));

	TriggerOnCreateSessionCompleteDelegates(false);
}

void FCreateLobbyListener::OnRichPresenceChangeSuccess()
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnRichPresenceChangeSuccess: lobbyID=%llu"), newLobbyID.ToUint64());

	TriggerOnCreateSessionCompleteDelegates(true);
}

void FCreateLobbyListener::OnRichPresenceChangeFailure(galaxy::api::IRichPresenceChangeListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("FCreateLobbyListener::OnRichPresenceChangeFailure: lobbyID=%llu"), newLobbyID.ToUint64());

	TriggerOnCreateSessionCompleteDelegates(false);
}

void FCreateLobbyListener::TriggerOnCreateSessionCompleteDelegates(bool InIsSuccessful)
{
	if (!InIsSuccessful)
	{
		galaxy::api::Matchmaking()->LeaveLobby(newLobbyID);
		sessionInterface.RemoveNamedSession(sessionName);
	}

	sessionInterface.TriggerOnCreateSessionCompleteDelegates(sessionName, InIsSuccessful);

	sessionInterface.FreeListener(MoveTemp(ListenerID));
}
