#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetworkingGOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTrafficGOG, Log, All);

#define UE_LOG_NETWORKING(Verbosity, Format, ...) UE_LOG(LogNetworkingGOG, Verbosity, Format,  ##__VA_ARGS__)
#define UE_LOG_TRAFFIC(Verbosity, Format, ...) UE_LOG(LogTrafficGOG, Verbosity, Format,  ##__VA_ARGS__)
