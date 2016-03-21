
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
	, CockpitHealthLightTime(0)
	, CockpitHealthLightPeriod(1.2)
	, CockpitPowerTime(0)
	, CockpitPowerPeriod(0.4)
	, CockpitMaterialInstance(NULL)
	, CockpitFrameMaterialInstance(NULL)
	, CockpitHUDTarget(NULL)
	, CockpitInstrumentsTarget(NULL)
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
	RootComponent = CockpitMesh;

	// Main camera
#if FLARE_USE_COCKPIT_RENDERTARGET
	CockpitCapture = PCIP.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("CockpitCapture"));
	CockpitCapture->bCaptureEveryFrame = true;
	CockpitCameraTarget = NULL;
#endif

	// FLIR camera
	CockpitFLIRCapture = PCIP.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("CockpitFLIRCapture"));
	CockpitFLIRCapture->bCaptureEveryFrame = true;
	CockpitFLIRCapture->FOVAngle = 10;
	CockpitFLIRCameraTarget = NULL;
	CockpitFLIRCapture->Deactivate();

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
	CockpitFLIRTargetSize = 512;
	IsInCockpit = false;
}

void AFlareCockpitManager::SetupCockpit(AFlarePlayerController* NewPC)
{
	PC = NewPC;

	// Cockpit on
	if (PC->UseCockpit && CockpitMaterialInstance == NULL)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		// Use same width as parent and 16:9 ratio
		int32 ParentViewportWidth = 1920;
		FVector2D ViewportSize = FVector2D (ParentViewportWidth, (ParentViewportWidth * 9) / 16);
		FLOGV("AFlareCockpitManager::SetupCockpit : will be using 3D cockpit (%dx%d", ViewportSize.X, ViewportSize.Y);

		// Screen material
		CockpitMaterialInstance = UMaterialInstanceDynamic::Create(CockpitMaterialTemplate, GetWorld());
		check(CockpitMaterialInstance);
		CockpitMaterialInstance->SetVectorParameterValue("IndicatorColorBorders", Theme.FriendlyColor);

		// Frame material
		CockpitFrameMaterialInstance = UMaterialInstanceDynamic::Create(CockpitFrameMaterialTemplate, GetWorld());
		check(CockpitMaterialInstance);
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorBorders", Theme.FriendlyColor);
		
		// HUD texture target
		CockpitHUDTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitHUDTarget);
		CockpitHUDTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitHUD);
		CockpitHUDTarget->ClearColor = FLinearColor::Black;
		CockpitHUDTarget->UpdateResource();
		CockpitMaterialInstance->SetTextureParameterValue("HUDTarget", CockpitHUDTarget);

		// Instruments texture target
		CockpitInstrumentsTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitInstrumentsTargetSize, CockpitInstrumentsTargetSize);
		check(CockpitInstrumentsTarget);
		CockpitInstrumentsTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitInstruments);
		CockpitInstrumentsTarget->ClearColor = FLinearColor::Black;
		CockpitInstrumentsTarget->UpdateResource();
		CockpitFrameMaterialInstance->SetTextureParameterValue("InstrumentsTarget", CockpitInstrumentsTarget);

		// FLIR camera target
		CockpitFLIRCameraTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitFLIRTargetSize, CockpitFLIRTargetSize);
		check(CockpitFLIRCameraTarget);
		CockpitFLIRCameraTarget->ClearColor = FLinearColor::Black;
		CockpitFLIRCameraTarget->UpdateResource();

		// Setup FLIR camera
		check(CockpitFLIRCapture);
		CockpitFLIRCapture->TextureTarget = CockpitFLIRCameraTarget;
		CockpitFrameMaterialInstance->SetTextureParameterValue("FLIRTarget", CockpitFLIRCameraTarget);

#if FLARE_USE_COCKPIT_RENDERTARGET
		// Main camera texture target
		CockpitCameraTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitCameraTarget);
		CockpitCameraTarget->ClearColor = FLinearColor::Black;
		CockpitCameraTarget->UpdateResource();

		// Setup main camera
		check(CockpitCapture);
		CockpitCapture->FOVAngle = PC->PlayerCameraManager->GetFOVAngle();
		CockpitCapture->TextureTarget = CockpitCameraTarget;
		CockpitMaterialInstance->SetTextureParameterValue("CameraTarget", CockpitCameraTarget);
#endif

		// Setup mesh
		CockpitMesh->SetStaticMesh(CockpitMeshTemplate);
		CockpitMesh->SetMaterial(0, CockpitMaterialInstance);
		CockpitMesh->SetMaterial(1, CockpitFrameMaterialInstance);
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

	// Reset data
	if (PlayerShip != NewPlayerShip)
	{
		CockpitPowerTime = 0;
	}
	PlayerShip = NewPlayerShip;

	// Setup new ship
	if (PlayerShip && PC->UseCockpit && !PlayerShip->GetStateManager()->IsExternalCamera())
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

		if (IsInCockpit && PlayerShip)
		{
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
			if (CockpitFrameMaterialInstance)
			{
				UpdateTarget(DeltaSeconds);
				UpdateInfo(DeltaSeconds);
				UpdateTemperature(DeltaSeconds);
				UpdatePower(DeltaSeconds);
			}
		}
	}
}

