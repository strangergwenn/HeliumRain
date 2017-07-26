
#include "FlarePlanetarium.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"

#include "EngineUtils.h"
#include "Materials/MaterialInstanceConstant.h"


//#define PLANETARIUM_DEBUG


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlanetarium::AFlarePlanetarium(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	TimeMultiplier = 1.0;
	SkipNightTimeRange = 0;
	Ready = false;
}

void AFlarePlanetarium::BeginPlay()
{
	Super::BeginPlay();
	FLOG("AFlarePlanetarium::BeginPlay");

	TArray<UActorComponent*> Components = GetComponentsByClass(UStaticMeshComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UStaticMeshComponent* PlanetCandidate = Cast<UStaticMeshComponent>(Components[ComponentIndex]);
		if (PlanetCandidate)
		{
			// Apply a new dynamic material to planets so that we can control shading parameters
			UMaterialInstanceConstant* BasePlanetMaterial = Cast<UMaterialInstanceConstant>(PlanetCandidate->GetMaterial(0));
			if (BasePlanetMaterial)
			{
				//FLOGV("AFlarePlanetarium::BeginPlay : found planet '%s'", *PlanetCandidate->GetName());

#if PLATFORM_LINUX
				int32 UseNormalAsLightingDirection = 1;
#else
				int32 UseNormalAsLightingDirection = 0;
#endif
				UMaterialInstanceDynamic* PlanetMaterial = UMaterialInstanceDynamic::Create(BasePlanetMaterial, GetWorld());
				PlanetCandidate->SetMaterial(0, PlanetMaterial);
				PlanetMaterial->SetScalarParameterValue("UseNormalAsLightingDirection", UseNormalAsLightingDirection);
			}
		}
	}
}

void AFlarePlanetarium::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SmoothTime += DeltaSeconds * TimeMultiplier;

	if (GetGame())
	{
		if (!GetGame()->GetActiveSector())
		{
			// No active sector, do nothing
			Ready = false;
			return;
		}

		UFlareWorld* World = GetGame()->GetGameWorld();

		if (World)
		{
			float LocalTime = GetGame()->GetActiveSector()->GetLocalTime();
			CurrentSector = GetGame()->GetActiveSector()->GetSimulatedSector()->GetIdentifier();

			if (Sky == NULL)
			{
				for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					if ((*ActorItr)->GetName().StartsWith("Skybox"))
					{
						FLOG("AFlarePlanetarium::Tick : found the sky");
						Sky = *ActorItr;
						break;
					}
				}
			}

			if (Light == NULL)
			{
				TArray<UActorComponent*> Components = GetComponentsByClass(UDirectionalLightComponent::StaticClass());
				for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
				{
					UDirectionalLightComponent* LightCandidate = Cast<UDirectionalLightComponent>(Components[ComponentIndex]);
					if (LightCandidate)
					{
						Light = LightCandidate;
						break;
					}
				}
			}

			Ready = true;

			do
			{

				Sun = World->GetPlanerarium()->GetSnapShot(LocalTime, SmoothTime);

				// Draw Player
				const FFlareSectorOrbitParameters* PlayerOrbit = GetGame()->GetActiveSector()->GetSimulatedSector()->GetOrbitParameters();
				
				FFlareCelestialBody* CurrentParent = World->GetPlanerarium()->FindCelestialBody(PlayerOrbit->CelestialBodyIdentifier);
				if (CurrentParent)
				{
					FPreciseVector ParentLocation = CurrentParent->AbsoluteLocation;

					double DistanceToParentCenter = CurrentParent->Radius + PlayerOrbit->Altitude;
					FPreciseVector PlayerLocation =  ParentLocation + World->GetPlanerarium()->GetRelativeLocation(CurrentParent, LocalTime, SmoothTime, DistanceToParentCenter, 0, PlayerOrbit->Phase);
					/*FLOGV("Parent location = %s", *CurrentParent->AbsoluteLocation.ToString());
					FLOGV("PlayerLocation = %s", *PlayerLocation.ToString());*/
#ifdef PLANETARIUM_DEBUG
					DrawDebugLine(GetWorld(), FVector(1000, 0 ,0), FVector(- 1000, 0 ,0), FColor::Red, false);
					DrawDebugLine(GetWorld(), FVector(0, 1000 ,0), FVector(0,- 1000 ,0), FColor::Green, false);
					DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, -1000), FColor::Blue, false);
					DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, 1000), FColor::Cyan, false);
