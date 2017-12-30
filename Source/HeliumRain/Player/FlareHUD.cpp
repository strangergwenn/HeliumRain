
#include "FlareHUD.h"
#include "../Flare.h"

#include "../Economy/FlareCargoBay.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareSector.h"
#include "../Game/AI/FlareCompanyAI.h"
#include "../Game/FlareGameUserSettings.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlarePilotHelper.h"
#include "../Spacecrafts/FlareTurret.h"
#include "../Spacecrafts/Subsystems/FlareSpacecraftDamageSystem.h"
#include "../Spacecrafts/Subsystems/FlareSpacecraftNavigationSystem.h"

#include "../UI/Components/FlareNotifier.h"
#include "../UI/HUD/FlareHUDMenu.h"
#include "../UI/HUD/FlareContextMenu.h"

#include "Engine/PostProcessVolume.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"


#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CombatMouseRadius(100)
	, HUDVisible(true)
	, PreviousScreenPercentage(0)
	, HasPlayerHit(false)
	, CurrentPowerTime(0)
	, PowerTransitionTime(0.5f)
	, ShowPerformance(false)
	, PerformanceTimer(0)
	, FrameTime(0)
	, GameThreadTime(0)
	, RenderThreadTime(0)
	, GPUFrameTime(0)
{
	// Load content (general icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDReticleIconObj         (TEXT("/Game/Gameplay/HUD/TX_Reticle.TX_Reticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatReticleIconobj   (TEXT("/Game/Gameplay/HUD/TX_CombatReticle.TX_CombatReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBackReticleIconObj     (TEXT("/Game/Gameplay/HUD/TX_BackReticle.TX_BackReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimIconObj             (TEXT("/Game/Gameplay/HUD/TX_Aim.TX_Aim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimHitIconObj          (TEXT("/Game/Gameplay/HUD/TX_AimHit.TX_AimHit"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombAimIconObj         (TEXT("/Game/Gameplay/HUD/TX_BombAim.TX_BombAim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombMarkerObj          (TEXT("/Game/Gameplay/HUD/TX_BombMarker.TX_BombMarker"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimHelperIconObj       (TEXT("/Game/Gameplay/HUD/TX_AimHelper.TX_AimHelper"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDNoseIconObj            (TEXT("/Game/Gameplay/HUD/TX_Nose.TX_Nose"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDObjectiveIconObj       (TEXT("/Game/Gameplay/HUD/TX_Objective.TX_Objective"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDockingCircleObj       (TEXT("/Game/Gameplay/HUD/TX_DockingCircle.TX_DockingCircle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDockingAxisObj         (TEXT("/Game/Gameplay/HUD/TX_DockingAxis.TX_DockingAxis"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDockingForbiddenObj    (TEXT("/Game/Slate/Icons/TX_Icon_Delete.TX_Icon_Delete"));

	// Load content (designator icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatMouseIconObj     (TEXT("/Game/Gameplay/HUD/TX_CombatCursor.TX_CombatCursor"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDSearchArrowIconObj     (TEXT("/Game/Gameplay/HUD/TX_SearchArrow.TX_SearchArrow"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorCornerObj    (TEXT("/Game/Gameplay/HUD/TX_DesignatorCorner.TX_DesignatorCorner"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHighlightSearchArrowObj(TEXT("/Game/Gameplay/HUD/TX_HighlightSearchArrow.TX_HighlightSearchArrow"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorMilCornerObj (TEXT("/Game/Gameplay/HUD/TX_DesignatorMilitaryCorner.TX_DesignatorMilitaryCorner"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectedCornerObj(TEXT("/Game/Gameplay/HUD/TX_DesignatorCornerSelected.TX_DesignatorCornerSelected"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectionObj (TEXT("/Game/Slate/Icons/TX_Icon_TargettingContextButton.TX_Icon_TargettingContextButton"));

	// Load content (status icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDTemperatureIconObj     (TEXT("/Game/Slate/Icons/TX_Icon_Temperature.TX_Icon_Temperature"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPowerIconObj           (TEXT("/Game/Slate/Icons/TX_Icon_Power.TX_Icon_Power"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPropulsionIconObj      (TEXT("/Game/Slate/Icons/TX_Icon_Propulsion.TX_Icon_Propulsion"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDRCSIconObj             (TEXT("/Game/Slate/Icons/TX_Icon_RCS.TX_Icon_RCS"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHealthIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_LifeSupport.TX_Icon_LifeSupport"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDWeaponIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_Shell.TX_Icon_Shell"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHarpoonedIconObj       (TEXT("/Game/Slate/Icons/TX_Icon_Harpooned.TX_Icon_Harpooned"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDOrientationIconObj     (TEXT("/Game/Slate/Icons/TX_Icon_Travel.TX_Icon_Travel"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDistanceIconObj        (TEXT("/Game/Slate/Icons/TX_Icon_Distance.TX_Icon_Distance"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDContractIconObj        (TEXT("/Game/Slate/Icons/TX_Icon_ContractTarget.TX_Icon_ContractTarget"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDShipyardIconObj        (TEXT("/Game/Slate/Icons/TX_Icon_Shipyard.TX_Icon_Shipyard"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDUpgradeIconObj         (TEXT("/Game/Slate/Icons/TX_Icon_UpgradeCapability.TX_Icon_UpgradeCapability"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDConstructionIconObj    (TEXT("/Game/Slate/Icons/TX_Icon_Build.TX_Icon_Build"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDConsumerIconObj        (TEXT("/Game/Slate/Icons/TX_Icon_Sector.TX_Icon_Sector"));

	// Load content (fonts)
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontSmallObj           (TEXT("/Game/Slate/Fonts/HudFontSmall.HudFontSmall"));
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontObj                (TEXT("/Game/Slate/Fonts/HudFont.HudFont"));
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontLargeObj           (TEXT("/Game/Slate/Fonts/HudFontLarge.HudFontLarge"));
	
	// Load content (materials)
	static ConstructorHelpers::FObjectFinder<UMaterial> HUDRenderTargetMaterialTemplateObj(TEXT("/Game/Gameplay/HUD/MT_HUDMaterial.MT_HUDMaterial"));

	// Set content (general icons)
	HUDReticleIcon = HUDReticleIconObj.Object;
	HUDCombatReticleIcon = HUDCombatReticleIconobj.Object;
	HUDBackReticleIcon = HUDBackReticleIconObj.Object;
	HUDAimIcon = HUDAimIconObj.Object;
	HUDAimHitIcon = HUDAimHitIconObj.Object;
	HUDBombAimIcon = HUDBombAimIconObj.Object;
	HUDBombMarker = HUDBombMarkerObj.Object;
	HUDAimHelperIcon = HUDAimHelperIconObj.Object;
	HUDNoseIcon = HUDNoseIconObj.Object;
	HUDObjectiveIcon = HUDObjectiveIconObj.Object;
	HUDDockingCircleTexture = HUDDockingCircleObj.Object;
	HUDDockingAxisTexture = HUDDockingAxisObj.Object;
	HUDDockingForbiddenTexture = HUDDockingForbiddenObj.Object;

	// Load content (designator icons)
	HUDCombatMouseIcon = HUDCombatMouseIconObj.Object;
	HUDSearchArrowIcon = HUDSearchArrowIconObj.Object;
	HUDHighlightSearchArrowTexture = HUDHighlightSearchArrowObj.Object;
	HUDDesignatorCornerTexture = HUDDesignatorCornerObj.Object;
	HUDDesignatorMilCornerTexture = HUDDesignatorMilCornerObj.Object;
	HUDDesignatorCornerSelectedTexture = HUDDesignatorSelectedCornerObj.Object;
	HUDDesignatorSelectionTexture = HUDDesignatorSelectionObj.Object;

	// Set content (status icons)
	HUDTemperatureIcon = HUDTemperatureIconObj.Object;
	HUDPowerIcon = HUDPowerIconObj.Object;
	HUDPropulsionIcon = HUDPropulsionIconObj.Object;
	HUDRCSIcon = HUDRCSIconObj.Object;
	HUDHealthIcon = HUDHealthIconObj.Object;
	HUDWeaponIcon = HUDWeaponIconObj.Object;
	HUDHarpoonedIcon = HUDHarpoonedIconObj.Object;
	HUDOrientationIcon = HUDOrientationIconObj.Object;
	HUDDistanceIcon = HUDDistanceIconObj.Object;
	HUDContractIcon = HUDContractIconObj.Object;
	HUDShipyardIcon = HUDShipyardIconObj.Object;
	HUDUpgradeIcon = HUDUpgradeIconObj.Object;
	HUDConstructionIcon = HUDConstructionIconObj.Object;
	HUDConsumerIcon = HUDConsumerIconObj.Object;

	// Set content (fonts)
	HUDFontSmall = HUDFontSmallObj.Object;
	HUDFont = HUDFontObj.Object;
	HUDFontLarge = HUDFontLargeObj.Object;

	// Set content (materials)
	HUDRenderTargetMaterialTemplate = HUDRenderTargetMaterialTemplateObj.Object;

	// Settings
	IconSize = 24;
	FocusDistance = 10000000;
	PlayerHitDisplayTime = 0.08f;
	ShadowColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

	// Cockpit instruments
	TopInstrument =   FVector2D(20, 10);
	LeftInstrument =  FVector2D(20, 165);
	RightInstrument = FVector2D(30, 320);
	InstrumentSize =  FVector2D(380, 115);
	InstrumentLine =  FVector2D(0, 20);
	SmallProgressBarSize = 50;
	LargeProgressBarSize = 120;
	ProgressBarHeight = 10;
	ProgressBarInternalMargin = 3;
	ProgressBarTopMargin = 6;
	CockpitIconSize = 20;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void AFlareHUD::BeginPlay()
{
	Super::BeginPlay();

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get HUD colors
	HudColorNeutral = Theme.NeutralColor.Desaturate(0.2f);
	HudColorFriendly = Theme.FriendlyColor.Desaturate(0.2f);
	HudColorEnemy = Theme.EnemyColor.Desaturate(0.2f);
	HudColorObjective = Theme.ObjectiveColor.Desaturate(0.2f);
	HudColorNeutral.A = 1;
	HudColorFriendly.A = 1;
	HudColorEnemy.A = 1;
	HudColorObjective.A = 1;

	// HUD material
	HUDRenderTargetMaterial = UMaterialInstanceDynamic::Create(HUDRenderTargetMaterialTemplate, GetWorld());
	FCHECK(HUDRenderTargetMaterial);
	MenuManager->GetGame()->GetPostProcessVolume()->AddOrUpdateBlendable(HUDRenderTargetMaterial);
}

void AFlareHUD::Setup(AFlareMenuManager* NewMenuManager)
{
	MenuManager = NewMenuManager;

	if (GEngine->IsValidLowLevel())
	{
		// Create HUD menus
		SAssignNew(HUDMenu, SFlareHUDMenu).MenuManager(MenuManager);
		SAssignNew(MouseMenu, SFlareMouseMenu).MenuManager(MenuManager);

		// Context menu
		SAssignNew(ContextMenuContainer, SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ContextMenu, SFlareContextMenu)
				.HUD(this)
				.MenuManager(MenuManager)
			];

		// Register menus at their Z-Index
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(HUDMenu.ToSharedRef()), 0);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MouseMenu.ToSharedRef()), 5);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ContextMenuContainer.ToSharedRef()), 10);
	}
}


