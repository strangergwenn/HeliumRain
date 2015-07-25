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

	/** Name */
	FString Identifier;

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

	/** Current celestial body location relative to its the root star*/
	FVector AbsoluteLocation;

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

	/** Get relative location of a body orbiting around its parent */
	virtual FVector GetRelativeLocation(FFlareCelestialBody* ParentBody, int64 Time, float OrbitDistance, float Mass, float InitialPhase);

	/** Return the celestial body with the given identifier */
	FFlareCelestialBody* FindCelestialBody(FString BodyIdentifier);

	/** Return the celestial body with the given identifier in the given body tree */
	FFlareCelestialBody* FindCelestialBody(FFlareCelestialBody* Body, FString BodyIdentifier);

protected:

	void ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 time);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	AFlareGame*                   Game;

	FFlareCelestialBody           Sun;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}



};
