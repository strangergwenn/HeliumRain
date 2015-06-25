#pragma once

#include "Engine.h"

/** Part size values */
UENUM()
namespace EFlarePartSize
{
	enum Type
	{
		S,
		M,
		L,
		Num
	};
}


struct SpacecraftHelper
{
	static float GetIntersectionPosition(FVector TargetLocation,
										 FVector TargetVelocity,
										 FVector SourceLocation,
										 FVector SourceVelocity,
										 float ProjectileSpeed,
										 float PredictionDelay,
										 FVector* ResultPosition);
};
