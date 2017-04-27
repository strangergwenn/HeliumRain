
#include "FlareSpacecraftPawn.h"
#include "../Flare.h"
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
	, CameraPanSpeed(250)
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

	// Init
	PreviousCameraName = NAME_None;
	CurrentCameraName = NAME_None;
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
	PreviousCameraName = CurrentCameraName;
	
	// Apply interpolated values
	if (!UseImmersiveCamera)
	{
		CurrentCameraName = NAME_None;
		CameraContainerPitch->SetRelativeRotation(FRotator(CameraOffsetPitch, 0, 0).GetNormalized());
		CameraContainerYaw->SetRelativeRotation(FRotator(0, CameraOffsetYaw, 0).GetNormalized());
		Camera->SetRelativeLocation(CameraLocalPosition - FVector(CameraOffsetDistance, 0, 0));
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
	else
	{
		CurrentCameraName = FName("Camera");
		FVector CameraLocation = GetRootComponent()->GetSocketLocation(CurrentCameraName);
		Camera->SetWorldLocationAndRotation(CameraLocation, ImmersiveTargetRotation);
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

void AFlareSpacecraftPawn::ConfigureImmersiveCamera(FQuat TargetRotation)
{
	if(!UseImmersiveCamera)
	{
		UseImmersiveCamera = true;
	}
	SetPhysicalVisibility(false);
	ImmersiveTargetRotation = TargetRotation;
}

void AFlareSpacecraftPawn::DisableImmersiveCamera()
{
	if(UseImmersiveCamera)
	{
		UseImmersiveCamera = false;
		SetPhysicalVisibility(true);
	}
}

void AFlareSpacecraftPawn::SetPhysicalVisibility(bool Visibility)
{
	// Find all components
	TArray<UActorComponent*> ActorComponents;
	GetComponents(ActorComponents);

	for (TArray<UActorComponent*>::TIterator ComponentIt(ActorComponents); ComponentIt; ++ComponentIt)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(*ComponentIt);
		if (Component)
		{
			Component->SetVisibility(Visibility);
			continue;
		}

		UParticleSystemComponent* PSComponent = Cast<UParticleSystemComponent>(*ComponentIt);
		if (PSComponent)
		{
			PSComponent->SetVisibility(Visibility);
			continue;
		}
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
