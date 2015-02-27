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

	virtual void UpdateEffects() override;

	virtual bool IsOrbitalEngine() const;

private:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	float AverageAlpha;


};
