
#include "../Flare.h"
#include "FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlarePilotHelper.h"
#include "../Spacecrafts/FlareTurret.h"
#include "../Spacecrafts/Subsystems/FlareSpacecraftDamageSystem.h"
#include "../Economy/FlareCargoBay.h"
#include "../Game/AI/FlareCompanyAI.h"


#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CombatMouseRadius(100)
	, HUDVisible(true)
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
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void AFlareHUD::BeginPlay()
{
	Super::BeginPlay();

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get HUD colors
	HudColorNeutral = Theme.NeutralColor;
	HudColorFriendly = Theme.HUDFriendlyColor;
	HudColorEnemy = Theme.EnemyColor;
	HudColorObjective = Theme.ObjectiveColor;
	HudColorNeutral.A = 1;
	HudColorFriendly.A = 1;
	HudColorEnemy.A = 1;
	HudColorObjective.A = 1;

	// HUD material
	HUDRenderTargetMaterial = UMaterialInstanceDynamic::Create(HUDRenderTargetMaterialTemplate, GetWorld());
	FCHECK(HUDRenderTargetMaterial);
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
	ContextMenu->SetVisibility((HUDVisible && !MenuManager->IsMenuOpen() && !MenuManager->IsSwitchingMenu()) ?
		EVisibility::Visible : EVisibility::Collapsed);
	MenuManager->GetNotifier()->SetVisibility((HUDVisible) ?
		EVisibility::SelfHitTestInvisible : EVisibility::Hidden);
}

void AFlareHUD::SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType)
{
	PlayerHitSpacecraft = HitSpacecraft;
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
		if (DrawRenderTarget)
		{
			DrawMaterialSimple(HUDRenderTargetMaterial, 0, 0, ViewportSize.X, ViewportSize.Y);
		}

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
					NoseIcon = (PlayerHitSpacecraft != NULL) ? HUDAimHitIcon : HUDAimIcon;
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
				FString VelocityText = FString::FromInt(PlayerShip->IsMovingForward() ? SpeedMS : -SpeedMS) + FString(" m/s");
				FlareDrawText(VelocityText, FVector2D(0, 70), HUDNosePowerColor, true);
			}
		}
	}

	// Player hit management
	if (PlayerHitTime >= PlayerHitDisplayTime)
	{
		PlayerHitSpacecraft = NULL;
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
	CurrentPowerTime = FMath::Clamp(CurrentPowerTime, 0.0f, PowerTransitionTime);

	// Update power on the HUD material when in cockpit mode
	if (HUDRenderTargetMaterial)
	{
		float PowerAlpha = PC->UseCockpit ? CurrentPowerTime / PowerTransitionTime : 1.0f;
		HUDRenderTargetMaterial->SetScalarParameterValue("PowerAlpha", PowerAlpha);
	}

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

	// Update data
	ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	PlayerHitTime += DeltaSeconds;

	// HUD texture target
	if (ViewportSize != PreviousViewportSize)
	{
		HUDRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);		
		if (HUDRenderTarget)
		{
			HUDRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlareHUD::DrawHUDTexture);
			HUDRenderTarget->ClearColor = FLinearColor::Black;
			HUDRenderTarget->UpdateResource();
			HUDRenderTargetMaterial->SetTextureParameterValue("Texture", HUDRenderTarget);
		}

		PreviousViewportSize = ViewportSize;
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
				FlareDrawText(PowerOut.ToString(), CurrentPos, Theme.EnemyColor, false, true);
				CurrentPos += 2 * InstrumentLine;
				FlareDrawText(PowerOutInfo.ToString(), CurrentPos, Theme.EnemyColor, false);
			}
		}
	}
}