/*----------------------------------------------------
	HUD interaction
----------------------------------------------------*/

void AFlareHUD::ToggleHUD()
{
	HUDVisible = !HUDVisible;
	UpdateHUDVisibility();
}

void AFlareHUD::SetInteractive(bool Status)
{
	IsInteractive = Status;
}

void AFlareHUD::SetWheelMenu(bool State, bool EnableActionOnClose)
{
	if (State)
	{
		MouseMenu->Open();
	}
	else
	{
		MouseMenu->Close(EnableActionOnClose);
	}
	UpdateHUDVisibility();
}

void AFlareHUD::SetWheelCursorMove(FVector2D Move)
{
	MouseMenu->SetWheelCursorMove(Move);
}

bool AFlareHUD::IsWheelMenuOpen() const
{
	return MouseMenu->IsOpen();
}

void AFlareHUD::OnTargetShipChanged()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->GetPlayerShip())
	{
		HUDMenu->OnPlayerShipChanged();
		UpdateHUDVisibility();
	}
}

void AFlareHUD::UpdateHUDVisibility()
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	HUDMenu->SetVisibility((HUDVisible && !MenuManager->IsMenuOpen() && PC->GetPlayerShip()->GetDamageSystem()->IsAlive() && !PC->UseCockpit) ?
		EVisibility::Visible : EVisibility::Collapsed);
	MenuManager->GetNotifier()->SetVisibility((HUDVisible) ?
		EVisibility::SelfHitTestInvisible : EVisibility::Hidden);
}

void AFlareHUD::SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType)
{
	PlayerHitSpacecraft = HitSpacecraft;
	HasPlayerHit = true;
	PlayerDamageType = DamageType;
	PlayerHitTime = 0;
}

void AFlareHUD::DrawHUD()
{
	Super::DrawHUD();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	// This is the normal HUD in 2D, it is always drawn no matter what
	// There is a second call to DrawHUDInternal() in DrawHUDTexture that can trigger HUD drawing

	if (PC)
	{
		// Setup data
		CurrentViewportSize = ViewportSize;
		CurrentCanvas = Canvas;
		IsDrawingHUD = true;

		// Look for a spacecraft to draw the context menu on
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		UpdateContextMenu(PlayerShip);

		// Draw nose
		if (HUDVisible && ShouldDrawHUD())
		{
			bool IsExternalCamera = PlayerShip->GetStateManager()->IsExternalCamera();
			EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();
			if (HUDVisible && !IsExternalCamera)
			{
				// Color
				FLinearColor HUDNosePowerColor = HudColorNeutral;
				HUDNosePowerColor.A = CurrentPowerTime / PowerTransitionTime;

				// Nose texture
				UTexture2D* NoseIcon = HUDNoseIcon;
				if (WeaponType == EFlareWeaponGroupType::WG_GUN)
				{
					NoseIcon = (HasPlayerHit) ? HUDAimHitIcon : HUDAimIcon;
				}

				// Nose drawing
				if (WeaponType != EFlareWeaponGroupType::WG_TURRET)
				{
					DrawHUDIcon(
						CurrentViewportSize / 2,
						IconSize,
						NoseIcon,
						HUDNosePowerColor,
						true);
				}

				// Speed indication
				FVector ShipSmoothedVelocity = PlayerShip->GetSmoothedLinearVelocity() * 100;
				int32 SpeedMS = (ShipSmoothedVelocity.Size() + 10.) / 100.0f;
				FText VelocityText = FText::FromString(FString::FromInt(PlayerShip->IsMovingForward() ? SpeedMS : -SpeedMS) + FString(" m/s"));
				FlareDrawText(VelocityText, FVector2D(0, 70), HUDNosePowerColor, true);
			}
		}
	}

	// Player hit management
	if (PlayerHitTime >= PlayerHitDisplayTime)
	{
		PlayerHitSpacecraft = NULL;
		HasPlayerHit = false;
	}
}

void AFlareHUD::DrawHUDTexture(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	CurrentViewportSize = FVector2D(Width, Height);
	CurrentCanvas = TargetCanvas;
	IsDrawingHUD = true;

	if (HUDVisible && ShouldDrawHUD())
	{
		DrawHUDInternal();
	}
}

void AFlareHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

	if (!GEngine)
	{
		return;
	}

	// Power timer
	if (PlayerShip
		&& PlayerShip->GetParent()->GetDamageSystem()->IsAlive()
		&& !PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage())
	{
		CurrentPowerTime += DeltaSeconds;
	}
	else
	{
		CurrentPowerTime -= DeltaSeconds;
	}

	// Update data
	PlayerHitTime += DeltaSeconds;
	ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	CurrentPowerTime = FMath::Clamp(CurrentPowerTime, 0.0f, PowerTransitionTime);
	
	// Ingame profiler
	if (ShowPerformance)
	{
		// Get time values
		float DiffTime = FApp::GetCurrentTime() - FApp::GetLastTime();
		float RawFrameTime = DiffTime * 1000.0f;
		float RawGameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
		float RawRenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
		float RawGPUFrameTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());

		// Average values
		FrameTime = 0.9 * FrameTime + 0.1 * RawFrameTime;
		GameThreadTime = 0.9 * GameThreadTime + 0.1 * RawGameThreadTime;
		RenderThreadTime = 0.9 * RenderThreadTime + 0.1 * RawRenderThreadTime;
		GPUFrameTime = 0.9 * GPUFrameTime + 0.1 * RawGPUFrameTime;
		
		// Show values every second
		PerformanceTimer += DeltaSeconds;
		if (PerformanceTimer >= 1.0f)
		{
			FNumberFormattingOptions Options;
			Options.MaximumFractionalDigits = 1;
			Options.MinimumFractionalDigits = 1;

			PerformanceText = FText::Format(LOCTEXT("PerfFormat", "Frame : {0} Game : {1} Render : {2} GPU : {3}"),
				FText::AsNumber(FrameTime, &Options),
				FText::AsNumber(GameThreadTime, &Options),
				FText::AsNumber(RenderThreadTime, &Options),
				FText::AsNumber(GPUFrameTime, &Options));

			FLOGV("AFlareHUD::Tick : %s", *PerformanceText.ToString());
			PerformanceTimer = 0;
		}
	}

	// Update HUD texture target if the viewport has changed size
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	if (!HUDRenderTarget || ViewportSize != PreviousViewportSize || MyGameSettings->ScreenPercentage != PreviousScreenPercentage)
	{
		FLOGV("AFlareHUD::Tick : Reallocating HUD target to %dx%d", (int)ViewportSize.X, (int)ViewportSize.Y);

		HUDRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		if (HUDRenderTarget)
		{
			HUDRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlareHUD::DrawHUDTexture);
			HUDRenderTarget->ClearColor = FLinearColor::Black;
			HUDRenderTarget->UpdateResource();
		}

		PreviousScreenPercentage = MyGameSettings->ScreenPercentage;
		PreviousViewportSize = ViewportSize;
	}

	// Update power on the HUD material when in cockpit mode
	if (HUDRenderTargetMaterial)
	{
		float PowerAlpha = PC->UseCockpit ? CurrentPowerTime / PowerTransitionTime : 1.0f;

		// Should we paint the render target ?
		bool DrawRenderTarget = false;
		if (PC->UseCockpit)
		{
			DrawRenderTarget = PlayerShip && !PC->IsInMenu();
		}
		else
		{
			DrawRenderTarget = PlayerShip && !PC->IsInMenu() && PlayerShip->GetParent()->GetDamageSystem()->IsAlive();
		}

		// Paint the render target
		HUDRenderTargetMaterial->SetScalarParameterValue("PowerAlpha", PowerAlpha);
		HUDRenderTargetMaterial->SetScalarParameterValue("MaxAlpha", DrawRenderTarget ? 1.0f : 0.0f);
		HUDRenderTargetMaterial->SetTextureParameterValue("Texture", HUDRenderTarget);
	}

	// Mouse control
	if (PC && !MouseMenu->IsOpen())
	{
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
		MousePos = 2 * ((MousePos - ViewportCenter) / ViewportSize);
		PC->MousePositionInput(MousePos);
	}

	// Update HUD
	if (HUDRenderTarget && ShouldDrawHUD())
	{
		HUDRenderTarget->UpdateResource();
	}
}

void AFlareHUD::TogglePerformance()
{
	ShowPerformance = !ShowPerformance;
}


/*----------------------------------------------------
	Cockpit HUD drawing
----------------------------------------------------*/

