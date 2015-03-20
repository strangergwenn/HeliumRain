
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
	, CurrentShip(NULL)
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
	CurrentPartA = PCIP.CreateDefaultSubobject<UFlareShipModule>(this, TEXT("PartA"));
	CurrentPartA->SetStaticMesh(ConstructorStatics.CurrentPart.Get());
	CurrentPartA->SetSimulatePhysics(false);
	CurrentPartA->AttachTo(PartContainer);

	// Create static mesh component for the part
	CurrentPartB = PCIP.CreateDefaultSubobject<UFlareShipModule>(this, TEXT("PartB"));
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
	if (CurrentShip)
	{
		CurrentShip->SetActorLocation(GetActorLocation() + CurrentShipOffset + SlideInDelta);
	}

	// Station sliding
	if (CurrentStation)
	{
		CurrentStation->SetActorLocation(GetActorLocation() + CurrentShipOffset + SlideInDelta);
	}
}


/*----------------------------------------------------
	Resource loading
----------------------------------------------------*/

void AFlareMenuPawn::ShowShip(const FFlareShipDescription* ShipDesc, const FFlareShipSave* ShipData)
{
	// Clean up
	ResetContent();

	// Spawn and setup the ship
	CurrentShip = GetWorld()->SpawnActor<AFlareShip>(ShipDesc->Template->GeneratedClass);
	CurrentShip->AttachRootComponentToActor(this, NAME_None, EAttachLocation::SnapToTarget);

	// Setup rotation and scale
	CurrentShip->SetActorScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentShip->GetMeshScale();
	FLOGV("DS=%f, MS=%f, S=%f", DisplaySize, CurrentShip->GetMeshScale(), Scale);
	CurrentShip->SetActorScale3D(Scale * FVector(1, 1, 1));
	CurrentShip->SetActorRelativeRotation(FRotator(0, InitialYaw, 0));

	// Slide
	SlideInOutCurrentTime = 0.0f;
	SlideInOutOffset = SlideInOutSideOffset;
	SetSlideDirection(true);

	// UI
	if (ShipData)
	{
		CurrentShip->Load(*ShipData);
	}
	CurrentShip->StartPresentation();
}

void AFlareMenuPawn::ShowStation(const FFlareStationDescription* StationDesc, const FFlareStationSave* StationData)
{
	// Clean up
	ResetContent();

	// Spawn and setup the station
	CurrentStation = GetWorld()->SpawnActor<AFlareStation>(StationDesc->Template->GeneratedClass);
	CurrentStation->AttachRootComponentToActor(this, NAME_None, EAttachLocation::SnapToTarget);

	// Setup rotation and scale
	CurrentStation->SetActorScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentStation->GetMeshScale();
	CurrentStation->SetActorScale3D(Scale * FVector(1, 1, 1));
	CurrentStation->SetActorRelativeRotation(FRotator(0, InitialYaw, 0));

	// Slide
	SlideInOutCurrentTime = 0.0f;
	SlideInOutOffset = SlideInOutSideOffset;
	SetSlideDirection(true);

	// UI
	if (StationData)
	{
		CurrentStation->Load(*StationData);
	}
}

void AFlareMenuPawn::ShowPart(const FFlareShipModuleDescription* PartDesc)
{
	// Clean up
	ResetContent();

	// Choose a part to work with
	SlideFromAToB = !SlideFromAToB;
	UFlareShipModule* CurrentPart = SlideFromAToB ? CurrentPartB : CurrentPartA;
	FVector& CurrentPartOffset = SlideFromAToB ? CurrentPartOffsetB : CurrentPartOffsetA;

	// Load the parts and scale accordingly
	CurrentPart->SetVisibility(true, true);
	FFlareShipModuleSave Data;
	Data.ModuleIdentifier = PartDesc->Identifier;
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
	if (CurrentShip && !Unsafe)
	{
		CurrentShip->Destroy();
		CurrentShip = NULL;
	}

	// Delete station if existing
	if (CurrentStation && !Unsafe)
	{
		CurrentStation->Destroy();
		CurrentStation = NULL;
	}

	// Hide parts
	CurrentPartA->SetVisibility(false, true);
	CurrentPartB->SetVisibility(false, true);
	
	// Setup panning
	SetCameraPitch(-CameraMaxPitch / 2);
	SetCameraDistance(DisplayDistance);
}

void AFlareMenuPawn::UpdateCustomization()
{
	if (CurrentShip)
	{
		CurrentShip->UpdateCustomization();
	}
	if (CurrentStation)
	{
		CurrentStation->UpdateCustomization();
	}
	else
	{
		CurrentPartA->UpdateCustomization();
		CurrentPartB->UpdateCustomization();
	}
}

void AFlareMenuPawn::SetHorizontalOffset(float Offset)
{
	CameraContainerYaw->SetRelativeLocation(FVector(0, -Offset, 0));
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

	InputComponent->BindAxis("MouseInputY", this, &AFlareMenuPawn::PitchInput);
	InputComponent->BindAxis("MouseInputX", this, &AFlareMenuPawn::YawInput);
}

void AFlareMenuPawn::PitchInput(float Val)
{
	if (Val)
	{
		FRotator CurrentRot = CameraContainerPitch->GetComponentRotation();
		SetCameraPitch(CurrentRot.Pitch + CameraPanSpeed * Val);
	}
}

void AFlareMenuPawn::YawInput(float Val)
{
	FRotator DeltaRot = FRotator::MakeFromEuler(-CameraPanSpeed * FVector(0, 0, Val));

	if (CurrentShip)
	{
		CurrentShip->AddActorLocalRotation(DeltaRot);
	}
	else if (CurrentStation)
	{
		CurrentStation->AddActorLocalRotation(DeltaRot);
	}
	else
	{
		PartContainer->AddLocalRotation(DeltaRot);
	}
}
