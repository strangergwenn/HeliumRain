
#include "FlareMenuPawn.h"
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Game/FlareGameUserSettings.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareMenuPawn::AFlareMenuPawn(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, InitialYaw(150)
	, DisplayDistance(800)
	, DisplaySize(300)
	, ShipDisplaySize(500)
	, SlideInOutUpOffset(0, 0, -2000)
	, SlideInOutSideOffset(0, 2000, 0)
	, SlideInOutTime(0.4)
	, CurrentSpacecraft(NULL)
	, SlideFromAToB(true)
	, SlideDirection(1)
	, LastMenuScale(1.0f)
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

	// Ship container turnplate
	ShipContainer = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("ShipContainer"));
	ShipContainer->AttachToComponent(RootComponent, AttachRules);
	
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
		ShipContainer->SetWorldLocation(GetActorLocation() + CurrentShipOffset + SlideInDelta);
	}

	// Camera


	float ManualAcc = 600; //°/s-2
	float Resistance = 1/360.f;
	float Brake = 4.f;
	float Brake2 = 2.f;

	float LastExternalCameraYaw = ExternalCameraYaw;
	{
		float Acc = FMath::Sign(ExternalCameraYawTarget) * ManualAcc;
		float Res = FMath::Sign(ExternalCameraYawSpeed) * (Resistance * FMath::Square(ExternalCameraYawSpeed) + (Acc == 0 ? Brake2 + Brake * FMath::Abs(ExternalCameraYawSpeed) : 0));

		float MaxResDeltaSpeed = ExternalCameraYawSpeed;


		float AccDeltaYawSpeed = Acc * DeltaSeconds;
		float ResDeltaYawSpeed = - (FMath::Abs(Res * DeltaSeconds) > FMath::Abs(MaxResDeltaSpeed) ? MaxResDeltaSpeed : Res * DeltaSeconds);
		ExternalCameraYawTarget = 0; // Consume
		ExternalCameraYawSpeed += AccDeltaYawSpeed + ResDeltaYawSpeed;
		ExternalCameraYaw += ExternalCameraYawSpeed * DeltaSeconds;
		ExternalCameraYaw = FMath::UnwindDegrees(ExternalCameraYaw);
	}


	{

		float Acc = FMath::Sign(ExternalCameraPitchTarget) * ManualAcc;
		float Res = FMath::Sign(ExternalCameraPitchSpeed) * (Resistance * FMath::Square(ExternalCameraPitchSpeed) + (Acc == 0 ? Brake2 + Brake * FMath::Abs(ExternalCameraPitchSpeed) : 0));

		float MaxResDeltaSpeed = ExternalCameraPitchSpeed;

		float AccDeltaPitchSpeed = Acc * DeltaSeconds;
		float ResDeltaPitchSpeed = - (FMath::Abs(Res * DeltaSeconds) > FMath::Abs(MaxResDeltaSpeed) ? MaxResDeltaSpeed : Res * DeltaSeconds);
		ExternalCameraPitchTarget = 0; // Consume
		ExternalCameraPitchSpeed += AccDeltaPitchSpeed + ResDeltaPitchSpeed;
		ExternalCameraPitch += ExternalCameraPitchSpeed * DeltaSeconds;
		ExternalCameraPitch = FMath::UnwindDegrees(ExternalCameraPitch);
		float ExternalCameraPitchClamped = FMath::Clamp(ExternalCameraPitch, -GetCameraMaxPitch(), GetCameraMaxPitch());
		if(ExternalCameraPitchClamped != ExternalCameraPitch)
		{
			ExternalCameraPitchSpeed = -ExternalCameraPitchSpeed/2;
		}
		ExternalCameraPitch = ExternalCameraPitchClamped;
	}


	SetCameraPitch(ExternalCameraPitch);
	FRotator DeltaRot = FRotator::MakeFromEuler(FVector(0, 0, ExternalCameraYaw - LastExternalCameraYaw));

	if (CurrentSpacecraft)
	{
		CurrentSpacecraft->AddActorLocalRotation(DeltaRot);
	}
	else
	{
		PartContainer->AddLocalRotation(DeltaRot);
	}

	// Update scale to keep constant scale at different FoVs
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	float Scale = FMath::Tan(FMath::DegreesToRadians(PC->GetNormalFOV() / 2.f)) / FMath::Tan(FMath::DegreesToRadians(PC->GetMinFOV() / 2.f));
	RootComponent->SetRelativeScale3D(FVector(1.0, LastMenuScale, LastMenuScale));
	PartContainer->SetRelativeScale3D(FVector(1.0, 1/LastMenuScale, 1/LastMenuScale));
	if (!FMath::IsNaN(Scale))
	{
		LastMenuScale = Scale;
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
	CurrentSpacecraft = GetWorld()->SpawnActor<AFlareSpacecraft>(Spacecraft->GetDescription()->SpacecraftTemplate, Params);
	CurrentSpacecraft->AttachToComponent(ShipContainer, AttachRules, NAME_None);

	// FOV scale
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	float MinFOV = GetPC()->GetMinVerticalFOV();
	float FOVScalingRatio = ((MyGameSettings->VerticalFOV - MinFOV) / (GetPC()->GetMaxVerticalFOV() - MinFOV)) / 3;

	// Setup rotation and scale
	CurrentSpacecraft->SetActorScale3D(FVector(1, 1, 1));
	float Scale = (1 + FOVScalingRatio) * (ShipDisplaySize / CurrentSpacecraft->GetMeshScale());
	FLOGV("AFlareMenuPawn::ShowShip : DS=%f, MS=%f, S=%f", ShipDisplaySize, CurrentSpacecraft->GetMeshScale(), Scale);
	CurrentSpacecraft->SetActorScale3D(Scale * FVector(1, 1, 1));
	ShipContainer->SetRelativeRotation(FRotator(0, InitialYaw, 0));

	// Center
	FVector Origin, Extent;
	CurrentSpacecraft->GetActorBounds(true, Origin, Extent);
	Origin -= CurrentSpacecraft->GetActorLocation();
	CurrentSpacecraft->SetActorRelativeLocation(-Origin);

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
	UFlareCompany* TargetCompany = CurrentSpacecraft ? CurrentSpacecraft->GetParent()->GetCompany() : GetPC()->GetCompany();
	FVector& CurrentPartOffset = SlideFromAToB ? CurrentPartOffsetB : CurrentPartOffsetA;

	// Clean up
	ResetContent();

	// Load the parts and scale accordingly
	CurrentPart->SetVisibleInUpgrade(true);
	FFlareSpacecraftComponentSave Data;
	Data.Damage = 0;
	Data.ComponentIdentifier = PartDesc->Identifier;
	CurrentPart->Initialize(&Data, TargetCompany, this, true);
	CurrentPart->SetWorldScale3D(FVector(1, 1, 1));
	float Scale = DisplaySize / CurrentPart->GetMeshScale();
	CurrentPart->SetWorldScale3D(Scale * FVector(1, 1, 1));

	// Reset position to compute the offsets
	CurrentPart->SetRelativeLocation(FVector::ZeroVector);
	FRotator OldRot = PartContainer->GetComponentRotation();
	PartContainer->SetRelativeRotation(FRotator::ZeroRotator);
	CurrentPart->SetRelativeRotation(FRotator(0, -InitialYaw, 0));

	// Except on RCSs, center the part
	if (PartDesc->Type != EFlarePartType::RCS)
	{
		CurrentPartOffset = GetActorLocation() - CurrentPart->Bounds.GetBox().GetCenter();
	}
	else
	{
		CurrentPartOffset = FVector::ZeroVector;
	}

	// Setup offset and rotation
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
	ExternalCameraPitchSpeed = 0;
	ExternalCameraYawSpeed = 0;
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
	UpdateBackgroundColor(1.0, 0.0);
}

