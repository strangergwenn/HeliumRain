#pragma once

#include "CommonGOG.h"

#include "Types/UniqueNetIdGOG.h"

class FOnlineSessionInfoGOG : public FOnlineSessionInfo
{
public:

	FOnlineSessionInfoGOG(const FUniqueNetIdGOG& InSessionId);

	bool operator==(const FOnlineSessionInfoGOG& InOther) const;

	bool IsValid() const override;

	FString ToString() const override;

	FString ToDebugString() const override;

	const FUniqueNetId& GetSessionId() const override;

	const uint8* GetBytes() const override;

	int32 GetSize() const override;

PACKAGE_SCOPE:

	FUniqueNetIdGOG SessionID;
};
