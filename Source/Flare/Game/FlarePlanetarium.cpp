
#include "../Flare.h"
#include "FlarePlanetarium.h"
#include "FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlanetarium::AFlarePlanetarium(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFlarePlanetarium::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AFlareGame* Game =  Cast<AFlareGame>(GetWorld()->GetAuthGameMode());

	if(Game)
	{
		if(!Game->GetActiveSector())
		{
			// No active sector, do nothing
			return;
		}

		UFlareWorld* World = Game->GetGameWorld();

		if (World)
		{
			if (Sky == NULL)
			{
				for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					FLOGV("Is Sky ? %s", *(*ActorItr)->GetName());
					if ((*ActorItr)->GetName().StartsWith("SM_Sky"))
					{
						FLOG("Sky found");
						Sky = Cast<AStaticMeshActor>(*ActorItr);
						break;
					}
				}
			}


			FVector BaseOffset = FVector(1000000*10,0,0);
			FFlareCelestialBody Sun = World->GetPlanerarium()->GetSnapShot(World->GetTime());


			// Draw Player
			FFlareCelestialBody* CurrentParent = World->GetPlanerarium()->FindCelestialBody("planet-nema"); // TODO use active sector parent planet
			if (CurrentParent)
			{
				float DistanceScaleRatio = 100./10000;
				FVector ParentLocation = CurrentParent->AbsoluteLocation;
				FVector PlayerLocation =  ParentLocation + World->GetPlanerarium()->GetRelativeLocation(CurrentParent, World->GetTime(), 200000, 0, 0); // TODO use sector phase and distance
			/*	FLOGV("Nema location = %s", *CurrentParent->AbsoluteLocation.ToString());
				FLOGV("PlayerLocation = %s", *PlayerLocation.ToString());
				FLOGV("Player relative = %s", *(World->GetPlanerarium()->GetRelativeLocation(CurrentParent, World->GetTime(), 100000, 0, 0)).ToString());
				DrawDebugLine(GetWorld(), FVector(1000, 0 ,0), FVector(- 1000, 0 ,0), FColor::Red, false);
				DrawDebugLine(GetWorld(), FVector(0, 1000 ,0), FVector(0,- 1000 ,0), FColor::Green, false);
				DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, -1000), FColor::Blue, false);
				DrawDebugLine(GetWorld(), FVector(0, 0, 900), FVector(0, 0, 1000), FColor::Cyan, false);
*/
				FVector DeltaLocation = ParentLocation - PlayerLocation;
				FVector SunDeltaLocation = Sun.AbsoluteLocation - PlayerLocation;

				float AngleOffset =  90 + FMath::RadiansToDegrees(FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
			/*	FLOGV("DeltaLocation = %s", *DeltaLocation.ToString());
				FLOGV("FMath::Atan2(DeltaLocation.Y,DeltaLocation.X)  = %f", FMath::Atan2(DeltaLocation.Z,DeltaLocation.X));
				FLOGV("AngleOffset  = %f", AngleOffset);
*/
				FVector SunDirection = -(SunDeltaLocation.RotateAngleAxis(AngleOffset, FVector(0,1,0))).GetUnsafeNormal();

				MoveCelestialBody(&Sun, -PlayerLocation, AngleOffset, SunDirection);

				if (Sky)
				{
					Sky->SetActorRotation(FRotator(-AngleOffset, 0 , 0));
					//FLOGV("Sky %s rotation= %s",*Sky->GetName(),  *Sky->GetActorRotation().ToString());
				}
				else
				{
					FLOG("Error: No sky found");
				}


			}
			else
			{
				FLOG("Error: Failed to find the current sector: 'planet-nema' in planetarium");
			}
		}
	}
}

void AFlarePlanetarium::MoveCelestialBody(FFlareCelestialBody* Body, FVector Offset, float AngleOffset, FVector SunDirection)
{

	//float BaseDistance = 1e9;
	float BaseDistance = 1e7;



	float DistanceScaleRatio = 100./10000;
	//float RadiusScaleRatio = 100./70000;
	float RadiusScaleRatio = 100./10000;
	FVector Location = Offset + Body->AbsoluteLocation;
	FVector AlignedLocation = Location.RotateAngleAxis(AngleOffset, FVector(0,1,0));


	float VisibleAngle = FMath::Atan2(Body->Radius, AlignedLocation.Size());

	float DisplayDistance = BaseDistance + AlignedLocation.Size() / 100;


	float VisibleRadius = FMath::Tan(VisibleAngle) * DisplayDistance;


	/*FLOGV("VisibleAngle %s VisibleAngle = %f", *Body->Name, VisibleAngle);
	FLOGV("VisibleRadius %s VisibleRadius = %f", *Body->Name, VisibleRadius);
	FLOGV("DisplayDistance %s DisplayDistance = %f", *Body->Name, DisplayDistance);


	FLOGV("MoveCelestialBody %s Location = %s", *Body->Name, *Location.ToString());
	FLOGV("MoveCelestialBody %s AlignedLocation = %s", *Body->Name, *AlignedLocation.ToString());
*/
	// Find the celestial body component
	TArray<UActorComponent*> Components = GetComponents();
	UStaticMeshComponent* BodyComponent = NULL;
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UStaticMeshComponent* ComponentCandidate = Cast<UStaticMeshComponent>(Components[ComponentIndex]);
		if(ComponentCandidate && ComponentCandidate->GetName() == Body->Identifier)
		{
			BodyComponent = ComponentCandidate;
			break;
		}
	}

	if (BodyComponent)
	{
		BodyComponent->SetRelativeLocation(DisplayDistance * AlignedLocation.GetUnsafeNormal());
		float Scale = VisibleRadius / 512; // Mesh size is 1024;
		BodyComponent->SetRelativeScale3D(FVector(Scale));

		//BodyComponent->SetRelativeRotation(FRotator(90, Body->RotationAngle + AngleOffset ,0));
		//BodyComponent->SetRelativeRotation(FRotator(0, -90 ,0));

		FTransform BaseRotation = FTransform(FRotator(0, 0 ,90));
		FTransform TimeRotation = FTransform(FRotator(0, Body->RotationAngle + AngleOffset, 0));

		FRotator Rotation = (TimeRotation * BaseRotation).Rotator();
		BodyComponent->SetRelativeRotation(Rotation);

		UMaterialInstanceDynamic* ComponentMaterial = Cast<UMaterialInstanceDynamic>(BodyComponent->GetMaterial(0));
		if(!ComponentMaterial)
		{
			ComponentMaterial = UMaterialInstanceDynamic::Create(BodyComponent->GetMaterial(0) , GetWorld());
			BodyComponent->SetMaterial(0, ComponentMaterial);
		}
		ComponentMaterial->SetVectorParameterValue("SunDirection", SunDirection);
	}
	else
	{
		FLOGV("ERROR: No planetarium component for '%s' celestial body", *Body->Identifier);
	}

	for(int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		MoveCelestialBody(CelestialBody, Offset, AngleOffset, SunDirection);
	}
}


