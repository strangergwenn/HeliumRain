#pragma once

#include <Engine/DPICustomScalingRule.h>
#include "FlareScalingRule.generated.h"


UCLASS()
class UFlareScalingRule : public UDPICustomScalingRule
{
	GENERATED_BODY()

public:

	virtual float GetDPIScaleBasedOnSize(FIntPoint Size) const override;

};
