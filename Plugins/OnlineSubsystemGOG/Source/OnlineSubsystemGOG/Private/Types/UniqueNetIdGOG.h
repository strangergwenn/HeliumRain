#pragma once

#include "CommonGOG.h"

class FUniqueNetIdGOG : public FUniqueNetId
{
public:

	explicit FUniqueNetIdGOG(uint64 InID);

	explicit FUniqueNetIdGOG(const FString& InStr);

	explicit FUniqueNetIdGOG(const FUniqueNetIdGOG& InUniqueNetIdGOG);

	FUniqueNetIdGOG(const uint8* InBytes, int32 InSize);

	FUniqueNetIdGOG(const galaxy::api::GalaxyID& InGalaxyID);

	FUniqueNetIdGOG(const FUniqueNetId& InUniqueNetId);

	const uint8* GetBytes() const override;

	int32 GetSize() const override;

	bool IsValid() const override;

	bool IsUser() const;

	bool IsLobby() const;

	FString ToString() const override;

	FString ToDebugString() const override;

	friend uint32 GetTypeHash(const FUniqueNetIdGOG& InUniqueNetIdGOG);

	operator galaxy::api::GalaxyID() const;

	FUniqueNetIdGOG& operator=(const galaxy::api::GalaxyID& InGalaxyID);

	bool operator==(const galaxy::api::GalaxyID& InGalaxyID) const;

PACKAGE_SCOPE:

	FUniqueNetIdGOG() = default;

private:

	uint64 id;
};
