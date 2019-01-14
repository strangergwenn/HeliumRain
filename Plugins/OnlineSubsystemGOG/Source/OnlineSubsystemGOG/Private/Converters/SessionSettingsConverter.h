#pragma once

#include "Session/LobbyData.h"

#include "OnlineSessionSettings.h"

namespace SessionSettingsConverter
{

	FLobbyData ToLobbyData(const FSessionSettings& InSessionSettings);

	FSessionSettings FromLobbyDataEntry(const FLobbyDataEntry& InLobbyData);
}
