
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
	, CockpitPowerPeriod(0.7)
	, FreighterCockpitMaterialInstance(NULL)
	, FreighterCockpitFrameMaterialInstance(NULL)
	, FighterCockpitMaterialInstance(NULL)
	, FighterCockpitFrameMaterialInstance(NULL)
	, CockpitHUDTarget(NULL)
	, CockpitInstrumentsTarget(NULL)
{
	// Cockpit meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FreighterCockpitMeshTemplateObj(TEXT("/Game/Gameplay/Cockpit/Freighter/SM_Cockpit_Freighter"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FighterCockpitMeshTemplateObj(TEXT("/Game/Gameplay/Cockpit/Fighter/SM_Cockpit_Fighter"));
	FreighterCockpitMeshTemplate = FreighterCockpitMeshTemplateObj.Object;
	FighterCockpitMeshTemplate = FighterCockpitMeshTemplateObj.Object;

	// Cockpit mesh
	CockpitMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Cockpit"));
	CockpitMesh->SetCollisionProfileName("NoCollision");
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
	CockpitLight->SetIntensity(5);
	CockpitLight->SetRelativeLocation(FVector(0, -30, -30));
	CockpitLight->SetCastShadows(false);
	CockpitLight->LightingChannels.bChannel0 = false;
	CockpitLight->LightingChannels.bChannel1 = true;
	CockpitLight->AttachToComponent(CockpitMesh, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Light 2
	CockpitLight2 = PCIP.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("CockpitLight2"));
	CockpitLight2->SetLightColor(FLinearColor(1.0f, 0.376f, 0.212f));
	CockpitLight2->SetIntensity(5);
	CockpitLight2->SetRelativeLocation(FVector(0, 10, 10));
	CockpitLight2->SetCastShadows(false);
	CockpitLight2->LightingChannels.bChannel0 = false;
	CockpitLight2->LightingChannels.bChannel1 = true;
	CockpitLight2->AttachToComponent(CockpitMesh, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

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
	if (PC->UseCockpit && (FreighterCockpitMaterialInstance == NULL || FighterCockpitMaterialInstance == NULL))
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		// Use same width as parent and 16:9 ratio
		int32 ParentViewportWidth = 1920;
		FVector2D ViewportSize = FVector2D (ParentViewportWidth, (ParentViewportWidth * 9.0f) / 16.0f);
		FLOGV("AFlareCockpitManager::SetupCockpit : will be using 3D cockpit (%dx%d)",
			FMath::RoundToInt(ViewportSize.X), FMath::RoundToInt(ViewportSize.Y));
		
		// HUD texture target
		CockpitHUDTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		check(CockpitHUDTarget);
		CockpitHUDTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitHUD);
		CockpitHUDTarget->ClearColor = FLinearColor::Black;
		CockpitHUDTarget->UpdateResource();

		// Instruments texture target
		CockpitInstrumentsTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitInstrumentsTargetSize, CockpitInstrumentsTargetSize);
		check(CockpitInstrumentsTarget);
		CockpitInstrumentsTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitInstruments);
		CockpitInstrumentsTarget->ClearColor = FLinearColor::Black;
		CockpitInstrumentsTarget->UpdateResource();

		// FLIR camera target
		CockpitFLIRCameraTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitFLIRTargetSize, CockpitFLIRTargetSize);
		check(CockpitFLIRCameraTarget);
		CockpitFLIRCameraTarget->ClearColor = FLinearColor::Black;
		CockpitFLIRCameraTarget->UpdateResource();

		// Setup FLIR camera
		check(CockpitFLIRCapture);
		CockpitFLIRCapture->TextureTarget = CockpitFLIRCameraTarget;

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
#endif

		// Freighter materials
		FreighterCockpitMaterialInstance = UMaterialInstanceDynamic::Create(FreighterCockpitMeshTemplate->GetMaterial(0), GetWorld());
		FreighterCockpitFrameMaterialInstance = UMaterialInstanceDynamic::Create(FreighterCockpitMeshTemplate->GetMaterial(1), GetWorld());
		SetupCockpitInstances(FreighterCockpitMaterialInstance, FreighterCockpitFrameMaterialInstance);

		// Fighter materials
		FighterCockpitMaterialInstance = UMaterialInstanceDynamic::Create(FighterCockpitMeshTemplate->GetMaterial(0), GetWorld());
		FighterCockpitFrameMaterialInstance = UMaterialInstanceDynamic::Create(FighterCockpitMeshTemplate->GetMaterial(1), GetWorld());
		SetupCockpitInstances(FighterCockpitMaterialInstance, FighterCockpitFrameMaterialInstance);

		// Enter if we have to
		if (PC->GetPlayerShip())
		{
			EnterCockpit(PC->GetPlayerShip()->GetActive());
		}
	}
	else
	{
		FLOG("AFlareCockpitManager::SetupCockpit : cockpit manager is disabled");
		FreighterCockpitMaterialInstance = NULL;
		FreighterCockpitFrameMaterialInstance = NULL;
		FighterCockpitMaterialInstance = NULL;
		FighterCockpitFrameMaterialInstance = NULL;
		ExitCockpit();
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
		ExitCockpit();
	}

	// Reset data
	if (PlayerShip != NewPlayerShip)
	{
		CockpitPowerTime = CockpitPowerPeriod;
	}
	PlayerShip = NewPlayerShip;

	// Setup new ship
	if (PlayerShip && PC->UseCockpit && !PlayerShip->GetStateManager()->IsExternalCamera())
	{
		EnterCockpit(PlayerShip);
	}
	else
	{
		ExitCockpit();
	}
}