void AFlareHUD::DrawCockpitInstruments(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->UseCockpit)
	{
		// Dynamic data
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		CurrentViewportSize = FVector2D(Width, Height);
		CurrentCanvas = TargetCanvas;
		IsDrawingHUD = false;

		// Draw instruments
		if (PlayerShip && PlayerShip->IsValidLowLevel() && PlayerShip->GetParent()->GetDamageSystem()->IsAlive())
		{
			// Regular case
			if (!PlayerShip->GetParent()->GetDamageSystem()->HasPowerOutage())
			{
				DrawCockpitSubsystems(PlayerShip);
				DrawCockpitEquipment(PlayerShip);
				DrawCockpitTarget(PlayerShip);
			}

			// Power out
			else
			{
				// Power out text
				FText PowerOut = LOCTEXT("PowerOut", "NO POWER");
				FText PowerOutInfo = FText::Format(LOCTEXT("PwBackInFormat", "Power back in {0}..."),
					FText::AsNumber((int32)(PlayerShip->GetParent()->GetDamageSystem()->GetPowerOutageDuration()) + 1));

				// Draw
				FVector2D CurrentPos = RightInstrument;
				const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
				FlareDrawText(PowerOut, CurrentPos, Theme.EnemyColor, false, true);
				CurrentPos += 2 * InstrumentLine;
				FlareDrawText(PowerOutInfo, CurrentPos, Theme.EnemyColor, false);
			}
		}
	}
}

void AFlareHUD::DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip)
{
	// Data
	FVector2D CurrentPos = LeftInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Temperature = PlayerShip->GetParent()->GetDamageSystem()->GetTemperature();
	int32 MaxTemperature = PlayerShip->GetParent()->GetDamageSystem()->GetOverheatTemperature();

	// Ship name
	FText FlyingText = UFlareGameTools::DisplaySpacecraftName(PlayerShip->GetParent());
	FlareDrawText(FlyingText, CurrentPos, Theme.FriendlyColor, false, true);
	CurrentPos += 2 * InstrumentLine;

	// Ship status
	FlareDrawText(PlayerShip->GetShipStatus(), CurrentPos, Theme.FriendlyColor, false);
	CurrentPos += InstrumentLine;
	
	// Temperature data
	FText TemperatureText;
	FLinearColor TemperatureColor;
	if (Temperature > MaxTemperature)
	{
		TemperatureText = FText::Format(LOCTEXT("TemperatureOverheatingFormat", "OVERHEATING ({0}K)"), FText::AsNumber(Temperature));
		TemperatureColor = Theme.EnemyColor;
	}
	else
	{
		TemperatureText = FText::Format(LOCTEXT("TemperatureFormat", "Temperature: {0}K"), FText::AsNumber(Temperature));
		TemperatureColor = GetTemperatureColor(Temperature, PlayerShip->GetParent()->GetDamageSystem()->GetOverheatTemperature());
	}

	// Temperature draw
	DrawHUDIcon(CurrentPos, CockpitIconSize, HUDTemperatureIcon, TemperatureColor);
	FlareDrawText(TemperatureText, CurrentPos + FVector2D(1.5 * CockpitIconSize, 0), TemperatureColor, false);
	CurrentPos += InstrumentLine;

	// Subsystem health
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Temperature, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Propulsion, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_RCS, CurrentPos);
	CurrentPos += FVector2D(InstrumentSize.X / 2, 0) - 3 * InstrumentLine;
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_LifeSupport, CurrentPos);
	DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Power, CurrentPos);

	// Weapons
	if (PlayerShip->GetParent()->IsMilitary())
	{
		DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Weapon, CurrentPos);
	}

	// Ship icon
	int32 ShipIconSize = 80;
	UTexture2D* ShipIcon = Cast<UTexture2D>(PlayerShip->GetParent()->GetDescription()->MeshPreviewBrush.GetResourceObject());
	DrawHUDIcon(LeftInstrument + FVector2D(InstrumentSize.X - ShipIconSize, 0), ShipIconSize, ShipIcon, Theme.FriendlyColor);
}

