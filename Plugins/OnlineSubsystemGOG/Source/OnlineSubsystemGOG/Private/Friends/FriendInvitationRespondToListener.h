#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "ListenerManager.h"

#include "OnlineFriendsInterface.h"

namespace
{

	FString ToString(galaxy::api::IFriendInvitationRespondToListener::FailureReason InFailureReason)
	{
		switch (InFailureReason)
		{
			case galaxy::api::IFriendInvitationRespondToListener::FAILURE_REASON_USER_DOES_NOT_EXIST:
				return TEXT("User does not exist.");

			case galaxy::api::IFriendInvitationRespondToListener::FAILURE_REASON_FRIEND_INVITATION_DOES_NOT_EXIST:
				return TEXT("Friend invitation does not exist.");

			case galaxy::api::IFriendInvitationRespondToListener::FAILURE_REASON_USER_ALREADY_FRIEND:
				return TEXT("User already on the friend list.");

			case galaxy::api::IFriendInvitationRespondToListener::FAILURE_REASON_UNDEFINED:
				break;
		}

		return TEXT("Unspecified error.");
	}

}

template<bool IsInvitationAccepted>
class FFriendInvitationRespondToListener
	: public IListenerGOG
	, public galaxy::api::IFriendInvitationRespondToListener
{
public:

	FFriendInvitationRespondToListener(
		class FOnlineFriendsGOG& InFriendsInterface,
		const FUniqueNetId& InFriendId,
		FString InListName,
		FOnAcceptInviteComplete InOnAcceptInviteCompleteDelegate = FOnAcceptInviteComplete())
		: friendsInterface{InFriendsInterface}
		, listenerManager{InFriendsInterface}
		, friendId{InFriendId}
		, listName{MoveTemp(InListName)}
		, onAcceptInviteCompleteDelegate{MoveTemp(InOnAcceptInviteCompleteDelegate)}
	{
	}

private:

	void OnFriendInvitationRespondToSuccess(galaxy::api::GalaxyID InUserID, bool InAccept) override
	{
		UE_LOG_ONLINE(Display, TEXT("FFriendInvitationRespondToListener::OnFriendInvitationRespondToSuccess()"));
		check(InUserID == friendId);
		check(InAccept == IsInvitationAccepted);
		TriggerOnInvitationRespondCompleteDelegates(true);
	}

	void OnFriendInvitationRespondToFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IFriendInvitationRespondToListener::FailureReason InFailureReason) override
	{
		check(InUserID == friendId);

		auto errorMsg = ToString(InFailureReason);
		UE_LOG_ONLINE(Warning, TEXT("FFriendInvitationRespondToListener::OnFriendInvitationRespondToFailure(%s): %s"),
			IsInvitationAccepted ? TEXT("accept") : TEXT("reject"),
			*errorMsg);

		TriggerOnInvitationRespondCompleteDelegates(false, MoveTemp(errorMsg));
	}

	void TriggerOnInvitationRespondCompleteDelegates(bool InWasSuccessful, FString InErrorMessage = FString{});

	IOnlineFriends& friendsInterface;
	FListenerManager& listenerManager;
	FUniqueNetIdGOG friendId;
	FString listName;
	const FOnAcceptInviteComplete onAcceptInviteCompleteDelegate;
};

template<>
void FFriendInvitationRespondToListener<true>::TriggerOnInvitationRespondCompleteDelegates(bool InWasSuccessful, FString InErrorMessage)
{
	onAcceptInviteCompleteDelegate.ExecuteIfBound(LOCAL_USER_NUM, InWasSuccessful, MoveTemp(friendId), MoveTemp(listName), MoveTemp(InErrorMessage));
	listenerManager.FreeListener(MoveTemp(ListenerID));
}

template<>
void FFriendInvitationRespondToListener<false>::TriggerOnInvitationRespondCompleteDelegates(bool InWasSuccessful, FString InErrorMessage)
{
	friendsInterface.TriggerOnRejectInviteCompleteDelegates(LOCAL_USER_NUM, InWasSuccessful, MoveTemp(friendId), MoveTemp(listName), MoveTemp(InErrorMessage));
	listenerManager.FreeListener(MoveTemp(ListenerID));
}

using FFriendInvitationAcceptListener = FFriendInvitationRespondToListener<true>;
using FFriendInvitationRejectListener = FFriendInvitationRespondToListener<false>;
