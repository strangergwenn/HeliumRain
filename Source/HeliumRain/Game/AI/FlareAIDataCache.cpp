
#include "FlareAIDataCache.h"
#include "../../Flare.h"


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareAIDataCache::UFlareAIDataCache(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareAIDataCache::Load(AFlareGame* GameParam)
{
	Game = GameParam;
}

int64 UFlareAIDataCache::GetInputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource)
{
	// TODO
	return 1;
}


int64 UFlareAIDataCache::GetOutputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource)
{
	// TODO
	return 1;
}
