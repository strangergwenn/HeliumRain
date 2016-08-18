
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
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);

	// Dummy root to allow better configuration of the subparts
	USceneComponent* SceneComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	CameraContainerYaw->AttachToComponent(SceneComponent, AttachRules);
	RootComponent = SceneComponent;

	// Part container turnplate
	PartContainer = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("PartContainer"));
	PartContainer->AttachToComponent(RootComponent, AttachRules);

	// Create static mesh component for the part
	CurrentPartA = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("PartA"));
	CurrentPartA->SetStaticMesh(ConstructorStatics.CurrentPart.Get());
	CurrentPartA->SetSimulatePhysics(false);
	CurrentPartA->AttachToComponent(PartContainer, AttachRules);

	// Create static mesh component for the part
	CurrentPartB = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("PartB"));
	CurrentPartB->SetStaticMesh(ConstructorStatics.CurrentPart.Get());
	CurrentPartB->SetSimulatePhysics(false);
	CurrentPartB->AttachToComponent(PartContainer, AttachRules);
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

void AFlareMenuPawn::ShowShip(UFlareSimulatedSpacecraft* Spacecraft)
{
	// Clean up
	ResetContent();

	// Spawn parameters
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn and setup the ship
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, false);
	CurrentSpacecraft = GetWorld()->SpawnActor<AFlareSpacecraft>(Spacecraft->GetDescription()->Template->GeneratedClass, Params);
	CurrentSpacecraft->AttachToActor(this, AttachRules, NAME_None);

	// Setup rotation and scale
	CurrentSpacecraft->SetActorScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentSpacecraft->GetMeshScale();
	FLOGV("AFlareMenuPawn::ShowShip : DS=%f, MS=%f, S=%f", DisplaySize, CurrentSpacecraft->GetMeshScale(), Scale);
	CurrentSpacecraft->SetActorScale3D(Scale * FVector(1, 1, 1));
	CurrentSpacecraft->SetActorRelativeRotation(FRotator(0, InitialYaw, 0));

	// Slide
	SlideInOutCurrentTime = 0.0f;
	SlideInOutOffset = SlideInOutSideOffset;
	SetSlideDirection(true);
	CurrentSpacecraft->StartPresentation();

	// UI
	CurrentSpacecraft->Load(Spacecraft);
}

void AFlareMenuPawn::ShowPart(const FFlareSpacecraftComponentDescription* PartDesc)
{
	// Choose a part to work with
	SlideFromAToB = !SlideFromAToB;
	UFlareSpacecraftComponent* CurrentPart = SlideFromAToB ? CurrentPartB : CurrentPartA;
	UFlareCompany* Company = CurrentSpacecraft ? CurrentSpacecraft->GetParent()->GetCompany() : GetPC()->GetCompany();
	FVector& CurrentPartOffset = SlideFromAToB ? CurrentPartOffsetB : CurrentPartOffsetA;

	// Clean up
	ResetContent();

	// Load the parts and scale accordingly
	CurrentPart->SetVisibleInUpgrade(true);
	FFlareSpacecraftComponentSave Data;
	Data.Damage = 0;
	Data.ComponentIdentifier = PartDesc->Identifier;
	CurrentPart->Initialize(&Data, Company, this, true);
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
	CurrentPartA->SetVisibleInUpgrade(false);
	CurrentPartB->SetVisibleInUpgrade(false);
	
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

void AFlareMenuPawn::UseLightBackground()
{
	UpdateBackgroundColor(0.8, 0.3);
}

void AFlareMenuPawn::UseDarkBackground()
{
	UpdateBackgroundColor(0.05, 0.85);
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
