#pragma once

#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
#include "FlarePlanetarium.generated.h"


struct FFlareCelestialBody;

UCLASS()
class HELIUMRAIN_API AFlarePlanetarium : public AActor
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public interface
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;

	void BeginPlay() override;

	/** Move a celestial body */
	void MoveCelestialBody(FFlareCelestialBody* Body, FPreciseVector Offset, double AngleOffset, FPreciseVector SunDirection);

	/** Reset the current time */
	void ResetTime();

	/** Set the current tim multiplier */
	void SetTimeMultiplier(float Multiplier);

	void SkipNight(float TimeRange);


	/*----------------------------------------------------
		Public Blueprint events
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


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	AActor* Sky;
	UDirectionalLightComponent* Light;

	FName CurrentSector;

	FFlareCelestialBody Sun;

	double SunOcclusion;

	double SunAnglularRadius;
	double SunPhase;

	float SmoothTime;
	float TimeMultiplier;
	float SkipNightTimeRange;
	bool Ready;

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

	inline FVector GetSunDirection()
	{
		FVector Direction;

		if (Light)
		{
			Direction = Light->GetDirection();
		}

		return Direction;
	}

	bool IsReady() {
		return Ready;
	}

};
