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
	};

	static TMap<FFlareResourceDescription*, FlareResourceStats> ComputeWorldResourceStats(AFlareGame* Game);


private:


};
