#pragma once

#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
#include "FlarePlanetarium.generated.h"


struct FFlareCelestialBody;

UCLASS()
class FLARE_API AFlarePlanetarium : public AActor
{
public:

	GENERATED_UCLASS_BODY()

	virtual void Tick(float DeltaSeconds) override;

	void MoveCelestialBody(FFlareCelestialBody* Body, FPreciseVector Offset, double AngleOffset, FPreciseVector SunDirection);

	void ResetTime();

	/*----------------------------------------------------
		Public events
	----------------------------------------------------*/

	/** Set the altitude of the current system, in km */
	UFUNCTION(BlueprintImplementableEvent)
	void SetAltitude(int32 Altitude);
	
	/** Set the sun rotation, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	void SetSunRotation(int32 RotationDegrees);
	
	/** Set the rotation of the planet, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	void SetPlanetRotation(int32 RotationDegrees);
	
	/** Set the rotation of the current system, in degrees */
	UFUNCTION(BlueprintImplementableEvent)
	void SetSectorRotation(int32 RotationDegrees);

	void SetTimeMultiplier(float Multiplier);

protected:
	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	AStaticMeshActor* Sky;
	UDirectionalLightComponent* Light;

	FName CurrentSector;

	FFlareCelestialBody Sun;

	double SunOcclusion;

	double SunAnglularRadius;
	double SunPhase;

	float SmoothTime;
	float TimeMultiplier;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

		inline AFlareGame* GetGame() const
		{
			return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
		}

		double GetSunOcclusion() const
		{
			return SunOcclusion;
		}

		inline float GetSmoothTime()
		{
			return SmoothTime;
		}

};
