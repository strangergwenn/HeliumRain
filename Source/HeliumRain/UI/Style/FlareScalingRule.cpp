
#include "FlareScalingRule.h"

float UFlareScalingRule::GetDPIScaleBasedOnSize(FIntPoint Size) const
{
	float NominalAspectRatio = (16.0f / 9.0f);

	// Loading game
	if (Size.X == 0 || Size.Y == 0)
	{
		return 1;
	}

	// Wide screen : scale 1 at 1920, scale 2 at 3840... 
	else if (Size.X / Size.Y > NominalAspectRatio)
	{
		return (Size.Y / 1080.0f);
	}

	// Square ratio : scale 1 at 1080p
	else
	{
		return (Size.X / 1920.0f);
	}
}
