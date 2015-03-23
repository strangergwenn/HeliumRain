#pragma once

#include "FlarePlanetarium.generated.h"


UCLASS()
class FLARE_API AFlarePlanetarium : public AActor
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public events
	----------------------------------------------------*/

	/** Set the altitude of the current system, in km */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void SetAltitude(int32 Altitude);
	
	/** Set the sun rotation, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void SetSunRotation(int32 RotationDegrees);
	
	/** Set the rotation of the planet, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void SetPlanetRotation(int32 RotationDegrees);
	
	/** Set the rotation of the current system, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void SetSectorRotation(int32 RotationDegrees);
	

};
