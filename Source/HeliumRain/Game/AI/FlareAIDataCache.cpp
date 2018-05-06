
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

int64 GetInputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource)
{

}


int64 GetOutputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource)
{

}
