#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareEngine.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareEngine : public UFlareSpacecraftComponent
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void Initialize(const FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu) override;


	/** Max engine thrust in Newtons*/
	float MaxThrust;

	/** Get engine thrust axis in world space */
	FVector GetThrustAxis() const;

	/** Get engine current max thrust ; Ccurrent max thrust can change with damages */
	float GetMaxThrust() const;

	/** Get engine max thrust from specification ; Initial max thrust doesn't change with damages */
	float GetInitialMaxThrust() const;

	/** Update the exhaust power for current thrust */
	void SetAlpha(float Alpha);

	/** Get the actual alpha */
	virtual float GetEffectiveAlpha() const;

	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const override;

	virtual float GetUsableRatio() const override;

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
