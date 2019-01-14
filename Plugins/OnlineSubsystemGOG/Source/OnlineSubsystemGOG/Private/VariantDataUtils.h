#pragma once

#include "OnlineKeyValuePair.h"

template<typename IntType, typename = std::enable_if_t<std::is_integral<IntType>::value>>
inline bool IsInInt32Range(const IntType& InValue)
{
	using min_t = int64;
	using max_t = uint64;

	return static_cast<min_t>(InValue) >= static_cast<min_t>(TNumericLimits<int32>::Lowest())
		&& static_cast<max_t>(InValue) <= static_cast<max_t>(TNumericLimits<int32>::Max());
}

template<typename IntType, typename = std::enable_if_t<std::is_integral<IntType>::value>>
inline bool GetInt32ValueFromType(const FVariantData& InVariantData, int32& OutValue)
{
	IntType value;
	InVariantData.GetValue(value);

	if(!IsInInt32Range(value))
		return false;

	OutValue = value;
	return true;
}

inline bool SafeGetInt32Value(const FVariantData& InVariantData, int32& OutValue)
{
	switch (InVariantData.GetType())
	{
		case EOnlineKeyValuePairDataType::Int32:
		{
			InVariantData.GetValue(OutValue);
			return true;
		}

		case EOnlineKeyValuePairDataType::UInt32:
			return GetInt32ValueFromType<uint32>(InVariantData, OutValue);

		case EOnlineKeyValuePairDataType::Int64:
			return GetInt32ValueFromType<int64>(InVariantData, OutValue);

		case EOnlineKeyValuePairDataType::UInt64:
			return GetInt32ValueFromType<uint64>(InVariantData, OutValue);
	}

	return false;
}