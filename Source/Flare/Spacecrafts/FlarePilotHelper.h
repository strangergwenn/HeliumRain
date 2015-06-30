#pragma once

#include "Engine.h"

class UFlareCompany;

struct PilotHelper
{
	static bool CheckFriendlyFire(UWorld* World, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);
};
