#pragma once

#include "FlareShipModule.h"
#include "FlareEngine.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), hidecategories = (Rendering, Lighting, Base), meta = (BlueprintSpawnableComponent))
class UFlareEngine : public UFlareShipModule
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/** Update the exhaust power command */
	virtual void UpdateAlpha(float DeltaTime);

	/** Update the exhaust special effect */
	virtual void UpdateEffects();
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	float ExhaustAlpha;

	float ExhaustAccumulator;


};