#endif
					FPreciseVector DeltaLocation = ParentLocation - PlayerLocation;
					FPreciseVector SunDeltaLocation = Sun.AbsoluteLocation - PlayerLocation;

					float AngleOffset =  90 + FMath::RadiansToDegrees(FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
					/*FLOGV("DeltaLocation = %s", *DeltaLocation.ToString());
					FLOGV("FMath::Atan2(DeltaLocation.Y,DeltaLocation.X)  = %f", FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
					FLOGV("AngleOffset  = %f", AngleOffset);*/

					SunDirection = -(SunDeltaLocation.RotateAngleAxis(AngleOffset, FPreciseVector(0,1,0))).GetUnsafeNormal();
					// Reset sun occlusion;
					SunOcclusion = 0;
					MinDistance = DistanceToParentCenter;

					BodyPositions.Empty();
					PrepareCelestialBody(&Sun, -PlayerLocation, AngleOffset);
					SetupCelestialBodies();

					// Try to find night
					if (SkipNightTimeRange > 0 && SunOcclusion >= 1)
					{
						SmoothTime = FMath::FRand() * SkipNightTimeRange;
						//FLOGV("AFlarePlanetarium::Tick : night, try to find light at %f (max %f)",SmoothTime, SkipNightTimeRange);
					}
					else
					{
						SkipNightTimeRange = 0;
					}


					if (Sky)
					{
						Sky->SetActorRotation(FRotator(-AngleOffset, 0 , 0));
						//FLOGV("Sky %s rotation= %s",*Sky->GetName(),  *Sky->GetActorRotation().ToString());
					}
					else
					{
						FLOG("AFlarePlanetarium::Tick : no sky found");
					}

					//FLOGV("SunOcclusion %f", SunOcclusion);
					if (Light)
					{
						float Intensity = 5 * FMath::Pow((1.0 - SunOcclusion), 2);
						//FLOGV("Light Intensity %f", Intensity);
						Light->SetIntensity(Intensity);
					}
					else
					{
						FLOG("AFlarePlanetarium::Tick : no sunlight found");
					}
				}
				else
				{
					FLOGV("AFlarePlanetarium::Tick : failed to find the current sector: '%s' in planetarium", *(PlayerOrbit->CelestialBodyIdentifier.ToString()));
				}


			} while(SkipNightTimeRange != 0);
		}
	}
}

inline static bool BodyDistanceComparator (const CelestialBodyPosition& ip1, const CelestialBodyPosition& ip2)
{
	return (ip1.Distance < ip2.Distance);
}

