#pragma once

#include "OnlinePresenceInterface.h"
#include "UniqueNetIdGOG.h"

namespace OnlineUserPresence
{

	// Special presence statuses. See galaxy::api::IFriends::SetRichPresence() for details
	constexpr const char* RICH_PRESENCE_STATUS = "status";
	constexpr const char* RICH_PRESENCE_METADATA = "metadata";
	constexpr const char* RICH_PRESENCE_CONNECT = "connect";

	bool Fill(const FUniqueNetIdGOG& InUserID, FOnlineUserPresence& InOutOnlineUserPresence);

};
