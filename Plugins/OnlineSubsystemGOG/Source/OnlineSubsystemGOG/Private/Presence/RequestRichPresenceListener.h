#pragma once

#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "OnlinePresenceInterface.h"

class FRequestRichPresenceListener
	: public IListenerGOG
	, public galaxy::api::IRichPresenceRetrieveListener
{
public:

	FRequestRichPresenceListener(
		class FOnlinePresenceGOG& InPresenceInterface,
		class FUniqueNetIdGOG InUserID,
		IOnlinePresence::FOnPresenceTaskCompleteDelegate InDelegate);

private:

	void OnRichPresenceRetrieveSuccess(galaxy::api::GalaxyID InUserID) override;

	void OnRichPresenceRetrieveFailure(galaxy::api::GalaxyID InUserID, galaxy::api::IRichPresenceRetrieveListener::FailureReason InFailureReason) override;

	void TriggerOnRichPresenceRetrieveCompleteDelegate(bool InWasSuccessful);

	FOnlinePresenceGOG& presenceInterface;
	class FUniqueNetIdGOG userID;
	IOnlinePresence::FOnPresenceTaskCompleteDelegate onPresenceRetrieveCompleteDelegate;
};