void AFlareMenuPawn::UseDarkBackground()
{
	UpdateBackgroundColor(0.04, 0.7);
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareMenuPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	FCHECK(PlayerInputComponent);
}

void AFlareMenuPawn::UpdateExternalCameraTarget()
{
	auto& App = FSlateApplication::Get();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	if (GEngine->GameViewport->GetGameViewportWidget()->HasMouseCapture() && !PC->GetNavHUD()->IsWheelMenuOpen())
	{
		FVector2D CursorPos = App.GetCursorPos();

		if (IsExternalCameraMouseOffsetInit && CursorPos != LastExternalCameraMouseOffset)
		{
			FVector2D MoveDirection = (CursorPos - LastExternalCameraMouseOffset).GetSafeNormal();
			ExternalCameraYawTarget += -MoveDirection.X;
			ExternalCameraPitchTarget += -MoveDirection.Y;
		}

		LastExternalCameraMouseOffset = CursorPos;
		IsExternalCameraMouseOffsetInit = true;
	}
	else
	{
		IsExternalCameraMouseOffsetInit = false;
	}
}

void AFlareMenuPawn::PitchInput(float Val)
{
	UpdateExternalCameraTarget();
}

void AFlareMenuPawn::YawInput(float Val)
{
	UpdateExternalCameraTarget();
}
