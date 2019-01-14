#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"

#include "OnlineFriendsInterface.h"

class FFriendDeleteListener
	: public IListenerGOG
	, public galaxy::api::IFriendDeleteListener
{
public:

	FFriendDeleteListener(class FOnlineFriendsGOG& InFriendsInterface, const FUniqueNetId& InFriendId, FString InListName);

private:

	void OnFriendDeleteSuccess(galaxy::api::GalaxyID InUserID) override;

	void OnFriendDeleteFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IFriendDeleteListener::FailureReason InFailureReason) override;

	void TriggerOnDeleteFriendCompleteDelegates(bool InWasSuccessful, FString InErrorMessage = FString{});

	class FOnlineFriendsGOG& friendsInterface;
	FUniqueNetIdGOG friendId;
	FString listName;
};
