#pragma once

#include "CommonGOG.h"
#include "OnlineSessionInfoGOG.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ListenerManager.h"

#include "OnlineSessionSettings.h"

class FOnlineSessionGOG
	: public IOnlineSession
	, public FListenerManager
	, public galaxy::api::GlobalLobbyLeftListener
	, public galaxy::api::GlobalGameInvitationReceivedListener
	, public galaxy::api::GlobalGameJoinRequestedListener
	, public galaxy::api::GlobalLobbyDataListener
	, public galaxy::api::GlobalLobbyMemberStateListener
{
public:

	bool CreateSession(int32 InHostingPlayerNum, FName InSessionName, const FOnlineSessionSettings& InSessionSettings) override;

	bool CreateSession(const FUniqueNetId& InHostingPlayerId, FName InSessionName, const FOnlineSessionSettings& InSessionSettings) override;

	FNamedOnlineSession* GetNamedSession(FName InSessionName) override;

	const FNamedOnlineSession* GetNamedSession(FName InSessionName) const;

	void RemoveNamedSession(FName InSessionName) override;

	bool HasPresenceSession() override;

	EOnlineSessionState::Type GetSessionState(FName InSessionName) const override;

	bool StartSession(FName InSessionName) override;

	bool UpdateSession(FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, bool InShouldRefreshOnlineData = true) override;

	bool EndSession(FName InSessionName) override;

	bool DestroySession(FName InSessionName, const FOnDestroySessionCompleteDelegate& InCompletionDelegate = FOnDestroySessionCompleteDelegate()) override;

	bool IsPlayerInSession(FName InSessionName, const FUniqueNetId& InUniqueId) override;

	bool StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>& InLocalPlayers, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, TSharedRef<FOnlineSessionSearch>& OutSearchSettings) override;

	bool CancelMatchmaking(int32 InSearchingPlayerNum, FName InSessionName) override;

	bool CancelMatchmaking(const FUniqueNetId& InSearchingPlayerId, FName InSessionName) override;

	bool FindSessions(int32 InSearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& InOutSearchSettings) override;

	bool FindSessions(const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InOutSearchSettings) override;

	bool FindSessionById(const FUniqueNetId& InSearchingUserId, const FUniqueNetId& InSessionId, const FUniqueNetId& InFriendId, const FOnSingleSessionResultCompleteDelegate& InCompletionDelegate) override;

	bool CancelFindSessions() override;

	bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override;

	bool JoinSession(int32 InLocalUserNum, FName InSessionName, const FOnlineSessionSearchResult& InDesiredSession) override;

	bool JoinSession(const FUniqueNetId& InLocalUserId, FName InSessionName, const FOnlineSessionSearchResult& InDesiredSession) override;

	bool FindFriendSession(int32 InLocalUserNum, const FUniqueNetId& InFriend) override;

	bool FindFriendSession(const FUniqueNetId& InLocalUserId, const FUniqueNetId& InFriend) override;

	bool FindFriendSession(const FUniqueNetId& InLocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& InFriendList) override;

	bool SendSessionInviteToFriend(int32 InLocalUserNum, FName InSessionName, const FUniqueNetId& InFriend) override;

	bool SendSessionInviteToFriend(const FUniqueNetId& InLocalUserId, FName InSessionName, const FUniqueNetId& InFriend) override;

	bool SendSessionInviteToFriends(int32 InLocalUserNum, FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InFriends) override;

	bool SendSessionInviteToFriends(const FUniqueNetId& InLocalUserId, FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InFriends) override;

	bool GetResolvedConnectString(FName InSessionName, FString& OutConnectInfo, FName InPortType = NAME_GamePort) override;

	bool GetResolvedConnectString(const class FOnlineSessionSearchResult& InSearchResult, FName InPortType, FString& OutConnectInfo) override;

	FOnlineSessionSettings* GetSessionSettings(FName InSessionName) override;

	bool RegisterPlayer(FName InSessionName, const FUniqueNetId& InPlayerId, bool InWasInvited) override;

	bool RegisterPlayers(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, bool InWasInvited = false) override;

	bool UnregisterPlayer(FName InSessionName, const FUniqueNetId& InPlayerId) override;

	bool UnregisterPlayers(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers) override;

	void RegisterLocalPlayer(const FUniqueNetId& InPlayerId, FName InSessionName, const FOnRegisterLocalPlayerCompleteDelegate& InDelegate) override;

	void UnregisterLocalPlayer(const FUniqueNetId& InPlayerId, FName InSessionName, const FOnUnregisterLocalPlayerCompleteDelegate& InDelegate) override;

	int32 GetNumSessions() override;

	void DumpSessionState() override;

PACKAGE_SCOPE:

	FOnlineSessionGOG(IOnlineSubsystem& InSubsystem, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount);

	~FOnlineSessionGOG();

	FNamedOnlineSession* AddNamedSession(FName InSessionName, const FOnlineSessionSettings& InSessionSettings) override;

	FNamedOnlineSession* AddNamedSession(FName InSessionName, const FOnlineSession& InSession) override;

private:

	template<typename... Args>
	FNamedOnlineSession* CreateNamedSession(FName InSessionName, Args... InArgs)
	{
		// TArray placement new constructs element in newly allocated space at the end of the container
		return new(storedSessions) FNamedOnlineSession{std::move(InSessionName), InArgs...};
	}

	bool GetResolvedConnectStringFromSession(const FOnlineSession& InSession, FString& OutConnectString) const;

	FNamedOnlineSession* FindSession(const FUniqueNetIdGOG& InSessionID);

	void OnLobbyLeft(const galaxy::api::GalaxyID& InLobbyID, bool InIoFailure) override;

	void OnGameInvitationReceived(galaxy::api::GalaxyID InUserID, const char* InConnectString) override;

	void OnGameJoinRequested(galaxy::api::GalaxyID InUserID, const char* InConnectString) override;

	void OnLobbyDataUpdated(const galaxy::api::GalaxyID& InLobbyID, const galaxy::api::GalaxyID& InMemberID) override;

	void OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& InLobbyID, const galaxy::api::GalaxyID& InMemberID, galaxy::api::LobbyMemberStateChange InMemberStateChange) override;

	TArray<FNamedOnlineSession> storedSessions;

	IOnlineSubsystem& subsystemGOG;
	TSharedRef<class FUserOnlineAccountGOG> ownUserOnlineAccount;
};
