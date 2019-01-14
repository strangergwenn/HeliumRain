#pragma once

// TODO: consider replacing with "CoreMinimal.h" and "UObject/CoreOnline.h" to reduce compilation time
#include "Core.h"
#include "OnlineSubsystem.h"

#include <galaxy/GalaxyApi.h>
#include <galaxy/GalaxyExceptionHelper.h>

#define TEXT_GOG TEXT("GOG")

#define TEXT_ONLINE_SUBSYSTEM_GOG TEXT("OnlineSubsystemGOG")

#define TEXT_CONFIG_SECTION_GOG TEXT_ONLINE_SUBSYSTEM_GOG
#define TEXT_CONFIG_KEY_ACHIEVEMENTS TEXT("Achievements")

#define STRINGIFY(X) STRINGIFYIMPL(X)
#define STRINGIFYIMPL(X) #X

// The controller number of the associated local user
constexpr int32 LOCAL_USER_NUM{0};

inline void CheckLocalUserNum(int32 InLocalUserNum)
{
	check(InLocalUserNum == LOCAL_USER_NUM && "Only single local player is supported")
}

inline uint64 CharLen(const FString& InString)
{
	return static_cast<uint64>(InString.Len()) * sizeof(FString::ElementType);
}