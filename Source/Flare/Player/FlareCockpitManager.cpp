
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
	, PlayerShip(NULL)
	, CockpitMaterialInstance(NULL)
	, CockpitFrameMaterialInstance(NULL)
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
	CockpitMesh->LightingChannels.bChannel0 = false;
	CockpitMesh->LightingChannels.bChannel1 = true;

	// Cockpit camera
	CockpitCapture = PCIP.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("CockpitCapture"));
	CockpitCapture->bCaptureEveryFrame = true;

	// Light
	CockpitLight = PCIP.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("CockpitLight"));
	CockpitLight->SetLightColor(FLinearColor(1.0f, 0.871f, 0.731));
	CockpitLight->SetIntensity(200);
	CockpitLight->SetRelativeLocation(FVector(0, -30, -30));
	CockpitLight->SetCastShadows(false);
	CockpitLight->LightingChannels.bChannel0 = false;
	CockpitLight->LightingChannels.bChannel1 = true;
	CockpitLight->AttachTo(CockpitMesh);

	// Light 2
	CockpitLight2 = PCIP.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("CockpitLight2"));
	CockpitLight2->SetLightColor(FLinearColor(1.0f, 0.376f, 0.212f));
	CockpitLight2->SetIntensity(200);
	CockpitLight2->SetRelativeLocation(FVector(0, 10, 10));
	CockpitLight2->SetCastShadows(false);
	CockpitLight2->LightingChannels.bChannel0 = false;
	CockpitLight2->LightingChannels.bChannel1 = true;
	CockpitLight2->AttachTo(CockpitMesh);

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
		ExitCockpit(PlayerShip);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void AFlareCockpitManager::OnFlyShip(AFlareSpacecraft* NewPlayerShip)
{
	// Reset existing ship
	if (PlayerShip)
	{
		ExitCockpit(PlayerShip);
	}

	PlayerShip = NewPlayerShip;

	// Setup new ship
	if (PlayerShip && PC->UseCockpit)
	{
		EnterCockpit(PlayerShip);
	}
	else
	{
		ExitCockpit(PlayerShip);
	}
}

void AFlareCockpitManager::SetExternalCamera(bool External)
{
	if (!PlayerShip)
	{
		return;
	}

	if (External || !PC->UseCockpit)
	{
		ExitCockpit(PlayerShip);
	}
	else
	{
		EnterCockpit(PlayerShip);
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

		// Update instruments target
		if (CockpitInstrumentsTarget)
		{
			CockpitInstrumentsTarget->UpdateResource();
		}

		// Update instruments
		if (PlayerShip && CockpitFrameMaterialInstance)
		{
			UpdateTarget(DeltaSeconds);
			UpdateInfo(DeltaSeconds);
			UpdateTemperature(DeltaSeconds);

			// Lights
			bool LightsActive = !PlayerShip->GetDamageSystem()->HasPowerOutage();
			CockpitLight->SetActive(LightsActive);
			CockpitLight2->SetActive(LightsActive);
		}
	}
}

void AFlareCockpitManager::EnterCockpit(AFlareSpacecraft* PlayerShip)
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
		CockpitMesh->AttachTo(PlayerShip->GetRootComponent(), NAME_None, EAttachLocation::SnapToTarget);
		FVector CameraOffset = PlayerShip->GetRootComponent()->GetSocketLocation(FName("Camera"));
		CockpitCapture->SetRelativeLocation(CameraOffset);
	}
	else
	{
		CockpitMesh->AttachTo(PlayerShip->GetCamera(), NAME_None, EAttachLocation::SnapToTarget);
	}

	CockpitMesh->SetVisibility(true, true);
}

void AFlareCockpitManager::ExitCockpit(AFlareSpacecraft* PlayerShip)
{
	CockpitMesh->SetVisibility(false, true);
}

void AFlareCockpitManager::UpdateTarget(float DeltaSeconds)
{
	float Intensity = 0;
	AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetShip)
	{
		Intensity = 1;
		FLinearColor Color = TargetShip->GetPlayerHostility() == EFlareHostility::Hostile ? Theme.EnemyColor : Theme.FriendlyColor;
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorTop", Color);
	}

	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityTop", Intensity);
}

void AFlareCockpitManager::UpdateInfo(float DeltaSeconds)
{
	float Intensity = 0;

	if (PlayerShip->IsMilitary())
	{
		FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();

		if (WeaponGroup)
		{
			Intensity = 1;
			float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex());
			CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorLeft", PC->GetNavHUD()->GetHealthColor(ComponentHealth));
		}
	}

	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityLeft", Intensity);
}

void AFlareCockpitManager::UpdateTemperature(float DeltaSeconds)
{
	float Temperature = PlayerShip->GetDamageSystem()->GetTemperature();
	float OverheatTemperature = PlayerShip->GetDamageSystem()->GetOverheatTemperature();
	FLinearColor TemperatureColor = PC->GetNavHUD()->GetTemperatureColor(Temperature, OverheatTemperature);

	CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorRight", TemperatureColor);
}


#undef LOCTEXT_NAMESPACE
