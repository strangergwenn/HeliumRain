
#include "../Flare.h"
#include "FlareMenuPawn.h"
#include "FlarePlayerController.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareMenuPawn::AFlareMenuPawn(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, InitialYaw(150)
	, DisplayDistance(1000)
	, DisplaySize(500)
	, SlideInOutUpOffset(0, 0, -2000)
	, SlideInOutSideOffset(0, 2000, 0)
	, SlideInOutTime(0.4)
	, CurrentSpacecraft(NULL)
	, SlideFromAToB(true)
	, SlideDirection(1)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> CurrentPart;
		FConstructorStatics()
			: CurrentPart(TEXT("/Game/Ships/Airframes/SM_Airframe_Ghoul"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	// Dummy root to allow better configuration of the subparts
	USceneComponent* SceneComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	CameraContainerYaw->AttachTo(SceneComponent);
	RootComponent = SceneComponent;

	// Part container turnplate
	PartContainer = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("PartContainer"));
	PartContainer->AttachTo(RootComponent);

	// Create static mesh component for the part
	CurrentPartA = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("PartA"));
	CurrentPartA->SetStaticMesh(ConstructorStatics.CurrentPart.Get());
	CurrentPartA->SetSimulatePhysics(false);
	CurrentPartA->AttachTo(PartContainer);

	// Create static mesh component for the part
	CurrentPartB = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("PartB"));
	CurrentPartB->SetStaticMesh(ConstructorStatics.CurrentPart.Get());
	CurrentPartB->SetSimulatePhysics(false);
	CurrentPartB->AttachTo(PartContainer);
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareMenuPawn::BeginPlay()
{
	Super::BeginPlay();
	ResetContent(true);
}

void AFlareMenuPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SlideInOutCurrentTime += DeltaSeconds;

	// Compute new positions for the current and old part
	float PartPositionAlpha = FMath::Clamp(SlideInOutCurrentTime / SlideInOutTime, 0.0f, 1.0f);
	FVector SlideInDelta = SlideDirection * FMath::InterpEaseOut(SlideInOutOffset, FVector::ZeroVector, PartPositionAlpha, 2);
	FVector SlideOutDelta = -SlideDirection * FMath::InterpEaseIn(FVector::ZeroVector, SlideInOutOffset, PartPositionAlpha, 1);

	// Part sliding
	CurrentPartA->SetRelativeLocation(CurrentPartOffsetA + (SlideFromAToB ? SlideOutDelta : SlideInDelta));
	CurrentPartB->SetRelativeLocation(CurrentPartOffsetB + (SlideFromAToB ? SlideInDelta : SlideOutDelta));

	// Ship sliding
	if (CurrentSpacecraft)
	{
		CurrentSpacecraft->SetActorLocation(GetActorLocation() + CurrentShipOffset + SlideInDelta);
	}

	// Camera
	float Speed = FMath::Clamp(DeltaSeconds * 12, 0.f, 1.f);
	ExternalCameraPitchTarget = FMath::Clamp(ExternalCameraPitchTarget, -GetCameraMaxPitch(), GetCameraMaxPitch());
	ExternalCameraPitch = ExternalCameraPitch * (1 - Speed) + ExternalCameraPitchTarget * Speed;

	SetCameraPitch(ExternalCameraPitch);

	float LastExternalCameraYaw = ExternalCameraYaw;
	ExternalCameraYaw = ExternalCameraYaw * (1 - Speed) + ExternalCameraYawTarget * Speed;
	FRotator DeltaRot = FRotator::MakeFromEuler(FVector(0, 0, ExternalCameraYaw - LastExternalCameraYaw));

	if (CurrentSpacecraft)
	{
		CurrentSpacecraft->AddActorLocalRotation(DeltaRot);
	}
	else
	{
		PartContainer->AddLocalRotation(DeltaRot);
	}
}


/*----------------------------------------------------
	Resource loading
----------------------------------------------------*/

