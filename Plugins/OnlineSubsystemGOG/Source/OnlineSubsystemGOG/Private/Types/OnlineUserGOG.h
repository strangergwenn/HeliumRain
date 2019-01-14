#pragma once

#include "UniqueNetIdGOG.h"
#include "UserInfoUtils.h"

#include "OnlineSubsystemTypes.h"

class FOnlineUserGOG : public FOnlineUser
{
public:

	static bool Fill(FOnlineUserGOG& InOutOnlineUser);

	static bool FillOwn(FOnlineUserGOG& InOutOnlineUser);

	explicit FOnlineUserGOG(FUniqueNetIdGOG InUserID);

	TSharedRef<const FUniqueNetId> GetUserId() const override;

	FString GetRealName() const override;

	FString GetDisplayName(const FString& InPlatform = FString()) const override;

	bool GetUserAttribute(const FString& InAttrName, FString& OutAttrValue) const override;

protected:

	bool IsValid() const;

	TSharedRef<const FUniqueNetIdGOG> userID;
	FString displayName;
	UserAttributesMap userAttributes;
};
