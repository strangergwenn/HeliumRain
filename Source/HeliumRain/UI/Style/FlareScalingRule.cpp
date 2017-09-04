
#include "FlareScalingRule.h"

float UFlareScalingRule::GetDPIScaleBasedOnSize(FIntPoint Size) const
{
	float NominalAspectRatio = (16.0f / 9.0f);

	// Wide screen : scale 1 at 1920, scale 2 at 3840... 
	if (Size.X / Size.Y > NominalAspectRatio)
	{
		return (Size.Y / 1080.0f);
	}

	// Square ratio : scale 1 at 1080p
	else
	{
		return (Size.X / 1920.0f);
	}
}
