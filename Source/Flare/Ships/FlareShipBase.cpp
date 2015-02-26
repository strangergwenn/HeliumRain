
#include "../Flare.h"
#include "FlareShipBase.h"
#include "FlareRCS.h"
#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareShipBase::AFlareShipBase(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CameraPanSpeed(2)
	, CameraMaxPitch(60)
	, CameraMaxYaw(179)
	, CameraMinDistance(1.5)
	, CameraMaxDistance(4)
	, CameraDistanceStepAmount(0.5)
	, Company(NULL)
{
	// Camera containers
	CameraContainerYaw = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("CameraContainerYaw"));
	CameraContainerPitch = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("CameraContainerPitch"));
	CameraContainerPitch->AttachTo(CameraContainerYaw);
	
	// Camera component 
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	Camera->AttachTo(CameraContainerPitch);
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareShipBase::BeginPlay()
{
	Super::BeginPlay();
}

void AFlareShipBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// Apply interpolated values
	CameraContainerPitch->SetRelativeRotation(FRotator(CameraOffsetPitch, 0, 0).GetNormalized());
	CameraContainerYaw->SetRelativeRotation(FRotator(0, CameraOffsetYaw, 0).GetNormalized());
	Camera->SetRelativeLocation(FVector(-CameraOffsetDistance, 0, 0));
}


/*----------------------------------------------------
	Camera control
----------------------------------------------------*/

void AFlareShipBase::SetCameraPitch(float Value, bool ForceUpdate)
{
	CameraOffsetPitch = FMath::Clamp(Value, -CameraMaxPitch, +CameraMaxPitch);
}

void AFlareShipBase::SetCameraYaw(float Value, bool ForceUpdate)
{
	CameraOffsetYaw = FMath::Clamp(Value, -CameraMaxYaw, +CameraMaxYaw);
}

void AFlareShipBase::SetCameraDistance(float Value, bool ForceUpdate)
{
	CameraOffsetDistance = Value;
}

void AFlareShipBase::StepCameraDistance(bool TowardCenter)
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

void AFlareShipBase::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}

void AFlareShipBase::ReloadPart(UFlareShipModule* Target, const FFlareShipModuleDescription* TargetDescription)
{
	Target->Initialize(TargetDescription, Company, this);
}

void AFlareShipBase::ReloadAllParts(const UClass* TargetClass, const FFlareShipModuleDescription* TargetDescription)
{
	// Required data
	TArray<UActorComponent*> ActorComponents;
	GetComponents(ActorComponents);

	// Find all components of the target type
	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		UFlareShipModule* Module = Cast<UFlareShipModule>(*ComponentIt);
		if (Module && Module->GetClass() == TargetClass)
		{
			ReloadPart(Module, TargetDescription);
		}
	}
}

void AFlareShipBase::UpdateCustomization()
{
	// Required data
	TArray<UActorComponent*> ActorComponents;
	GetComponents(ActorComponents);

	// Find all components
	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		UFlareShipModule* Module = Cast<UFlareShipModule>(*ComponentIt);
		if (Module)
		{
			if (!Module->IsInitialized())
			{
				ReloadPart(Module, NULL);
			}
			Module->UpdateCustomization();
		}
	}
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

float AFlareShipBase::GetRotationAmount(const FQuat& X, bool ClampToHalfTurn)
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

FQuat AFlareShipBase::ClampQuaternion(const FQuat& X, float Angle)
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

float AFlareShipBase::TriStateRegulator(float Target, float Current, float Threshold)
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

FQuat AFlareShipBase::WorldToLocal(FQuat World)
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

FVector AFlareShipBase::WorldToLocal(FVector World)
{
	return RootComponent->GetComponentTransform().InverseTransformVectorNoScale(World);
}

AFlarePlayerController* AFlareShipBase::GetPC() const
{
	return Cast<AFlarePlayerController>(GetController());
}

AFlareGame* AFlareShipBase::GetGame() const
{
	return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
}

float AFlareShipBase::GetMeshScale() const
{
	FVector Origin;
	FVector Extent;
	FBox Box = GetComponentsBoundingBox();
	return FMath::Max(Box.GetExtent().Size(), 1.0f);
}