void AFlareHUD::DrawCockpitEquipment(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = RightInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareTacticManager* TacticManager = MenuManager->GetPC()->GetCompany()->GetTacticManager();
	
	// Docking info
	FText DockInfo;
	AFlareSpacecraft* DockSpacecraft = NULL;
	FFlareDockingParameters DockParameters;
	bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);
	
	// Scanner
	if (PlayerShip->IsInScanningMode())
	{
		// Get progress
		bool AngularIsActive, LinearIsActive, ScanningIsActive;
		float ScanningAngularRatio, ScanningLinearRatio, ScanningAnalyzisRatio;
		float ScanningDistance, ScanningSpeed;
		PlayerShip->GetScanningProgress(AngularIsActive, LinearIsActive, ScanningIsActive,
			ScanningAngularRatio, ScanningLinearRatio, ScanningAnalyzisRatio,
			ScanningDistance, ScanningSpeed);

		// Texts
		FText ScanningText = LOCTEXT("Scanning", "Unknown signal detected");
		FText ScanningInfoText = LOCTEXT("ScanningInfo", "Scanning unknown signal...");
		FText AlignmentText = FText::Format(LOCTEXT("AlignmentFormat", "Signal alignment : {0}%"),
			FText::AsNumber(FMath::RoundToInt(100 * FMath::Clamp(ScanningAngularRatio, 0.0f, 1.0f))));
		FText DistanceText = FText::Format(LOCTEXT("DistanceFormat", "Signal distance : {0}m"),
			FText::AsNumber(FMath::RoundToInt(ScanningDistance)));
		FText AnalyzisText = FText::Format(LOCTEXT("AnalyzisFormat", "Signal analyzis : {0}%"),
			FText::AsNumber(FMath::RoundToInt(100 * ScanningAnalyzisRatio)));

		// Draw panel
		FlareDrawText(ScanningText, CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;
		FlareDrawText(ScanningInfoText, CurrentPos, Theme.FriendlyColor, false, false);
		CurrentPos += 1.5 * InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDOrientationIcon, AlignmentText,
			AngularIsActive ? Theme.FriendlyColor : Theme.EnemyColor,
			ScanningAngularRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDDistanceIcon, DistanceText,
			LinearIsActive ? Theme.FriendlyColor : Theme.EnemyColor,
			ScanningLinearRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDPowerIcon, AnalyzisText,
			ScanningIsActive ? Theme.FriendlyColor : Theme.EnemyColor,
			ScanningAnalyzisRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
	}

	// Docking helper
	else if (DockSpacecraft)
	{
		// Angular / linear errors
		float AngularDot = FVector::DotProduct(DockParameters.ShipDockAxis.GetSafeNormal(), -DockParameters.StationDockAxis.GetSafeNormal());
		float AngularError = FMath::RadiansToDegrees(FMath::Acos(AngularDot));
		float LinearError = FVector::VectorPlaneProject(DockParameters.DockToDockDeltaLocation, DockParameters.StationDockAxis).Size() / 100;

		// Roll error
		FVector StationTopAxis = DockParameters.StationDockTopAxis;
		FVector TopAxis = PlayerShip->GetActorRotation().RotateVector(FVector(0, 0, 1));
		FVector LocalTopAxis = FVector::VectorPlaneProject(TopAxis, DockParameters.StationDockAxis).GetSafeNormal();
		float RollDot = FVector::DotProduct(StationTopAxis, LocalTopAxis);
		float RollError = FMath::RadiansToDegrees(FMath::Acos(RollDot));

		// Ratios
		float AngularTarget = 2; // 2° max error
		float LinearTarget = 1; // 2m max error
		float DistanceTarget = (PlayerShip->GetSize() == EFlarePartSize::S) ? 250 : 500;
		float DistanceRatio = 1 - FMath::Clamp(DockParameters.DockToDockDistance / 10000, 0.0f, 1.0f); // 100m distance range
		float RollRatio = 1 - FMath::Clamp(RollError / 30, 0.0f, 1.0f); // 30° range
		float AngularRatio = 1 - FMath::Clamp(AngularError / 30, 0.0f, 1.0f); // 30° range
		float LinearRatio = 1 - FMath::Clamp(LinearError / 10, 0.0f, 1.0f); // 10m range

		// Texts
		FText DockingText = LOCTEXT("Docking", "Docking computer");
		FText DistanceText = FText::Format(LOCTEXT("DockingDistanceFormat", "Distance : {0}m"),
			FText::AsNumber(FMath::RoundToInt(DockParameters.DockToDockDistance / 100)));
		FText RollText = FText::Format(LOCTEXT("DockingRollFormat", "Roll error : {0}\u00B0"),
			FText::AsNumber(FMath::RoundToInt(RollError)));
		FText AngularText = FText::Format(LOCTEXT("DockingAlignmentFormat", "Angular error : {0}\u00B0"),
			FText::AsNumber(FMath::RoundToInt(AngularError)));
		FText LinearText = FText::Format(LOCTEXT("DockingOffsetFormat", "Lateral error : {0}m"),
			FText::AsNumber(FMath::RoundToInt(LinearError)));

		// Draw panel
		FlareDrawText(DockingText, CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;
		FlareDrawText(DockInfo, CurrentPos, DockingInProgress ? Theme.FriendlyColor : Theme.EnemyColor, false, false);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDDistanceIcon, DistanceText,
			DockParameters.DockToDockDistance < DistanceTarget ? Theme.FriendlyColor : Theme.EnemyColor,
			DistanceRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDRCSIcon, LinearText,
			LinearError < LinearTarget ? Theme.FriendlyColor : Theme.EnemyColor,
			LinearRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDRCSIcon, RollText,
			RollError < AngularTarget ? Theme.FriendlyColor : Theme.EnemyColor,
			RollRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
		DrawProgressBarIconText(CurrentPos, HUDRCSIcon, AngularText,
			AngularError < AngularTarget ? Theme.FriendlyColor : Theme.EnemyColor,
			AngularRatio, LargeProgressBarSize);
		CurrentPos += InstrumentLine;
	}

	// Military version
	else if (PlayerShip->GetParent()->IsMilitary())
	{
		// Data
		int IndexSize = 35;
		FLinearColor HealthColor = Theme.FriendlyColor;
		int32 CurrentWeapongroupIndex = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex();
		FFlareWeaponGroup* CurrentWeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();

		// Disarmed text
		FText DisarmText;
		if (PlayerShip->GetDescription()->Size == EFlarePartSize::S)
		{
			DisarmText = LOCTEXT("WeaponsDisabledLight", "Standing down");
		}
		else
		{
			DisarmText = LOCTEXT("WeaponsDisabledHeavy", "Navigation mode");
		}

		// Title
		FText TitleText = (CurrentWeaponGroup) ? CurrentWeaponGroup->Description->Name : DisarmText;
		FlareDrawText(TitleText, CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;

		// Info
		if (CurrentWeaponGroup)
		{
			// Get ammo count
			int32 RemainingAmmo = 0;
			int32 TotalAmmo = 0;
			for (int32 i = 0; i < CurrentWeaponGroup->Weapons.Num(); i++)
			{
				TotalAmmo += CurrentWeaponGroup->Weapons[i]->GetMaxAmmo();
				if (CurrentWeaponGroup->Weapons[i]->GetUsableRatio() <= 0.0f)
				{
					continue;
				}
				RemainingAmmo += CurrentWeaponGroup->Weapons[i]->GetCurrentAmmo();
			}

			// Texts
			float WeaponHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(CurrentWeapongroupIndex);
			FText WeaponFiringText = PlayerShip->GetStateManager()->IsWantFire() ? LOCTEXT("Firing", "FIR") : FText();
			FText WeaponInfoText = FText::Format(LOCTEXT("WeaponInfoFormat", "{0} rounds left"), FText::AsNumber(RemainingAmmo));

			// Layout
			HealthColor = (float)RemainingAmmo / (float)TotalAmmo < 0.3f ? Theme.EnemyColor : Theme.FriendlyColor;
			FVector2D TextPosition = CurrentPos + FVector2D(IndexSize + SmallProgressBarSize + 10, 0);
			FVector2D BarPosition = CurrentPos + FVector2D(IndexSize, ProgressBarTopMargin);

			// Draw
			FlareDrawText(WeaponFiringText, CurrentPos, HealthColor, false);
			FlareDrawText(WeaponInfoText, TextPosition, HealthColor, false);
			FlareDrawProgressBar(BarPosition, SmallProgressBarSize, HealthColor, (float)RemainingAmmo / (float)TotalAmmo);
		}
		else
		{
			FText InfoText;
			if (PlayerShip->GetDescription()->Size == EFlarePartSize::S)
			{
				InfoText = LOCTEXT("StandingDownInfo", "Weapons disabled");
			}
			else
			{
				InfoText = LOCTEXT("NavigationInfo", "Turrets are automated");
			}
			FlareDrawText(InfoText, CurrentPos, HealthColor, false);
		}
		CurrentPos += InstrumentLine;

		// Weapon icon
		if (CurrentWeaponGroup)
		{
			int32 WeaponIconSize = 80;
			UTexture2D* WeaponIcon = Cast<UTexture2D>(CurrentWeaponGroup->Weapons[0]->GetDescription()->MeshPreviewBrush.GetResourceObject());
			DrawHUDIcon(RightInstrument + FVector2D(InstrumentSize.X - WeaponIconSize, 0), WeaponIconSize, WeaponIcon, Theme.FriendlyColor);
		}

		// Weapon list
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetWeaponsSystem()->GetWeaponGroupList();
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			float WeaponHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(i);
			FString WeaponIndexString = ((i == CurrentWeapongroupIndex) ? FString("> ") : FString("   ")) + FString::FromInt(i + 2);
			FText WeaponText = WeaponGroupList[i]->Description->Name;

			// Layout
			HealthColor = GetHealthColor(WeaponHealth);
			FVector2D TextPosition = CurrentPos + FVector2D(IndexSize + SmallProgressBarSize + 10, 0);
			FVector2D BarPosition = CurrentPos + FVector2D(IndexSize, ProgressBarTopMargin);

			// Draw
			FlareDrawText(FText::FromString(WeaponIndexString), CurrentPos, HealthColor, false);
			FlareDrawText(WeaponText, TextPosition, HealthColor, false);
			FlareDrawProgressBar(BarPosition, SmallProgressBarSize, HealthColor, WeaponHealth);
			CurrentPos += InstrumentLine;
		}

		// No weapon
		FString DisarmedName = FString("1. ") + DisarmText.ToString();
		DisarmedName = ((CurrentWeapongroupIndex == -1) ? FString("> ") : FString("    ")) + DisarmedName;
		FlareDrawText(FText::FromString(DisarmedName), CurrentPos, HealthColor, false);
		CurrentPos += InstrumentLine;
	}

	// Unarmed version
	else
	{
		UFlareCargoBay* CargoBay = PlayerShip->GetParent()->GetActiveCargoBay();
		FCHECK(CargoBay);

		// Title
		FText CargoText = FText::Format(LOCTEXT("CargoInfoFormat", "Cargo bay ({0} / {1})"),
			FText::AsNumber(CargoBay->GetUsedCargoSpace()), FText::AsNumber(CargoBay->GetCapacity()));
		FlareDrawText(CargoText, CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 3 * InstrumentLine;

		// Cargo bay slots
		uint32 MaxCargoBayCount = 8;
		TArray<FFlareCargo>& CargoBaySlots = CargoBay->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			// Create text
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];
			FText CargoSlotResourceText = Cargo.Resource ? Cargo.Resource->Acronym : LOCTEXT("CargoBaySlotEmpty", "Empty slot");
			FText CargoBaySlotText = FText::Format(LOCTEXT("CargoBaySlotFormat", "{0} ({1}/{2})"),
				CargoSlotResourceText, FText::AsNumber(Cargo.Quantity), FText::AsNumber(CargoBay->GetSlotCapacity()));

			// Go to the right column after n/2 slots
			if (CargoIndex == MaxCargoBayCount / 2)
			{
				CurrentPos += FVector2D(InstrumentSize.X / 2, 0) - (MaxCargoBayCount / 2) * InstrumentLine;
			}

			// Draw
			FlareDrawText(CargoBaySlotText, CurrentPos, Theme.FriendlyColor, false);
			CurrentPos += InstrumentLine;
		}
	}
}

