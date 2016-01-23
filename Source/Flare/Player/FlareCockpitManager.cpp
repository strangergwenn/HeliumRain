
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
	static ConstructorHelpers::FObjectFinder<UMaterial> CockpitFrameMaterialInstanceObj(TEXT("/Game/Gameplay/Cockpit/MT_CockpitFrame"));
	CockpitFrameMaterialTemplate = CockpitFrameMaterialInstanceObj.Object;
	
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
		CockpitInstrumentsTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 256, 256);
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
	}
}

void AFlareCockpitManager::OnFlyShip(AFlareSpacecraft* NewShipPawn)
{
	// Reset existing ship
	if (ShipPawn)
	{
		ShipPawn->ExitCockpit();
	}

	ShipPawn = NewShipPawn;

	// Setup new ship
	if (PC->UseCockpit)
	{
		ShipPawn->EnterCockpit(CockpitMeshTemplate, CockpitMaterialInstance, CockpitFrameMaterialInstance, CockpitCameraTarget);
		ShipPawn->GetCamera()->PostProcessSettings.bOverride_AntiAliasingMethod = true;
	}
}


#undef LOCTEXT_NAMESPACE
