
#include "../Flare.h"
#include "FlareSpacecraftPawn.h"
#include "FlareRCS.h"
#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"


#define LOCTEXT_NAMESPACE "FlareSpacecraftPawn"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareSpacecraftPawn::AFlareSpacecraftPawn(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, PresentationMode(false)
	, CameraPanSpeed(2)
	, CameraMaxPitch(60)
	, CameraMaxYaw(180)
	, Company(NULL)
{
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);

	// Camera containers
	CameraContainerYaw = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("CameraContainerYaw"));
	CameraContainerPitch = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("CameraContainerPitch"));
	CameraContainerPitch->AttachToComponent(CameraContainerYaw, AttachRules);
	
	// Camera component 
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	Camera->AttachToComponent(CameraContainerPitch, AttachRules);
	MeshScaleCache = -1;
	UseImmersiveCamera = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareSpacecraftPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AFlareSpacecraftPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// Apply interpolated values
	if(!UseImmersiveCamera)
	{
		CameraContainerPitch->SetRelativeRotation(FRotator(CameraOffsetPitch, 0, 0).GetNormalized());
		CameraContainerYaw->SetRelativeRotation(FRotator(0, CameraOffsetYaw, 0).GetNormalized());
		Camera->SetRelativeLocation(CameraLocalPosition - FVector(CameraOffsetDistance, 0, 0));
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
	else
	{
		// Find best FLIR camera
		TArray<FName> SocketNames = GetRootComponent()->GetAllSocketNames();
		float BestAngle = 0;
		FVector CameraMainDirection;
		FVector BestCameraLocation;
		bool FlirCameraFound = false;
		FName BestCameraName;

		// Find the best FLIR camera on the ship if any
		for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); SocketIndex++)
		{
			if (SocketNames[SocketIndex].ToString().StartsWith("FLIR"))
			{
				FTransform CameraWorldTransform = GetRootComponent()->GetSocketTransform(SocketNames[SocketIndex]);

				FVector CameraLocation = CameraWorldTransform.GetTranslation();
				FVector CandidateCameraMainDirection = CameraWorldTransform.GetRotation().RotateVector(FVector(1, 0, 0));

				float Angle = FMath::RadiansToDegrees((FMath::Acos(FVector::DotProduct(ImmersiveTargetDirection, CandidateCameraMainDirection))));

				if (!FlirCameraFound || Angle < BestAngle)
				{
					// Select camera
					BestAngle = Angle;
					BestCameraLocation = CameraLocation;
					CameraMainDirection = CandidateCameraMainDirection;
					FlirCameraFound = true;
					BestCameraName = SocketNames[SocketIndex];
				}
			}
		}

		// Update the FLIR camera
		if (FlirCameraFound)
		{
			FRotator CameraRotation = ImmersiveTargetDirection.Rotation();

			// Try to keep top
			FVector OldTopVector = GetCameraTopVector();
			FLOGV("OldTopVector %s", *OldTopVector.ToString());



			// TODO field
			//static float LastRoll = 0;

			CameraRotation.Roll = 0;
			FVector NewTopVector = CameraRotation.RotateVector(FVector(0, 0, 1));
			float Angle = FMath::RadiansToDegrees((FMath::Acos(FVector::DotProduct(OldTopVector, NewTopVector))));

			//FLOGV("LastRoll %f", LastRoll);
			//FLOGV("Angle %f", Angle);


			CameraRotation.Roll = Angle ;

			/*if(!FMath::IsNearlyZero(Angle))
			{
				FVector AngleSignAxis = FVector::CrossProduct(OldTopVector, NewTopVector);
				float AngleSignAxisDot = FVector::DotProduct(ImmersiveTargetDirection, AngleSignAxis);

				FLOGV("AngleSignAxisDot %f", AngleSignAxisDot);

				CameraRotation.Roll += Angle ;// FMath::Sign(AngleSignAxisDot);

			}*/
			//LastRoll = CameraRotation.Roll;

			FVector NewTopVector2 = CameraRotation.RotateVector(FVector(0, 0, 1));
			float Angle2 = FMath::RadiansToDegrees((FMath::Acos(FVector::DotProduct(OldTopVector, NewTopVector2))));
			FLOGV("Angle2 %f", Angle2);

			CameraContainerPitch->SetRelativeRotation(FRotator(0, 0, 0).GetNormalized());
			CameraContainerYaw->SetRelativeRotation(FRotator(0, 0, 0).GetNormalized());
			Camera->SetWorldLocationAndRotation(BestCameraLocation , CameraRotation);
			ImmersiveTopDirection = Camera->ComponentToWorld.TransformVector(FVector(0, 0, 1));

			FLOGV("NewTopVector %s", *GetCameraTopVector().ToString());
		}
	}
}


