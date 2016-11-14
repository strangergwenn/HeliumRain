
#include "../Flare.h"
#include "FlareCockpitManager.h"

#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSpacecraftComponent.h"
#include "../Spacecrafts/Subsystems/FlareSimulatedSpacecraftDamageSystem.h"

#define LOCTEXT_NAMESPACE "FlareCockpitManager"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareCockpitManager::AFlareCockpitManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, PlayerShip(NULL)
	, CockpitLightingIntensity(5)
	, CockpitHealthLightTimer(0)
	, CockpitHealthLightPeriod(1.58)
	, CockpitTargetLightTimer(0)
	, CockpitTargetLightPeriod(0.8)
	, CockpitPowerTimer(0)
	, CockpitPowerPeriod(0.7)
	, CameraSwitchTimer(0)
	, CameraSwitchPeriod(0.15)
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
		FCHECK(CockpitHUDTarget);
		CockpitHUDTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitHUD);
		CockpitHUDTarget->ClearColor = FLinearColor::Black;
		CockpitHUDTarget->UpdateResource();

		// Instruments texture target
		CockpitInstrumentsTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CockpitInstrumentsTargetSize, CockpitInstrumentsTargetSize);
		FCHECK(CockpitInstrumentsTarget);
		CockpitInstrumentsTarget->OnCanvasRenderTargetUpdate.AddDynamic(PC->GetNavHUD(), &AFlareHUD::DrawCockpitInstruments);
		CockpitInstrumentsTarget->ClearColor = FLinearColor::Black;
		CockpitInstrumentsTarget->UpdateResource();
		
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
		CockpitPowerTimer = 0;
		CameraSwitchTimer = 0;
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
				FCHECK(PlayerShip);

				// TemporalAA needs some tweaking
				float TemporalAAValue = PC->UseTemporalAA ? 1.0f : 0.0f;
				FreighterCockpitFrameMaterialInstance->SetScalarParameterValue("UseTemporalAA", TemporalAAValue);
				FighterCockpitFrameMaterialInstance->SetScalarParameterValue("UseTemporalAA", TemporalAAValue);

				// Update instruments
				UpdateTarget(DeltaSeconds);
				UpdateInfo(DeltaSeconds);
				UpdateHealth(DeltaSeconds);
				UpdatePower(DeltaSeconds);
			}
		}

		// Update scale - minimum scale value depends on MinimalFOV in PC
		float Scale = FMath::Tan(FMath::DegreesToRadians(PC->GetCurrentFOV() / 2.f)) / FMath::Tan(FMath::DegreesToRadians(PC->GetNormalFOV() / 2.f));
		CockpitMesh->SetRelativeScale3D(FVector(1.0, Scale, Scale));
	}
}

void AFlareCockpitManager::SetupCockpitInstances(UMaterialInstanceDynamic* ScreenInstance, UMaterialInstanceDynamic* FrameInstance)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FCHECK(ScreenInstance);
	ScreenInstance->SetVectorParameterValue( "IndicatorColorBorders", Theme.FriendlyColor);
	ScreenInstance->SetTextureParameterValue("HUDTarget",             CockpitHUDTarget);
	ScreenInstance->SetScalarParameterValue( "ScreenLuminosity",      CockpitLightingIntensity);

	FCHECK(FrameInstance);
	FrameInstance->SetVectorParameterValue( "IndicatorColorBorders",  Theme.FriendlyColor);
	FrameInstance->SetTextureParameterValue("InstrumentsTarget",      CockpitInstrumentsTarget);
	FrameInstance->SetScalarParameterValue( "ScreenLuminosity",       CockpitLightingIntensity);
}

void AFlareCockpitManager::EnterCockpit(AFlareSpacecraft* TargetPlayerShip)
{
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);

	// Ensure we're not doing anything stupid
	FCHECK(PC->UseCockpit);
	FCHECK(CockpitMesh);
	FCHECK(FreighterCockpitMeshTemplate);
	FCHECK(FighterCockpitMeshTemplate);

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
	CockpitMesh->AttachToComponent(TargetPlayerShip->GetCamera(), AttachRules, NAME_None);

	// General data
	IsInCockpit = true;
	CockpitMesh->SetVisibility(true, true);
}

void AFlareCockpitManager::ExitCockpit()
{
	IsInCockpit = false;
	CockpitMesh->SetVisibility(false, true);
}

void AFlareCockpitManager::UpdateHealth(float DeltaSeconds)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor IndicatorColor = Theme.FriendlyColor;
	float IndicatorIntensity = 0;

	// Update light timer
	CockpitHealthLightTimer += DeltaSeconds;
	if (CockpitHealthLightTimer > CockpitHealthLightPeriod)
	{
		CockpitHealthLightTimer = 0;
	}

	// Lights are enabled on powered ships
	if (PlayerShipIsPowered())
	{
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = PlayerShip->GetParent()->GetDamageSystem();
		
		if (!DamageSystem->IsAlive())
		{
			IndicatorColor = Theme.DamageColor;
			IndicatorIntensity = 1.0f;
		}
		else if (DamageSystem->IsCrewEndangered() || DamageSystem->IsUncontrollable())
		{
			IndicatorColor = Theme.DamageColor;
			IndicatorIntensity = (CockpitHealthLightTimer > CockpitHealthLightPeriod / 2) ? 1.0f : 0.0f;
		}
		else
		{
			IndicatorColor = Theme.FriendlyColor;
			IndicatorIntensity = 1.0f;
		}
	}

	GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorLeft", IndicatorColor);
	GetCurrentFrameMaterial()->SetScalarParameterValue("IndicatorIntensityLeft", IndicatorIntensity);
}