void AFlareMenuPawn::ShowShip(const FFlareSpacecraftDescription* ShipDesc, const FFlareSpacecraftSave* ShipData)
{
	// Clean up
	ResetContent();

	// Spawn and setup the ship
	CurrentSpacecraft = GetWorld()->SpawnActor<AFlareSpacecraft>(ShipDesc->Template->GeneratedClass);
	CurrentSpacecraft->AttachRootComponentToActor(this, NAME_None, EAttachLocation::SnapToTarget);

	// Setup rotation and scale
	CurrentSpacecraft->SetActorScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentSpacecraft->GetMeshScale();
	FLOGV("DS=%f, MS=%f, S=%f", DisplaySize, CurrentSpacecraft->GetMeshScale(), Scale);
	CurrentSpacecraft->SetActorScale3D(Scale * FVector(1, 1, 1));
	CurrentSpacecraft->SetActorRelativeRotation(FRotator(0, InitialYaw, 0));

	// Slide
	SlideInOutCurrentTime = 0.0f;
	SlideInOutOffset = SlideInOutSideOffset;
	SetSlideDirection(true);

	CurrentSpacecraft->StartPresentation();

	// UI
	if (ShipData)
	{
		CurrentSpacecraft->Load(*ShipData);
	}
}

void AFlareMenuPawn::ShowPart(const FFlareSpacecraftComponentDescription* PartDesc)
{
	// Clean up
	ResetContent();

	// Choose a part to work with
	SlideFromAToB = !SlideFromAToB;
	UFlareSpacecraftComponent* CurrentPart = SlideFromAToB ? CurrentPartB : CurrentPartA;
	FVector& CurrentPartOffset = SlideFromAToB ? CurrentPartOffsetB : CurrentPartOffsetA;

	// Load the parts and scale accordingly
	CurrentPart->SetVisibility(true, true);
	FFlareSpacecraftComponentSave Data;
	Data.Damage = 0;
	Data.ComponentIdentifier = PartDesc->Identifier;
	CurrentPart->Initialize(&Data, GetPC()->GetCompany(), this, true);
	CurrentPart->SetWorldScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentPart->GetMeshScale();
	CurrentPart->SetWorldScale3D(Scale * FVector(1, 1, 1));

	// Reset position to compute the offsets
	CurrentPart->SetRelativeLocation(FVector::ZeroVector);
	FRotator OldRot = PartContainer->GetComponentRotation();
	PartContainer->SetRelativeRotation(FRotator::ZeroRotator);
	CurrentPart->SetRelativeRotation(FRotator(0, -InitialYaw, 0));

	// Setup offset and rotation
	CurrentPartOffset = GetActorLocation() - CurrentPart->Bounds.GetBox().GetCenter();
	PartContainer->SetRelativeRotation(OldRot);
	SlideInOutOffset = SlideInOutUpOffset;
	SlideInOutCurrentTime = 0.0f;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareMenuPawn::ResetContent(bool Unsafe)
{
	// Delete ship if existing
	if (CurrentSpacecraft && !Unsafe)
	{
		CurrentSpacecraft->Destroy();
		CurrentSpacecraft = NULL;
	}

	// Hide parts
	CurrentPartA->SetVisibility(false, true);
	CurrentPartB->SetVisibility(false, true);
	
	// Setup panning
	SetCameraPitch(-CameraMaxPitch / 2);
	ExternalCameraPitchTarget = -CameraMaxPitch / 2;
	ExternalCameraPitch = ExternalCameraPitchTarget;
	SetCameraDistance(DisplayDistance);
}

void AFlareMenuPawn::UpdateCustomization()
{
	if (CurrentSpacecraft)
	{
		CurrentSpacecraft->UpdateCustomization();
	}
	else
	{
		CurrentPartA->UpdateCustomization();
		CurrentPartB->UpdateCustomization();
	}
}

void AFlareMenuPawn::SetCameraOffset(FVector2D Offset)
{
	SetCameraLocalPosition(FVector(0, -Offset.X, -Offset.Y));
}

void AFlareMenuPawn::SetSlideDirection(bool GoUp)
{
	SlideDirection = GoUp ? 1 : -1;
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareMenuPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);
}

void AFlareMenuPawn::PitchInput(float Val)
{
	if (Val)
	{
		ExternalCameraPitchTarget += CameraPanSpeed * Val;
	}
}

void AFlareMenuPawn::YawInput(float Val)
{
	if (Val)
	{
		ExternalCameraYawTarget += - CameraPanSpeed * Val;
	}
}
