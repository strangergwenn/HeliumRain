#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"

#include "OnlineFriendsInterface.h"

class FFriendInvitationSendListener
	: public IListenerGOG
	, public galaxy::api::IFriendInvitationSendListener
{
public:

	FFriendInvitationSendListener(class FOnlineFriendsGOG& InFriendsInterface, const FUniqueNetId& InFriendId, FString InListName, FOnSendInviteComplete InOnSendInviteCompleteDelegate);

private:

	void OnFriendInvitationSendSuccess(galaxy::api::GalaxyID InUserID) override;

	void OnFriendInvitationSendFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IFriendInvitationSendListener::FailureReason InFailureReason) override;

	void TriggerOnSendInviteCompleteDelegates(bool InWasSuccessful, FString InErrorMessage = FString{});

	class FOnlineFriendsGOG& friendsInterface;
	FUniqueNetIdGOG friendId;
	FString listName;
	const FOnSendInviteComplete onSendInviteCompleteDelegate;
};
