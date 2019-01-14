#pragma once

#include "Session/LobbyData.h"

#include "OnlineSessionSettings.h"

#include <bitset>
#include <utility>


namespace OnlineSessionSettingsConverter
{

	FLobbyData ToLobbyData(const FOnlineSessionSettings& InSessionSettings);

	FOnlineSessionSettings FromLobbyData(const FLobbyData& InLobbyData);

}
