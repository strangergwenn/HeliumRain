#include "RequestFriendListListener.h"

#include "OnlineFriendsGOG.h"

FRequestFriendListListener::FRequestFriendListListener(FOnlineFriendsGOG& InFriendsInterface, FString InListName, FOnReadFriendsListComplete InOnReadFriendsListCompleteDelegate)
	: friendsInterface{InFriendsInterface}
	, listName{MoveTemp(InListName)}
	, onReadFriendsListCompleteDelegate{MoveTemp(InOnReadFriendsListCompleteDelegate)}
{
}

void FRequestFriendListListener::OnFriendListRetrieveSuccess()
{
	UE_LOG_ONLINE(Display, TEXT("FRequestFriendListListener: OnFriendListRetrieveSuccess()"));
	TriggerOnReadFriendsListCompleteDelegates(true);
}

void FRequestFriendListListener::OnFriendListRetrieveFailure(galaxy::api::IFriendListListener::FailureReason /*InFailureReason*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FRequestFriendListListener::OnFriendListRetrieveFailure()"));
	TriggerOnReadFriendsListCompleteDelegates(false, TEXT("Undefined error"));
}

void FRequestFriendListListener::TriggerOnReadFriendsListCompleteDelegates(bool InWasSuccessful, FString InErrorMessage)
{
	onReadFriendsListCompleteDelegate.ExecuteIfBound(LOCAL_USER_NUM, InWasSuccessful, MoveTemp(listName), MoveTemp(InErrorMessage));
	friendsInterface.FreeListener(MoveTemp(ListenerID));
}
