#pragma once
#include "../Economy/FlareResource.h"
#include "FlareWorld.h"

struct WorldHelper
{
	struct FlareResourceStats
	{
		float Production;
		float Consumption;
		float Balance;
		int32 Stock;
		int32 Capacity;
	};

	static TMap<FFlareResourceDescription*, FlareResourceStats> ComputeWorldResourceStats(AFlareGame* Game, bool IncludeStorage);


private:


};
