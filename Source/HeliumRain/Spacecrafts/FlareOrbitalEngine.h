#pragma once

#include "FlareEngine.h"
#include "FlareOrbitalEngine.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareOrbitalEngine : public UFlareEngine
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual bool IsDestroyedEffectRelevant() override;

};
