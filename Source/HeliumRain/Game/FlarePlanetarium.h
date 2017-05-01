#pragma once

#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
#include "Components/DirectionalLightComponent.h"
#include "FlarePlanetarium.generated.h"


struct CelestialBodyPosition
{
	UStaticMeshComponent* BodyComponent;
	FFlareCelestialBody* Body;
	double Distance;
	double Radius;
	double TotalRotation;
	FPreciseVector AlignedLocation;
};


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

	/** Prepare a celestial body to future setup */
	void PrepareCelestialBody(FFlareCelestialBody* Body, FPreciseVector Offset, double AngleOffset);

	void SetupCelestialBodies();

	void SetupCelestialBody(CelestialBodyPosition* BodyPosition, double DisplayDistance, double DisplayRadius);

	/** Reset the current time */
	void ResetTime();

	/** Set the current tim multiplier */
	void SetTimeMultiplier(float Multiplier);

	void SkipNight(float TimeRange);

	/** Get the speed of dust particles for travel */
	UFUNCTION(BlueprintCallable, Category = "Flare")
	FVector GetStellarDustVelocity() const;


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
	double MinDistance;

	double SunOcclusionAngle;
	double SunPhase;

	float SmoothTime;
	float TimeMultiplier;
	float SkipNightTimeRange;
	bool Ready;

	TArray<CelestialBodyPosition> BodyPositions;
	FPreciseVector SunDirection;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	float GetSunOcclusion() const
	{
		return SunOcclusion;
	}

	inline float GetSmoothTime()
	{
		return SmoothTime;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	FVector GetSunDirection() const
	{
		FVector Direction;

		if (Light)
		{
			Direction = Light->GetDirection();
		}

		return Direction;
	}

	bool IsReady()
	{
		return Ready;
	}

};
