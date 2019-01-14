#pragma once

#include "OnlineUserGOG.h"

#include "OnlineSubsystemTypes.h"
#include "OnlinePresenceInterface.h"

class FOnlineFriendGOG : public FOnlineFriend, public FOnlineUserGOG
{
public:

	static bool Fill(FOnlineFriendGOG& InOutOnlineFriend);

	FOnlineFriendGOG(FUniqueNetIdGOG InFriendID);

	// FOnlineUser interface implementation

	TSharedRef<const FUniqueNetId> GetUserId() const override;

	FString GetRealName() const override;

	FString GetDisplayName(const FString& InPlatform = FString()) const override;

	bool GetUserAttribute(const FString& InAttrName, FString& OutAttrValue) const override;

	// FOnlineFriend interface implementation

	EInviteStatus::Type GetInviteStatus() const override;

	const FOnlineUserPresence& GetPresence() const override;

	void SetPresence(FOnlineUserPresence InUserPresence);

private:

	FOnlineUserPresence userPresence;
};