void AFlareHUD::DrawCockpitTarget(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = TopInstrument + FVector2D(0, InstrumentSize.Y) - 5 * InstrumentLine;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();

	if (CurrentSector)
	{
		// Get sector name
		FText SectorText;
		if (PlayerShip->GetParent()->GetCurrentFleet()->IsTraveling())
		{
			SectorText = PlayerShip->GetParent()->GetCurrentFleet()->GetStatusInfo();
		}
		else
		{
			SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "Current sector : {0} ({1})"),
				CurrentSector->GetSectorName(),
				CurrentSector->GetSectorFriendlynessText(PC->GetCompany()));
		}
		FlareDrawText((ShowPerformance ? PerformanceText : SectorText), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Sector forces
		FlareDrawText(CurrentSector->GetSectorBalanceText(true), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Battle status
		FText BattleStatusText = CurrentSector->GetSectorBattleStateText(PC->GetCompany());
		FlareDrawText(BattleStatusText, CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += 2 * InstrumentLine;

		// Target info
		AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
		if (TargetShip && TargetShip->IsValidLowLevel())
		{
			FText ShipText = FText::Format(LOCTEXT("CurrentTargetFormat", "Targeting {0}"),
				UFlareGameTools::DisplaySpacecraftName(TargetShip->GetParent()));

			// Get target color
			FLinearColor TargetColor;
			if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->TargetSpacecrafts.Find(TargetShip->GetParent()) != INDEX_NONE)
			{
				TargetColor = Theme.ObjectiveColor;
			}
			else if (TargetShip->GetParent()->GetPlayerWarState() == EFlareHostility::Hostile)
			{
				TargetColor = Theme.EnemyColor;
			}
			else
			{
				TargetColor = Theme.FriendlyColor;
			}

			// Draw
			FlareDrawText(ShipText, CurrentPos, TargetColor, false);
			DrawHUDDesignatorStatus(CurrentPos + FVector2D(InstrumentSize.X, 0) * 0.8, IconSize, TargetShip);
		}
		CurrentPos += InstrumentLine;

		// Get player threats
		bool Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth;
		UFlareSimulatedSpacecraft* Threat;
		PC->GetPlayerShipThreatStatus(Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth, Threat);

		// Fired on ?
		if (FiredUpon)
		{
			if (Threat)
			{
				FText WarningText = FText::Format(LOCTEXT("ThreatFiredUponFormat", "UNDER FIRE FROM {0} ({1})"),
					UFlareGameTools::DisplaySpacecraftName(Threat),
					FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
				FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
			}
			else
			{
				FText WarningText = FText(LOCTEXT("ThreatFiredUponMissile", "INCOMING MISSILE"));
				FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
			}
		}

		// Collision ?
		else if (CollidingSoon)
		{
			FText WarningText = LOCTEXT("ThreatCollisionFormat", "IMMINENT COLLISION");
			FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
		}

		// Leaving sector ?
		else if (ExitingSoon)
		{
			FText WarningText = LOCTEXT("ThreatLeavingFormat", "LEAVING SECTOR");
			FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
		}

		// Targeted ?
		else if (Targeted)
		{
			FText WarningText = FText::Format(LOCTEXT("ThreatTargetFormat", "TARGETED BY {0} ({1})"),
				UFlareGameTools::DisplaySpacecraftName(Threat),
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
			FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
		}

		// Low health
		else if (LowHealth)
		{
			FText WarningText = LOCTEXT("ThreatHealthFormat", "LIFE SUPPORT COMPROMISED");
			FlareDrawText(WarningText, CurrentPos, Theme.EnemyColor, false);
		}

		// Okay
		else
		{
			FText WarningText = LOCTEXT("ThreatNone", "No active threat");
			FlareDrawText(WarningText, CurrentPos, Theme.FriendlyColor, false);
		}
	}
}

void AFlareHUD::DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position)
{
	UFlareSimulatedSpacecraftDamageSystem* DamageSystem = MenuManager->GetPC()->GetShipPawn()->GetParent()->GetDamageSystem();
	FText SystemText = UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName(Subsystem);
		
	// Drawing data
	UTexture2D* Icon = NULL;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;
	FLinearColor Color = NormalColor;
	bool HideProgressBar = false;

	// Icon
	switch (Subsystem)
	{
		case EFlareSubsystem::SYS_Propulsion:
			Icon = HUDPropulsionIcon;
			Color = DamageSystem->IsStranded() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsStranded() ? LOCTEXT("CockpitStranded", "STRANDED") : SystemText;
			HideProgressBar = DamageSystem->IsStranded();
			break;

		case EFlareSubsystem::SYS_RCS:
			Icon = HUDRCSIcon;
			Color = DamageSystem->IsUncontrollable() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsUncontrollable() ? LOCTEXT("CockpitUncontrollable", "UNCONTROLLABLE") : SystemText;
			HideProgressBar = DamageSystem->IsUncontrollable();
			break;

		case EFlareSubsystem::SYS_Weapon:
			Icon = HUDWeaponIcon;
			Color = DamageSystem->IsDisarmed() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsDisarmed() ? LOCTEXT("CockpitDisarmed", "DISARMED") : SystemText;
			HideProgressBar = DamageSystem->IsDisarmed();
			break;

		case EFlareSubsystem::SYS_Temperature:
			Icon = HUDTemperatureIcon;
			Color = (DamageSystem->GetSubsystemHealth(Subsystem) < 0.3f) ? DamageColor : NormalColor;
			break;

		case EFlareSubsystem::SYS_Power:
			Icon = HUDPowerIcon;
			Color = DamageSystem->HasPowerOutage() ? DamageColor : NormalColor;
			SystemText = DamageSystem->HasPowerOutage() ? LOCTEXT("CockpitOutage", "POWER OUTAGE") : SystemText;
			HideProgressBar = DamageSystem->HasPowerOutage();
			break;

		case EFlareSubsystem::SYS_LifeSupport:
			Icon = HUDHealthIcon;
			break;
	}

	// Draw
	DrawProgressBarIconText(Position, Icon, SystemText, Color, DamageSystem->GetSubsystemHealth(Subsystem), HideProgressBar ? 0 : SmallProgressBarSize);
	Position += InstrumentLine;
}

void AFlareHUD::DrawProgressBarIconText(FVector2D Position, UTexture2D* Icon, FText Text, FLinearColor Color, float Progress, int ProgressWidth)
{
	FVector2D BarPosition = Position + FVector2D(1.5 * CockpitIconSize, ProgressBarTopMargin);
	FVector2D TextPosition = Position + FVector2D(1.5 * CockpitIconSize + ProgressWidth + 10, 0);

	DrawHUDIcon(Position, CockpitIconSize, Icon, Color);
	if (ProgressWidth > 0)
	{
		FlareDrawProgressBar(BarPosition, ProgressWidth, Color, Progress);
	}
	FlareDrawText(Text, TextPosition, Color, false);
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void AFlareHUD::UpdateContextMenu(AFlareSpacecraft* PlayerShip)
{
	if (!PlayerShip || !PlayerShip->GetGame())
	{
		return;
	}

	ContextMenuSpacecraft = NULL;
	UFlareSector* ActiveSector = PlayerShip->GetGame()->GetActiveSector();
	
	// Look for a ship
	if (ActiveSector && IsInteractive)
	{
		for (int SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Spacecraft = ActiveSector->GetSpacecrafts()[SpacecraftIndex];
			if (Spacecraft->IsValidLowLevel() && Spacecraft != PlayerShip && !Spacecraft->IsComplexElement())
			{
				// Calculation data
				FVector2D ScreenPosition;
				AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
				FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
				FVector TargetLocation, TargetSize;
				Spacecraft->GetActorBounds(true, TargetLocation, TargetSize);

				if (PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition))
				{
					// Compute apparent size in screenspace
					float ShipSize = 2 * Spacecraft->GetMeshScale();
					float Distance = (TargetLocation - PlayerLocation).Size();
					float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
					float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * CurrentViewportSize.X;
					FVector2D ObjectSize = FMath::Min(0.66f * Size, 300.0f) * FVector2D(1, 1);

					// Check if the mouse is there
					int ToleranceRange = 3;
					FVector2D MousePos = PC->GetMousePosition();
					FVector2D ShipBoxMin = ScreenPosition - ObjectSize / 2;
					FVector2D ShipBoxMax = ScreenPosition + ObjectSize / 2;
					bool Hovering = (MousePos.X + ToleranceRange >= ShipBoxMin.X
						&& MousePos.Y + ToleranceRange >= ShipBoxMin.Y
						&& MousePos.X - ToleranceRange <= ShipBoxMax.X
						&& MousePos.Y - ToleranceRange <= ShipBoxMax.Y);

					// Draw the context menu
					if (Hovering)
					{
						// Update state
						ContextMenuPosition = ScreenPosition;
						ContextMenuSpacecraft = Spacecraft;
						if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
						{
							ContextMenu->SetSpacecraft(Spacecraft);
							return;
						}
					}
				}
			}
		}
	}

	// Hide the context menu if nothing was found
	if (ContextMenuSpacecraft == NULL || !IsInteractive)
	{
		ContextMenu->SetSpacecraft(NULL);
	}
}

FLinearColor AFlareHUD::GetTemperatureColor(float Current, float Max)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	float Distance = Current - Max;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FMath::Lerp(NormalColor, DamageColor, Ratio);
}

FLinearColor AFlareHUD::GetHealthColor(float Current)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	float Ratio = FMath::Clamp(1 - Current, 0.0f, 1.0f);
	return FMath::Lerp(NormalColor, DamageColor, Ratio);
}

	
/*----------------------------------------------------
	HUD drawing
----------------------------------------------------*/

