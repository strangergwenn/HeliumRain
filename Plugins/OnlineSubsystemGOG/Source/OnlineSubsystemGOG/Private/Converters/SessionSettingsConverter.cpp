#include "SessionSettingsConverter.h"

#include "NamedVariantDataConverter.h"

namespace SessionSettingsConverter
{

	FLobbyData ToLobbyData(const FSessionSettings& InSessionSettings)
	{
		FLobbyData convertedSettings;

		for (const auto& setting : InSessionSettings)
		{
			const auto& value = setting.Value;
			if ((value.AdvertisementType == EOnlineDataAdvertisementType::ViaOnlineService
				|| value.AdvertisementType == EOnlineDataAdvertisementType::ViaOnlineServiceAndPing)
				&& value.Data.GetType() != EOnlineKeyValuePairDataType::Empty)
			{
				auto convertedData = NamedVariantDataConverter::ToLobbyDataEntry(setting.Key, value.Data);
				convertedSettings.Add(convertedData.Key, convertedData.Value);
			}
		}

		return convertedSettings;
	}

	FSessionSettings FromLobbyDataEntry(const FLobbyDataEntry& InLobbyData)
	{
		auto convertedData = NamedVariantDataConverter::FromLobbyDataEntry(InLobbyData);
		FSessionSettings ret;
		ret.Emplace(convertedData.Key, FOnlineSessionSetting{convertedData.Value, EOnlineDataAdvertisementType::ViaOnlineService});
		return ret;
	}

}
