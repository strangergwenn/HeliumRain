#include "NamedVariantDataConverter.h"

#include "OnlineKeyValuePair.h"

#include "OnlineSubsystem.h"

namespace NamedVariantDataConverter
{

	namespace
	{

		constexpr auto CONVERTED_NAME_FORMAT = TEXT("%c_%s");
		constexpr uint32 NAME_TYPE_PREFIX_LENGTH = 2;
		constexpr uint32 NAME_TYPE_PREFIX_POSITION = 0;

		enum InternalDataType : TCHAR
		{
			INVALID_TYPE = TCHAR('\0'),
			Empty = TCHAR('e'),
			Int32 = TCHAR('i'),
			UInt32 = TCHAR('u'),
			Int64 = TCHAR('I'),
			UInt64 = TCHAR('U'),
			Double = TCHAR('d'),
			String = TCHAR('s'),
			Float = TCHAR('f'),
			Bool = TCHAR('b')
		};

		InternalDataType GetKeyPrefixFromType(EOnlineKeyValuePairDataType::Type InType)
		{
			switch (InType)
			{
				case EOnlineKeyValuePairDataType::Int32:
					return InternalDataType::Int32;
				case EOnlineKeyValuePairDataType::UInt32:
					return InternalDataType::UInt32;
				case EOnlineKeyValuePairDataType::Int64:
					return InternalDataType::Int64;
				case EOnlineKeyValuePairDataType::UInt64:
					return InternalDataType::UInt64;
				case EOnlineKeyValuePairDataType::Double:
					return InternalDataType::Double;
				case EOnlineKeyValuePairDataType::String:
					return InternalDataType::String;
				case EOnlineKeyValuePairDataType::Float:
					return InternalDataType::Float;
				case EOnlineKeyValuePairDataType::Bool:
					return InternalDataType::Bool;
				case EOnlineKeyValuePairDataType::Empty:
				case EOnlineKeyValuePairDataType::Blob:
				default:
					checkf(false, TEXT("Unsupported VariantData type '%s'"), EOnlineKeyValuePairDataType::ToString(InType));
					return InternalDataType::INVALID_TYPE;
			}
		}

		auto& GetTypeFromLobbyDataKey(const FLobbyDataKey& InKey)
		{
			return InKey[NAME_TYPE_PREFIX_POSITION];
		}

		auto GetNameFromLobbyDataKey(const FLobbyDataKey& InKey)
		{
			return InKey.Mid(NAME_TYPE_PREFIX_POSITION + NAME_TYPE_PREFIX_LENGTH);
		}

	}

	FLobbyDataEntry ToLobbyDataEntry(const FName& InName, const FVariantData& InData)
	{
		if (InData.GetType() == EOnlineKeyValuePairDataType::Empty)
			return{};

		if (InData.GetType() == EOnlineKeyValuePairDataType::Blob)
		{
			UE_LOG_ONLINE(Error, TEXT("Cannot serialize Blob data"));
			return{};
		}

		return MakeTuple(
			FString::Printf(CONVERTED_NAME_FORMAT,
				static_cast<TCHAR>(GetKeyPrefixFromType(InData.GetType())),
				*InName.ToString()),
			InData.ToString());
	}

	TPair<FName, FVariantData> FromLobbyDataEntry(const FLobbyDataEntry& InLobbyDataEntry)
	{
		using NamedVariantPair = TPair<FName, FVariantData>;

		const auto& name = InLobbyDataEntry.Key;
		const auto& value = InLobbyDataEntry.Value;

		const auto& type = GetTypeFromLobbyDataKey(name);
		const auto& convertedName = GetNameFromLobbyDataKey(name);

		constexpr int32 decimalBase = 10;
		switch (type)
		{
			case InternalDataType::Empty:
				return{};

			case InternalDataType::Int32:
				return NamedVariantPair{*convertedName, FCString::Atoi(*value)};

			case InternalDataType::UInt32:
				return NamedVariantPair{*convertedName, static_cast<uint32>(FCString::Strtoui64(*value, nullptr, decimalBase))};

			case InternalDataType::Int64:
				return NamedVariantPair{*convertedName, FCString::Atoi64(*value)};

			case InternalDataType::UInt64:
				return NamedVariantPair{*convertedName, FCString::Strtoui64(*value, nullptr, decimalBase)};

			case InternalDataType::Double:
				return NamedVariantPair{*convertedName, FCString::Atod(*value)};

			case InternalDataType::String:
				return NamedVariantPair{*convertedName, value};

			case InternalDataType::Float:
				return NamedVariantPair{*convertedName, FCString::Atof(*value)};

			case InternalDataType::Bool:
				return NamedVariantPair{*convertedName, value.Equals(TEXT("true"), ESearchCase::IgnoreCase) ? true : false};
		}

		UE_LOG_ONLINE(Error, TEXT("Cannot de-serialize lobby value: dataType=%d, dataSize=%d"), type, value.Len());

		return{};
	}

}
