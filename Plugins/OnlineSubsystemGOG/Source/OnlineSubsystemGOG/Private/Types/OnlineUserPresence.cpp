#include "OnlineUserPresence.h"

#include <array>

namespace
{

	constexpr uint32_t MAX_RICH_PRESENCE_KEY_LENGTH = 4096;
	constexpr uint32_t MAX_RICH_PRESENCE_VALUE_LENGTH = 4096;

	inline bool IsEqual(const FCStringAnsi::CharType* InString1, const FCStringAnsi::CharType* InString2)
	{
		return 0 == FCStringAnsi::Strcmp(InString1, InString2);
	}

	bool GetPersonaState(const FUniqueNetIdGOG& InUserID, FOnlineUserPresence& InOutUserPresence)
	{
		auto userState = galaxy::api::Friends()->GetFriendPersonaState(InUserID);
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to get user state: userID='%s'; %s: %s"),
				*InUserID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			return false;
		}

		switch (userState)
		{
			case galaxy::api::PERSONA_STATE_OFFLINE:
			{
				InOutUserPresence.bIsOnline = false;
				InOutUserPresence.Status.State = EOnlinePresenceState::Offline;
				return true;
			}
			case galaxy::api::PERSONA_STATE_ONLINE:
			{
				InOutUserPresence.bIsOnline = true;
				InOutUserPresence.Status.State = EOnlinePresenceState::Online;
				return true;
			}
		}

		checkf(false, TEXT("Invalid user state: %u"), userState);
		return false;
	}

	bool GetConnectStatus(FString InPresenceConnect, FOnlineUserPresence& InOutonlineUserPresence)
	{
		InOutonlineUserPresence.bIsPlaying = true;
		InOutonlineUserPresence.bIsPlayingThisGame = true;

		FUniqueNetIdGOG lobbyID{InPresenceConnect};
		if (!lobbyID.IsValid() || !lobbyID.IsLobby())
		{
			UE_LOG_ONLINE(Warning, TEXT("Cannot parse SessionID from connect rich presence: presenceConnect='%s'"), *InPresenceConnect);
			return false;
		}

		InOutonlineUserPresence.SessionId = MakeShared<decltype(lobbyID)>(MoveTemp(lobbyID));
		InOutonlineUserPresence.bIsJoinable = true;
		return true;
	}

	bool ParseRichPresence(const char* InKey, FString InValue, FOnlineUserPresence& InOutOnlineUserPresence)
	{
		if (IsEqual(InKey, OnlineUserPresence::RICH_PRESENCE_STATUS))
		{
			InOutOnlineUserPresence.Status.StatusStr = MoveTemp(InValue);
			return true;
		}

		if (IsEqual(InKey, OnlineUserPresence::RICH_PRESENCE_CONNECT))
			return GetConnectStatus(MoveTemp(InValue), InOutOnlineUserPresence);

		// TODO: copy rest of the keys unconditionally once we allow custom presence data
		if (IsEqual(InKey, OnlineUserPresence::RICH_PRESENCE_METADATA))
		{
			InOutOnlineUserPresence.Status.Properties.Emplace(DefaultPresenceKey, MoveTemp(InValue));
			return true;
		}

		UE_LOG_ONLINE(Warning, TEXT("Unsupported rich presence data: key='%s': valueLength=%u "), UTF8_TO_TCHAR(InKey), InValue.Len());
		return false;
	}

}

namespace OnlineUserPresence
{

	bool Fill(const FUniqueNetIdGOG& InUserID, FOnlineUserPresence& InOutOnlineUserPresence)
	{
		if (!GetPersonaState(InUserID, InOutOnlineUserPresence))
			return false;

		std::array<char, MAX_RICH_PRESENCE_KEY_LENGTH> keyBuffer;
		std::array<char, MAX_RICH_PRESENCE_VALUE_LENGTH> valueBuffer;

		auto presenceCount = galaxy::api::Friends()->GetRichPresenceCount(InUserID);
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed get rich presence count: userID='%s'; %s: %s"),
				*InUserID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			return false;
		}

		// TODO: Uncomment after 1.128 or newer is released
		//	if (galaxy::api::Friends()->IsUserInTheSameGame(InUserID))
		//	{
		//		bIsPlaying = true;
		//		bIsPlayingThisGame = true;
		//	}

		InOutOnlineUserPresence.Status.Properties.Reserve(presenceCount);
		for (decltype(presenceCount) idx{0}; idx < presenceCount; ++idx)
		{
			galaxy::api::Friends()->GetRichPresenceByIndex(
				idx,
				keyBuffer.data(),
				MAX_RICH_PRESENCE_KEY_LENGTH,
				valueBuffer.data(),
				MAX_RICH_PRESENCE_VALUE_LENGTH,
				InUserID);

			err = galaxy::api::GetError();
			if (err)
			{
				UE_LOG_ONLINE(Warning, TEXT("Failed get rich presence: userID='%s'; %s: %s"),
					*InUserID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
				return false;
			}

			if (!ParseRichPresence(keyBuffer.data(), FString{UTF8_TO_TCHAR(valueBuffer.data())}, InOutOnlineUserPresence))
				return false;
		}

		return true;
	}

}