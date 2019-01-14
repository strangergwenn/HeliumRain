#include "OnlineFriendsGOG.h"

#include "OnlineSubsystemGOG.h"
#include "RequestFriendListListener.h"
#include "FriendInvitationSendListener.h"
#include "FriendInvitationRespondToListener.h"
#include "FriendDeleteListener.h"
#include "Types/UniqueNetIdGOG.h"
#include "Types/OnlineFriendGOG.h"
#include "Types/UserOnlineAccountGOG.h"

namespace
{

	bool CheckFriendListName(const FString& InListName)
	{
		if (InListName != FOnlineFriendsGOG::GetDefaultFriendsListName())
		{
			UE_LOG_ONLINE(Warning, TEXT("Only a default list name '%s' is supported: listName='%s'"),
				*FOnlineFriendsGOG::GetDefaultFriendsListName(), *InListName);
			return false;
		}

		return true;
	}

}

const FString& FOnlineFriendsGOG::GetDefaultFriendsListName()
{
	static const FString defaultFriendsListName{EFriendsLists::ToString(EFriendsLists::Default)};
	return defaultFriendsListName;
}

FOnlineFriendsGOG::FOnlineFriendsGOG(FOnlineSubsystemGOG& InSubsystem, TSharedRef<FUserOnlineAccountGOG> InUserOnlineAccount)
	: subsystemGOG{InSubsystem}
	, ownUserOnlineAccount{MoveTemp(InUserOnlineAccount)}
{
}

void FOnlineFriendsGOG::UpdateFriendPresence(const FUniqueNetIdGOG& InFriendID, TSharedRef<FOnlineUserPresence> InFriendPresence)
{
	auto cachedFriend = FindFriend(InFriendID, GetDefaultFriendsListName());
	if (!cachedFriend)
		return;

	StaticCastSharedRef<FOnlineFriendGOG>(*cachedFriend)->SetPresence(*InFriendPresence);
}

