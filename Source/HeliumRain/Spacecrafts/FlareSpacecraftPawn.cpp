
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
	, CameraMinDistance(1.5)
	, CameraMaxDistance(4)
	, CameraDistanceStepAmount(0.5)
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
	CameraContainerPitch->SetRelativeRotation(FRotator(CameraOffsetPitch, 0, 0).GetNormalized());
	CameraContainerYaw->SetRelativeRotation(FRotator(0, CameraOffsetYaw, 0).GetNormalized());
	Camera->SetRelativeLocation(CameraLocalPosition - FVector(CameraOffsetDistance, 0, 0));
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

void AFlareSpacecraftPawn::StepCameraDistance(bool TowardCenter)
{
	// Compute camera data
	float Scale = GetMeshScale();
	float LimitNear = Scale * CameraMinDistance;
	float LimitFar = Scale * CameraMaxDistance;
	float Offset = Scale * (TowardCenter ? -CameraDistanceStepAmount : CameraDistanceStepAmount);

	// Move camera
	SetCameraDistance(FMath::Clamp(CameraOffsetDistance + Offset, LimitNear, LimitFar));
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

float AFlareSpacecraftPawn::GetRotationAmount(const FQuat& X, bool ClampToHalfTurn)
{
	float Angle;
	FVector Vector;
	X.ToAxisAndAngle(Vector, Angle);

	if (ClampToHalfTurn)
	{
		return FMath::Clamp(FMath::RadiansToDegrees(Angle), 0.0f, 180.0f);
	}
	else
	{
		return FMath::RadiansToDegrees(Angle);
	}
}

FQuat AFlareSpacecraftPawn::ClampQuaternion(const FQuat& X, float Angle)
{
	float RotAngle = GetRotationAmount(X);

	if (RotAngle > Angle)
	{
		return FQuat::Slerp(FQuat::MakeFromEuler(FVector::ZeroVector), X, Angle / RotAngle);
	}
	else
	{
		return X;
	}
}

float AFlareSpacecraftPawn::TriStateRegulator(float Target, float Current, float Threshold)
{
	float Diff = Target - Current;
	float AbsDiff = FMath::Abs(Diff);

	if (AbsDiff > Threshold)
	{
		return (Diff > 0) ? -1 : 1;
	}
	else
	{
		return 0;
	}
}

FQuat AFlareSpacecraftPawn::WorldToLocal(const FQuat& World)
{
	FRotator WorldRotator = World.Rotator();
	FTransform ParentWorldTransform = RootComponent->GetComponentTransform();

	FVector Forward = ParentWorldTransform.InverseTransformVector(FRotationMatrix(WorldRotator).GetScaledAxis(EAxis::X));
	FVector Right = ParentWorldTransform.InverseTransformVector(FRotationMatrix(WorldRotator).GetScaledAxis(EAxis::Y));
	FVector Up = ParentWorldTransform.InverseTransformVector(FRotationMatrix(WorldRotator).GetScaledAxis(EAxis::Z));

	FMatrix RotMatrix(Forward, Right, Up, FVector::ZeroVector);
	FQuat Result = RotMatrix.ToQuat();
	Result.Normalize();

	return Result;
}

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
	// TODO check diameter/radius usage
	FVector Origin;
	FVector Extent;
	FBox Box = GetComponentsBoundingBox();
	return FMath::Max(Box.GetExtent().Size(), 1.0f);
}

#undef LOCTEXT_NAMESPACE
