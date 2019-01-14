#include "UserInfoUtils.h"

#include <array>

namespace UserInfoUtils
{

	namespace
	{

		constexpr uint32_t MAX_USERNAME_LENGHT = 1024;
		constexpr uint32_t MAX_USERDATA_KEY_LENGHT = 1024;
		constexpr uint32_t MAX_USERDATA_VALUE_LENGHT = 1024;

		constexpr uint32_t MAX_AVATAR_URL_LENGHT = 2048;

		bool GetUserData(const galaxy::api::GalaxyID& InUserID, UserAttributesMap& InUserAttributes)
		{
			if (!galaxy::api::User()->IsUserDataAvailable(InUserID))
				return true;

			auto userDataCount = galaxy::api::User()->GetUserDataCount(InUserID);
			auto err = galaxy::api::GetError();
			if (err)
			{
				UE_LOG_ONLINE(Warning, TEXT("Failed to get user data count: userID=%llu; %s: %s"), InUserID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
				return false;
			}

			if (!userDataCount)
				return true;

			InUserAttributes.Reserve(userDataCount);

			std::array<char, MAX_USERDATA_KEY_LENGHT> keyBuffer;
			std::array<char, MAX_USERDATA_VALUE_LENGHT> valueBuffer;

			for (decltype(userDataCount) i = 0; i < userDataCount; ++i)
			{
				galaxy::api::User()->GetUserDataByIndex(i, keyBuffer.data(), keyBuffer.size(), valueBuffer.data(), valueBuffer.size(), InUserID);
				err = galaxy::api::GetError();
				if (err)
				{
					UE_LOG_ONLINE(Warning, TEXT("Failed to get user name: userID=%llu; %s: %s"), InUserID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
					return false;
				}

				InUserAttributes.Emplace(UTF8_TO_TCHAR(keyBuffer.data()), UTF8_TO_TCHAR(valueBuffer.data()));
			}

			return true;
		}

		bool GetPlayerAvatarUrl(const galaxy::api::GalaxyID& InUserID, galaxy::api::AvatarType InAvatarType, FString& OutAvatarlURL)
		{
			std::array<char, MAX_AVATAR_URL_LENGHT> avatarUrlBuffer;

			galaxy::api::Friends()->GetFriendAvatarUrlCopy(InUserID, InAvatarType, avatarUrlBuffer.data(), avatarUrlBuffer.size());
			auto err = galaxy::api::GetError();
			if (err)
			{
				UE_LOG_ONLINE(Warning, TEXT("Failed to get avatar url: userID='%llu'; %s: %s"),
					InUserID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
				return false;
			}

			OutAvatarlURL = UTF8_TO_TCHAR(avatarUrlBuffer.data());
			return true;
		}

	}

	bool GetOwnPlayerNickname(FString& OutPlayerNickname)
	{
		std::array<char, MAX_USERNAME_LENGHT> usernameBuffer;

		galaxy::api::Friends()->GetPersonaNameCopy(usernameBuffer.data(), usernameBuffer.size());
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to get players user name: %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			return false;
		}

		OutPlayerNickname = FString{UTF8_TO_TCHAR(usernameBuffer.data())};
		return true;
	}

	bool GetPlayerNickname(const FUniqueNetIdGOG& InUserId, FString& OutPlayerNickname)
	{
		if (!InUserId.IsValid())
		{
			UE_LOG_ONLINE(Warning, TEXT("Invalid UserID"));
			return false;
		}

		std::array<char, MAX_USERNAME_LENGHT> usernameBuffer;

		galaxy::api::Friends()->GetFriendPersonaNameCopy(InUserId, usernameBuffer.data(), usernameBuffer.size());
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to get user name: userID='%s'; %s: %s"),
				*InUserId.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			return false;
		}

		OutPlayerNickname = UTF8_TO_TCHAR(usernameBuffer.data());
		return true;
	}

	bool IsUserInfoAvailable(const FUniqueNetIdGOG& InUserId)
	{
		if (!InUserId.IsValid())
		{
			UE_LOG_ONLINE(Warning, TEXT("Invalid UserID"));
			return false;
		}

		return galaxy::api::Friends()->IsUserInformationAvailable(InUserId);
	}

	FUniqueNetIdGOG GetOwnUserID()
	{
		auto galaxyID = galaxy::api::User()->GetGalaxyID();
		auto err = galaxy::api::GetError();
		if (err)
			UE_LOG_ONLINE(Warning, TEXT("Failed to get own Galaxy UserID: %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		return FUniqueNetIdGOG{galaxyID};
	}

	bool GetUserAttributes(const FUniqueNetIdGOG& InUserId, UserAttributesMap& OutUserAttributes)
	{
		if (!InUserId.IsValid())
		{
			UE_LOG_ONLINE(Warning, TEXT("Invalid UserID"));
			return false;
		}

		FString avatarURL;
		if (!GetPlayerAvatarUrl(InUserId, galaxy::api::AVATAR_TYPE_SMALL, avatarURL))
			return false;

		OutUserAttributes.Emplace(AVATAR_SMALL_KEY, avatarURL);

		if (!GetPlayerAvatarUrl(InUserId, galaxy::api::AVATAR_TYPE_MEDIUM, avatarURL))
			return false;

		OutUserAttributes.Emplace(AVATAR_SMALL_KEY, avatarURL);


		if (!GetPlayerAvatarUrl(InUserId, galaxy::api::AVATAR_TYPE_LARGE, avatarURL))
			return false;

		OutUserAttributes.Emplace(AVATAR_SMALL_KEY, avatarURL);

		return GetUserData(InUserId, OutUserAttributes);
	}

}