void AFlarePlanetarium::SetupCelestialBodies()
{
	// Sort by incresing distance
	BodyPositions.Sort(&BodyDistanceComparator);
	double BaseDistance = GetGame()->GetActiveSector()->GetSectorLimits() * 2; // Min distance, 15km
	double BaseIncrement = BaseDistance;
#ifdef PLANETARIUM_DEBUG
	DrawDebugSphere(GetWorld(), FVector::ZeroVector, (BaseDistance /2) /1000 , 32, FColor::Red, false);
#endif
	for(int32 BodyIndex = 0; BodyIndex < BodyPositions.Num(); BodyIndex++)
	{
		CelestialBodyPosition* BodyPosition = &BodyPositions[BodyIndex];

		// The surface must be at the base distance.
		// So the display is the base distance + the display radius
		// But the display radius depend on the base distance

		double EffectiveRadius = BodyPosition->Radius;

		// If outside the ring take ring radius instead of planet
		if(BodyIndex == 0 && BodyPosition->Distance > BodyPosition->Radius + BodyPosition->Body->RingsOuterAltitude)
		{
			EffectiveRadius += BodyPosition->Body->RingsOuterAltitude;
		}

		double AngularRadius = FPreciseMath::Atan(BodyPosition->Radius / BodyPosition->Distance);
		double AngularRadiusWithRings = FPreciseMath::Atan(EffectiveRadius / BodyPosition->Distance);

		double DisplayDistance = (BaseDistance) / (1- FPreciseMath::Tan(AngularRadiusWithRings));
		double DisplayRadius = FPreciseMath::Tan(AngularRadius) * DisplayDistance;

		SetupCelestialBody(BodyPosition, DisplayDistance, DisplayRadius);
#ifdef PLANETARIUM_DEBUG
		FLOGV("SetupCelestialBodies %s BodyPosition->Radius = %f", *BodyPosition->Body->Identifier.ToString(), BodyPosition->Radius);
		FLOGV("SetupCelestialBodies %s BodyPosition->Distance = %f", *BodyPosition->Body->Identifier.ToString(), BodyPosition->Distance);
		FLOGV("SetupCelestialBodies %s BaseDistance = %f", *BodyPosition->Body->Identifier.ToString(), BaseDistance);
		FLOGV("SetupCelestialBodies %s AngularRadius = %f", *BodyPosition->Body->Identifier.ToString(), AngularRadius);
		FLOGV("SetupCelestialBodies %s DisplayDistance = %f", *BodyPosition->Body->Identifier.ToString(), DisplayDistance);
		FLOGV("SetupCelestialBodies %s DisplayRadius = %f", *BodyPosition->Body->Identifier.ToString(), DisplayRadius);
		FLOGV("SetupCelestialBodies %s EffectiveRadius = %f", *BodyPosition->Body->Identifier.ToString(), EffectiveRadius);
#endif

		// Update BaseDistance for future bodies. Take margin for Nema rings
		if(BodyIndex == 0 && BodyPosition->Body->RingsOuterAltitude == 0)
		{
			BaseDistance = DisplayDistance + DisplayRadius / 2;
		}
		else
		{
			BaseDistance = DisplayDistance + DisplayRadius;
		}
	}
}

