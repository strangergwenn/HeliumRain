
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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CockpitMeshTemplateObj(TEXT("/Game/Gameplay/Cockpit/SM_Cockpit"));
	CockpitMeshTemplate = CockpitMeshTemplateObj.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial> CockpitMaterialInstanceObj(TEXT("/Game/Gameplay/Cockpit/MT_Cockpit"));
	CockpitMaterialTemplate = CockpitMaterialInstanceObj.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CockpitFrameMaterialInstanceObj(TEXT("/Game/Gameplay/Cockpit/MI_CockpitFrame"));
	CockpitFrameMaterialTemplate = CockpitFrameMaterialInstanceObj.Object;

	// Cockpit mesh
	CockpitMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Cockpit"));

	// Cockpit camera
	CockpitCapture = PCIP.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("CockpitCapture"));
	CockpitCapture->bCaptureEveryFrame = true;

	// Settings
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	CockpitInstrumentsTargetSize = 512;
}

void AFlareCockpitManager::SetupCockpit(AFlarePlayerController* NewPC)
{
	PC = NewPC;

	// Cockpit on
	if (PC->UseCockpit && CockpitMaterialInstance == NULL)
	{
		FVector2D ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		if (ViewportSize.Size() == 0)
		{
			return;
		}
		FLOGV("AFlareCockpitManager::SetupCockpit : will be using 3D cockpit (%dx%d", ViewportSize.X, ViewportSize.Y);

		// Cockpit camera texture target
		if (PC->UseCockpitRenderTarget)
		{
			CockpitCameraTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
			check(CockpitCameraTarget);
			CockpitCameraTarget->ClearColor = FLinearColor::Black;
			CockpitCameraTarget->UpdateResource();
		}

		// Cockpit HUD texture target
		CockpitHUDTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitHUDTarget);
		CockpitHUDTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitHUD);
		CockpitHUDTarget->ClearColor = FLinearColor::Black;
		CockpitHUDTarget->UpdateResource();

		// Cockpit instruments texture target
		CockpitInstrumentsTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitInstrumentsTargetSize, CockpitInstrumentsTargetSize);
		check(CockpitInstrumentsTarget);
		CockpitInstrumentsTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitInstruments);
		CockpitInstrumentsTarget->ClearColor = FLinearColor::Black;
		CockpitInstrumentsTarget->UpdateResource();

		// Cockpit material
		CockpitMaterialInstance = UMaterialInstanceDynamic::Create(CockpitMaterialTemplate, GetWorld());
		check(CockpitMaterialInstance);
		CockpitMaterialInstance->SetTextureParameterValue("CameraTarget", CockpitCameraTarget);
		CockpitMaterialInstance->SetTextureParameterValue("HUDTarget", CockpitHUDTarget);
		CockpitMaterialInstance->SetScalarParameterValue("CockpitOpacity", PC->UseCockpitRenderTarget ? 1:0);

		// Cockpit frame material
		CockpitFrameMaterialInstance = UMaterialInstanceDynamic::Create(CockpitFrameMaterialTemplate, GetWorld());
		check(CockpitMaterialInstance);
		CockpitFrameMaterialInstance->SetTextureParameterValue("InstrumentsTarget", CockpitInstrumentsTarget);

		// Setup mesh
		CockpitMesh->SetStaticMesh(CockpitMeshTemplate);
		CockpitMesh->SetMaterial(0, CockpitMaterialInstance);
		CockpitMesh->SetMaterial(1, CockpitFrameMaterialInstance);

		// Setup render target camera
		if (PC->UseCockpitRenderTarget)
		{
			check(CockpitCapture);
			check(CockpitCameraTarget);
			CockpitCapture->FOVAngle = PC->PlayerCameraManager->GetFOVAngle();
			CockpitCapture->TextureTarget = CockpitCameraTarget;
		}
	}
	else
	{
		FLOG("AFlareCockpitManager::SetupCockpit : cockpit manager is disabled");
		CockpitMaterialInstance = NULL;
		CockpitFrameMaterialInstance = NULL;
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void AFlareCockpitManager::OnFlyShip(AFlareSpacecraft* NewShipPawn)
{
	// Reset existing ship
	if (ShipPawn)
	{
		ExitCockpit(ShipPawn);
	}

	ShipPawn = NewShipPawn;

	// Setup new ship
	if (ShipPawn && PC->UseCockpit)
	{
		EnterCockpit(ShipPawn);
	}
}

void AFlareCockpitManager::SetExternalCamera(bool External)
{
	if (!ShipPawn)
	{
		return;
	}

	if (External)
	{
		ExitCockpit(ShipPawn);
	}
	else
	{
		EnterCockpit(ShipPawn);
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

void AFlareCockpitManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PC->UseCockpit)
	{
		// Ensure cockpit existence
		if (CockpitMaterialInstance == NULL)
		{
			SetupCockpit(PC);
		}

		// Update HUD target
		if (CockpitHUDTarget)
		{
			CockpitHUDTarget->UpdateResource();
		}

		// Update instruments
		if (ShipPawn && CockpitFrameMaterialInstance && CockpitInstrumentsTarget)
		{
			UpdateTarget(DeltaSeconds);
			UpdateInfo(DeltaSeconds);
			UpdateDamages(DeltaSeconds);
			UpdateTemperature(DeltaSeconds);
			UpdatePower(DeltaSeconds);

			CockpitInstrumentsTarget->UpdateResource();
		}
	}
}

void AFlareCockpitManager::EnterCockpit(AFlareSpacecraft* ShipPawn)
{
	// Ensure we're not doing anything stupid
	check(PC->UseCockpit);
	check(CockpitMesh);
	check(CockpitMesh);
	check(CockpitMaterialTemplate);
	check(CockpitFrameMaterialTemplate);

	// Offset the cockpit
	if (PC->UseCockpitRenderTarget)
	{
		CockpitMesh->AttachTo(ShipPawn->GetRootComponent(), NAME_None, EAttachLocation::SnapToTarget);
		FVector CameraOffset = ShipPawn->GetRootComponent()->GetSocketLocation(FName("Camera"));
		CockpitCapture->SetRelativeLocation(CameraOffset);
	}
	else
	{
		CockpitMesh->AttachTo(ShipPawn->GetCamera(), NAME_None, EAttachLocation::SnapToTarget);
	}

	CockpitMesh->SetVisibility(true, true);
}

void AFlareCockpitManager::ExitCockpit(AFlareSpacecraft* ShipPawn)
{
	CockpitMesh->SetVisibility(false, true);
}

void AFlareCockpitManager::UpdateTarget(float DeltaSeconds)
{
	CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorTop", FFlareStyleSet::GetDefaultTheme().FriendlyColor);
}

void AFlareCockpitManager::UpdateInfo(float DeltaSeconds)
{
	CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorLeft", FFlareStyleSet::GetDefaultTheme().FriendlyColor);
}

void AFlareCockpitManager::UpdateDamages(float DeltaSeconds)
{
}

void AFlareCockpitManager::UpdateTemperature(float DeltaSeconds)
{
	float Temperature = ShipPawn->GetDamageSystem()->GetTemperature();
	float OverheatTemperature = ShipPawn->GetDamageSystem()->GetOverheatTemperature();
	FLinearColor TemperatureColor = PC->GetNavHUD()->GetHealthColor(Temperature, OverheatTemperature);

	CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorRight", TemperatureColor);
}

void AFlareCockpitManager::UpdatePower(float DeltaSeconds)
{/*
	bool Outage = ShipPawn->GetDamageSystem()->HasPowerOutage();

	float Intensity = Outage ? 0 : 1;

	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityTop",   Intensity);
	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityLeft",  Intensity);
	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityRight", Intensity);*/
}


#undef LOCTEXT_NAMESPACE