bool AFlareHUD::ShouldDrawHUD() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

	if (!ActiveSector || !PlayerShip || !PlayerShip->GetParent()->GetDamageSystem()->IsAlive() || PC->IsInMenu())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AFlareHUD::DrawHUDInternal()
{
	// Initial data
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();
	bool IsExternalCamera = PlayerShip->GetStateManager()->IsExternalCamera();
	EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();

	// Draw combat mouse pointer
	if (HUDVisible && !PlayerShip->GetNavigationSystem()->IsAutoPilot())
	{
		// Compute clamped mouse position
		FVector2D MousePosDelta = CombatMouseRadius * PlayerShip->GetStateManager()->GetPlayerAim();
		FVector MousePosDelta3D = FVector(MousePosDelta.X, MousePosDelta.Y, 0);
		MousePosDelta3D = MousePosDelta3D.GetClampedToMaxSize(CombatMouseRadius);
		MousePosDelta = FVector2D(MousePosDelta3D.X, MousePosDelta3D.Y);

		// Keep an offset
		FVector2D MinimalOffset = MousePosDelta;
		MinimalOffset.Normalize();
		MousePosDelta += 5 * MinimalOffset;

		// Draw
		FLinearColor PointerColor = HudColorNeutral;
		PointerColor.A = FMath::Clamp((MousePosDelta.Size() / CombatMouseRadius) - 0.1f, 0.0f, PointerColor.A);
		DrawHUDIconRotated(CurrentViewportSize / 2 + MousePosDelta, IconSize, HUDCombatMouseIcon, PointerColor, MousePosDelta3D.Rotation().Yaw);
	}

	// Draw docking helper
	DrawDockingHelper();

	// Iterate on all 'other' ships to show designators, markings, etc
	for (int SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex ++)
	{
		AFlareSpacecraft* Spacecraft = ActiveSector->GetSpacecrafts()[SpacecraftIndex];
		if (Spacecraft != PlayerShip && !Spacecraft->IsComplexElement())
		{
			// Draw designators
			bool ShouldDrawSearchMarker = DrawHUDDesignator(Spacecraft);
						
			// Get more info
			bool Highlighted = (PlayerShip && Spacecraft == PlayerShip->GetCurrentTarget());
			bool IsObjective = (PC->GetCurrentObjective() && PC->GetCurrentObjective()->TargetSpacecrafts.Find(Spacecraft->GetParent()) != INDEX_NONE);

			// Draw search markers for alive ships or highlighted stations when not in external camera
			if (!IsExternalCamera && ShouldDrawSearchMarker
				&& PlayerShip->GetParent()->GetDamageSystem()->IsAlive()
				&& Spacecraft->GetParent()->GetDamageSystem()->IsAlive()
				&& (Highlighted || IsObjective || !Spacecraft->IsStation())
			)
			{
				DrawSearchArrow(Spacecraft->GetActorLocation(), GetHostilityColor(PC, Spacecraft), Highlighted, FocusDistance);
			}
		}
	}

	// Draw inertial vectors
	FVector ShipSmoothedVelocity = PlayerShip->GetSmoothedLinearVelocity() * 100;
	DrawSpeed(PC, PlayerShip, HUDReticleIcon, ShipSmoothedVelocity);
	DrawSpeed(PC, PlayerShip, HUDBackReticleIcon, -ShipSmoothedVelocity);

	// Draw objective
	if (PC->HasObjective() && PC->GetCurrentObjective()->TargetList.Num() > 0
		&& (PC->GetCurrentObjective()->TargetSectors.Num() == 0 || PC->GetCurrentObjective()->TargetSectors.Contains(PlayerShip->GetParent()->GetCurrentSector())))
	{
		FVector2D ScreenPosition;

		for (int TargetIndex = 0; TargetIndex < PC->GetCurrentObjective()->TargetList.Num(); TargetIndex++)
		{
			const FFlarePlayerObjectiveTarget* Target = &PC->GetCurrentObjective()->TargetList[TargetIndex];
			FVector ObjectiveLocation = Target->Location;

			bool ShouldDrawMarker = false;

			// Draw icon & distance
			if (ProjectWorldLocationToCockpit(ObjectiveLocation, ScreenPosition))
			{
				if (IsInScreen(ScreenPosition) && !Target->RequiresScan)
				{
					if (Target->Active)
					{
						DrawHUDIcon(ScreenPosition, IconSize, HUDObjectiveIcon, HudColorNeutral, true);

						// Draw distance
						float Distance = FMath::Max(0.f, ((ObjectiveLocation - PlayerShip->GetActorLocation()).Size() - Target->Radius) / 100);
						FText ObjectiveText = FormatDistance(Distance);
						FVector2D CenterScreenPosition = ScreenPosition - CurrentViewportSize / 2 + FVector2D(0, IconSize);
						FlareDrawText(ObjectiveText, CenterScreenPosition, (Target->Active ? HudColorObjective : HudColorNeutral));
					}
					else
					{
						DrawHUDIcon(ScreenPosition, IconSize, HUDNoseIcon, HudColorObjective, true);
					}
				}

				// Tell the HUD to draw the search marker only if we are outside this
				ShouldDrawMarker = !IsInScreen(ScreenPosition);
			}
			else
			{
				ShouldDrawMarker = true;
			}

			// Draw objective
			if (ShouldDrawMarker && !IsExternalCamera && Target->Active && !Target->RequiresScan)
			{
				DrawSearchArrow(ObjectiveLocation, HudColorObjective, true);
			}
		}
	}

	// Draw turrets in fire director
	if (WeaponType == EFlareWeaponGroupType::WG_TURRET)
	{
		bool HasOneTurretInPosition = false;
		int TurretReadyCount = 0;

		for (auto Weapon : PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup()->Weapons)
		{
			UFlareTurret* Turret = Cast<UFlareTurret>(Weapon);

			if(Turret->IsIgnoreManualAim())
			{
				continue;
			}

			FVector EndPoint = Turret->GetTurretBaseLocation() + Turret->GetFireAxis() * Turret->GetTurretPilot()->GetFireDistance();
			FVector2D ScreenPosition;

			if (ProjectWorldLocationToCockpit(EndPoint, ScreenPosition))
			{
				// Update turret data
				if (Turret->IsCloseToAim())
				{
					AActor* Unused;
					HasOneTurretInPosition = true;
					if (Turret->IsReadyToFire() && Turret->IsSafeToFire(0, Unused))
					{
						TurretReadyCount++;
					}
				}

				// Draw turret reticle
				FLinearColor TurretColor = HudColorNeutral;
				TurretColor.A = GetFadeAlpha(ScreenPosition, ViewportSize / 2);
				DrawHUDIcon(ScreenPosition, IconSize, HUDAimIcon, TurretColor, true);
			}
		}

		// Get color
		FLinearColor TurretColor = HudColorEnemy;
		if (TurretReadyCount)
		{
			TurretColor = HudColorFriendly;
		}
		else if (HasOneTurretInPosition)
		{
			TurretColor = HudColorNeutral;
		}

		// Get info text
		FText TurretText;
		if (TurretReadyCount > 1)
		{
			TurretText = FText::Format(LOCTEXT("ReadyTurretFormatPlural", "{0} turrets ready"), FText::AsNumber(TurretReadyCount));
		}
		else
		{
			TurretText = FText::Format(LOCTEXT("ReadyTurretFormatSingular", "{0} turret ready"), FText::AsNumber(TurretReadyCount));
		}

		// Draw main reticle
		UTexture2D* NoseIcon = NoseIcon = (HasPlayerHit) ? HUDAimHitIcon : HUDAimIcon;
		DrawHUDIcon(CurrentViewportSize / 2, IconSize, NoseIcon, TurretColor, true);
		FlareDrawText(TurretText, FVector2D(0, -70), HudColorNeutral);
	}

	// Draw bomb marker
	if (WeaponType == EFlareWeaponGroupType::WG_BOMB
		&& PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup()->Weapons[0]->GetDescription()->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration == 0)
	{
		float AmmoVelocity = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup()->Weapons[0]->GetAmmoVelocity();
		FRotator ShipAttitude = PlayerShip->GetActorRotation();
		FVector ShipVelocity = 100.f * PlayerShip->GetLinearVelocity();

		// Bomb velocity
		FVector BombVelocity = ShipAttitude.Vector();
		BombVelocity.Normalize();
		BombVelocity *= 100.f * AmmoVelocity;

		FVector BombDirection = (ShipVelocity + BombVelocity).GetUnsafeNormal();
		FVector BombTarget = PlayerShip->GetActorLocation() + BombDirection * 1000000;

		FVector2D ScreenPosition;
		if (ProjectWorldLocationToCockpit(BombTarget, ScreenPosition))
		{
			DrawHUDIcon(ScreenPosition, IconSize, HUDBombAimIcon, HudColorNeutral, true);
		}
	}

	// Draw bombs
	if(ActiveSector->GetBombs().Num())
	{
		bool PreciseBombAim = false;

		if (PlayerShip && (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN || PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET))
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				PreciseBombAim = true;
			}
		}

		for (AFlareBomb* Bomb : ActiveSector->GetBombs())
		{
			FVector2D ScreenPosition;

			FVector AimLocation =  Bomb->GetActorLocation();

			if(PreciseBombAim)
			{
				UPrimitiveComponent* BombRootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());

				if(BombRootComponent)
				{
					FVector AmmoIntersectionLocation;
					FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
					float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
					FVector CameraLocation = PlayerShip->GetCamera()->GetComponentLocation();

					float InterceptTime = SpacecraftHelper::GetIntersectionPosition(Bomb->GetActorLocation(), BombRootComponent->GetPhysicsLinearVelocity(), CameraLocation, PlayerShip->GetLinearVelocity() * 100, AmmoVelocity * 100, 0.0, &AmmoIntersectionLocation);

					if(InterceptTime > 0)
					{
						AimLocation = AmmoIntersectionLocation;
					}
				}
			}


			if (Bomb && ProjectWorldLocationToCockpit(AimLocation, ScreenPosition) && !Bomb->IsHarpooned())
			{
				if (IsInScreen(ScreenPosition))
				{
					DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, GetHostilityColor(PC, Bomb->GetFiringSpacecraft()) , true);
				}
			}
		}
	}

	if(ActiveSector->GetMeteorites().Num())
	{
		bool PreciseMeteoriteAim = false;

		float Range = 0;
		float AmmoLifeTime = 0;

		if (PlayerShip && (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN || PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET|| PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB))
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				PreciseMeteoriteAim = true;
				Range = WeaponGroup->Weapons[0]->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange;
				float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
				AmmoLifeTime = Range / AmmoVelocity;

			}
		}

		for (AFlareMeteorite* Meteorite : ActiveSector->GetMeteorites())
		{
			if(Meteorite->IsBroken())
			{
				continue;
			}
			FVector2D ScreenPosition;

			FVector AimLocation =  Meteorite->GetActorLocation();

			float InterceptTime = 0;

			if(PreciseMeteoriteAim)
			{
				UPrimitiveComponent* MeteoriteRootComponent = Cast<UPrimitiveComponent>(Meteorite->GetRootComponent());

				if(MeteoriteRootComponent)
				{
					FVector AmmoIntersectionLocation;
					FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
					float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
					FVector CameraLocation = PlayerShip->GetCamera()->GetComponentLocation();

					InterceptTime = SpacecraftHelper::GetIntersectionPosition(Meteorite->GetActorLocation(), MeteoriteRootComponent->GetPhysicsLinearVelocity(), CameraLocation, PlayerShip->GetLinearVelocity() * 100, AmmoVelocity * 100, 0.0, &AmmoIntersectionLocation);

					if(InterceptTime > 0 && InterceptTime < 40)
					{
						AimLocation = AmmoIntersectionLocation;
					}
				}
			}
			bool ShouldDrawMarker = true;


			if (Meteorite && ProjectWorldLocationToCockpit(AimLocation, ScreenPosition))
			{
				if (IsInScreen(ScreenPosition))
				{

					if(PreciseMeteoriteAim && (Range==0 || InterceptTime < AmmoLifeTime))
					{
						DrawHUDIcon(ScreenPosition, IconSize, HUDAimHelperIcon, HudColorObjective, true);
					}
					else
					{
						DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, HudColorObjective, true);
					}
					ShouldDrawMarker = false;
				}



			}


			// Draw objective
			if (ShouldDrawMarker)
			{
				DrawSearchArrow(AimLocation, HudColorObjective, false);
			}

		}
	}
}

FText AFlareHUD::FormatDistance(float Distance)
{
	if (Distance < 1000)
	{
		return FText::Format(LOCTEXT("FormatDistanceM", "{0}m"), FText::AsNumber(FMath::RoundToFloat(Distance)));
	}
	else
	{
		int Kilometers = ((int) Distance)/1000;
		if (Kilometers < 10)
		{
			int Hectometer = ((int)(Distance - Kilometers * 1000)) / 100;
			return FText::Format(LOCTEXT("FormatDistanceHM", "{0}.{1}km"), FText::AsNumber(Kilometers), FText::AsNumber(Hectometer));
		}
		else
		{
			return FText::Format(LOCTEXT("FormatDistanceKM", "{0}km"), FText::AsNumber(Kilometers));
		}
	}
}

