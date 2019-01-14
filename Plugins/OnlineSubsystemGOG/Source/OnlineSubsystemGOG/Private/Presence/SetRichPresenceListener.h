#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "OnlinePresenceInterface.h"

class FSetRichPresenceListener
	: public IListenerGOG
	, public galaxy::api::IRichPresenceChangeListener
{
public:

	FSetRichPresenceListener(
		class FOnlinePresenceGOG& InPresenceInterface,
		FUniqueNetIdGOG InUserID,
		IOnlinePresence::FOnPresenceTaskCompleteDelegate InDelegate);

private:

	void OnRichPresenceChangeSuccess() override;

	void OnRichPresenceChangeFailure(galaxy::api::IRichPresenceChangeListener::FailureReason InFailureReason) override;

	void TriggerOnPresenceChangeCompleteDelegate(bool InWasSuccessful);

	FOnlinePresenceGOG& presenceInterface;
	class FUniqueNetIdGOG userID;
	IOnlinePresence::FOnPresenceTaskCompleteDelegate onPresenceChangeCompleteDelegate;
};
