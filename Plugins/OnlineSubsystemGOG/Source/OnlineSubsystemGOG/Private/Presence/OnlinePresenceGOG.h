#pragma once

#include "CommonGOG.h"
#include "ListenerManager.h"

#include "Interfaces/OnlinePresenceInterface.h"
#include "Types/UniqueNetIdGOG.h"

class FOnlinePresenceGOG
	: public IOnlinePresence
	, public FListenerManager
	, public galaxy::api::GlobalRichPresenceListener
{
public:

	void SetPresence(const FUniqueNetId& InUserID, const FOnlineUserPresenceStatus& InStatus, const FOnPresenceTaskCompleteDelegate& InDelegate = FOnPresenceTaskCompleteDelegate()) override;

	void QueryPresence(const FUniqueNetId& InUserID, const FOnPresenceTaskCompleteDelegate& InDelegate = FOnPresenceTaskCompleteDelegate()) override;

	EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& InUserID, TSharedPtr<FOnlineUserPresence>& OutPresence) override;

	EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& InLocalUserId, const FUniqueNetId& InUserID, const FString& InAppId, TSharedPtr<FOnlineUserPresence>& OutPresence) override;

PACKAGE_SCOPE:

	FOnlinePresenceGOG(const class FOnlineSubsystemGOG& InOnlineSubsystemGOG, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount);

private:

	TSharedRef<FOnlineUserPresence> AddOrReplaceUserPresence(const FUniqueNetIdGOG& InUserID, FOnlineUserPresence InOutOnlineUserPresence);

	FOnlinePresenceGOG() = delete;

	void OnRichPresenceUpdated(galaxy::api::GalaxyID InUserID) override;

	TMap<FUniqueNetIdGOG, TSharedRef<FOnlineUserPresence>> presenceCache;

	const class FOnlineSubsystemGOG& onlineSubsystemGOG;
	TSharedRef<class FUserOnlineAccountGOG> ownUserOnlineAccount;
};