void AFlareCockpitManager::OnStopFlying()
{
	ExitCockpit();
	PlayerShip = NULL;
}

void AFlareCockpitManager::SetExternalCamera(bool External)
{
	if (!PlayerShip)
	{
		return;
	}

	if (External || !PC->UseCockpit)
	{
		ExitCockpit();
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
		if (FreighterCockpitMaterialInstance == NULL || FighterCockpitMaterialInstance == NULL)
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
			if (FreighterCockpitFrameMaterialInstance && FighterCockpitFrameMaterialInstance)
			{
				UpdateTarget(DeltaSeconds);
				UpdateInfo(DeltaSeconds);
				UpdateTemperature(DeltaSeconds);
				UpdatePower(DeltaSeconds);
			}
		}
	}
}

void AFlareCockpitManager::SetupCockpitInstances(UMaterialInstanceDynamic* ScreenInstance, UMaterialInstanceDynamic* FrameInstance)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	check(ScreenInstance);
	check(FrameInstance);

	ScreenInstance->SetVectorParameterValue( "IndicatorColorBorders", Theme.FriendlyColor);
	ScreenInstance->SetTextureParameterValue("HUDTarget",             CockpitHUDTarget);
#if FLARE_USE_COCKPIT_RENDERTARGET
	ScreenInstance->SetTextureParameterValue("CameraTarget",          CockpitCameraTarget);
#endif

	FrameInstance->SetVectorParameterValue( "IndicatorColorBorders",  Theme.FriendlyColor);
	FrameInstance->SetTextureParameterValue("FLIRTarget",             CockpitFLIRCameraTarget);
	FrameInstance->SetTextureParameterValue("InstrumentsTarget",      CockpitInstrumentsTarget);
}

void AFlareCockpitManager::EnterCockpit(AFlareSpacecraft* TargetPlayerShip)
{
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);

	// Ensure we're not doing anything stupid
	check(PC->UseCockpit);
	check(CockpitMesh);
	check(FreighterCockpitMeshTemplate);
	check(FighterCockpitMeshTemplate);

	// Set the correct variation
	if (TargetPlayerShip->GetParent()->IsMilitary())
	{
		CockpitMesh->SetStaticMesh( FighterCockpitMeshTemplate);
		CockpitMesh->SetMaterial(0, FighterCockpitMaterialInstance);
		CockpitMesh->SetMaterial(1, FighterCockpitFrameMaterialInstance);
	}
	else
	{
		CockpitMesh->SetStaticMesh( FreighterCockpitMeshTemplate);
		CockpitMesh->SetMaterial(0, FreighterCockpitMaterialInstance);
		CockpitMesh->SetMaterial(1, FreighterCockpitFrameMaterialInstance);
	}

	// Offset the cockpit
#if FLARE_USE_COCKPIT_RENDERTARGET
	CockpitCapture->AttachToComponent(TargetPlayerShip->GetRootComponent(), AttachRules, "Camera");
#endif
	CockpitMesh->AttachToComponent(TargetPlayerShip->GetCamera(), AttachRules, NAME_None);

	// FLIR camera
	CockpitFLIRCapture->AttachToComponent(TargetPlayerShip->GetRootComponent(), AttachRules, "Dock");

	// General data
	IsInCockpit = true;
	CockpitFLIRCapture->Activate();
	CockpitMesh->SetVisibility(true, true);
}

void AFlareCockpitManager::ExitCockpit()
{
	IsInCockpit = false;
	CockpitFLIRCapture->Deactivate();
	CockpitMesh->SetVisibility(false, true);
}

