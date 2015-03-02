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

	/** Apply the current thrust to a ship*/
	virtual void TickModule(float deltaTime) override;

	float CurrentThrust;

	// TODO init with characteristics
	float MaxThrust;

	float TargetThrust;

	// TODO remove if always in same axis
	FVector ThrustAxis;
	
	/**
	  * Configure the target thrust
	  * 1 for max trust
	  * 0 for no thrust
	  */
	void SetTargetThrustRatio(float Ratio);

	/** Get engine thrust axis in world space */
	FVector GetThurstAxis() const;
	
	/** Get engine current max thrust 
	  current max thrust can change with damages
	 */
	float GetMaxThrust() const;
	
protected:
	/** Update the exhaust power from current thrust */
	virtual void UpdateAlpha(float DeltaTime);

	/** Update the exhaust special effect */
	virtual void UpdateEffects();

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	float ExhaustAlpha;

	float ExhaustAccumulator;


};
