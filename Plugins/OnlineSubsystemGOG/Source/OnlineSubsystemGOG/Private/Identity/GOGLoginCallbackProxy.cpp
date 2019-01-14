#include "GOGLoginCallbackProxy.h"

#include "Online.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "OnlineSubsystemUtils.h"

UGOGLoginCallbackProxy::UGOGLoginCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super{ObjectInitializer}
{
}

UGOGLoginCallbackProxy* UGOGLoginCallbackProxy::Login(UObject* InWorldContextObject, class APlayerController* InPlayerController, FString AuthType, FString InUserID, FString InUserToken)
{
	auto onlineSubsystem = Online::GetSubsystem(GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::ReturnNull));
	if (onlineSubsystem == nullptr)
	{
		FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Login - Invalid or uninitialized OnlineSubsystem"), "Login"), ELogVerbosity::Warning);
		return nullptr;
	}

	auto onlineIdentity = onlineSubsystem->GetIdentityInterface();
	if (!onlineIdentity.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("Login - OnlineIdentity interface is not available for current platform."), ELogVerbosity::Error);
		return nullptr;
	}

	auto callbackProxy = NewObject<UGOGLoginCallbackProxy>();
	callbackProxy->worldContextObject = InWorldContextObject;
	callbackProxy->playerControllerWeakPtr = InPlayerController;
	callbackProxy->authType = AuthType;
	callbackProxy->userID = InUserID;
	callbackProxy->userToken = InUserToken;

	callbackProxy->onlineIdentity = onlineIdentity;

	return callbackProxy;
}

void UGOGLoginCallbackProxy::Activate()
{
	if (!playerControllerWeakPtr.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("Login - invalid PlayerController."), ELogVerbosity::Error);
		OnFailure.Broadcast(playerControllerWeakPtr.Get());
		return;
	}

	if (!onlineIdentity.IsValid())
		return;

	auto* localPlayer = Cast<ULocalPlayer>(playerControllerWeakPtr->Player);
	if (!localPlayer)
	{
		FFrame::KismetExecutionMessage(TEXT("Login - invalid Player object for given PlayerController."), ELogVerbosity::Error);
		OnFailure.Broadcast(playerControllerWeakPtr.Get());
		return;
	}

	const auto ControllerId = localPlayer->GetControllerId();
	if (ControllerId == INVALID_CONTROLLERID)
	{
		FFrame::KismetExecutionMessage(TEXT("Login - invalid PlayerControllerID is invalid."), ELogVerbosity::Error);
		OnFailure.Broadcast(playerControllerWeakPtr.Get());
		return;
	}

	if (onlineIdentity->OnLoginCompleteDelegates[ControllerId].IsBoundToObject(this))
	{
		FFrame::KismetExecutionMessage(TEXT("Login - another authentication process is already in progress."), ELogVerbosity::Error);
		return;
	}

	loginCompleteDelegateHandle = onlineIdentity->AddOnLoginCompleteDelegate_Handle(ControllerId, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
	if (!onlineIdentity->Login(ControllerId, {authType.IsEmpty() ? onlineIdentity->GetAuthType() : authType, userID, userToken}))
	{
		FFrame::KismetExecutionMessage(TEXT("Login failed."), ELogVerbosity::Error);
		OnFailure.Broadcast(playerControllerWeakPtr.Get());
		return;
	}
}

void UGOGLoginCallbackProxy::OnLoginComplete(int32 InLocalUserNum, bool InWasSuccessful, const FUniqueNetId& InUserId, const FString& InErrorVal)
{
	if (onlineIdentity.IsValid())
		onlineIdentity->ClearOnLogoutCompleteDelegate_Handle(InLocalUserNum, loginCompleteDelegateHandle);

	if (InWasSuccessful)
		OnSuccess.Broadcast(playerControllerWeakPtr.Get());
	else
		OnFailure.Broadcast(playerControllerWeakPtr.Get());
}