void AFlareHUD::DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed)
{
	// Get HUD data
	FVector2D ScreenPosition;
	int32 SpeedMS = (Speed.Size() + 10.) / 100.0f;

	// Draw inertial vector
	FVector EndPoint = Object->GetActorLocation() + FocusDistance * Speed;
	if (ProjectWorldLocationToCockpit(EndPoint, ScreenPosition) && SpeedMS > 1)
	{
		// Cap screen pos
		float ScreenBorderDistanceX = 100;
		float ScreenBorderDistanceY = 20;
		ScreenPosition.X = FMath::Clamp(ScreenPosition.X, ScreenBorderDistanceX, CurrentViewportSize.X - ScreenBorderDistanceX);
		ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, ScreenBorderDistanceY, CurrentViewportSize.Y - ScreenBorderDistanceY);
		
		// Icon
		FLinearColor DrawColor = HudColorNeutral;
		DrawColor.A = GetFadeAlpha(ScreenPosition, ViewportSize / 2);
		FVector2D IndicatorPosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(0, 30);
		DrawHUDIcon(ScreenPosition, IconSize, Icon, DrawColor, true);
	}
}

void AFlareHUD::DrawSearchArrow(FVector TargetLocation, FLinearColor Color, bool Highlighted, float MaxDistance)
{
	// Data
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* Ship = PC->GetShipPawn();
	FVector Direction = TargetLocation - Ship->GetActorLocation();

	if (Direction.Size() < MaxDistance)
	{
		// Compute position
		FVector LocalDirection = Ship->GetCamera()->GetComponentToWorld().GetRotation().Inverse().RotateVector(Direction);
		FVector2D ScreenspacePosition = FVector2D(LocalDirection.Y, -LocalDirection.Z);
		ScreenspacePosition.Normalize();
		ScreenspacePosition *= 1.2 * CombatMouseRadius;

		// Make highlights more visible by centering them more
		if (!Highlighted)
		{
			ScreenspacePosition *= 1.2;
		}

		// Draw
		FVector Position3D = FVector(ScreenspacePosition.X, ScreenspacePosition.Y, 0);
		UTexture2D* Texture = Highlighted ? HUDHighlightSearchArrowTexture : HUDSearchArrowIcon;
		DrawHUDIconRotated(CurrentViewportSize / 2 + ScreenspacePosition, IconSize, Texture, Color, Position3D.Rotation().Yaw);
	}
}

bool AFlareHUD::DrawHUDDesignator(AFlareSpacecraft* Spacecraft)
{
	// Calculation data
	FVector2D ScreenPosition;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = Spacecraft->GetActorLocation();
	bool ScreenPositionValid = false;

	if (ProjectWorldLocationToCockpit(TargetLocation, ScreenPosition) && Spacecraft != ContextMenuSpacecraft)
	{
		ScreenPositionValid = true;

		// Compute apparent size in screenspace
		float ShipSize = 2 * Spacecraft->GetMeshScale();
		float Distance = (TargetLocation - PlayerLocation).Size();
		float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * CurrentViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.66f * Size, 300.0f) * FVector2D(1, 1);

		// Draw the HUD designator
		if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
		{
			float CornerSize = 8;
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
			FVector2D CenterPos = ScreenPosition - ObjectSize / 2;
			FLinearColor Color = GetHostilityColor(PC, Spacecraft);

			// Draw designator corners
			bool Highlighted = (PlayerShip && Spacecraft == PlayerShip->GetCurrentTarget());
			bool Dangerous = PilotHelper::IsShipDangerous(Spacecraft);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0,     Color, Dangerous, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90,   Color, Dangerous, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180,  Color, Dangerous, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270,  Color, Dangerous, Highlighted);

			// Draw the target's distance if selected
			if (Spacecraft == PlayerShip->GetCurrentTarget())
			{
				FText DistanceText = FormatDistance(Distance / 100);
				FVector2D DistanceTextPosition = ScreenPosition - (CurrentViewportSize / 2)
					+ FVector2D(-ObjectSize.X / 2, ObjectSize.Y / 2)
					+ FVector2D(2 * CornerSize, 3 * CornerSize);
				FlareDrawText(DistanceText, DistanceTextPosition, Color);
			}

			// Prepare icon layout
			FVector2D StatusPos = CenterPos;
			int32 NumberOfIcons = Spacecraft->GetParent()->IsMilitary() ? 3 : 2;
			StatusPos.X += 0.5 * (ObjectSize.X - NumberOfIcons * IconSize);
			StatusPos.Y -= (IconSize + 0.5 * CornerSize);

			// Draw the status for close targets or highlighted
			FVector2D TempPos = DrawHUDDesignatorHint(StatusPos, IconSize, Spacecraft, Color);
			if (!Spacecraft->GetParent()->IsStation() && (ObjectSize.X > 0.15 * IconSize || Highlighted))
			{
				DrawHUDDesignatorStatus(TempPos, IconSize, Spacecraft);
			}
		}
	}

	if (Spacecraft != ContextMenuSpacecraft && Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		float Distance = (TargetLocation - PlayerLocation).Size();

		// Combat helper
		if (Spacecraft == PlayerShip->GetCurrentTarget()
		 && PlayerShip && PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
		{
			FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
			if (WeaponGroup)
			{
				FVector2D HelperScreenPosition;
				FVector AmmoIntersectionLocation;
				float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
				float Range = WeaponGroup->Weapons[0]->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange;
				float AmmoLifeTime = Range / AmmoVelocity;
				float InterceptTime = Spacecraft->GetAimPosition(PlayerShip, AmmoVelocity, 0.0, &AmmoIntersectionLocation);

				if (InterceptTime > 0 && ProjectWorldLocationToCockpit(AmmoIntersectionLocation, HelperScreenPosition) && (Range == 0 || InterceptTime < AmmoLifeTime))
				{
					FLinearColor HUDAimHelperColor = GetHostilityColor(PC, Spacecraft);

					// Draw aiming helper for ships
					if (!Spacecraft->IsStation())
					{
						DrawHUDIcon(HelperScreenPosition, IconSize, HUDAimHelperIcon, HUDAimHelperColor, true);
						if (ScreenPositionValid)
						{
							FlareDrawLine(ScreenPosition, HelperScreenPosition, HUDAimHelperColor);
						}
					}

					// Snip helpers
					float ZoomAlpha = PC->GetShipPawn()->GetStateManager()->GetCombatZoomAlpha();
					if (ScreenPositionValid && !Spacecraft->IsStation() && Spacecraft->GetSize() == EFlarePartSize::L && ZoomAlpha > 0
						&& PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
					{
						FVector2D AimOffset = ScreenPosition - HelperScreenPosition;
						UTexture2D* NoseIcon = (HasPlayerHit) ? HUDAimHitIcon : HUDAimIcon;

						FLinearColor AimOffsetColor = HudColorFriendly;
						AimOffsetColor.A = ZoomAlpha * 0.5;

						DrawHUDIcon(AimOffset + CurrentViewportSize / 2, IconSize *0.75 , NoseIcon, AimOffsetColor, true);
						FlareDrawLine(CurrentViewportSize / 2, AimOffset + CurrentViewportSize / 2, AimOffsetColor);
					}
					
					// Bomber UI (time display)
					EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();
					if (WeaponType == EFlareWeaponGroupType::WG_BOMB)
					{
						FText TimeText = FText::FromString(FString::FromInt(InterceptTime) + FString(".") + FString::FromInt( (InterceptTime - (int) InterceptTime ) *10) + FString(" s"));
						FVector2D TimePosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(42,0);
						FlareDrawText(TimeText, TimePosition, HUDAimHelperColor);
					}
				}
			}
		}
	}

	// Tell the HUD to draw the search marker only if we are outside this
	if (ScreenPositionValid)
	{
		return !IsInScreen(ScreenPosition);
	}
	else
	{
		return true;
	}
}

void AFlareHUD::DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float DesignatorIconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Dangerous, bool Highlighted)
{
	float ScaledDesignatorIconSize = DesignatorIconSize * (Highlighted ? 2 : 1);
	ObjectSize = FMath::Max(ObjectSize, ScaledDesignatorIconSize * FVector2D::UnitVector);
	UTexture2D* Texture = HUDDesignatorCornerTexture;

	if (Highlighted)
	{
		Texture = HUDDesignatorCornerSelectedTexture;
	}
	else if (Dangerous)
	{
		Texture = HUDDesignatorMilCornerTexture;
	}

	FlareDrawTexture(Texture,
		Position.X + (ObjectSize.X + DesignatorIconSize) * MainOffset.X / 2,
		Position.Y + (ObjectSize.Y + DesignatorIconSize) * MainOffset.Y / 2,
		ScaledDesignatorIconSize, ScaledDesignatorIconSize, 0, 0, 1, 1,
		HudColor,
		BLEND_Translucent, 1.0f, false,
		Rotation);
}

FVector2D AFlareHUD::DrawHUDDesignatorHint(FVector2D Position, float DesignatorIconSize, AFlareSpacecraft* TargetSpacecraft, FLinearColor Color)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->TargetSpacecrafts.Find(TargetSpacecraft->GetParent()) != INDEX_NONE)
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDContractIcon, Color);
	}

	if (TargetSpacecraft->IsStation() && TargetSpacecraft->GetParent()->IsUnderConstruction(true))
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDConstructionIcon, Color);
	}
	
	if (TargetSpacecraft->GetParent()->IsShipyard())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDShipyardIcon, Color);
	}
	else if (TargetSpacecraft->GetParent()->HasCapability(EFlareSpacecraftCapability::Upgrade))
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDUpgradeIcon, Color);
	}

	if (TargetSpacecraft->IsStation() && TargetSpacecraft->GetParent()->HasCapability(EFlareSpacecraftCapability::Consumer))
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDConsumerIcon, Color);
	}

	return Position;
}

