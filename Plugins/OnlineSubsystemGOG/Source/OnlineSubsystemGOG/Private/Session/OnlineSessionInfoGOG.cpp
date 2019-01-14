#include "OnlineSessionInfoGOG.h"

FOnlineSessionInfoGOG::FOnlineSessionInfoGOG(const FUniqueNetIdGOG& InSessionId)
	: SessionID{InSessionId}
{
}

bool FOnlineSessionInfoGOG::operator==(const FOnlineSessionInfoGOG& InOther) const
{
	return false;
}

bool FOnlineSessionInfoGOG::IsValid() const
{
	return SessionID.IsValid() && SessionID.IsLobby();
}

FString FOnlineSessionInfoGOG::ToString() const
{
	return FString::Printf(TEXT("Session: %s"), *SessionID.ToString());
}

FString FOnlineSessionInfoGOG::ToDebugString() const
{
	return ToString();
}

const FUniqueNetId& FOnlineSessionInfoGOG::GetSessionId() const
{
	return SessionID;
}

const uint8* FOnlineSessionInfoGOG::GetBytes() const
{
	return nullptr;
}

int32 FOnlineSessionInfoGOG::GetSize() const
{
	return SessionID.GetSize();
}