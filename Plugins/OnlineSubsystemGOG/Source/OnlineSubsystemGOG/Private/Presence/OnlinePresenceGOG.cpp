#include "OnlinePresenceGOG.h"

#include "OnlineSubsystemGOG.h"
#include "SetRichPresenceListener.h"
#include "RequestRichPresenceListener.h"
#include "Types/UserOnlineAccountGOG.h"
#include "Types/OnlineUserPresence.h"
#include "Friends/OnlineFriendsGOG.h"

FOnlinePresenceGOG::FOnlinePresenceGOG(const class FOnlineSubsystemGOG& InOnlineSubsystemGOG, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount)
	: onlineSubsystemGOG{InOnlineSubsystemGOG}
	, ownUserOnlineAccount{InUserOnlineAccount}
{
}

void FOnlinePresenceGOG::SetPresence(const FUniqueNetId& InUserID, const FOnlineUserPresenceStatus& InStatus, const FOnPresenceTaskCompleteDelegate& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlinePresenceGOG::SetPresence()"));

	if (*ownUserOnlineAccount->GetUserId() != InUserID)
	{
		UE_LOG_ONLINE(Warning, TEXT("User presence can only be set for own player: userID='%s', ownUserID='%s'"),
			*InUserID.ToString(), *ownUserOnlineAccount->GetUserId()->ToString());
		InDelegate.ExecuteIfBound(InUserID, false);
		return;
	}

	galaxy::api::Friends()->SetRichPresence(OnlineUserPresence::RICH_PRESENCE_STATUS, TCHAR_TO_UTF8(*InStatus.StatusStr));
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to set rich presence status: %s; %s: %s"), *InStatus.StatusStr, UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		InDelegate.ExecuteIfBound(InUserID, false);
		return;
	}

	auto presenceMetaData = InStatus.Properties.Find(DefaultPresenceKey);
	auto numOfNonCustomProperties = InStatus.Properties.Num() - (presenceMetaData ? 1 : 0);
	if (numOfNonCustomProperties > 0)
		// TODO: re-consider if more RichPresence data is implemented
		UE_LOG_ONLINE(Warning, TEXT("GalaxySDK only supports '%s' as a custom rich presence property. All other properties will be ignored"), *DefaultPresenceKey);

	// Specific listener is given only to the last set as they will be squashed into one operation
	auto listener = CreateListener<FSetRichPresenceListener>(*this, InUserID, InDelegate);

	galaxy::api::Friends()->SetRichPresence(OnlineUserPresence::RICH_PRESENCE_METADATA, TCHAR_TO_UTF8(*presenceMetaData->ToString()), listener.Value);
	err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to set rich presence metadata: %s; %s: %s"), *presenceMetaData->ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		InDelegate.ExecuteIfBound(InUserID, false);
		return;
	}
}

void FOnlinePresenceGOG::QueryPresence(const FUniqueNetId& InUserID, const FOnPresenceTaskCompleteDelegate& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlinePresenceGOG::SetPresence()"));

	FUniqueNetIdGOG userID{InUserID};

	auto listener = CreateListener<FRequestRichPresenceListener>(*this, userID, InDelegate);

	galaxy::api::Friends()->RequestRichPresence(userID, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to request rich presence: userID='%s'; %s: %s"), *userID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		InDelegate.ExecuteIfBound(InUserID, false);
		return;
	}
}

EOnlineCachedResult::Type FOnlinePresenceGOG::GetCachedPresence(const FUniqueNetId& InUserID, TSharedPtr<FOnlineUserPresence>& OutPresence)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlinePresenceGOG::GetCachedPresence()"));

	auto cachedUserPresence = presenceCache.Find(InUserID);
	if (!cachedUserPresence)
	{
		UE_LOG_ONLINE(Display, TEXT("Presence for user not found in cache: userID='%s'"), *InUserID.ToString());
		return EOnlineCachedResult::NotFound;
	}

	OutPresence = *cachedUserPresence;
	return EOnlineCachedResult::Success;
}

EOnlineCachedResult::Type FOnlinePresenceGOG::GetCachedPresenceForApp(
	const FUniqueNetId& /*InLocalUserId*/,
	const FUniqueNetId& /*InUserID*/,
	const FString& /*InAppId*/,
	TSharedPtr<FOnlineUserPresence>& /*OutPresence*/)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlinePresenceGOG::GetCachedPresenceForApp()"));
	UE_LOG_ONLINE(Warning, TEXT("GalaxySDK has no support for Application IDs yet. Ignored."));
	return EOnlineCachedResult::NotFound;
}

TSharedRef<FOnlineUserPresence> FOnlinePresenceGOG::AddOrReplaceUserPresence(const FUniqueNetIdGOG& InUserID, FOnlineUserPresence InOutOnlineUserPresence)
{
	// TMap::FindOrAdd cannot create TSharedRef with default initialized object
	auto cachedPresence = presenceCache.Find(InUserID);
	if (cachedPresence)
	{
		**cachedPresence = MoveTemp(InOutOnlineUserPresence);
		return *cachedPresence;
	}

	return presenceCache.Emplace(InUserID, MakeShared<FOnlineUserPresence>(MoveTemp(InOutOnlineUserPresence)));
}

void FOnlinePresenceGOG::OnRichPresenceUpdated(galaxy::api::GalaxyID InUserID)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlinePresenceGOG::OnRichPresenceUpdated(%llu)"), InUserID.ToUint64());

	FUniqueNetIdGOG userID{InUserID};

	FOnlineUserPresence userPresence;
	if (!OnlineUserPresence::Fill(InUserID, userPresence))
	{
		UE_LOG_ONLINE(Display, TEXT("Failed to get online presence for user: userID='%llu'"), InUserID.ToUint64());
		return;
	}

	auto cachedUserPresence = AddOrReplaceUserPresence(InUserID, userPresence);

	TArray<TSharedRef<FOnlineUserPresence>> presenceArray;
	presenceArray.Emplace(cachedUserPresence);

	TriggerOnPresenceArrayUpdatedDelegates(userID, presenceArray);
	TriggerOnPresenceReceivedDelegates(userID, cachedUserPresence);

	if (*ownUserOnlineAccount->GetUserId() == userID)
		return;

	// Update rich presence info in friends cache
	TSharedPtr<FOnlineFriendsGOG, ESPMode::ThreadSafe> onlineFriendsGOG{StaticCastSharedPtr<FOnlineFriendsGOG>(onlineSubsystemGOG.GetFriendsInterface())};
	if (!onlineFriendsGOG.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot update rich presence for a user as Friends Interface is NULL: userID=%llu"), InUserID.ToUint64());
		return;
	}

	onlineFriendsGOG->UpdateFriendPresence(userID, cachedUserPresence);
}