void AFlarePlanetarium::SetupCelestialBody(CelestialBodyPosition* BodyPosition, double DisplayDistance, double DisplayRadius)
{
	FVector PlayerShipLocation = FVector::ZeroVector;
	if (GetGame()->GetPC()->GetShipPawn())
	{
		PlayerShipLocation = GetGame()->GetPC()->GetShipPawn()->GetActorLocation();
	}

#ifdef PLANETARIUM_DEBUG
	DrawDebugSphere(GetWorld(), FVector::ZeroVector, DisplayDistance /1000 , 32, FColor::Blue, false);

	PlayerShipLocation = FVector::ZeroVector;
	DisplayRadius /= 1000;
	DisplayDistance /= 1000;
#endif

	BodyPosition->BodyComponent->SetRelativeLocation((DisplayDistance * BodyPosition->AlignedLocation.GetUnsafeNormal()).ToVector() + PlayerShipLocation);

	float Scale = DisplayRadius / 512; // Mesh size is 1024;
	BodyPosition->BodyComponent->SetRelativeScale3D(FPreciseVector(Scale).ToVector());

	FTransform BaseRotation = FTransform(FRotator(0, 0 ,90));
	FTransform TimeRotation = FTransform(FRotator(0, BodyPosition->TotalRotation, 0));

	FQuat Rotation = (TimeRotation * BaseRotation).GetRotation();

	// TODO Rotation float time interpolation
	BodyPosition->BodyComponent->SetRelativeRotation(FQuat::Identity);
	BodyPosition->BodyComponent->SetRelativeRotation(Rotation);

	// Apply sun direction to component
	UMaterialInstanceDynamic* ComponentMaterial = Cast<UMaterialInstanceDynamic>(BodyPosition->BodyComponent->GetMaterial(0));
	if (!ComponentMaterial)
	{
		ComponentMaterial = UMaterialInstanceDynamic::Create(BodyPosition->BodyComponent->GetMaterial(0) , GetWorld());
		BodyPosition->BodyComponent->SetMaterial(0, ComponentMaterial);
	}
	ComponentMaterial->SetVectorParameterValue("SunDirection", SunDirection.ToVector());

	// Look for rings and orient them
	TArray<USceneComponent*> RingCandidates;
	BodyPosition->BodyComponent->GetChildrenComponents(true, RingCandidates);
	for (int32 ComponentIndex = 0; ComponentIndex < RingCandidates.Num(); ComponentIndex++)
	{
		UStaticMeshComponent* RingComponent = Cast<UStaticMeshComponent>(RingCandidates[ComponentIndex]);

		if (RingComponent && RingComponent->GetName().Contains("ring"))
		{
			// Get or create the material
			UMaterialInstanceDynamic* RingMaterial = Cast<UMaterialInstanceDynamic>(RingComponent->GetMaterial(0));
			if (!RingMaterial)
			{
				RingMaterial = UMaterialInstanceDynamic::Create(RingComponent->GetMaterial(0), GetWorld());
				RingComponent->SetMaterial(0, RingMaterial);
			}

			// Get world-space rotation angles for the ring and the sun
			float SunRotationPitch = FMath::RadiansToDegrees(FMath::Atan2(SunDirection.Z,SunDirection.X)) + 180;
			float RingRotationPitch = -BodyPosition->TotalRotation;

			// Feed params to the shader
			RingMaterial->SetScalarParameterValue("RingPitch", RingRotationPitch / 360);
			RingMaterial->SetScalarParameterValue("SunPitch", SunRotationPitch / 360);
		}
	}

	// Sun also rotates to track direction
	if (BodyPosition->Body == &Sun)
	{
		BodyPosition->BodyComponent->SetRelativeRotation(SunDirection.ToVector().Rotation());
	}

	// Compute sun occlusion
	if (BodyPosition->Body != &Sun)
	{
		double OcclusionAngle = FPreciseMath::Asin(BodyPosition->Radius / BodyPosition->Distance);



		float BodyPhase =  FMath::UnwindRadians(FMath::Atan2(BodyPosition->AlignedLocation.Z, BodyPosition->AlignedLocation.X));
		float CenterAngularDistance = FMath::Abs(FMath::UnwindRadians(SunPhase - BodyPhase));
		float AngleSum = (SunOcclusionAngle + OcclusionAngle);
		float AngleDiff = FMath::Abs(SunOcclusionAngle - OcclusionAngle);

		/*FLOGV("SetupCelestialBody %s BodyPhase = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(BodyPhase));
		FLOGV("SetupCelestialBody %s SunPhase = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(SunPhase));
		FLOGV("SetupCelestialBody %s OcclusionAngle = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(OcclusionAngle));
		FLOGV("SetupCelestialBody %s SunOcclusionAngle = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(SunOcclusionAngle));
		FLOGV("SetupCelestialBody %s AngleDiff = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(AngleDiff));

		FLOGV("SetupCelestialBody %s CenterAngularDistance = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(CenterAngularDistance));
		FLOGV("SetupCelestialBody %s AngleSum = %f", *BodyPosition->Body->Name.ToString(), FMath::RadiansToDegrees(AngleSum));*/


		if (CenterAngularDistance < AngleSum)
		{
			// There is occlusion
			float OcclusionRatio;

			if (CenterAngularDistance < AngleDiff)
			{
				// Maximum occlusion
				OcclusionRatio = 1.0;
			}
			else
			{
				// Partial occlusion
				OcclusionRatio = (AngleSum - CenterAngularDistance) / (2* FMath::Min(SunOcclusionAngle, OcclusionAngle));

				// OcclusionRatio = ((SunOcclusionAngle + OcclusionAngle) + FMath::Max(SunOcclusionAngle, OcclusionAngle) - FMath::Min(SunOcclusionAngle, OcclusionAngle)) / (2 * CenterAngularDistance);
			}
			//FLOGV("MoveCelestialBody %s OcclusionRatio = %f", *Body->Name, OcclusionRatio);

			// Now, find the surface occlusion
			float SunAngularSurface = PI*FMath::Square(SunOcclusionAngle);
			float MaxOcclusionAngularSurface = PI*FMath::Square(FMath::Min(SunOcclusionAngle, OcclusionAngle));
			float MaxOcclusion = MaxOcclusionAngularSurface / SunAngularSurface;
			float Occlusion = OcclusionRatio * MaxOcclusion;

			/*FLOGV("SetupCelestialBody %s CenterAngularDistance = %f", *BodyPosition->Body->Name.ToString(), CenterAngularDistance);
			FLOGV("SetupCelestialBody %s SunOcclusionAngle = %f", *BodyPosition->Body->Name.ToString(), SunOcclusionAngle);
			FLOGV("SetupCelestialBody %s OcclusionAngle = %f", *BodyPosition->Body->Name.ToString(), OcclusionAngle);
			FLOGV("SetupCelestialBody %s SunAngularSurface = %f", *BodyPosition->Body->Name.ToString(), SunAngularSurface);
			FLOGV("SetupCelestialBody %s MaxOcclusionAngularSurface = %f", *BodyPosition->Body->Name.ToString(), MaxOcclusionAngularSurface);
			FLOGV("SetupCelestialBody %s MaxOcclusion = %f", *BodyPosition->Body->Name.ToString(), MaxOcclusion);
			FLOGV("SetupCelestialBody %s Occlusion = %f", *BodyPosition->Body->Name.ToString(), Occlusion);*/

			if (Occlusion > SunOcclusion)
			{
				// Keep only best occlusion
				SunOcclusion = Occlusion;
			}
		}
	}

}