void AFlareHUD::DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip)
{
	// Data
	float CockpitIconSize = 20;
	FVector2D CurrentPos = LeftInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Temperature = PlayerShip->GetParent()->GetDamageSystem()->GetTemperature();
	int32 MaxTemperature = PlayerShip->GetParent()->GetDamageSystem()->GetOverheatTemperature();

	// Ship name
	FString FlyingText = PlayerShip->GetParent()->GetImmatriculation().ToString();
	FlareDrawText(FlyingText, CurrentPos, Theme.FriendlyColor, false, true);
	CurrentPos += 2 * InstrumentLine;

	// Ship status
	FlareDrawText(PlayerShip->GetShipStatus().ToString(), CurrentPos, Theme.FriendlyColor, false);
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
	FlareDrawText(TemperatureText.ToString(), CurrentPos + FVector2D(1.5 * CockpitIconSize, 0), TemperatureColor, false);
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

	// Military version
	if (PlayerShip->GetParent()->IsMilitary())
	{
		FText TitleText;
		FText InfoText;
		FLinearColor HealthColor = Theme.FriendlyColor;
		int32 CurrentWeapongroupIndex = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex();
		FFlareWeaponGroup* CurrentWeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();

		FText DisarmText;
		if (PlayerShip->GetDescription()->Size == EFlarePartSize::S)
		{
			DisarmText = LOCTEXT("WeaponsDisabledLight", "Standing down");
		}
		else
		{
			DisarmText = LOCTEXT("WeaponsDisabledHeavy", "Navigation mode");
		}		

		if (CurrentWeaponGroup)
		{
			float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(CurrentWeapongroupIndex);
			HealthColor = GetHealthColor(ComponentHealth);

			// Get ammo count
			int32 RemainingAmmo = 0;
			for (int32 i = 0; i < CurrentWeaponGroup->Weapons.Num(); i++)
			{
				if (CurrentWeaponGroup->Weapons[i]->GetUsableRatio() <= 0.0f)
				{
					continue;
				}
				RemainingAmmo += CurrentWeaponGroup->Weapons[i]->GetCurrentAmmo();
			}

			// Final strings
			TitleText = CurrentWeaponGroup->Description->Name;
			InfoText = FText::Format(LOCTEXT("WeaponInfoFormat", "{0}: {1}%"),
				FText::Format(LOCTEXT("Rounds", "{0} rounds"), FText::AsNumber(RemainingAmmo)),
				FText::AsNumber((int32)(100 * ComponentHealth)));
		}
		else
		{
			TitleText = DisarmText;
		}

		// Draw text
		FlareDrawText(TitleText.ToString(), CurrentPos, Theme.FriendlyColor, false, true);
		CurrentPos += 2 * InstrumentLine;
		FlareDrawText(InfoText.ToString(), CurrentPos, HealthColor, false);
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
			FText WeaponText = FText::Format(LOCTEXT("WeaponListInfoFormat", "{0}. {1}: {2}%"),
				FText::AsNumber(i + 2), 
				WeaponGroupList[i]->Description->Name,
				FText::AsNumber((int32)(100 * WeaponHealth)));
			FString WeaponString = ((i == CurrentWeapongroupIndex) ? FString("> ") : FString("   ")) + WeaponText.ToString();

			HealthColor = GetHealthColor(WeaponHealth);
			FlareDrawText(WeaponString, CurrentPos, HealthColor, false);
			CurrentPos += InstrumentLine;
		}

		// No weapon
		FString DisarmedName = FString("1. ") + DisarmText.ToString();
		DisarmedName = ((CurrentWeapongroupIndex == -1) ? FString("> ") : FString("    ")) + DisarmedName;
		FlareDrawText(DisarmedName, CurrentPos, HealthColor, false);
		CurrentPos += InstrumentLine;
	}

	// Unarmed version
	else
	{
		UFlareCargoBay* CargoBay = PlayerShip->GetParent()->GetCargoBay();
		FCHECK(CargoBay);

		// Title
		FText CargoText = FText::Format(LOCTEXT("CargoInfoFormat", "Cargo bay ({0} / {1})"),
			FText::AsNumber(CargoBay->GetUsedCargoSpace()), FText::AsNumber(CargoBay->GetCapacity()));
		FlareDrawText(CargoText.ToString(), CurrentPos, Theme.FriendlyColor, false, true);
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
			FlareDrawText(CargoBaySlotText.ToString(), CurrentPos, Theme.FriendlyColor, false);
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
		FlareDrawText((ShowPerformance ? PerformanceText : SectorText).ToString(), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Sector forces
		FlareDrawText(CurrentSector->GetSectorBalanceText(true).ToString(), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += InstrumentLine;

		// Battle status
		FText BattleStatusText = CurrentSector->GetSectorBattleStateText(PC->GetCompany());
		FlareDrawText(BattleStatusText.ToString(), CurrentPos, Theme.FriendlyColor, false);
		CurrentPos += 2 * InstrumentLine;

		// Target info
		AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
		if (TargetShip && TargetShip->IsValidLowLevel())
		{
			FText ShipText = FText::Format(LOCTEXT("CurrentTargetFormat", "Targeting {0} ({1})"),
				FText::FromString(TargetShip->GetParent()->GetImmatriculation().ToString()),
				FText::FromString(TargetShip->GetCompany()->GetShortName().ToString()));
			FlareDrawText(ShipText.ToString(), CurrentPos, Theme.FriendlyColor, false);

			CurrentPos += FVector2D(InstrumentSize.X, 0) * 0.8;
			DrawHUDDesignatorStatus(CurrentPos, IconSize, TargetShip);
			CurrentPos -= FVector2D(InstrumentSize.X, 0) * 0.8;
		}
		CurrentPos += InstrumentLine;

		// Get player threats
		bool Targeted, FiredUpon;
		UFlareSimulatedSpacecraft* Threat;
		PC->GetPlayerShipThreatStatus(Targeted, FiredUpon, Threat);

		// Fired on ?
		if (FiredUpon)
		{
			if(Threat)
			{
				FText WarningText = FText::Format(LOCTEXT("ThreatFiredUponFormat", "UNDER FIRE FROM {0} ({1})"),
					FText::FromString(Threat->GetImmatriculation().ToString()),
					FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
				FlareDrawText(WarningText.ToString(), CurrentPos, Theme.EnemyColor, false);
			}
			else
			{
				FText WarningText = FText(LOCTEXT("ThreatFiredUponMissile", "INCOMING MISSILE"));
				FlareDrawText(WarningText.ToString(), CurrentPos, Theme.EnemyColor, false);
			}
		}

		// Targeted ?
		else if (Targeted)
		{
			FText WarningText = FText::Format(LOCTEXT("ThreatTargetFormat", "TARGETED BY {0} ({1})"),
				FText::FromString(Threat->GetImmatriculation().ToString()),
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
			FlareDrawText(WarningText.ToString(), CurrentPos, Theme.EnemyColor, false);
		}

		// Okay
		else
		{
			FText WarningText = LOCTEXT("ThreatNone", "No active threat");
			FlareDrawText(WarningText.ToString(), CurrentPos, Theme.FriendlyColor, false);
		}
	}
}

void AFlareHUD::DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position)
{
	UFlareSimulatedSpacecraftDamageSystem* DamageSystem = MenuManager->GetPC()->GetShipPawn()->GetParent()->GetDamageSystem();

	FText SystemText = FText::Format(LOCTEXT("SubsystemInfoFormat", "{0}: {1}%"),
		UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName(Subsystem),
		FText::AsNumber((int32)(100 * DamageSystem->GetSubsystemHealth(Subsystem))));
	
	// Drawing data
	UTexture2D* Icon = NULL;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;
	FLinearColor Color = NormalColor;

	// Icon
	switch (Subsystem)
	{
		case EFlareSubsystem::SYS_Propulsion:
			Icon = HUDPropulsionIcon;
			Color = DamageSystem->IsStranded() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsStranded() ? LOCTEXT("CockpitStranded", "STRANDED") : SystemText;
			break;

		case EFlareSubsystem::SYS_RCS:
			Icon = HUDRCSIcon;
			Color = DamageSystem->IsUncontrollable() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsUncontrollable() ? LOCTEXT("CockpitUncontrollable", "UNCONTROLLABLE") : SystemText;
			break;

		case EFlareSubsystem::SYS_Weapon:
			Icon = HUDWeaponIcon;
			Color = DamageSystem->IsDisarmed() ? DamageColor : NormalColor;
			SystemText = DamageSystem->IsDisarmed() ? LOCTEXT("CockpitDisarmed", "DISARMED") : SystemText;
			break;

		case EFlareSubsystem::SYS_Temperature:
			Icon = HUDTemperatureIcon;
			break;

		case EFlareSubsystem::SYS_Power:
			Icon = HUDPowerIcon;
			Color = DamageSystem->HasPowerOutage() ? DamageColor : NormalColor;
			SystemText = DamageSystem->HasPowerOutage() ? LOCTEXT("CockpitOutage", "POWER OUTAGE") : SystemText;
			break;

		case EFlareSubsystem::SYS_LifeSupport:
			Icon = HUDHealthIcon;
			break;
	}

	// Draw
	float CockpitIconSize = 20;
	DrawHUDIcon(Position, CockpitIconSize, Icon, Color);
	FlareDrawText(SystemText.ToString(), Position + FVector2D(1.5 * CockpitIconSize, 0), Color, false);
	Position += InstrumentLine;
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
			if (Spacecraft->IsValidLowLevel() && Spacecraft != PlayerShip)
			{
				// Calculation data
				FVector2D ScreenPosition;
				AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
				FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
				FVector TargetLocation = Spacecraft->GetActorLocation();

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

						ContextMenu->SetSpacecraft(Spacecraft);
						if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
						{
							ContextMenu->Show();
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
		ContextMenu->Hide();
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

	if (!ActiveSector || !PlayerShip || !PlayerShip->GetParent()->GetDamageSystem()->IsAlive() || PC->IsInMenu() || IsWheelMenuOpen())
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
	if (HUDVisible && !PlayerShip->GetNavigationSystem()->IsAutoPilot()
	 && (WeaponType == EFlareWeaponGroupType::WG_NONE || WeaponType == EFlareWeaponGroupType::WG_GUN))
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

	// Iterate on all 'other' ships to show designators, markings, etc
	for (int SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex ++)
	{
		AFlareSpacecraft* Spacecraft = ActiveSector->GetSpacecrafts()[SpacecraftIndex];
		if (Spacecraft != PlayerShip)
		{
			// Draw designators
			bool ShouldDrawSearchMarker = DrawHUDDesignator(Spacecraft);

			// Draw docking guides
			bool Highlighted = (PlayerShip && Spacecraft == PlayerShip->GetCurrentTarget());
			if (Highlighted)
			{
				DrawDockingHelper(Spacecraft);
			}

			// Draw search markers for alive ships or highlighted stations when not in external camera
			if (!IsExternalCamera && ShouldDrawSearchMarker
				&& PlayerShip->GetParent()->GetDamageSystem()->IsAlive()
				&& Spacecraft->GetParent()->GetDamageSystem()->IsAlive()
				&& (Highlighted || !Spacecraft->IsStation())
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
	if (PC->HasObjective() && PC->GetCurrentObjective()->TargetList.Num() > 0)
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
				if (IsInScreen(ScreenPosition))
				{
					if (Target->Active)
					{
						DrawHUDIcon(ScreenPosition, IconSize, HUDObjectiveIcon, HudColorNeutral, true);

						// Draw distance
						float Distance = FMath::Max(0.f, ((ObjectiveLocation - PlayerShip->GetActorLocation()).Size() - Target->Radius) / 100);
						FString ObjectiveText = FormatDistance(Distance);
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
			if (ShouldDrawMarker && !IsExternalCamera && Target->Active)
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
		UTexture2D* NoseIcon = NoseIcon = (PlayerHitSpacecraft != NULL) ? HUDAimHitIcon : HUDAimIcon;
		DrawHUDIcon(CurrentViewportSize / 2, IconSize, NoseIcon, TurretColor, true);
		FlareDrawText(TurretText.ToString(), FVector2D(0, -70), HudColorNeutral);
	}

	// Draw bomb marker
	if (WeaponType == EFlareWeaponGroupType::WG_BOMB)
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
	for (int32 BombIndex = 0; BombIndex < ActiveSector->GetBombs().Num(); BombIndex++)
	{
		FVector2D ScreenPosition;
		AFlareBomb* Bomb = ActiveSector->GetBombs()[BombIndex];
			
		if (Bomb && ProjectWorldLocationToCockpit(Bomb->GetActorLocation(), ScreenPosition) && !Bomb->IsHarpooned())
		{
			if (IsInScreen(ScreenPosition))
			{
				DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, GetHostilityColor(PC, Bomb->GetFiringSpacecraft()) , true);
			}
		}
	}
}

FString AFlareHUD::FormatDistance(float Distance)
{
	if (Distance < 1000)
	{
		return FString::FromInt(Distance) + FString("m");
	}
	else
	{
		int Kilometers = ((int) Distance)/1000;
		if (Kilometers < 10)
		{
			int Hectometer = ((int)(Distance - Kilometers * 1000)) / 100;
			return FString::FromInt(Kilometers) + "." + FString::FromInt(Hectometer) + FString("km");
		}
		else
		{
			return FString::FromInt(Kilometers) + FString("km");
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
				FString DistanceText = FormatDistance(Distance / 100);
				FVector2D DistanceTextPosition = ScreenPosition - (CurrentViewportSize / 2)
					+ FVector2D(-ObjectSize.X / 2, ObjectSize.Y / 2)
					+ FVector2D(2 * CornerSize, 3 * CornerSize);
				FlareDrawText(DistanceText, DistanceTextPosition, Color);
			}

			// Draw the status for close targets or highlighted
			if (!Spacecraft->GetParent()->IsStation() && (ObjectSize.X > 0.15 * IconSize || Highlighted))
			{
				int32 NumberOfIcons = Spacecraft->GetParent()->IsMilitary() ? 3 : 2;
				FVector2D StatusPos = CenterPos;
				StatusPos.X += 0.5 * (ObjectSize.X - NumberOfIcons * IconSize);
				StatusPos.Y -= (IconSize + 0.5 * CornerSize);
				DrawHUDDesignatorStatus(StatusPos, IconSize, Spacecraft);
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
				float Range = 100 * WeaponGroup->Weapons[0]->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange;
				float InterceptTime = Spacecraft->GetAimPosition(PlayerShip, AmmoVelocity, 0.0, &AmmoIntersectionLocation);


				if (InterceptTime > 0 && ProjectWorldLocationToCockpit(AmmoIntersectionLocation, HelperScreenPosition) && (Range == 0 || Distance < Range))
				{
					// Draw
					FLinearColor HUDAimHelperColor = GetHostilityColor(PC, Spacecraft);
					DrawHUDIcon(HelperScreenPosition, IconSize, HUDAimHelperIcon, HUDAimHelperColor, true);
					if(ScreenPositionValid)
					{
						FlareDrawLine(ScreenPosition, HelperScreenPosition, HUDAimHelperColor);
					}


					// Snip helpers
					float ZoomAlpha = PC->GetShipPawn()->GetStateManager()->GetCombatZoomAlpha();
					if(ScreenPositionValid && Spacecraft->GetSize() == EFlarePartSize::L && ZoomAlpha > 0)
					{
						FVector2D AimOffset = ScreenPosition - HelperScreenPosition;
						UTexture2D* NoseIcon = (PlayerHitSpacecraft != NULL) ? HUDAimHitIcon : HUDAimIcon;

						FLinearColor AimOffsetColor = HudColorFriendly;
						AimOffsetColor.A = ZoomAlpha * 0.5;

						DrawHUDIcon(AimOffset + CurrentViewportSize / 2, IconSize *0.75 , NoseIcon, AimOffsetColor, true);
						FlareDrawLine(CurrentViewportSize / 2, AimOffset + CurrentViewportSize / 2, AimOffsetColor);

						// Draw component outline
						/*TArray<UActorComponent*> Components = Spacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
						for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
						{
							UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

							if (!Component->GetDescription() || Component->IsBroken() )
							{
								continue;
							}

							if (Component->GetDescription()->Type != EFlarePartType::OrbitalEngine &&
									Component->GetDescription()->Type != EFlarePartType::RCS &&
									Component->GetDescription()->Type != EFlarePartType::Weapon)
							{
								continue;
							}

							FVector2D ComponentScreenPosition;

							FVector ComponentLocation;
							float ComponentRadius;
							Component->GetBoundingSphere(ComponentLocation, ComponentRadius);
							if (!ProjectWorldLocationToCockpit(Component->GetComponentLocation(), ComponentScreenPosition))
							{
								continue;
							}

							FLinearColor AimComponentColor = Color;
							AimComponentColor.A = ZoomAlpha;

							DrawHUDIcon(ComponentScreenPosition, IconSize , HUDNoseIcon, AimComponentColor, true);
						}*/
					}


					// Bomber UI
					EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();
					if (WeaponType == EFlareWeaponGroupType::WG_BOMB)
					{
						// Time display
						FString TimeText = FString::FromInt(InterceptTime) + FString(".") + FString::FromInt( (InterceptTime - (int) InterceptTime ) *10) + FString(" s");
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

void AFlareHUD::DrawHUDDesignatorStatus(FVector2D Position, float DesignatorIconSize, AFlareSpacecraft* Ship)
{
	UFlareSimulatedSpacecraftDamageSystem* DamageSystem = Ship->GetParent()->GetDamageSystem();

	if (DamageSystem->IsStranded())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDPropulsionIcon);
	}

	if (DamageSystem->IsUncontrollable())
	{
		Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDRCSIcon);
	}

	if (Ship->GetParent()->IsMilitary() && DamageSystem->IsDisarmed())
	{
		DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDWeaponIcon);
	}

	if (Ship->GetParent()->IsHarpooned() && Ship->GetParent()->GetCompany()->GetPlayerHostility() != EFlareHostility::Owned)
	{
		DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, HUDHarpoonedIcon);
	}
}

FVector2D AFlareHUD::DrawHUDDesignatorStatusIcon(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.DamageColor;
	Color.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;

	DrawHUDIcon(Position, DesignatorIconSize, Texture, Color);

	return Position + DesignatorIconSize * FVector2D(1, 0);
}

void AFlareHUD::DrawDockingHelper(AFlareSpacecraft* Spacecraft)
{
	int32 DockingIconSize = 64;
	int32 DockingRoolIconSize = 16;

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	// Reasons we might not draw
	if (!HUDVisible
	 || PC->GetShipPawn()->GetStateManager()->IsExternalCamera()
	 || !Spacecraft->IsStation()
	 || !Spacecraft->GetParent()->GetDamageSystem()->IsAlive()
	 ||  Spacecraft->GetCompany()->GetPlayerWarState() == EFlareHostility::Hostile
	 || !PC->GetShipPawn()->GetNavigationSystem()->IsManualPilot()
	 ||  PC->GetShipPawn()->GetWeaponsSystem()->GetActiveWeaponGroupIndex() >= 0)
	{
		return;
	}

	// Calculation data
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = Spacecraft->GetActorLocation();

	// Too far
	float Distance = (TargetLocation - PlayerLocation).Size();
	if (Distance > 40000)
	{
		return;
	}

	for (int32 DockingPortIndex = 0; DockingPortIndex < Spacecraft->GetDockingSystem()->GetDockCount(); DockingPortIndex++)
	{
		FFlareDockingInfo DockingPort = Spacecraft->GetDockingSystem()->GetDockInfo(DockingPortIndex);
		
		// Not compatible
		if (DockingPort.DockSize != PC->GetShipPawn()->GetSize())
		{
			continue;
		}

		FVector CameraLocation = PC->GetShipPawn()->Airframe->GetSocketLocation(FName("Camera"));
		FFlareDockingParameters DockingParameters = PC->GetShipPawn()->GetNavigationSystem()->GetDockingParameters(DockingPort, CameraLocation);
		struct FVector DockingPortLocation = DockingParameters.StationDockLocation;

		// Docked
		if (DockingParameters.DockingPhase == EFlareDockingPhase::Docked)
		{
			continue;
		}

		// Locked or rendez-vous
		else if (DockingParameters.DockingPhase == EFlareDockingPhase::Locked || DockingParameters.DockingPhase == EFlareDockingPhase::Distant)
		{
			continue;
		}

		FVector2D CameraTargetScreenPosition;
		if (ProjectWorldLocationToCockpit(DockingParameters.ShipCameraTargetLocation, CameraTargetScreenPosition))
		{
			FLinearColor HelperColor = HudColorEnemy;
			DrawHUDIcon(CameraTargetScreenPosition, DockingIconSize, HUDDockingCircleTexture, HelperColor, true);
			
			// Distance display
			float DockDistance = DockingParameters.DockToDockDistance / 100;
			FString TimeText = FString::FromInt(DockDistance) + FString(".") + FString::FromInt( (DockDistance - (int) DockDistance ) *10) + FString(" m");
			FVector2D TimePosition = CameraTargetScreenPosition - CurrentViewportSize / 2 + FVector2D(0,85);
			FlareDrawText(TimeText, TimePosition, HelperColor);
			
			FVector2D CameraTargetDockDirectionScreenPosition;
			FVector CameraTargetDockDirectionLocation = DockingParameters.ShipCameraTargetLocation + (DockingPortLocation - DockingParameters.ShipCameraTargetLocation) * 0.01;

			// Top icon
			if (ProjectWorldLocationToCockpit(CameraTargetDockDirectionLocation, CameraTargetDockDirectionScreenPosition))
			{
				FVector2D DockAxis = (CameraTargetScreenPosition - CameraTargetDockDirectionScreenPosition).GetSafeNormal();

				FVector2D TopPosition = CameraTargetScreenPosition + DockAxis * 26;
				float Rotation = -FMath::RadiansToDegrees(FMath::Atan2(DockAxis.X, DockAxis.Y)) - 90;

				DrawHUDIconRotated(TopPosition, DockingRoolIconSize, HUDCombatMouseIcon, HelperColor, Rotation);
			}

			FVector2D AlignementScreenPosition;
			FVector AlignementLocation = DockingParameters.ShipCameraTargetLocation + DockingParameters.StationDockAxis * 5.0f;

			// Axis icon
			if (ProjectWorldLocationToCockpit(AlignementLocation, AlignementScreenPosition))
			{
				FVector2D Alignement = CameraTargetScreenPosition + (AlignementScreenPosition - CameraTargetScreenPosition) * 100;

				DrawHUDIcon(Alignement, DockingIconSize, HUDDockingAxisTexture, HelperColor, true);
			}

			// Auto-dock when ready
			if (!PC->GetShipPawn()->GetNavigationSystem()->IsAutoPilot()
					&& (DockingParameters.DockingPhase == EFlareDockingPhase::Dockable ||
					DockingParameters.DockingPhase == EFlareDockingPhase::FinalApproach ||
					DockingParameters.DockingPhase == EFlareDockingPhase::Approach)
					&& DockingParameters.DockToDockDistance < (PC->GetShipPawn()->GetSize() == EFlarePartSize::S ? 500: 1500))
			{
				PC->GetShipPawn()->GetNavigationSystem()->DockAt(Spacecraft);
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

void AFlareHUD::FlareDrawText(FString Text, FVector2D Position, FLinearColor Color, bool Center, bool Large)
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
			CurrentCanvas->TextSize(Font, Text, XL, YL);
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

			FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
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
	float FadeDistance = 10.0f;
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

