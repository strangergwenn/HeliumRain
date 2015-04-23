#pragma once

#include "FlareShipComponent.h"
#include "FlareEngine.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareEngine : public UFlareShipComponent
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu) override;

	/** Max engine thrust in Newtons*/
	float MaxThrust;

	/** Get engine thrust axis in world space */
	FVector GetThrustAxis() const;
	
	/** Get engine current max thrust ; Ccurrent max thrust can change with damages */
	float GetMaxThrust() const;
	
	/** Get engine max thrust from specification ; Initial max thrust doesn't change with damages */
	float GetInitialMaxThrust() const;
	
	/** Return true if the engine is an Orbital engine. */
	virtual bool IsOrbitalEngine() const { return false; };

	/** Update the exhaust power for current thrust */
	void SetAlpha(float Alpha);

	/** Get the actual alpha */
	virtual float GetEffectiveAlpha() const;
	
	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const override;

	/** Apply damage to this component only if it is used. */
	virtual void ApplyHeatDamage(float Energy) override;


protected:

	/*----------------------------------------------------
		Protected methods
	----------------------------------------------------*/

	/** Update the exhaust special effect */
	virtual void UpdateEffects();


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	float ExhaustAlpha;

	float ExhaustAccumulator;


};