FVector2D AFlareHUD::DrawHUDDesignatorStatus(FVector2D Position, float DesignatorIconSize, AFlareSpacecraft* Ship)
{
	UFlareSimulatedSpacecraftDamageSystem* DamageSystem = Ship->GetParent()->GetDamageSystem();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.DamageColor;
	Color.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;

	if (DamageSystem->IsStranded())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDPropulsionIcon, Color);
	}

	if (DamageSystem->IsUncontrollable())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDRCSIcon, Color);
	}

	if (Ship->GetParent()->IsMilitary() && DamageSystem->IsDisarmed())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDWeaponIcon, Color);
	}

	if (Ship->GetParent()->IsHarpooned() && Ship->GetParent()->GetCompany()->GetPlayerHostility() != EFlareHostility::Owned)
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDHarpoonedIcon, Color);
	}

	return Position;
}

FVector2D AFlareHUD::DrawHUDDesignatorStatusIcon(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color)
{
	DrawHUDIcon(Position, DesignatorIconSize, Texture, Color);

	return Position + DesignatorIconSize * FVector2D(1, 0);
}

void AFlareHUD::DrawDockingHelper()
{
	int32 DockingIconSize = 96;
	int32 DockingRollIconSize = 32;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	// Docking info
	FText DockInfo;
	AFlareSpacecraft* DockSpacecraft = NULL;
	FFlareDockingParameters DockParameters;
	bool DockingInProgress = PC->GetShipPawn()->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

	// Valid docking target found
	if (DockSpacecraft)
	{
		struct FVector DockingPortLocation = DockParameters.StationDockLocation;		
		FVector2D CameraTargetScreenPosition;

		if (ProjectWorldLocationToCockpit(DockParameters.ShipCameraTargetLocation, CameraTargetScreenPosition))
		{
			// Get color
			FLinearColor HelperColor = HudColorNeutral;
			if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->TargetSpacecrafts.Find(DockSpacecraft->GetParent()) != INDEX_NONE)
			{
				HelperColor = HudColorObjective;
			}
			if (!DockingInProgress)
			{
				HelperColor = HudColorEnemy;
			}

			// Draw circle
			DrawHUDIcon(CameraTargetScreenPosition, DockingIconSize, HUDDockingCircleTexture, HelperColor, true);

			// Docking in progress
			if (DockingInProgress)
			{
				// Distance display
				float DockDistance = DockParameters.DockToDockDistance / 100;
				FText TimeText = FText::FromString(FString::FromInt(DockDistance) + FString(".") + FString::FromInt((DockDistance - (int)DockDistance) * 10) + FString(" m"));
				FVector2D TimePosition = CameraTargetScreenPosition - CurrentViewportSize / 2 + FVector2D(0,85);
				FlareDrawText(TimeText, TimePosition, HelperColor);
			
				// Top icon
				FVector2D DockTopAxis;
				FVector CameraTargetDockDirectionLocation = DockParameters.ShipCameraTargetLocation + DockParameters.StationDockTopAxis * 10;
				if (ProjectWorldLocationToCockpit(CameraTargetDockDirectionLocation, DockTopAxis))
				{
					FVector2D DockAxis = (CameraTargetScreenPosition - DockTopAxis).GetSafeNormal();
					float Rotation = -FMath::RadiansToDegrees(FMath::Atan2(DockAxis.X, DockAxis.Y)) + 90;
					FVector2D TopPosition = CameraTargetScreenPosition - DockAxis * 24;

					DrawHUDIconRotated(TopPosition, DockingRollIconSize, HUDSearchArrowIcon, HelperColor, Rotation);
				}
			
				// Axis icon
				FVector2D AlignementScreenPosition;
				FVector AlignementLocation = DockParameters.ShipCameraTargetLocation + DockParameters.StationDockAxis * 5.0f;
				if (ProjectWorldLocationToCockpit(AlignementLocation, AlignementScreenPosition))
				{
					FVector2D Alignement = CameraTargetScreenPosition + (AlignementScreenPosition - CameraTargetScreenPosition) * 100;

					DrawHUDIcon(Alignement, DockingIconSize, HUDDockingAxisTexture, HelperColor, true);
				}
			}

			// Docking denied
			else
			{
				DrawHUDIcon(CameraTargetScreenPosition, DockingIconSize / 2, HUDDockingForbiddenTexture, HelperColor, true);
			}
		}
	}
}

void AFlareHUD::DrawHUDIcon(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, bool Center)
{
	if (Center)
	{
		Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	}
	FlareDrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color);
}

void AFlareHUD::DrawHUDIconRotated(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, float Rotation)
{
	Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	FlareDrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color,
		EBlendMode::BLEND_Translucent, 1.0f, false, Rotation, FVector2D::UnitVector / 2);
}

void AFlareHUD::FlareDrawText(FText Text, FVector2D Position, FLinearColor Color, bool Center, bool Large)
{
	if (CurrentCanvas)
	{
		float X, Y;
		UFont* Font = NULL;
		
		// HUD fonts
		if (IsDrawingHUD)
		{
			if (Large)
			{
				Font = HUDFont;
			}
			else
			{
				Font = HUDFontSmall;
			}
		}

		// Instrument fonts
		else
		{
			if (Large)
			{
				Font = HUDFontLarge;
			}
			else
			{
				Font = HUDFont;
			}
		}

		// Optional centering
		if (Center)
		{
			float XL, YL;
			CurrentCanvas->TextSize(Font, Text.ToString(), XL, YL);
			X = CurrentCanvas->ClipX / 2.0f - XL / 2.0f + Position.X;
			Y = CurrentCanvas->ClipY / 2.0f - YL / 2.0f + Position.Y;
		}
		else
		{
			X = Position.X;
			Y = Position.Y;
		}

		// Text
		{
			float ShadowIntensity = 0.05f;

			FCanvasTextItem TextItem(FVector2D(X, Y), Text, Font, Color);
			TextItem.Scale = FVector2D(1, 1);
			TextItem.bOutlined = true;
			TextItem.OutlineColor = FLinearColor(ShadowIntensity, ShadowIntensity, ShadowIntensity, 1.0f);
			CurrentCanvas->DrawItem(TextItem);
		}
	}
}

void AFlareHUD::FlareDrawLine(FVector2D Start, FVector2D End, FLinearColor Color)
{
	if (CurrentCanvas)
	{
		FCanvasLineItem LineItem(Start, End);
		LineItem.SetColor(Color);
		LineItem.LineThickness = 1.0f;
		CurrentCanvas->DrawItem(LineItem);
	}
}

void AFlareHUD::FlareDrawProgressBar(FVector2D Position, int BarWidth, FLinearColor Color, float Ratio)
{
	float ClampedRatio = FMath::Clamp(Ratio, 0.0f, 1.0f);
	FVector2D ProgressBarPos = Position + FVector2D(ProgressBarInternalMargin + 1, ProgressBarInternalMargin + 1);
	FVector2D ProgressBarSize = FVector2D(ClampedRatio * (BarWidth - 2 - ProgressBarInternalMargin * 2), ProgressBarHeight - 2 * ProgressBarInternalMargin - 2);

	// Draw border
	FCanvasBoxItem BarItem(Position, FVector2D(BarWidth, ProgressBarHeight));
	BarItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Translucent);
	BarItem.SetColor(Color);
	CurrentCanvas->DrawItem(BarItem);
	
	// Draw progress bar
	FCanvasTileItem ProgressBarItem(ProgressBarPos, ProgressBarSize, Color);
	ProgressBarItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Translucent);
	CurrentCanvas->DrawItem(ProgressBarItem);
}

void AFlareHUD::FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor Color, EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
	if (CurrentCanvas && Texture)
	{
		// Setup texture (in dark)
		FCanvasTileItem TileItem(FVector2D(ScreenX, ScreenY),
			Texture->Resource,
			FVector2D(ScreenW, ScreenH) * Scale,
			FVector2D(TextureU, TextureV),
			FVector2D(TextureU + TextureUWidth, TextureV + TextureVHeight),
			Color);

		// More setup
		TileItem.Rotation = FRotator(0, Rotation, 0);
		TileItem.PivotPoint = RotPivot;
		if (bScalePosition)
		{
			TileItem.Position *= Scale;
		}
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(BlendMode);
		
		// Draw texture
		TileItem.SetColor(Color);
		CurrentCanvas->DrawItem(TileItem);
	}
}

float AFlareHUD::GetFadeAlpha(FVector2D A, FVector2D B)
{
	float FadePower = 2.0f;
	float FadeDistance = 100.0f;
	float CenterDistance = FMath::Clamp(FadeDistance * (A - B).Size() / ViewportSize.Y, 0.0f, 1.0f);
	return FMath::Pow(CenterDistance, FadePower);
}

bool AFlareHUD::IsInScreen(FVector2D ScreenPosition) const
{
	int32 ScreenBorderDistance = 150;

	if (ScreenPosition.X > CurrentViewportSize.X - ScreenBorderDistance || ScreenPosition.X < ScreenBorderDistance
	 || ScreenPosition.Y > CurrentViewportSize.Y - ScreenBorderDistance || ScreenPosition.Y < ScreenBorderDistance)
	{
		return false;
	}
	else
	{
		return true;
	}
}

FLinearColor AFlareHUD::GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraft* Target)
{
	if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->TargetSpacecrafts.Find(Target->GetParent()) != INDEX_NONE)
	{
		return HudColorObjective;
	}

	EFlareHostility::Type Hostility = Target->GetParent()->GetPlayerWarState();
	switch (Hostility)
	{
		case EFlareHostility::Hostile:
			return HudColorEnemy;

		case EFlareHostility::Owned:
			return HudColorFriendly;

		case EFlareHostility::Neutral:
		case EFlareHostility::Friendly:
		default:
			return HudColorNeutral;
	}
}

bool AFlareHUD::ProjectWorldLocationToCockpit(FVector World, FVector2D& Cockpit)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	FVector2D Screen;
	if (PC->ProjectWorldLocationToScreen(World, Screen))
	{
		Cockpit = Screen;
		return true;
	}
	else
	{
		return false;
	}

}

bool AFlareHUD::IsFlyingMilitaryShip() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC && PC->GetPlayerShip())
	{
		if (PC->GetPlayerShip()->IsMilitary())
		{
			return true;
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE

