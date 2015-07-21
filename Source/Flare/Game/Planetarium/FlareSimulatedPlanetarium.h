#pragma once

#include "Object.h"
#include "FlareSimulatedPlanetarium.generated.h"


/** Celestial body structure */
struct FFlareCelestialBody
{
	/*----------------------------------------------------
		Static parameters
	----------------------------------------------------*/

	/** Name */
	FString Name;

	/** Mass of the celestial body. In kg */
	float Mass;

	/** Radius of the celestial body. In km */
	float Radius;

	/** Orbit distance. The orbit are circular. In km. */
	float OrbitDistance;

	/** Self rotation velocity */
	float RotationVelocity;

	/** Sattelites list */
	TArray<FFlareCelestialBody> Sattelites;

	/*----------------------------------------------------
		Dynamic parameters
	----------------------------------------------------*/

	/** Current celestial body location relative to its parent celestial body*/
	FVector RelativeLocation;

	/** Current celestial body self rotation angle*/
	float RotationAngle;

};


UCLASS()
class FLARE_API UFlareSimulatedPlanetarium : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the planetarium */
	virtual void Load();


	/** Load the planetarium */
	virtual FFlareCelestialBody GetSnapShot(int64 Time);

protected:

	void ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 time);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	AFlareGame*                   Game;

	FFlareCelestialBody           Star;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}



};
