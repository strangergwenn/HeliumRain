
#include "../Flare.h"
#include "FlarePlanetarium.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"


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
				FLOGV("AFlarePlanetarium::BeginPlay : found planet '%s'", *PlanetCandidate->GetName());

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
				FFlareSectorOrbitParameters* PlayerOrbit = GetGame()->GetActiveSector()->GetSimulatedSector()->GetOrbitParameters();


				FFlareCelestialBody* CurrentParent = World->GetPlanerarium()->FindCelestialBody(PlayerOrbit->CelestialBodyIdentifier);
				if (CurrentParent)
				{
					FPreciseVector ParentLocation = CurrentParent->AbsoluteLocation;

					double DistanceToParentCenter = CurrentParent->Radius + PlayerOrbit->Altitude;
					FPreciseVector PlayerLocation =  ParentLocation + World->GetPlanerarium()->GetRelativeLocation(CurrentParent, LocalTime, SmoothTime, DistanceToParentCenter, 0, PlayerOrbit->Phase);
					/*FLOGV("Parent location = %s", *CurrentParent->AbsoluteLocation.ToString());
					FLOGV("PlayerLocation = %s", *PlayerLocation.ToString());*/
		/*			DrawDebugLine(GetWorld(), FVector(1000, 0 ,0), FVector(- 1000, 0 ,0), FColor::Red, false);
					DrawDebugLine(GetWorld(), FVector(0, 1000 ,0), FVector(0,- 1000 ,0), FColor::Green, false);
					DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, -1000), FColor::Blue, false);
					DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, 1000), FColor::Cyan, false);
	*/
					FPreciseVector DeltaLocation = ParentLocation - PlayerLocation;
					FPreciseVector SunDeltaLocation = Sun.AbsoluteLocation - PlayerLocation;

					float AngleOffset =  90 + FMath::RadiansToDegrees(FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
					/*FLOGV("DeltaLocation = %s", *DeltaLocation.ToString());
					FLOGV("FMath::Atan2(DeltaLocation.Y,DeltaLocation.X)  = %f", FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
					FLOGV("AngleOffset  = %f", AngleOffset);*/

					FPreciseVector SunDirection = -(SunDeltaLocation.RotateAngleAxis(AngleOffset, FPreciseVector(0,1,0))).GetUnsafeNormal();
					// Reset sun occlusion;
					SunOcclusion = 0;

					MoveCelestialBody(&Sun, -PlayerLocation, AngleOffset, SunDirection);

					if(SkipNightTimeRange > 0 && SunOcclusion >= 1)
					{
						//Try to find night
						SmoothTime = FMath::FRand() * SkipNightTimeRange;
						FLOGV("AFlarePlanetarium::Tick : night, try to find light at %f (max %f)",SmoothTime, SkipNightTimeRange);
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
						float Intensity = 10 * FMath::Pow((1.0 - SunOcclusion), 2);
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

void AFlarePlanetarium::MoveCelestialBody(FFlareCelestialBody* Body, FPreciseVector Offset, double AngleOffset, FPreciseVector SunDirection)
{
	double BaseDistance = 1e7;

	FPreciseVector Location = Offset + Body->AbsoluteLocation;
	FPreciseVector AlignedLocation = Location.RotateAngleAxis(AngleOffset, FPreciseVector(0,1,0));


	double AngularRadius = FMath::Asin(Body->Radius / AlignedLocation.Size());

	double DisplayDistance = FMath::Sin(AngularRadius) * BaseDistance / 5 + BaseDistance + AlignedLocation.Size() / 10;


	double VisibleRadius = FMath::Sin(AngularRadius) * DisplayDistance;


	/*FLOGV("MoveCelestialBody %s VisibleRadius = %f", *Body->Name, VisibleRadius);
	FLOGV("MoveCelestialBody %s AngularRadius = %f", *Body->Name, AngularRadius);
	FLOGV("MoveCelestialBody %s DisplayDistance = %f", *Body->Name, DisplayDistance);


	FLOGV("MoveCelestialBody %s Location = %s", *Body->Name, *Location.ToString());
	FLOGV("MoveCelestialBody %s AlignedLocation = %s", *Body->Name, *AlignedLocation.ToString());*/

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
		FVector PlayerShipLocation = FVector::ZeroVector;
		if (GetGame()->GetPC()->GetShipPawn())
		{
			PlayerShipLocation = GetGame()->GetPC()->GetShipPawn()->GetActorLocation();
		}
		BodyComponent->SetRelativeLocation((DisplayDistance * AlignedLocation.GetUnsafeNormal()).ToVector() + PlayerShipLocation);
		float Scale = VisibleRadius / 512; // Mesh size is 1024;
		BodyComponent->SetRelativeScale3D(FPreciseVector(Scale).ToVector());

		// BodyComponent->SetRelativeRotation(FRotator(90, Body->RotationAngle + AngleOffset ,0));
		// BodyComponent->SetRelativeRotation(FRotator(0, -90 ,0));

		/*FLOGV("MoveCelestialBody %s Body->RotationAngle = %f", *Body->Name, Body->RotationAngle);
		FLOGV("MoveCelestialBody %s AngleOffset = %f", *Body->Name, AngleOffset);
		FLOGV("MoveCelestialBody %s Body->RotationAngle + AngleOffset = %f", *Body->Name, (Body->RotationAngle + AngleOffset));*/

		double TotalRotation = Body->RotationAngle + AngleOffset;

		FTransform BaseRotation = FTransform(FRotator(0, 0 ,90));
		FTransform TimeRotation = FTransform(FRotator(0, TotalRotation, 0));

		FQuat Rotation = (TimeRotation * BaseRotation).GetRotation();

		// TODO Rotation float time interpolation
		BodyComponent->SetRelativeRotation(FQuat::Identity);
		BodyComponent->SetRelativeRotation(Rotation);

		// Apply sun direction to component
		UMaterialInstanceDynamic* ComponentMaterial = Cast<UMaterialInstanceDynamic>(BodyComponent->GetMaterial(0));
		if (!ComponentMaterial)
		{
			ComponentMaterial = UMaterialInstanceDynamic::Create(BodyComponent->GetMaterial(0) , GetWorld());
			BodyComponent->SetMaterial(0, ComponentMaterial);
		}
		ComponentMaterial->SetVectorParameterValue("SunDirection", SunDirection.ToVector());

		// Look for rings and orient them
		TArray<USceneComponent*> RingCandidates;
		BodyComponent->GetChildrenComponents(true, RingCandidates);
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
				float RingRotationPitch = -TotalRotation;

				// Feed params to the shader
				RingMaterial->SetScalarParameterValue("RingPitch", RingRotationPitch / 360);
				RingMaterial->SetScalarParameterValue("SunPitch", SunRotationPitch / 360);
			}
		}

		// Sun also rotates to track direction
		if (Body == &Sun)
		{
			BodyComponent->SetRelativeRotation(SunDirection.ToVector().Rotation());
		}
	}
	else
	{
		FLOGV("AFlarePlanetarium::MoveCelestialBody : no planetarium component for celestial body '%s'", *(Body->Identifier.ToString()));
	}

	/*DrawDebugLine(GetWorld(), FVector(0, 0, 0), AlignedLocation * 100000, FColor::Blue, false, 1.f);
	DrawDebugLine(GetWorld(), FVector(0, 0, 0), AlignedLocation.RotateAngleAxis(FMath::RadiansToDegrees(AngularRadius), FVector(0,1,0)) * 100000, FColor::Red, false, 1.f);
	DrawDebugLine(GetWorld(), FVector(0, 0, 0), AlignedLocation.RotateAngleAxis(-FMath::RadiansToDegrees(AngularRadius), FVector(0,1,0)) * 100000, FColor::Green, false, 1.f);
*/
	// Compute sun occlusion
	if (Body != &Sun)
	{
		float BodyPhase =  FMath::UnwindRadians(FMath::Atan2(AlignedLocation.Z, AlignedLocation.X));



		float CenterAngularDistance = FMath::Abs(FMath::UnwindRadians(SunPhase - BodyPhase));
		float AngleSum = (SunAnglularRadius + AngularRadius);
		float AngleDiff = FMath::Abs(SunAnglularRadius - AngularRadius);

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
				OcclusionRatio = (AngleSum - CenterAngularDistance) / (2* FMath::Min(SunAnglularRadius, AngularRadius));

				// OcclusionRatio = ((SunAnglularRadius + AngularRadius) + FMath::Max(SunAnglularRadius, AngularRadius) - FMath::Min(SunAnglularRadius, AngularRadius)) / (2 * CenterAngularDistance);
			}
			//FLOGV("MoveCelestialBody %s OcclusionRatio = %f", *Body->Name, OcclusionRatio);

			// Now, find the surface occlusion
			float SunAngularSurface = PI*FMath::Square(SunAnglularRadius);
			float MaxOcclusionAngularSurface = PI*FMath::Square(FMath::Min(SunAnglularRadius, AngularRadius));
			float MaxOcclusion = MaxOcclusionAngularSurface/SunAngularSurface;
			float Occlusion = OcclusionRatio * MaxOcclusion;


			//FLOGV("MoveCelestialBody %s OcclusionRatioSmooth = %f", *Body->Name, OcclusionRatioSmooth);
			/*FLOGV("MoveCelestialBody %s CenterAngularDistance = %f", *Body->Name, CenterAngularDistance);
			FLOGV("MoveCelestialBody %s SunAnglularRadius = %f", *Body->Name, SunAnglularRadius);
			FLOGV("MoveCelestialBody %s AngularRadius = %f", *Body->Name, AngularRadius);
			FLOGV("MoveCelestialBody %s SunAngularSurface = %f", *Body->Name, SunAngularSurface);
			FLOGV("MoveCelestialBody %s MaxOcclusionAngularSurface = %f", *Body->Name, MaxOcclusionAngularSurface);
			FLOGV("MoveCelestialBody %s MaxOcclusion = %f", *Body->Name, MaxOcclusion);

			FLOGV("MoveCelestialBody %s Occlusion = %f", *Body->Name, Occlusion);*/


			if (Occlusion > SunOcclusion)
			{
				// Keep only best occlusion
				SunOcclusion = Occlusion;
			}

		}
	}
	else
	{
		SunAnglularRadius = AngularRadius;
		SunPhase = FMath::UnwindRadians(FMath::Atan2(AlignedLocation.Z, AlignedLocation.X));
	}

	for (int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		MoveCelestialBody(CelestialBody, Offset, AngleOffset, SunDirection);
	}
}

void AFlarePlanetarium::ResetTime()
{
	FLOGV("AFlarePlanetarium::ResetTime : %f", SmoothTime);
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