bool FOnlineFriendsGOG::ReadFriendsList(int32 InLocalUserNum, const FString& InListName, const FOnReadFriendsListComplete& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::ReadFriendsList(%s)"), *InListName);

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
	{
		InDelegate.ExecuteIfBound(InLocalUserNum, false, InListName, TEXT("Only default list name is supported"));
		return false;
	}

	auto listener = CreateListener<FRequestFriendListListener>(*this, InListName, InDelegate);

	galaxy::api::Friends()->RequestFriendList(listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		const auto errorMessage = FString::Printf(TEXT("%s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		UE_LOG_ONLINE(Warning, TEXT("Failed to request friends list: listname=%s; %s"), *InListName, *errorMessage);

		FreeListener(MoveTemp(listener.Key));

		InDelegate.ExecuteIfBound(InLocalUserNum, false, InListName, errorMessage);
		return false;
	}

	return true;
}

bool FOnlineFriendsGOG::DeleteFriendsList(int32 InLocalUserNum, const FString& InListName, const FOnDeleteFriendsListComplete& InDelegate)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::DeleteFriendsList() is not supported by GOG platform"));
	InDelegate.ExecuteIfBound(InLocalUserNum, false, InListName, TEXT("DeleteFriendsList() is not supported by GOG platform"));
	return false;
}

bool FOnlineFriendsGOG::SendInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnSendInviteComplete& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::SendInvite(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
	{
		InDelegate.ExecuteIfBound(InLocalUserNum, false, InFriendId, InListName, TEXT("Only default list name is supported"));
		return false;
	}

	auto listener = CreateListener<FFriendInvitationSendListener>(*this, InFriendId, InListName, InDelegate);

	galaxy::api::Friends()->SendFriendInvitation(FUniqueNetIdGOG{InFriendId}, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		const auto errorMessage = FString::Printf(TEXT("%s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		UE_LOG_ONLINE(Warning, TEXT("Failed to send friend invitation: listname=%s, friendID=%s; %s"), *InListName, *InFriendId.ToString(), *errorMessage);

		FreeListener(MoveTemp(listener.Key));

		InDelegate.ExecuteIfBound(InLocalUserNum, false, InFriendId, InListName, errorMessage);
		return false;
	}

	return true;
}

bool FOnlineFriendsGOG::AcceptInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName, const FOnAcceptInviteComplete& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::AcceptInvite(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
	{
		InDelegate.ExecuteIfBound(InLocalUserNum, false, InFriendId, InListName, TEXT("Only default list name is supported"));
		return false;
	}

	auto listener = CreateListener<FFriendInvitationAcceptListener>(*this, InFriendId, InListName, InDelegate);

	galaxy::api::Friends()->RespondToFriendInvitation(FUniqueNetIdGOG{InFriendId}, true, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		const auto errorMessage = FString::Printf(TEXT("%s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		UE_LOG_ONLINE(Warning, TEXT("Failed to accept friend invitation: listname=%s, friendID=%s; %s"), *InListName, *InFriendId.ToString(), *errorMessage);

		FreeListener(MoveTemp(listener.Key));

		InDelegate.ExecuteIfBound(InLocalUserNum, false, InFriendId, InListName, errorMessage);
		return false;
	}

	return true;
}

bool FOnlineFriendsGOG::RejectInvite(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::RejectInvite(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
	{
		TriggerOnRejectInviteCompleteDelegates(InLocalUserNum, false, InFriendId, InListName, TEXT("Only default list name is supported"));
		return false;
	}

	auto listener = CreateListener<FFriendInvitationRejectListener>(*this, InFriendId, InListName);

	galaxy::api::Friends()->RespondToFriendInvitation(FUniqueNetIdGOG{InFriendId}, false, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		const auto errorMessage = FString::Printf(TEXT("%s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		UE_LOG_ONLINE(Warning, TEXT("Failed to reject friend invitation: listname=%s, friendID=%s; %s"), *InListName, *InFriendId.ToString(), *errorMessage);

		FreeListener(MoveTemp(listener.Key));

		TriggerOnRejectInviteCompleteDelegates(InLocalUserNum, false, InFriendId, InListName, errorMessage);
		return false;
	}

	return true;
}

bool FOnlineFriendsGOG::DeleteFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::DeleteFriend(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
	{
		TriggerOnDeleteFriendCompleteDelegates(InLocalUserNum, false, InFriendId, InListName, TEXT("Only default list name is supported"));
		return false;
	}

	auto listener = CreateListener<FFriendDeleteListener>(*this, InFriendId, InListName);

	galaxy::api::Friends()->DeleteFriend(FUniqueNetIdGOG{InFriendId}, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		const auto errorMessage = FString::Printf(TEXT("%s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		UE_LOG_ONLINE(Warning, TEXT("Failed to accept friend invitation: listname=%s, friendID=%s; %s"), *InListName, *InFriendId.ToString(), *errorMessage);

		FreeListener(MoveTemp(listener.Key));

		TriggerOnDeleteFriendCompleteDelegates(InLocalUserNum, false, InFriendId, InListName, errorMessage);
		return false;
	}

	return true;
}

bool FOnlineFriendsGOG::GetFriendsList(int32 InLocalUserNum, const FString& InListName, TArray<TSharedRef<FOnlineFriend>>& OutFriends)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::GetFriendsList(%s)"), *InListName);

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
		return false;

	OutFriends = cachedFriends;

	return true;
}

TSharedPtr<FOnlineFriend> FOnlineFriendsGOG::GetFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::GetFriend(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
		return nullptr;

	auto cachedFriend = FindFriend(InFriendId, InListName);
	if (!cachedFriend)
		return{};

	return *cachedFriend;
}

TSharedRef<FOnlineFriend>* const FOnlineFriendsGOG::FindFriend(const FUniqueNetId& InFriendId, const FString& InListName)
{
	auto cachedFriend = cachedFriends.FindByPredicate([&](const auto& cachedFriend)
	{
		return *cachedFriend->GetUserId() == InFriendId;
	});

	if (!cachedFriend)
	{
		UE_LOG_ONLINE(Display, TEXT("Friend not found on the list: listName='%s', friendID='%s'"), *InListName, *InFriendId.ToString());
		return{};
	}

	return cachedFriend;
}

bool FOnlineFriendsGOG::IsFriend(int32 InLocalUserNum, const FUniqueNetId& InFriendId, const FString& InListName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::IsFriend(%s, %s)"), *InListName, *InFriendId.ToString());

	CheckLocalUserNum(InLocalUserNum);
	if (!CheckFriendListName(InListName))
		return false;

	return FindFriend(InFriendId, InListName) != nullptr;
}

bool FOnlineFriendsGOG::QueryRecentPlayers(const FUniqueNetId& InUserId, const FString& InNamespace)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::QueryRecentPlayers() is not supported by GOG platform"));
	TriggerOnQueryRecentPlayersCompleteDelegates(InUserId, InNamespace, false, TEXT(""));
	return false;
}

bool FOnlineFriendsGOG::GetRecentPlayers(const FUniqueNetId& InUserId, const FString& InNamespace, TArray<TSharedRef<FOnlineRecentPlayer>>& /*OutRecentPlayers*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::GetRecentPlayers() is not supported by GOG platform"));
	return false;
}

bool FOnlineFriendsGOG::BlockPlayer(int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::BlockPlayer() is not supported by GOG platform"));
	TriggerOnBlockedPlayerCompleteDelegates(InLocalUserNum, false, InPlayerId, GetDefaultFriendsListName(), TEXT("BlockPlayer() is not supported by GOG platform"));
	return false;
}

bool FOnlineFriendsGOG::UnblockPlayer(int32 InLocalUserNum, const FUniqueNetId& InPlayerId)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::UnblockPlayer() is not supported by GOG platform"));
	TriggerOnUnblockedPlayerCompleteDelegates(InLocalUserNum, false, InPlayerId, GetDefaultFriendsListName(), TEXT("UnblockPlayer() is not supported by GOG platform"));
	return false;
}

bool FOnlineFriendsGOG::QueryBlockedPlayers(const FUniqueNetId& InUserId)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::QueryBlockedPlayers() is not supported by GOG platform"));
	TriggerOnQueryBlockedPlayersCompleteDelegates(InUserId, false, TEXT("QueryBlockedPlayers() is not supported by GOG platform"));
	return false;
}

bool FOnlineFriendsGOG::GetBlockedPlayers(const FUniqueNetId& InUserId, TArray<TSharedRef<FOnlineBlockedPlayer>>& /*OutBlockedPlayers*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::GetBlockedPlayers() is not supported by GOG platform"));
	return false;
}

void FOnlineFriendsGOG::DumpBlockedPlayers() const
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::GetBlockedPlayers() is not supported by GOG platform"));
}

void FOnlineFriendsGOG::OnFriendListRetrieveSuccess()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::OnFriendListRetrieveSuccess()"));

	auto friendCount = galaxy::api::Friends()->GetFriendCount();
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to get players friend count: %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		return;
	}

	cachedFriends.Reserve(friendCount);

	bool isFriendsListChanged{false};
	TSet<FUniqueNetIdGOG> retrievedFriendIDList;

	for (decltype(friendCount) friendIdx{0}; friendIdx < friendCount; ++friendIdx)
	{
		auto friendID = FUniqueNetIdGOG{galaxy::api::Friends()->GetFriendByIndex(friendIdx)};
		err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to get player friends: %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			continue;
		}

		if (!UserInfoUtils::IsUserInfoAvailable(friendID))
		{
			UE_LOG_ONLINE(Error, TEXT("Friend retrieved yet no user infromation available: friendID='%s'"), *friendID.ToString());
			continue;
		}

		FOnlineFriendGOG onlineFriend{friendID};
		if (!FOnlineFriendGOG::Fill(onlineFriend))
			continue;

		auto onlinePresenceInterface = subsystemGOG.GetPresenceInterface();
		checkf(onlinePresenceInterface.IsValid(), TEXT("Presence Interface is NULL"));

		TSharedPtr<FOnlineUserPresence> userPresence;
		if (onlinePresenceInterface->GetCachedPresence(friendID, userPresence) == EOnlineCachedResult::Success)
			onlineFriend.SetPresence(MoveTemp(*userPresence));

		retrievedFriendIDList.Add(friendID);

		if (AddOrUpdateCachedFriend(MoveTemp(onlineFriend)))
			isFriendsListChanged = true;
	}

	RemoveUnfriended(MoveTemp(retrievedFriendIDList), isFriendsListChanged);

	if (isFriendsListChanged)
		TriggerOnFriendsChangeDelegates(LOCAL_USER_NUM);
}

bool FOnlineFriendsGOG::AddOrUpdateCachedFriend(FOnlineFriendGOG InOnlineFriend)
{
	auto cachedFriend = FindFriend(*InOnlineFriend.GetUserId(), GetDefaultFriendsListName());
	if (!cachedFriend)
	{
		cachedFriends.Emplace(MakeShared<FOnlineFriendGOG>(MoveTemp(InOnlineFriend)));
		return true;
	}

	// Update friend's information not invalidating shared reference
	**cachedFriend = MoveTemp(InOnlineFriend);
	return false;
}

void FOnlineFriendsGOG::RemoveUnfriended(TSet<FUniqueNetIdGOG> retrievedFriendIDList, bool &InOutIsFriendsListChanged)
{
	cachedFriends.RemoveAllSwap([&](const auto& cachedUser)
	{
		const auto friendID = retrievedFriendIDList.Find(*cachedUser->GetUserId());
		if (friendID)
		{
			retrievedFriendIDList.Remove(*friendID);
			return false;
		}

		TriggerOnFriendRemovedDelegates(*ownUserOnlineAccount->GetUserId(), *cachedUser->GetUserId());
		InOutIsFriendsListChanged = true;
		return true;
	});
}

void FOnlineFriendsGOG::OnFriendListRetrieveFailure(galaxy::api::IFriendListListener::FailureReason /*InFailureReason*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineFriendsGOG::OnFriendListRetrieveFailure()"));
}

void FOnlineFriendsGOG::OnFriendInvitationReceived(galaxy::api::GalaxyID InFriendID, uint32_t /*InSendTime*/)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::OnFriendInvitationReceived()"));

	TriggerOnInviteReceivedDelegates(*ownUserOnlineAccount->GetUserId(), FUniqueNetIdGOG{InFriendID});
}

void FOnlineFriendsGOG::OnFriendAdded(galaxy::api::GalaxyID InFriendID, galaxy::api::IFriendAddListener::InvitationDirection InInvitationDirection)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFriendsGOG::OnFriendAdded()"));

	if (InInvitationDirection == galaxy::api::IFriendAddListener::INVITATION_DIRECTION_INCOMING)
		return;

	TriggerOnInviteAcceptedDelegates(*ownUserOnlineAccount->GetUserId(), FUniqueNetIdGOG{InFriendID});
}
