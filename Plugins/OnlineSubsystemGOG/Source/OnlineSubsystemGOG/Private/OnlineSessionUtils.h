#pragma once

#include "Types/UniqueNetIdGOG.h"
#include "OnlineSessionSettings.h"

namespace OnlineSessionUtils
{
	bool ShouldAdvertiseViaPresence(const FOnlineSessionSettings& InSettings);

	bool SetLobbyData(const FUniqueNetIdGOG& InSessionID, const FOnlineSessionSettings& InOutSessionSettings);

	bool DeleteLobbyData(const FUniqueNetIdGOG& InSessionID, const TSet<FString>& InOutSessionSettings);

	bool GetSessionOpenConnections(const FUniqueNetIdGOG& InLobbyID, FOnlineSession& InOutOnlineSession);

	bool Fill(const FUniqueNetIdGOG& InLobbyID, FOnlineSessionSettings& InOutOnlineSessionSettings);

	bool Fill(const FUniqueNetIdGOG& InLobbyID, FOnlineSession& InOutOnlineSession);

	bool Fill(const FUniqueNetIdGOG& InLobbyID, FOnlineSessionSearchResult& InOutOnlineSessionSearchResult);

}
