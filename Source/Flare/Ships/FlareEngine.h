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

	virtual void Initialize(const FFlareShipModuleDescription* Description, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu) override;

	/** Max engine thrust in Newtons*/
	float MaxThrust;

	/** Get engine thrust axis in world space */
	FVector GetThurstAxis() const;
	
	/** Get engine current max thrust 
	  current max thrust can change with damages
	 */
	float GetMaxThrust() const;
	
	/** Get engine max thrust from specification
	  initial max thrust don't change with damages
	 */
	float GetInitialMaxThrust() const;
	
	/** Return true if the engine is an Orbital engine.
	 */
	virtual bool IsOrbitalEngine() const { return false; };

	/** Update the exhaust power for current thrust */
	void SetAlpha(float Alpha);
	
protected:

	/** Update the exhaust special effect */
	virtual void UpdateEffects();

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	float ExhaustAlpha;

	float ExhaustAccumulator;


};
