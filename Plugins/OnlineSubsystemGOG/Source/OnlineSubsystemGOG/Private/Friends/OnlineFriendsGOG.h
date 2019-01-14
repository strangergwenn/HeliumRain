#pragma once

#include "CommonGOG.h"

#include "Types/IListenerGOG.h"
#include "ListenerManager.h"

#include "OnlineFriendsInterface.h"

class FOnlineFriendsGOG
	: public IOnlineFriends
	, public FListenerManager
	, public galaxy::api::GlobalFriendListListener
	, public galaxy::api::GlobalFriendInvitationListener
	, public galaxy::api::GlobalFriendAddListener
{
public:

	// TOOD: Following delegates are not supported by GOG:
	// - OnInviteRejected
	// - OnBlockedPlayerComplete, OnUnblockedPlayerComplete, OnBlockListChange
	// - OnQueryRecentPlayersComplete, OnQueryBlockedPlayersComplete

	bool ReadFriendsList(int32 InLocalUserNum, const FString& InListName, const FOnReadFriendsListComplete& InDelegate) override;

	bool DeleteFriendsList(int32 InLocalUserNum, const FString& InListName, const FOnDeleteFriendsListComplete& InDelegate) override;

	bool SendInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate) override;

	bool AcceptInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnAcceptInviteComplete& InDelegate) override;

	bool RejectInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName) override;

	bool DeleteFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName) override;

	bool GetFriendsList(int32 InLocalUserNum, const FString& InListName, TArray<TSharedRef<FOnlineFriend>>& OutFriends) override;

	TSharedPtr<FOnlineFriend> GetFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName) override;

	TSharedRef<FOnlineFriend>* const FindFriend(const FUniqueNetId& InFriendId, const FString& InListName);

	bool IsFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName) override;

	bool QueryRecentPlayers(const FUniqueNetId& InUserId, const FString& InNamespace) override;

	bool GetRecentPlayers(const FUniqueNetId& InUserId, const FString& InNamespace, TArray< TSharedRef<FOnlineRecentPlayer> >& OutRecentPlayers) override;

	bool BlockPlayer(int32 InLocalUserNum, const FUniqueNetId& InPlayerId) override;

	bool UnblockPlayer(int32 InLocalUserNum, const FUniqueNetId& InPlayerId) override;

	bool QueryBlockedPlayers(const FUniqueNetId& InUserId) override;

	bool GetBlockedPlayers(const FUniqueNetId& InUserId, TArray< TSharedRef<FOnlineBlockedPlayer> >& OutBlockedPlayers) override;

	void DumpBlockedPlayers() const override;

	static const FString& GetDefaultFriendsListName();

PACKAGE_SCOPE:

	FOnlineFriendsGOG(class FOnlineSubsystemGOG& InSubsystem, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount);

	void UpdateFriendPresence(const class FUniqueNetIdGOG& InFriendID, TSharedRef<class FOnlineUserPresence> InFriendPresence);

private:

	void OnFriendListRetrieveSuccess() override;

	bool AddOrUpdateCachedFriend(class FOnlineFriendGOG InOnlineFriend);

	void RemoveUnfriended(TSet<FUniqueNetIdGOG> InRetrievedFriendIDList, bool &InOutIsFriendsListChanged);

	void OnFriendListRetrieveFailure(galaxy::api::IFriendListListener::FailureReason InFailureReason) override;

	void OnFriendInvitationReceived(galaxy::api::GalaxyID InFriendID, uint32_t InSendTime) override;

	void OnFriendAdded(galaxy::api::GalaxyID InFriendID, galaxy::api::IFriendAddListener::InvitationDirection InInvitationDirection) override;

	class FOnlineSubsystemGOG& subsystemGOG;

	TSharedRef<class FUserOnlineAccountGOG> ownUserOnlineAccount;

	TArray<TSharedRef<FOnlineFriend>> cachedFriends;
};
