#include "FriendInvitationSendListener.h"

#include "OnlineFriendsGOG.h"

namespace
{

	FString ToString(galaxy::api::IFriendInvitationSendListener::FailureReason InFailureReason)
	{
		switch (InFailureReason)
		{
			case galaxy::api::IFriendInvitationSendListener::FAILURE_REASON_USER_DOES_NOT_EXIST:
				return TEXT("User does not exist.");
			case galaxy::api::IFriendInvitationSendListener::FAILURE_REASON_USER_ALREADY_INVITED:
				return TEXT("Friend invitation already sent to the user.");
			case galaxy::api::IFriendInvitationSendListener::FAILURE_REASON_USER_ALREADY_FRIEND:
				return TEXT("User already on the friend list.");
			case galaxy::api::IFriendInvitationSendListener::FAILURE_REASON_UNDEFINED:
				break;
		}

		return TEXT("Unspecified error.");
	}

}

FFriendInvitationSendListener::FFriendInvitationSendListener(
	FOnlineFriendsGOG& InFriendsInterface,
	const FUniqueNetId& InFriendId,
	FString InListName,
	FOnSendInviteComplete InOnSendInviteCompleteDelegate)
	: friendsInterface{InFriendsInterface}
	, friendId{InFriendId}
	, listName{MoveTemp(InListName)}
	, onSendInviteCompleteDelegate{MoveTemp(InOnSendInviteCompleteDelegate)}
{
}

void FFriendInvitationSendListener::OnFriendInvitationSendSuccess(galaxy::api::GalaxyID InUserID)
{
	UE_LOG_ONLINE(Display, TEXT("FFriendInvitationSendListener::OnFriendInvitationSendSuccess()"));
	check(InUserID == friendId);
	TriggerOnSendInviteCompleteDelegates(true);
}

void FFriendInvitationSendListener::OnFriendInvitationSendFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IFriendInvitationSendListener::FailureReason InFailureReason)
{
	check(InUserID == friendId);

	auto errorMsg = ToString(InFailureReason);
	UE_LOG_ONLINE(Warning, TEXT("FFriendInvitationSendListener::OnFriendInvitationSendFailure(): %s"), *errorMsg);

	TriggerOnSendInviteCompleteDelegates(false, MoveTemp(errorMsg));
}

void FFriendInvitationSendListener::TriggerOnSendInviteCompleteDelegates(bool InWasSuccessful, FString InErrorMessage)
{
	onSendInviteCompleteDelegate.ExecuteIfBound(LOCAL_USER_NUM, InWasSuccessful, MoveTemp(friendId), MoveTemp(listName), MoveTemp(InErrorMessage));
	friendsInterface.FreeListener(MoveTemp(ListenerID));
}
