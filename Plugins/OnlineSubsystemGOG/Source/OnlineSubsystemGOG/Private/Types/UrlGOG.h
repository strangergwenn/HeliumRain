#pragma once

#include "Types/UniqueNetIdGOG.h"

#include "Engine/EngineBaseTypes.h"

class FUrlGOG : public FURL
{
public:

	explicit FUrlGOG(const FString& InRemoteAddress)
		: FURL{nullptr, *InRemoteAddress, ETravelType::TRAVEL_Absolute}
	{
	}

	explicit FUrlGOG(const FUniqueNetIdGOG& InRemoteID)
		: FUrlGOG{InRemoteID.ToString().Append(TEXT(".galaxy"))}
	{
	}

	bool operator==(const FURL& InOther) const
	{
		return FURL::operator==(InOther);
	}

	bool operator!=(const FURL& InOther) const
	{
		return !(*this == InOther);
	}

};
