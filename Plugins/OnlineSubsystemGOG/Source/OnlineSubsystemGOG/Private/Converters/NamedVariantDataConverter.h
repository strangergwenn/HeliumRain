#pragma once

#include "Session/LobbyData.h"

namespace NamedVariantDataConverter
{

	FLobbyDataEntry ToLobbyDataEntry(const FName& InName, const FVariantData& InData);

	TPair<FName, FVariantData> FromLobbyDataEntry(const FLobbyDataEntry& InDataEntry);

}
