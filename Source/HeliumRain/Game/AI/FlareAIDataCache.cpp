
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