void AFlarePlanetarium::PrepareCelestialBody(FFlareCelestialBody* Body, FPreciseVector Offset, double AngleOffset)
{
	CelestialBodyPosition BodyPosition;

	BodyPosition.Body = Body;
	FPreciseVector Location = Offset + Body->AbsoluteLocation;
	BodyPosition.AlignedLocation = Location.RotateAngleAxis(AngleOffset, FPreciseVector(0,1,0));
	BodyPosition.Radius = Body->Radius;
	BodyPosition.Distance = BodyPosition.AlignedLocation.Size();
	BodyPosition.TotalRotation = Body->RotationAngle + AngleOffset;

	// Find the celestial body component
	UStaticMeshComponent* BodyComponent = NULL;
	TArray<UActorComponent*> Components = GetComponentsByClass(UStaticMeshComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UStaticMeshComponent* ComponentCandidate = Cast<UStaticMeshComponent>(Components[ComponentIndex]);
		if (ComponentCandidate && ComponentCandidate->GetName() == Body->Identifier.ToString()	)
		{
			BodyComponent = ComponentCandidate;
			break;
		}
	}

	if (BodyComponent)
	{
		BodyPosition.BodyComponent = BodyComponent;
		BodyPositions.Add(BodyPosition);
	}
	else
	{
		FLOGV("AFlarePlanetarium::PrepareCelestialBody : no planetarium component for celestial body '%s'", *(Body->Identifier.ToString()));
	}


	if (Body == &Sun)
	{
		SunOcclusionAngle = FPreciseMath::Asin(BodyPosition.Radius / BodyPosition.Distance);
		SunPhase = FMath::UnwindRadians(FMath::Atan2(BodyPosition.AlignedLocation.Z, BodyPosition.AlignedLocation.X));
	}

	for (int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		PrepareCelestialBody(CelestialBody, Offset, AngleOffset);
	}
}

void AFlarePlanetarium::ResetTime()
{
	SmoothTime = 0;
}

void AFlarePlanetarium::SetTimeMultiplier(float Multiplier)
{
	TimeMultiplier = Multiplier;
}

void AFlarePlanetarium::SkipNight(float TimeRange)
{
	SkipNightTimeRange = TimeRange;
}

FVector AFlarePlanetarium::GetStellarDustVelocity() const
{
	return FVector(25000, 0, 0);
}

AFlareGame* AFlarePlanetarium::GetGame() const
{
	return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
}
