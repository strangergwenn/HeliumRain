#pragma once

#include "FlareSpacecraftNavigationSystemInterface.h"
#include "FlareSimulatedSpacecraftNavigationSystem.generated.h"


/** Spacecraft navigation system class */
UCLASS()
class FLARE_API UFlareSimulatedSpacecraftNavigationSystem : public UObject, public IFlareSpacecraftNavigationSystemInterface
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Initialize this system */
	virtual void Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	virtual bool IsAutoPilot();

	virtual bool IsDocked();

	/** Get the current command */
	virtual FFlareShipCommandData GetCurrentCommand();


protected:


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSpacecraft*                               Spacecraft;
	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

};
