#pragma once

#include "Object.h"
#include "FlareSimulatedPlanetarium.generated.h"

class AFlareGame;

struct FPreciseMath
{

	static inline double Sin( double Value ) { return sin(Value); }
	static inline double Cos( double Value ) { return cos(Value); }
	static inline double Tan( double Value ) { return tan(Value); }
	static inline double Asin( double Value ) { return asin(Value); }
	static inline double Atan( double Value ) { return atan(Value); }
	static inline double Sqrt( double Value ) { return sqrt(Value); }

	/**
	* Computes the sine and cosine of a scalar float.
	*
	* @param ScalarSin	Pointer to where the Sin result should be stored
	* @param ScalarCos	Pointer to where the Cos result should be stored
	* @param Value  input angles
	*/
	static inline void SinCos( double* ScalarSin, double* ScalarCos, double  Value )
	{
		*ScalarSin = sin(Value);
		*ScalarCos = cos(Value);
	}

	/**
	 * Converts degrees to radians.
	 * @param	DegVal			Value in degrees.
	 * @return					Value in radians.
	 */
	static inline double DegreesToRadians(double const& DegVal)
	{
		return DegVal * (PI / 180.);
	}

	/** Computes a fully accurate inverse square root */
	static inline double InvSqrt( double F )
	{
		return 1.0 / sqrt( F );
	}

	/** Utility to ensure angle is between +/- 180 degrees by unwinding. */
	static double UnwindDegrees(double A)
	{
		while(A > 180.)
		{
			A -= 360.;
		}

		while(A < -180.)
		{
			A += 360.;
		}

		return A;
	}
};

/**
 * A point or direction in 3d space.
  */
struct FPreciseVector
{
	double X;

	double Y;

	double Z;

	/** A zero vector (0,0,0) */
	static const FPreciseVector ZeroVector;

	inline FPreciseVector()
		: X(0), Y(0), Z(0)
	{
	}


	inline FPreciseVector(double InF)
		: X(InF), Y(InF), Z(InF)
	{
	}

	inline FPreciseVector( double InX, double InY, double InZ )
		: X(InX), Y(InY), Z(InZ)
	{
	}

	inline FPreciseVector GetUnsafeNormal() const
	{
		const double Scale = FPreciseMath::InvSqrt(X*X+Y*Y+Z*Z);
		return FPreciseVector( X*Scale, Y*Scale, Z*Scale );
	}

	inline FPreciseVector RotateAngleAxis( const double AngleDeg, const FPreciseVector& Axis ) const
	{
		double S, C;
		FPreciseMath::SinCos(&S, &C, FPreciseMath::DegreesToRadians(AngleDeg));

		const double XX	= Axis.X * Axis.X;
		const double YY	= Axis.Y * Axis.Y;
		const double ZZ	= Axis.Z * Axis.Z;

		const double XY	= Axis.X * Axis.Y;
		const double YZ	= Axis.Y * Axis.Z;
		const double ZX	= Axis.Z * Axis.X;

		const double XS	= Axis.X * S;
		const double YS	= Axis.Y * S;
		const double ZS	= Axis.Z * S;

		const double OMC	= 1.f - C;

		return FPreciseVector(
			(OMC * XX + C ) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
			(OMC * XY + ZS) * X + (OMC * YY + C ) * Y + (OMC * YZ - XS) * Z,
			(OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C ) * Z
			);
	}

	inline double Size() const
	{
		return FPreciseMath::Sqrt( X*X + Y*Y + Z*Z );
	}

	inline FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);
	}

	inline FVector ToVector() const
	{
		return FVector(X, Y, Z);
	}

	inline FPreciseVector operator*( double Scale ) const
	{
		return FPreciseVector( X * Scale, Y * Scale, Z * Scale );
	}

	inline FPreciseVector operator+( const FPreciseVector& V ) const
	{
		return FPreciseVector( X + V.X, Y + V.Y, Z + V.Z );
	}

	inline FPreciseVector operator-( const FPreciseVector& V ) const
	{
		return FPreciseVector( X - V.X, Y - V.Y, Z - V.Z );
	}

	inline FPreciseVector operator-() const
	{
		return FPreciseVector( -X, -Y, -Z );
	}
};


/**
 * Multiplies a vector by a scaling factor.
 *
 * @param Scale Scaling factor.
 * @param V Vector to scale.
 * @return Result of multiplication.
 */
inline FPreciseVector operator*( double Scale, const FPreciseVector& V )
{
	return V.operator*( Scale );
}


/** Celestial body structure */
struct FFlareCelestialBody
{
	/*----------------------------------------------------
		Static parameters
	----------------------------------------------------*/

	/** Name */
	FText Name;

	/** Name */
	FName Identifier;

	/** Mass of the celestial body. In kg */
	double Mass;

	/** Radius of the celestial body. In km */
	double Radius;

	/** Altitude of the outer part of the rings. In km */
	double RingsOuterAltitude;

	/** Orbit distance. The orbit are circular. In km. */
	double OrbitDistance;

	/** Self rotation velocity */
	double RotationVelocity;

	/** Sattelites list */
	TArray<FFlareCelestialBody> Sattelites;

	/*----------------------------------------------------
		Dynamic parameters
	----------------------------------------------------*/

	/** Current celestial body location relative to its parent celestial body*/
	FPreciseVector RelativeLocation;

	/** Current celestial body location relative to its the root star*/
	FPreciseVector AbsoluteLocation;

	/** Current celestial body self rotation angle*/
	double RotationAngle;

};


UCLASS()
class HELIUMRAIN_API UFlareSimulatedPlanetarium : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the planetarium */
	virtual void Load();


	/** Load the planetarium */
	virtual FFlareCelestialBody GetSnapShot(int64 Time, float SmoothTime);

	/** Get relative location of a body orbiting around its parent */
	virtual FPreciseVector GetRelativeLocation(FFlareCelestialBody* ParentBody, int64 Time, float SmoothTime, double OrbitDistance, double Mass, double InitialPhase);

	/** Return the celestial body with the given identifier */
	FFlareCelestialBody* FindCelestialBody(FName BodyIdentifier);

	/** Return the celestial body with the given identifier in the given body tree */
	FFlareCelestialBody* FindCelestialBody(FFlareCelestialBody* Body, FName BodyIdentifier);

	/** Return the parent of the given celestial body */
	FFlareCelestialBody* FindParent(FFlareCelestialBody* Body);

	/** Return the parent of the given celestial body in the given root tree*/
	FFlareCelestialBody* FindParent(FFlareCelestialBody* Body, FFlareCelestialBody* Root);

	/** Return true if the target body is sattelite of the parent body */
	bool IsSatellite(FFlareCelestialBody* Body, FFlareCelestialBody* Parent);

	float GetLightRatio(FFlareCelestialBody* Body, double OrbitDistance);

protected:

	void ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 time, float SmoothTime);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	AFlareGame*                   Game;

	FFlareCelestialBody           Sun;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const;



};
