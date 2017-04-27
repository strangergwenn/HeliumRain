
#include "FlareSimulatedPlanetarium.h"
#include "../../Flare.h"
#include "../FlareGame.h"

const FPreciseVector FPreciseVector::ZeroVector = FPreciseVector();


#define LOCTEXT_NAMESPACE "UFlareSimulatedPlanetarium"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedPlanetarium::UFlareSimulatedPlanetarium(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UFlareSimulatedPlanetarium::Load()
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();
	Sun.Sattelites.Empty();
	
	// Init the sun
	Sun.Name = LOCTEXT("Sun", "Sun");
	Sun.Identifier = "star-sun";
	Sun.Mass = 2.472e30;
	Sun.Radius = 739886*2; // TODO fix in spec
	Sun.RingsOuterAltitude = 0.;
	Sun.RotationVelocity = 0;
	Sun.OrbitDistance = 0;
	Sun.RelativeLocation = FPreciseVector::ZeroVector;
	Sun.AbsoluteLocation = FPreciseVector::ZeroVector;
	Sun.RotationAngle = 0;

	// Nema
	FFlareCelestialBody Nema;
	{
		Nema.Name = LOCTEXT("Nema", "Nema");
		Nema.Identifier = "nema";
		Nema.Mass = 8.421e26;
		Nema.Radius = 69586;
		Nema.RingsOuterAltitude = 30000;
		Nema.RotationVelocity = -0.0037254354102635744;
		Nema.OrbitDistance = 110491584;

		FFlareCelestialBody Anka;
		Anka.Name = LOCTEXT("Anka", "Anka");
		Anka.Identifier = "anka";
		Anka.Mass = 1.3e23;
		Anka.Radius = 2600;
		Anka.RingsOuterAltitude = 0.;
		Anka.RotationVelocity = -0.03;
		Anka.OrbitDistance = 320000;
		Nema.Sattelites.Add(Anka);

		FFlareCelestialBody Hela;
		Hela.Name = LOCTEXT("Hela", "Hela");
		Hela.Identifier = "hela";
		Hela.Mass = 5.3e23;
		Hela.Radius = 4600;
		Hela.RingsOuterAltitude = 0.;
		Hela.RotationVelocity = 0.003;
		Hela.OrbitDistance = 571000;
		Nema.Sattelites.Add(Hela);

		FFlareCelestialBody Asta;
		Asta.Name = LOCTEXT("Asta", "Asta");
		Asta.Identifier = "asta";
		Asta.Mass = 0.9e23;
		Asta.Radius = 2000;
		Asta.RingsOuterAltitude = 0.;
		Asta.RotationVelocity = 0.05;
		Asta.OrbitDistance = 870000;
		Nema.Sattelites.Add(Asta);

		FFlareCelestialBody Adena;
		Adena.Name = LOCTEXT("Adena", "Adena");
		Adena.Identifier = "adena";
		Adena.Mass = 2e23;
		Adena.Radius = 2500;
		Adena.RingsOuterAltitude = 0.;
		Adena.RotationVelocity = 0.04;
		Adena.OrbitDistance = 1150000;
		Nema.Sattelites.Add(Adena);
	}
	Sun.Sattelites.Add(Nema);
}


FFlareCelestialBody* UFlareSimulatedPlanetarium::FindCelestialBody(FName BodyIdentifier)
{
	return FindCelestialBody(&Sun, BodyIdentifier);
}

FFlareCelestialBody* UFlareSimulatedPlanetarium::FindCelestialBody(FFlareCelestialBody* Body, FName BodyIdentifier)
{
	if (Body->Identifier == BodyIdentifier)
	{
		return Body;
	}

	for (int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		FFlareCelestialBody* Result = FindCelestialBody(CelestialBody, BodyIdentifier);
		if (Result)
		{
			return Result;
		}
	}

	return NULL;
}

FFlareCelestialBody* UFlareSimulatedPlanetarium::FindParent(FFlareCelestialBody* Body)
{
	if (&Sun == Body)
	{
		return NULL;
	}

	return FindParent(Body, &Sun);
}

FFlareCelestialBody* UFlareSimulatedPlanetarium::FindParent(FFlareCelestialBody* Body, FFlareCelestialBody* Root)
{
	if (IsSatellite(Body, Root))
	{
		return Root;
	}
	else
	{
		for (int SatteliteIndex = 0; SatteliteIndex < Root->Sattelites.Num(); SatteliteIndex++)
		{
			FFlareCelestialBody* ParentCandidate = FindParent(Body, &Root->Sattelites[SatteliteIndex]);
			if (ParentCandidate)
			{
				return ParentCandidate;
			}
		}
	}

	return NULL;
}

bool UFlareSimulatedPlanetarium::IsSatellite(FFlareCelestialBody* Body, FFlareCelestialBody* Parent)
{
	for (int SatteliteIndex = 0; SatteliteIndex < Parent->Sattelites.Num(); SatteliteIndex++)
	{
		if (&Parent->Sattelites[SatteliteIndex] == Body)
		{
			return true;
		}
	}
	return false;
}

float UFlareSimulatedPlanetarium::GetLightRatio(FFlareCelestialBody* Body, double OrbitDistance)
{
	return 0.5 + FMath::Acos(Body->Radius / (Body->Radius + OrbitDistance)) / PI;
}

FFlareCelestialBody UFlareSimulatedPlanetarium::GetSnapShot(int64 Time, float SmoothTime)
{
	ComputeCelestialBodyLocation(NULL, &Sun, Time, SmoothTime);
	return Sun;
}

FPreciseVector UFlareSimulatedPlanetarium::GetRelativeLocation(FFlareCelestialBody* ParentBody, int64 Time, float SmoothTime, double OrbitDistance, double Mass, double InitialPhase)
{
	// TODO extract the constant
	double G = 6.674e-11; // Gravitational constant

	double MassSum = ParentBody->Mass + Mass;
	double OrbitalVelocity = FPreciseMath::Sqrt(G * ((MassSum) / (1000 * OrbitDistance)));

	double OrbitalCircumference = 2 * PI * 1000 * OrbitDistance;
	int64 RevolutionTime = (int64) (OrbitalCircumference / OrbitalVelocity);

	double CurrentRevolutionTime = fmod(((double) (Time % RevolutionTime) + SmoothTime), (double) RevolutionTime);

	double Phase = (360 * CurrentRevolutionTime / (double) RevolutionTime) + InitialPhase;


	FPreciseVector RelativeLocation = OrbitDistance * FPreciseVector(FPreciseMath::Cos(FPreciseMath::DegreesToRadians(Phase)),
			0,
			FPreciseMath::Sin(FPreciseMath::DegreesToRadians(Phase)));


	return RelativeLocation;
}


void UFlareSimulatedPlanetarium::ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 Time, float SmoothTime)
{
	if (ParentBody)
	{
		Body->RelativeLocation = GetRelativeLocation(ParentBody, Time, SmoothTime, Body->OrbitDistance, Body->Mass, 0);
		Body->AbsoluteLocation = ParentBody->AbsoluteLocation + Body->RelativeLocation;
	}

	int64 RotationPeriod = 360/Body->RotationVelocity;

	Body->RotationAngle = FPreciseMath::UnwindDegrees(Body->RotationVelocity * (Time % RotationPeriod)) + Body->RotationVelocity * SmoothTime;
	for (int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		ComputeCelestialBodyLocation(Body, CelestialBody, Time, SmoothTime);
	}
}

AFlareGame* UFlareSimulatedPlanetarium::GetGame() const
{
	return Game;
}

#undef LOCTEXT_NAMESPACE
