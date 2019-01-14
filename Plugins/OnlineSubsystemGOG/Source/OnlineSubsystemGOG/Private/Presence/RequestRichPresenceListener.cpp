#include "RequestRichPresenceListener.h"

#include "OnlinePresenceGOG.h"

FRequestRichPresenceListener::FRequestRichPresenceListener(FOnlinePresenceGOG& InPresenceInterface, FUniqueNetIdGOG InUserID, IOnlinePresence::FOnPresenceTaskCompleteDelegate InDelegate)
	: presenceInterface{InPresenceInterface}
	, userID{MoveTemp(InUserID)}
	, onPresenceRetrieveCompleteDelegate{MoveTemp(InDelegate)}
{
}

void FRequestRichPresenceListener::OnRichPresenceRetrieveSuccess(galaxy::api::GalaxyID InUserID)
{
	UE_LOG_ONLINE(Display, TEXT("FRequestRichPresenceListener: OnRichPresenceRetrieveSuccess()"));
	checkf(userID == InUserID, TEXT("Got callback for a user different than expected. This shall never happened. Please report this to the GalaxySDK team"));
	TriggerOnRichPresenceRetrieveCompleteDelegate(true);
}

void FRequestRichPresenceListener::OnRichPresenceRetrieveFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IRichPresenceRetrieveListener::FailureReason /*InFailureReason*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FRequestRichPresenceListener::OnRichPresenceRetrieveFailure()"));
	checkf(userID == InUserID, TEXT("Got callback for a user different than expected. This shall never happened. Please report this to the GalaxySDK team"));
	TriggerOnRichPresenceRetrieveCompleteDelegate(false);
}

void FRequestRichPresenceListener::TriggerOnRichPresenceRetrieveCompleteDelegate(bool InWasSuccessful)
{
	onPresenceRetrieveCompleteDelegate.ExecuteIfBound(MoveTemp(userID), InWasSuccessful);
	presenceInterface.FreeListener(MoveTemp(ListenerID));
}