/*----------------------------------------------------
	Camera control
----------------------------------------------------*/

void AFlareSpacecraftPawn::SetCameraPitch(float Value)
{
	CameraOffsetPitch = FMath::Clamp(Value, -CameraMaxPitch, +CameraMaxPitch);
}

void AFlareSpacecraftPawn::SetCameraYaw(float Value)
{
	CameraOffsetYaw = FMath::Clamp(Value, -CameraMaxYaw, +CameraMaxYaw);
}

void AFlareSpacecraftPawn::SetCameraLocalPosition(FVector Value)
{
	CameraLocalPosition = Value;
}

void AFlareSpacecraftPawn::SetCameraDistance(float Value)
{
	CameraOffsetDistance = Value;
}

void AFlareSpacecraftPawn::ConfigureImmersiveCamera(FVector TargetDirection)
{
	if (!UseImmersiveCamera)
	{
		ImmersiveTopDirection = GetCameraTopVector();
	}

	UseImmersiveCamera = true;
	ImmersiveTargetDirection = TargetDirection;
}

void AFlareSpacecraftPawn::DisableImmersiveCamera()
{
	UseImmersiveCamera = false;
}

FVector AFlareSpacecraftPawn::GetCameraTopVector()
{
	if(UseImmersiveCamera)
	{
		return Camera->ComponentToWorld.TransformVector(FVector(0, 0, 1));
	}
	else
	{
		return ImmersiveTopDirection;
	}
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlareSpacecraftPawn::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}

void AFlareSpacecraftPawn::ReloadPart(UFlareSpacecraftComponent* Target, FFlareSpacecraftComponentSave* Data)
{
	Target->Initialize(Data, Company, this);
}

void AFlareSpacecraftPawn::UpdateCustomization()
{
	// Required data
	TArray<UActorComponent*> ActorComponents;
	GetComponents(ActorComponents);

	// Find all components
	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(*ComponentIt);
		if (Component)
		{
			if (!Component->IsInitialized())
			{
				ReloadPart(Component, NULL);
			}
			Component->UpdateCustomization();
		}
	}
}

void AFlareSpacecraftPawn::StartPresentation()
{
	PresentationMode = true;
	PresentationModeStarted();
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FVector AFlareSpacecraftPawn::WorldToLocal(FVector World)
{
	return RootComponent->GetComponentTransform().InverseTransformVectorNoScale(World);
}

AFlarePlayerController* AFlareSpacecraftPawn::GetPC() const
{
	return GetGame()->GetPC();
}

bool AFlareSpacecraftPawn::IsFlownByPlayer() const
{
	AFlarePlayerController* PC = GetGame()->GetPC();
	if (PC)
	{
		return (PC->GetShipPawn() == this);
	}
	else
	{
		return false;
	}
}

AFlareGame* AFlareSpacecraftPawn::GetGame() const
{
	return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
}

float AFlareSpacecraftPawn::GetMeshScale() const
{
	if(MeshScaleCache < 0)
	{
		FBox Box = GetComponentsBoundingBox();
		AFlareSpacecraftPawn* UnprotectedThis = const_cast<AFlareSpacecraftPawn*>(this);
		UnprotectedThis->MeshScaleCache = FMath::Max(Box.GetExtent().Size(), 1.0f);
	}
	return MeshScaleCache;
}

bool AFlareSpacecraftPawn::IsPlayerShip()
{
	return (this == GetPC()->GetShipPawn());
}

#undef LOCTEXT_NAMESPACE
