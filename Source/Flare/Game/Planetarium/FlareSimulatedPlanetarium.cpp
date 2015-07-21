#include "../../Flare.h"
#include "FlareSimulatedPlanetarium.h"
#include "../FlareGame.h"


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
	Star.Sattelites.Empty();


	// Init the sun
	Star.Name = "Sun";
	Star.Mass = 2.472e30;
	Star.Radius = 739886;
	Star.RotationVelocity = 0;
	Star.OrbitDistance = 0;
	Star.RelativeLocation = FVector::ZeroVector;
	Star.RotationAngle = 0;

	// Small Hot planet
	FFlareCelestialBody SmallHotPlanet;
	SmallHotPlanet.Name = "Small Hot Planet";
	SmallHotPlanet.Mass = 330e21;
	SmallHotPlanet.Radius = 2439;
	SmallHotPlanet.RotationVelocity = 0.0003;
	SmallHotPlanet.OrbitDistance = 57000000;
	Star.Sattelites.Add(SmallHotPlanet);

	// Nema

	FFlareCelestialBody Nema;
	{
		Nema.Name = "Nema";
		Nema.Mass = 8.421e26;
		Nema.Radius = 69586;
		Nema.RotationVelocity = 0.0037254354102635744;
		Nema.OrbitDistance = 110491584;

		FFlareCelestialBody NemaMun1;
		NemaMun1.Name = "Nema Mun 1";
		NemaMun1.Mass = 1.3e23;
		NemaMun1.Radius = 2600;
		NemaMun1.RotationVelocity = -0.03;
		NemaMun1.OrbitDistance = 1000000;
		Nema.Sattelites.Add(NemaMun1);

		FFlareCelestialBody NemaMun2;
		NemaMun2.Name = "Nema Mun 2";
		NemaMun2.Mass = 5.3e23;
		NemaMun2.Radius = 4600;
		NemaMun2.RotationVelocity = 0.003;
		NemaMun2.OrbitDistance = 3000000;
		Nema.Sattelites.Add(NemaMun2);

		FFlareCelestialBody NemaMun3;
		NemaMun3.Name = "Nema Mun 3";
		NemaMun3.Mass = 0.9e23;
		NemaMun3.Radius = 2000;
		NemaMun3.RotationVelocity = 0.05;
		NemaMun3.OrbitDistance = 4000000;
		Nema.Sattelites.Add(NemaMun3);
	}
	Star.Sattelites.Add(Nema);
}


FFlareCelestialBody UFlareSimulatedPlanetarium::GetSnapShot(int64 Time)
{
	ComputeCelestialBodyLocation(NULL, &Star, Time);
	return Star;
}

void UFlareSimulatedPlanetarium::ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 Time)
{
	if(ParentBody)
	{
		// TODO extract the constant
		float G = 6.674e-11; // Gravitational constant

		float MassSum = ParentBody->Mass + Body->Mass;
		float Distance = 1000 * Body->OrbitDistance;
		float SquareVelocity = G * (MassSum / Distance);
		float FragmentedOrbitalVelocity = FMath::Sqrt(SquareVelocity);

		float OrbitalVelocity = FMath::Sqrt(G * ((ParentBody->Mass + Body->Mass) / (1000 * Body->OrbitDistance)));

		OrbitalVelocity = FragmentedOrbitalVelocity;

		float OrbitalCircumference = 2 * PI * 1000 * Body->OrbitDistance;
		int64 RevolutionTime = (int64) (OrbitalCircumference / OrbitalVelocity);

		int64 CurrentRevolutionTime = Time % RevolutionTime;

		float Phase = 360 * (float) CurrentRevolutionTime / (float) RevolutionTime;


		Body->RelativeLocation = Body->OrbitDistance * FVector(FMath::Cos(FMath::DegreesToRadians(Phase)),
				FMath::Sin(FMath::DegreesToRadians(Phase)),
				0);
	}

	Body->RotationAngle = Body->RotationVelocity * Time;

	for(int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		ComputeCelestialBodyLocation(Body, CelestialBody, Time);
	}
}