void AFlareCockpitManager::UpdateTarget(float DeltaSeconds)
{
	check(PlayerShip);
	float Intensity = 0;
	AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Power out
	bool HasPower = !PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage() && PlayerShip->GetParent()->GetDamageSystem()->IsAlive();
	if (!HasPower)
	{
		Intensity = 0;
	}

	// Target exists
	else if (TargetShip)
	{
		Intensity = 1;
		FLinearColor Color = TargetShip->GetParent()->GetPlayerWarState() == EFlareHostility::Hostile ? Theme.EnemyColor : Theme.FriendlyColor;
		GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorTop", Color);

		UFlareSpacecraftNavigationSystem* Nav = PlayerShip->GetNavigationSystem();
		if (Nav->IsDocked())
		{
			CockpitFLIRCapture->Deactivate();
		}
		else
		{
			float MaxFlirCameraAngle = 60.0f;
			FVector TargetLocation = TargetShip->GetActorLocation();
			FFlareShipCommandData Command = Nav->GetCurrentCommand();

			// Get docking data
			if (Command.Type == EFlareCommandDataType::CDT_Dock)
			{
				AFlareSpacecraft* DockStation = Command.ActionTarget;
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

			// Find the best FLIR camera on the ship if any
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

			// Update the FLIR camera
			if (FlirCameraFound)
			{
				FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
				CockpitFLIRCapture->AttachToComponent(PlayerShip->GetRootComponent(), AttachRules, BestCameraName);

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

	GetCurrentFrameMaterial()->SetScalarParameterValue("IndicatorIntensityTop", Intensity);
}

void AFlareCockpitManager::UpdateInfo(float DeltaSeconds)
{
	check(PlayerShip);
	float Intensity = 0;

	if (PlayerShip->GetParent()->IsMilitary())
	{
		// Fighter
		if (PlayerShip->GetParent()->GetDescription()->Size == EFlarePartSize::S)
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				Intensity = 1;
				float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex());
				GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorRight", PC->GetNavHUD()->GetHealthColor(ComponentHealth));
			}
		}

		// Capital
		else
		{
			Intensity = 1;
			GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorRight", PC->GetNavHUD()->GetHealthColor(1));
		}
	}

	GetCurrentFrameMaterial()->SetScalarParameterValue("IndicatorIntensityRight", Intensity);
}

void AFlareCockpitManager::UpdateTemperature(float DeltaSeconds)
{
	check(PlayerShip);

	// Update timer
	CockpitHealthLightTime += DeltaSeconds;
	if (CockpitHealthLightTime > CockpitHealthLightPeriod)
	{
		CockpitHealthLightTime -= CockpitHealthLightPeriod;
	}

	// Power out
	if (PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage())
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorLeft", Theme.EnemyColor);
	}

	// Temperature
	else if (CockpitHealthLightTime > CockpitHealthLightPeriod / 2)
	{
		float Temperature = PlayerShip->GetParent()->GetDamageSystem()->GetTemperature();
		float OverheatTemperature = PlayerShip->GetParent()->GetDamageSystem()->GetOverheatTemperature();
		FLinearColor TemperatureColor = PC->GetNavHUD()->GetTemperatureColor(Temperature, OverheatTemperature);
		GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorLeft", TemperatureColor);
	}

	// Cockpit health
	else
	{
		float ComponentHealth = PlayerShip->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport);
		FLinearColor HealthColor = PC->GetNavHUD()->GetHealthColor(ComponentHealth);
		GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorLeft", HealthColor);
	}
}

void AFlareCockpitManager::UpdatePower(float DeltaSeconds)
{
	check(PlayerShip);

	// Update timer
	bool HasPower = !PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage() && PlayerShip->GetParent()->GetDamageSystem()->IsAlive();
	CockpitPowerTime += (HasPower ? 1.0f : -1.0f) * DeltaSeconds;
	CockpitPowerTime = FMath::Clamp(CockpitPowerTime, 0.0f, CockpitPowerPeriod);
	float PowerAlpha = CockpitPowerTime / CockpitPowerPeriod;

	// Update lights
	float Intensity = 20 + PowerAlpha * 200;
	CockpitLight->SetIntensity(Intensity);
	CockpitLight2->SetIntensity(Intensity);

	// Update materials
	FLinearColor HealthColor = PC->GetNavHUD()->GetHealthColor(PowerAlpha);
	GetCurrentScreenMaterial()->SetScalarParameterValue("Power", PowerAlpha);
	GetCurrentFrameMaterial()->SetScalarParameterValue( "Power", PowerAlpha);
	GetCurrentScreenMaterial()->SetVectorParameterValue("IndicatorColorBorders", HealthColor);
	GetCurrentFrameMaterial()->SetVectorParameterValue( "IndicatorColorBorders", HealthColor);
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

UMaterialInstanceDynamic* AFlareCockpitManager::GetCurrentScreenMaterial()
{
	return (PlayerShip->GetParent()->IsMilitary()) ? FighterCockpitMaterialInstance : FreighterCockpitMaterialInstance;
}

UMaterialInstanceDynamic* AFlareCockpitManager::GetCurrentFrameMaterial()
{
	return (PlayerShip->GetParent()->IsMilitary()) ? FighterCockpitFrameMaterialInstance : FreighterCockpitFrameMaterialInstance;
}


#undef LOCTEXT_NAMESPACE