void AFlareCockpitManager::EnterCockpit(AFlareSpacecraft* TargetPlayerShip)
{
	// Ensure we're not doing anything stupid
	check(PC->UseCockpit);
	check(CockpitMesh);
	check(CockpitMesh);
	check(CockpitMaterialTemplate);
	check(CockpitFrameMaterialTemplate);

	// Offset the cockpit
#if FLARE_USE_COCKPIT_RENDERTARGET
	CockpitCapture->AttachTo(TargetPlayerShip->GetRootComponent(), "Camera", EAttachLocation::SnapToTarget);
#endif
	CockpitMesh->AttachTo(TargetPlayerShip->GetCamera(), NAME_None, EAttachLocation::SnapToTarget);

	// FLIR camera
	CockpitFLIRCapture->AttachTo(TargetPlayerShip->GetRootComponent(), "Dock", EAttachLocation::SnapToTarget);

	// General data
	IsInCockpit = true;
	CockpitFLIRCapture->Activate();
	CockpitMesh->SetVisibility(true, true);
}

void AFlareCockpitManager::ExitCockpit(AFlareSpacecraft* TargetPlayerShip)
{
	IsInCockpit = false;
	CockpitFLIRCapture->Deactivate();
	CockpitMesh->SetVisibility(false, true);
}

void AFlareCockpitManager::UpdateTarget(float DeltaSeconds)
{
	float Intensity = 0;
	AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Power out
	bool HasPower = !PlayerShip->GetDamageSystem()->HasPowerOutage() && PlayerShip->GetDamageSystem()->IsAlive();
	if (!HasPower)
	{
		Intensity = 0;
	}

	// Target exists
	else if (TargetShip)
	{
		Intensity = 1;
		FLinearColor Color = TargetShip->GetPlayerWarState() == EFlareHostility::Hostile ? Theme.EnemyColor : Theme.FriendlyColor;
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorTop", Color);

		IFlareSpacecraftNavigationSystemInterface* Nav = PlayerShip->GetNavigationSystem();


		if(Nav->IsDocked())
		{
			CockpitFLIRCapture->Deactivate();
		}
		else
		{
			float MaxFlirCameraAngle = 60.0f;

			FVector TargetLocation = TargetShip->GetActorLocation();
			FFlareShipCommandData Command = Nav->GetCurrentCommand();

			if(Command.Type == EFlareCommandDataType::CDT_Dock)
			{
				AFlareSpacecraft* DockStation = Cast<AFlareSpacecraft>(Command.ActionTarget);
				int32 DockId = Command.ActionTargetParam;

				FFlareDockingInfo StationDockInfo = DockStation->GetDockingSystem()->GetDockInfo(DockId);
				FVector StationDockLocation =  DockStation->Airframe->GetComponentTransform().TransformPosition(StationDockInfo.LocalLocation);
				TargetLocation = StationDockLocation;
			}


			// Find best FLIR camera
			TArray<FName> SocketNames  = PlayerShip->Airframe->GetAllSocketNames();
			float BestAngle = 0;
			FVector TargetDirection;
			FVector CameraMainDirection;
			FVector TargetLocationDelta;
			bool FlirCameraFound = false;
			FName BestCameraName;

			for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); SocketIndex++)
			{
				if (SocketNames[SocketIndex] == "Dock" || SocketNames[SocketIndex].ToString().StartsWith("FLIR"))
				{
					FTransform CameraWorldTransform = PlayerShip->Airframe->GetSocketTransform(SocketNames[SocketIndex]);

					FVector CameraLocation = CameraWorldTransform.GetTranslation();
					FVector CandidateCameraMainDirection = CameraWorldTransform.GetRotation().RotateVector(FVector(1,0,0));

					FVector CandidateTargetLocationDelta = TargetLocation - CameraLocation;
					FVector CandidateTargetDirection = CandidateTargetLocationDelta.GetUnsafeNormal();

					float Angle = FMath::RadiansToDegrees((FMath::Acos(FVector::DotProduct(CandidateTargetDirection, CandidateCameraMainDirection))));

					if (!FlirCameraFound || Angle < BestAngle)
					{
						// Select camera
						BestAngle  = Angle;
						TargetDirection = CandidateTargetDirection;
						CameraMainDirection = CandidateCameraMainDirection;
						TargetLocationDelta = CandidateTargetLocationDelta;
						FlirCameraFound = true;
						BestCameraName = SocketNames[SocketIndex];
					}
				}
			}

			if(FlirCameraFound)
			{
				CockpitFLIRCapture->AttachTo(PlayerShip->GetRootComponent(), BestCameraName, EAttachLocation::SnapToTarget);

				FRotator CameraRotation = TargetDirection.Rotation();

				if(BestAngle > MaxFlirCameraAngle)
				{
					// Clamp max rotation
					FVector RotationAxis = FVector::CrossProduct(CameraMainDirection, TargetDirection).GetUnsafeNormal();
					FVector MaxTurnDirection = CameraMainDirection.RotateAngleAxis(MaxFlirCameraAngle, RotationAxis);

					CameraRotation = MaxTurnDirection.Rotation();
				}

				CameraRotation.Roll = PlayerShip->GetActorRotation().Roll;

				CockpitFLIRCapture->SetWorldRotation(CameraRotation);

				FBox CandidateBox = TargetShip->GetComponentsBoundingBox();
				float TargetSize = FMath::Max(CandidateBox.GetExtent().Size(), 1.0f);

				CockpitFLIRCapture->FOVAngle = 3 * FMath::RadiansToDegrees(FMath::Atan2(TargetSize, TargetLocationDelta.Size()));
				CockpitFLIRCapture->FOVAngle = FMath::Min(CockpitFLIRCapture->FOVAngle, 30.0f);

				CockpitFLIRCapture->Activate();
			}
			else
			{
				CockpitFLIRCapture->Deactivate();
			}
		}
	}
	else
	{
		CockpitFLIRCapture->Deactivate();
	}

	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityTop", Intensity);
}

