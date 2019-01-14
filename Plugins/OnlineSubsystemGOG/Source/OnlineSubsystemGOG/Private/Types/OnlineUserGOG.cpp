#include "OnlineUserGOG.h"
#include "UserInfoUtils.h"

bool FOnlineUserGOG::Fill(FOnlineUserGOG& InOutOnlineUser)
{
	if (!InOutOnlineUser.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid User: userID='%s'"), *InOutOnlineUser.userID->ToString());
		return false;
	}

	if (!UserInfoUtils::GetPlayerNickname(*InOutOnlineUser.userID, InOutOnlineUser.displayName)
		|| !UserInfoUtils::GetUserAttributes(*InOutOnlineUser.userID, InOutOnlineUser.userAttributes))
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to get user information: userID='%s'"), *InOutOnlineUser.userID->ToString());
		return false;
	}

	return true;
}

bool FOnlineUserGOG::FillOwn(FOnlineUserGOG& InOutOnlineUser)
{
	if (!InOutOnlineUser.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid User: userID='%s'"), *InOutOnlineUser.userID->ToString());
		return false;
	}

	if (!UserInfoUtils::GetOwnPlayerNickname(InOutOnlineUser.displayName)
		|| !UserInfoUtils::GetUserAttributes(*InOutOnlineUser.userID, InOutOnlineUser.userAttributes))
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to get own user information: userID='%s'"), *InOutOnlineUser.userID->ToString());
		return false;
	}

	return true;
}

FOnlineUserGOG::FOnlineUserGOG(FUniqueNetIdGOG InUserID)
	: userID{MakeShared<const FUniqueNetIdGOG>(MoveTemp(InUserID))}
{
}

TSharedRef<const FUniqueNetId> FOnlineUserGOG::GetUserId() const
{
	return userID;
}

FString FOnlineUserGOG::GetRealName() const
{
	return{};
}

FString FOnlineUserGOG::GetDisplayName(const FString& /*InPlatform = FString()*/) const
{
	return displayName;
}

bool FOnlineUserGOG::GetUserAttribute(const FString& InAttrName, FString& OutAttrValue) const
{
	const auto foundAttr = userAttributes.Find(InAttrName);
	if (!foundAttr)
		return false;

	OutAttrValue = *foundAttr;
	return true;
}

bool FOnlineUserGOG::IsValid() const
{
	return userID->IsValid() && userID->IsUser();
}
