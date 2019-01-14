#pragma once

#include "CommonGOG.h"
#include "Types/UserOnlineAccountGOG.h"

#include "OnlineIdentityInterface.h"

class FOnlineIdentityGOG
	: public IOnlineIdentity
	, public galaxy::api::GlobalAuthListener
{
public:

	FOnlineIdentityGOG(class FOnlineSubsystemGOG& InOnlineSubsystemGOG);
	~FOnlineIdentityGOG();

	bool Login(int32 InLocalUserNum, const FOnlineAccountCredentials& InAccountCredentials) override;

	bool Logout(int32 InLocalUserNum) override;

	bool AutoLogin(int32 InLocalUserNum) override;

	TSharedPtr<FUserOnlineAccount> GetUserAccount(const FUniqueNetId& InUserId) const override;

	TArray<TSharedPtr<FUserOnlineAccount>> GetAllUserAccounts() const override;

	TSharedPtr<const FUniqueNetId> GetUniquePlayerId(int32 InLocalUserNum) const override;

	TSharedPtr<const FUniqueNetId> GetSponsorUniquePlayerId(int32 InLocalUserNum) const override;

	TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(uint8* InBytes, int32 InSize) override;

	TSharedPtr<const FUniqueNetId> CreateUniquePlayerId(const FString& InStr) override;

	ELoginStatus::Type GetLoginStatus(int32 InLocalUserNum) const override;

	ELoginStatus::Type GetLoginStatus(const FUniqueNetId& InUserId) const override;

	FString GetPlayerNickname(int32 InLocalUserNum) const override;

	FString GetPlayerNickname(const FUniqueNetId& InUserId) const override;

	FString GetAuthToken(int32 InLocalUserNum) const override;

	void GetUserPrivilege(const FUniqueNetId& InUserId, EUserPrivileges::Type InPrivilege, const FOnGetUserPrivilegeCompleteDelegate& InDelegate) override;

	FPlatformUserId GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& InUniqueNetId) const override;

	void RevokeAuthToken(const FUniqueNetId& InUserId, const FOnRevokeAuthTokenCompleteDelegate& InDelegate) override;

	FString GetAuthType() const override;

	TSharedRef<FUserOnlineAccountGOG> GetOwnUserOnlineAccount() const;

private:

	// IAuthListener

	void OnAuthSuccess() override;

	void OnAuthFailure(FailureReason failureReason) override;

	void OnAuthLost() override;

	FString FailureReasonToFString(FailureReason failureReason);

	class FOnlineSubsystemGOG& onlineSubsystemGOG;

	bool isAuthInProgress{false};

	TSharedRef<FUserOnlineAccountGOG> ownUserOnlineAccount;
};

using FOnlineIdentityGOGPtr = TSharedPtr<class FOnlineIdentityGOG, ESPMode::ThreadSafe>;