void AFlareCockpitManager::UpdateTarget(float DeltaSeconds)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor IndicatorColor = Theme.FriendlyColor;
	float IndicatorIntensity = 0;
	
	// Update light timer
	CockpitTargetLightTimer += DeltaSeconds;
	if (CockpitTargetLightTimer > CockpitTargetLightPeriod)
	{
		CockpitTargetLightTimer = 0;
	}

	// Lights are enabled on powered ships
	if (PlayerShipIsPowered())
	{
		bool Targeted, FiredUpon;
		UFlareSimulatedSpacecraft* Threat;
		PC->GetPlayerShipThreatStatus(Targeted, FiredUpon, Threat);

		// Player ship is fired upon
		if (FiredUpon)
		{
			IndicatorColor = Theme.DamageColor;
			IndicatorIntensity = 1.0f;
		}

		// Player ship is being targeted
		else if (Targeted)
		{
			IndicatorColor = Theme.DamageColor;
			IndicatorIntensity = (CockpitTargetLightTimer > CockpitTargetLightPeriod / 2) ? 1.0f : 0.0f;
		}
	}

	GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorTop", IndicatorColor);
	GetCurrentFrameMaterial()->SetScalarParameterValue("IndicatorIntensityTop", IndicatorIntensity);
}

void AFlareCockpitManager::UpdateInfo(float DeltaSeconds)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor IndicatorColor = Theme.FriendlyColor;
	float IndicatorIntensity = 0.0f;

	// Lights are enabled on powered military ships
	if (PlayerShipIsPowered() && PlayerShip->GetParent()->IsMilitary())
	{
		// Fighter
		if (PlayerShip->GetParent()->GetDescription()->Size == EFlarePartSize::S)
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				IndicatorIntensity = 1.0f;
				IndicatorColor = PlayerShip->GetParent()->GetDamageSystem()->IsDisarmed() ? Theme.DamageColor : Theme.FriendlyColor;
			}
		}

		// Capital
		else
		{
			IndicatorIntensity = 1.0f;
		}
	}

	GetCurrentFrameMaterial()->SetVectorParameterValue("IndicatorColorRight", IndicatorColor);
	GetCurrentFrameMaterial()->SetScalarParameterValue("IndicatorIntensityRight", IndicatorIntensity);
}

void AFlareCockpitManager::UpdatePower(float DeltaSeconds)
{		
	// Update camera switch timer
	if (PlayerShip->HasFLIRCameraChanged())
	{
		FLOG("AFlareCockpitManager::UpdatePower : FLIR camera switch");
		CameraSwitchTimer = CameraSwitchPeriod;
	}
	else
	{
		CameraSwitchTimer -= DeltaSeconds;
	}
	CameraSwitchTimer = FMath::Clamp(CameraSwitchTimer, 0.0f, CameraSwitchPeriod);
	float CameraSwitchAlpha = CameraSwitchTimer / CameraSwitchPeriod;

	// Update power timer
	CockpitPowerTimer += (PlayerShipIsPowered() ? 1.0f : -1.0f) * DeltaSeconds;
	CockpitPowerTimer = FMath::Clamp(CockpitPowerTimer, 0.0f, CockpitPowerPeriod);
	float PowerAlpha = CockpitPowerTimer / CockpitPowerPeriod;
	
	// Update lights
	float ZoomIntensity = FMath::Lerp(1.0f, 0.2f, PC->GetShipPawn()->GetStateManager()->GetCombatZoomAlpha());
	float LightIntensity = PowerAlpha * 50 * ZoomIntensity;
	CockpitLight->SetIntensity(LightIntensity);
	CockpitLight2->SetIntensity(LightIntensity);
	
	// Update materials
	FLinearColor HealthColor = PC->GetNavHUD()->GetHealthColor(PowerAlpha);
	GetCurrentScreenMaterial()->SetScalarParameterValue("Power", PowerAlpha);
	GetCurrentFrameMaterial()->SetScalarParameterValue( "Power", PowerAlpha);
	GetCurrentScreenMaterial()->SetScalarParameterValue("CameraSwitched", CameraSwitchAlpha);
	GetCurrentScreenMaterial()->SetVectorParameterValue("IndicatorColorBorders", HealthColor);
	GetCurrentFrameMaterial()->SetVectorParameterValue( "IndicatorColorBorders", HealthColor);
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

inline UStaticMeshComponent* AFlareCockpitManager::GetCockpitMesh() const
{
	return CockpitMesh;
}

bool AFlareCockpitManager::PlayerShipIsPowered() const
{
	return PlayerShip->GetParent()->GetDamageSystem()->IsAlive() && !PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage();
}

UMaterialInstanceDynamic* AFlareCockpitManager::GetCurrentScreenMaterial()
{
	return (PlayerShip->GetParent()->IsMilitary()) ? FighterCockpitMaterialInstance : FreighterCockpitMaterialInstance;
}

UMaterialInstanceDynamic* AFlareCockpitManager::GetCurrentFrameMaterial()
{
	return (PlayerShip->GetParent()->IsMilitary()) ? FighterCockpitFrameMaterialInstance : FreighterCockpitFrameMaterialInstance;
}


#undef LOCTEXT_NAMESPACE