void AFlareCockpitManager::UpdateInfo(float DeltaSeconds)
{
	float Intensity = 0;

	if (PlayerShip->IsMilitary())
	{
		// Fighter
		if (PlayerShip->GetDescription()->Size == EFlarePartSize::S)
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				Intensity = 1;
				float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex());
				CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorLeft", PC->GetNavHUD()->GetHealthColor(ComponentHealth));
			}
		}

		// Capital
		else
		{
			CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorLeft", PC->GetNavHUD()->GetHealthColor(1));
		}
	}

	CockpitFrameMaterialInstance->SetScalarParameterValue("IndicatorIntensityLeft", Intensity);
}

void AFlareCockpitManager::UpdateTemperature(float DeltaSeconds)
{
	// Update timer
	CockpitHealthLightTime += DeltaSeconds;
	if (CockpitHealthLightTime > CockpitHealthLightPeriod)
	{
		CockpitHealthLightTime -= CockpitHealthLightPeriod;
	}

	// Power out
	if (PlayerShip->GetDamageSystem()->HasPowerOutage())
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorRight", Theme.EnemyColor);
	}

	// Temperature
	else if (CockpitHealthLightTime > CockpitHealthLightPeriod / 2)
	{
		float Temperature = PlayerShip->GetDamageSystem()->GetTemperature();
		float OverheatTemperature = PlayerShip->GetDamageSystem()->GetOverheatTemperature();
		FLinearColor TemperatureColor = PC->GetNavHUD()->GetTemperatureColor(Temperature, OverheatTemperature);
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorRight", TemperatureColor);
	}

	// Cockpit health
	else
	{
		float ComponentHealth = PlayerShip->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport);
		FLinearColor HealthColor = PC->GetNavHUD()->GetHealthColor(ComponentHealth);
		CockpitFrameMaterialInstance->SetVectorParameterValue("IndicatorColorRight", HealthColor);
	}
}

void AFlareCockpitManager::UpdatePower(float DeltaSeconds)
{
	// Update timer
	bool HasPower = !PlayerShip->GetDamageSystem()->HasPowerOutage() && PlayerShip->GetDamageSystem()->IsAlive();
	CockpitPowerTime += (HasPower ? 1.0f : -1.0f) * DeltaSeconds;
	CockpitPowerTime = FMath::Clamp(CockpitPowerTime, 0.0f, CockpitPowerPeriod);
	float PowerAlpha = CockpitPowerTime / CockpitPowerPeriod;

	// Update lights
	float Intensity = 20 + PowerAlpha * 200;
	CockpitLight->SetIntensity(Intensity);
	CockpitLight2->SetIntensity(Intensity);

	// Update materials
	FLinearColor HealthColor = PC->GetNavHUD()->GetHealthColor(PowerAlpha);
	CockpitMaterialInstance->SetScalarParameterValue(      "Power", PowerAlpha);
	CockpitFrameMaterialInstance->SetScalarParameterValue( "Power", PowerAlpha);
	CockpitMaterialInstance->SetVectorParameterValue(      "IndicatorColorBorders", HealthColor);
	CockpitFrameMaterialInstance->SetVectorParameterValue( "IndicatorColorBorders", HealthColor);
}


#undef LOCTEXT_NAMESPACE
