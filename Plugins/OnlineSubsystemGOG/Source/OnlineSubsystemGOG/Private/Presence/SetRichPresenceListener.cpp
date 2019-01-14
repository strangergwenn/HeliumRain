#include "SetRichPresenceListener.h"

#include "OnlinePresenceGOG.h"

FSetRichPresenceListener::FSetRichPresenceListener(
	FOnlinePresenceGOG& InPresenceInterface,
	FUniqueNetIdGOG InUserID,
	IOnlinePresence::FOnPresenceTaskCompleteDelegate InDelegate)
	: presenceInterface{InPresenceInterface}
	, userID{MoveTemp(InUserID)}
	, onPresenceChangeCompleteDelegate{MoveTemp(InDelegate)}
{
}

void FSetRichPresenceListener::OnRichPresenceChangeSuccess()
{
	UE_LOG_ONLINE(Display, TEXT("FSetRichPresenceListener: OnRichPresenceChangeSuccess()"));

	TriggerOnPresenceChangeCompleteDelegate(true);
}

void FSetRichPresenceListener::OnRichPresenceChangeFailure(galaxy::api::IRichPresenceChangeListener::FailureReason /*InFailureReason*/)
{
	UE_LOG_ONLINE(Warning, TEXT("FSetRichPresenceListener::OnRichPresenceChangeFailure()"));

	TriggerOnPresenceChangeCompleteDelegate(false);
}

void FSetRichPresenceListener::TriggerOnPresenceChangeCompleteDelegate(bool InWasSuccessful)
{
	onPresenceChangeCompleteDelegate.ExecuteIfBound(MoveTemp(userID), InWasSuccessful);
	presenceInterface.FreeListener(MoveTemp(ListenerID));
}
