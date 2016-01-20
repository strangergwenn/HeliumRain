
#include "../Flare.h"
#include "FlareCockpitManager.h"

#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSpacecraftComponent.h"

#define LOCTEXT_NAMESPACE "FlareCockpitManager"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareCockpitManager::AFlareCockpitManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, ShipPawn(NULL)
	, CockpitMaterialInstance(NULL)
	, CockpitCameraTarget(NULL)
	, CockpitHUDTarget(NULL)
{
	// Cockpit data
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CockpitMeshTemplateObj(TEXT("/Game/Gameplay/HUD/SM_Cockpit"));
	CockpitMeshTemplate = CockpitMeshTemplateObj.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial> CockpitMaterialInstanceObj(TEXT("/Game/Gameplay/HUD/MT_Cockpit"));
	CockpitMaterialTemplate = CockpitMaterialInstanceObj.Object;
	
	// Settings
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void AFlareCockpitManager::SetupCockpit(AFlarePlayerController* NewPC)
{
	PC = NewPC;

	// Cockpit on
	if (PC->UseCockpit && CockpitMaterialInstance == NULL)
	{
		FVector2D ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		FLOG("AFlareCockpitManager::SetupCockpit : will be using 3D cockpit");

		// Cockpit camera texture target
		CockpitCameraTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitCameraTarget);
		CockpitCameraTarget->ClearColor = FLinearColor::Black;

		// Cockpit HUD texture target
		CockpitHUDTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitHUDTarget);
		CockpitHUDTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawToCanvasRenderTarget);
		CockpitHUDTarget->ClearColor = FLinearColor::Black;

		// Cockpit material
		CockpitMaterialInstance = UMaterialInstanceDynamic::Create(CockpitMaterialTemplate, GetWorld());
		check(CockpitMaterialInstance);
		CockpitMaterialInstance->SetTextureParameterValue("CameraTexture", CockpitCameraTarget);
		CockpitMaterialInstance->SetTextureParameterValue("HUDTexture", CockpitHUDTarget);
	}
	else
	{
		FLOG("AFlareCockpitManager::SetupCockpit : cockpit manager is disabled");
		CockpitMaterialInstance = NULL;
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void AFlareCockpitManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update HUD target
	if (PC->UseCockpit && CockpitHUDTarget)
	{
		CockpitHUDTarget->UpdateResource();
	}
}

void AFlareCockpitManager::OnFlyShip(AFlareSpacecraft* NewShipPawn)
{
	if (PC->UseCockpit && ShipPawn)
	{
		ShipPawn->HideCockpit();
	}

	ShipPawn = NewShipPawn;

	if (PC->UseCockpit)
	{
		ShipPawn->SetCockpit(CockpitMeshTemplate, CockpitMaterialInstance, CockpitCameraTarget);
	}
}

/*
void AFlareCockpitManager::Configure(UFlareSpacecraftComponent* Airframe, UStaticMeshComponent* CockpitMesh, USceneCaptureComponent2D* CockpitCapture)
{
	check(PC->UseCockpit);

	// Setup render target camera
	FVector CameraOffset = WorldToLocal(Airframe->GetSocketLocation(FName("Camera")) - GetActorLocation());
	CockpitCapture->SetRelativeLocation(2 * CameraOffset);
	CockpitCapture->FOVAngle = PC->PlayerCameraManager->GetFOVAngle();
	CockpitCapture->PostProcessSettings.AntiAliasingMethod = EAntiAliasingMethod::AAM_TemporalAA;

	// Setup cockpit camera
	Cast<UCameraComponent>(Camera)->PostProcessSettings.AntiAliasingMethod = EAntiAliasingMethod::AAM_FXAA;

	// Setup data
	CockpitMesh->SetStaticMesh(Mesh);
	CockpitMesh->SetMaterial(0, Material);
	CockpitMesh->SetWorldScale3D(0.1 * FVector(1, 1, 1));

	// Update material
	if (Material && CockpitCapture && CameraTarget)
	{
		CockpitCapture->TextureTarget = CameraTarget;
		CockpitCapture->bCaptureEveryFrame = true;
		CockpitCapture->UpdateContent();
	}
}

void AFlareCockpitManager::HideCockpit()
{
	CockpitMesh = NULL;
	CockpitCapture->TextureTarget = NULL;
	StateManager->SetExternalCamera(false, true);
}*/



#undef LOCTEXT_NAMESPACE
