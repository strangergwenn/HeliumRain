
#include "../Flare.h"
#include "FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../FlareLoadingScreen/FlareLoadingScreen.h"


#define LOCTEXT_NAMESPACE "FlareHUD"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CombatMouseRadius(100)
	, HUDVisible(true)
	, MenuIsOpen(false)
	, FadeDuration(0.15)
{
	// Load content (general icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDReticleIconObj      (TEXT("/Game/Gameplay/HUD/TX_Reticle.TX_Reticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBackReticleIconObj  (TEXT("/Game/Gameplay/HUD/TX_BackReticle.TX_BackReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimIconObj          (TEXT("/Game/Gameplay/HUD/TX_Aim.TX_Aim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimHelperIconObj    (TEXT("/Game/Gameplay/HUD/TX_AimHelper.TX_AimHelper"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDNoseIconObj         (TEXT("/Game/Gameplay/HUD/TX_Nose.TX_Nose"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatMouseIconObj  (TEXT("/Game/Gameplay/HUD/TX_CombatCursor.TX_CombatCursor"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorCornerObj (TEXT("/Game/Gameplay/HUD/TX_DesignatorCorner.TX_DesignatorCorner"));

	// Load content (status icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDTemperatureIconObj  (TEXT("/Game/Slate/Icons/TX_Icon_Temperature.TX_Icon_Temperature"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPowerIconObj        (TEXT("/Game/Slate/Icons/TX_Icon_Power.TX_Icon_Power"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPropulsionIconObj   (TEXT("/Game/Slate/Icons/TX_Icon_Propulsion.TX_Icon_Propulsion"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDRCSIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_RCS.TX_Icon_RCS"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDWeaponIconObj       (TEXT("/Game/Slate/Icons/TX_Icon_Shell.TX_Icon_Shell"));

	// Load content (font)
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontObj             (TEXT("/Game/Slate/Fonts/HudFont.HudFont"));

	// Set content (general icons)
	HUDReticleIcon = HUDReticleIconObj.Object;
	HUDBackReticleIcon = HUDBackReticleIconObj.Object;
	HUDAimIcon = HUDAimIconObj.Object;
	HUDAimHelperIcon = HUDAimHelperIconObj.Object;
	HUDNoseIcon = HUDNoseIconObj.Object;
	HUDCombatMouseIcon = HUDCombatMouseIconObj.Object;
	HUDDesignatorCornerTexture = HUDDesignatorCornerObj.Object;

	// Set content (status icons)
	HUDTemperatureIcon = HUDTemperatureIconObj.Object;
	HUDPowerIcon = HUDPowerIconObj.Object;
	HUDPropulsionIcon = HUDPropulsionIconObj.Object;
	HUDRCSIcon = HUDRCSIconObj.Object;
	HUDWeaponIcon = HUDWeaponIconObj.Object;

	// Set content (font)
	HUDFont = HUDFontObj.Object;

	// Settings
	FocusDistance = 1000000;
	IconSize = 24;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareHUD::BeginPlay()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	HudColorNeutral = Theme.NeutralColor;
	HudColorFriendly = Theme.FriendlyColor;
	HudColorEnemy = Theme.EnemyColor;
	HudColorNeutral.A = Theme.DefaultAlpha;
	HudColorFriendly.A = Theme.DefaultAlpha;
	HudColorEnemy.A = Theme.DefaultAlpha;

	FadeTimer = FadeDuration;
	Super::BeginPlay();
}

void AFlareHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

	// Mouse control
	if (PC && !MouseMenu->IsOpen())
	{
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
		MousePos = 2 * ((MousePos - ViewportCenter) / ViewportSize);
		PC->MousePositionInput(MousePos);
	}

	// Fade system
	if (Fader.IsValid() && FadeTimer >= 0)
	{
		FadeTimer += DeltaSeconds;
		FLinearColor Color = FLinearColor::Black;
		float Alpha = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);

		// Fade process
		if (Alpha < 1)
		{
			Color.A = FadeFromBlack ? 1 - Alpha : Alpha;
			Fader->SetVisibility(EVisibility::Visible);
			Fader->SetBorderBackgroundColor(Color);
		}

		// Callback
		else if (FadeTarget != EFlareMenu::MENU_None)
		{
			ProcessFadeTarget();
		}

		// Done
		else
		{
			Fader->SetVisibility(EVisibility::Hidden);
		}
	}
}

void AFlareHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!HUDVisible)
	{
		return;
	}

	// Initial data
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* Ship = PC->GetShipPawn();

	// Draw designators and context menu
	if (!MenuIsOpen && !IsWheelOpen())
	{
		// Draw ship designators and markers
		FoundTargetUnderMouse = false;
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraftPawn* ShipBase = Cast<AFlareSpacecraftPawn>(*ActorItr);
			if (PC && ShipBase && ShipBase != PC->GetShipPawn() && ShipBase != PC->GetMenuPawn())
			{
				// Draw designators
				bool ShouldDrawSearchMarker;
				if (PC->LineOfSightTo(ShipBase))
				{
					ShouldDrawSearchMarker = DrawHUDDesignator(ShipBase);
				}
				else
				{
					AFlareSpacecraft* LocalShip = Cast<AFlareSpacecraft>(ShipBase);
					ShouldDrawSearchMarker = !LocalShip || (LocalShip && LocalShip->GetDamageSystem()->IsAlive());
				}

				// Draw search markers
				if (Ship->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE && ShouldDrawSearchMarker)
				{
					FVector Direction = ShipBase->GetActorLocation() - Ship->GetActorLocation();
					if (Direction.Size() < FocusDistance)
					{
						// Compute position
						FVector LocalDirection = Ship->GetRootComponent()->GetComponentToWorld().GetRotation().Inverse().RotateVector(Direction);
						FVector2D ScreenspacePosition = FVector2D(LocalDirection.Y, -LocalDirection.Z);
						ScreenspacePosition.Normalize();
						ScreenspacePosition *= 1.2 * CombatMouseRadius;

						// Draw
						FLinearColor Color = GetHostilityColor(PC, ShipBase);
						FVector Position3D = FVector(ScreenspacePosition.X, ScreenspacePosition.Y, 0);
						DrawHUDIconRotated(ViewportSize / 2 + ScreenspacePosition, IconSize, HUDCombatMouseIcon, Color, Position3D.Rotation().Yaw);
					}
				}
			}
		}

		// Hide the context menu
		if (!FoundTargetUnderMouse || !IsInteractive)
		{
			ContextMenu->Hide();
		}
	}

	// Update HUD materials
	if (PC && Ship && !MenuIsOpen && !IsWheelOpen())
	{
		// Draw inertial vectors
		DrawSpeed(PC, Ship, HUDReticleIcon, Ship->GetLinearVelocity() * 100, LOCTEXT("Forward", "FWD"));
		DrawSpeed(PC, Ship, HUDBackReticleIcon, -Ship->GetLinearVelocity() * 100, LOCTEXT("Backward", "BWD"));
		
		// Draw nose
		if (!Ship->GetStateManager()->IsExternalCamera())
		{
			DrawHUDIcon(ViewportSize / 2, IconSize, Ship->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE ? HUDAimIcon : HUDNoseIcon, HudColorNeutral, true);
		}

		// Draw combat mouse pointer
		if (Ship->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
		{
			// Compute clamped mouse position
			FVector2D MousePosDelta = CombatMouseRadius * Ship->GetStateManager()->GetPlayerMouseOffset();
			FVector MousePosDelta3D = FVector(MousePosDelta.X, MousePosDelta.Y, 0);
			MousePosDelta3D = MousePosDelta3D.GetClampedToMaxSize(CombatMouseRadius);
			MousePosDelta = FVector2D(MousePosDelta3D.X, MousePosDelta3D.Y);

			// Keep an offset
			FVector2D MinimalOffset = MousePosDelta;
			MinimalOffset.Normalize();
			MousePosDelta += 12 * MinimalOffset;

			// Draw
			FLinearColor PointerColor = HudColorNeutral;
			PointerColor.A = FMath::Clamp((MousePosDelta.Size() / CombatMouseRadius) - 0.1f, 0.0f, PointerColor.A);
			DrawHUDIconRotated(ViewportSize / 2 + MousePosDelta, IconSize, HUDCombatMouseIcon, PointerColor, MousePosDelta3D.Rotation().Yaw);
		}
	}
}

/*----------------------------------------------------
	HUD library
----------------------------------------------------*/

void AFlareHUD::DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation)
{
	// Get HUD data
	FVector2D ScreenPosition;
	int32 SpeedMS = Speed.Size() / 100.0f;

	// Draw inertial vector
	FVector EndPoint = Object->GetActorLocation() + FocusDistance * Speed;
	if (PC->ProjectWorldLocationToScreen(EndPoint, ScreenPosition))
	{
		// Clamp the drawing
		int32 ScreenBorderDistance = 100;
		if (ScreenPosition.X > ViewportSize.X - ScreenBorderDistance || ScreenPosition.X < ScreenBorderDistance
		 || ScreenPosition.Y > ViewportSize.Y - ScreenBorderDistance || ScreenPosition.Y < ScreenBorderDistance)
		{
			return;
		}

		// Label
		FString IndicatorText = Designation.ToString();
		FVector2D IndicatorPosition = ScreenPosition - ViewportSize / 2 - FVector2D(42, 0);
		DrawText(IndicatorText, IndicatorPosition + FVector2D::UnitVector, HUDFont, FVector2D::UnitVector, FLinearColor::Black);
		DrawText(IndicatorText, IndicatorPosition, HUDFont, FVector2D::UnitVector, FLinearColor::White);

		// Icon
		DrawHUDIcon(ScreenPosition, IconSize, Icon, HudColorNeutral, true);

		// Speed 
		FString VelocityText = FString::FromInt(SpeedMS) + FString(" m/s");
		FVector2D VelocityPosition = ScreenPosition - ViewportSize / 2 + FVector2D(42, 0);
		DrawText(VelocityText, VelocityPosition + FVector2D::UnitVector, HUDFont, FVector2D::UnitVector, FLinearColor::Black);
		DrawText(VelocityText, VelocityPosition, HUDFont, FVector2D::UnitVector, FLinearColor::White);
	}
}

bool AFlareHUD::DrawHUDDesignator(AFlareSpacecraftPawn* ShipBase)
{
	// Calculation data
	FVector2D ScreenPosition;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = ShipBase->GetActorLocation();
	AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(ShipBase);

	if (PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition))
	{
		// Compute apparent size in screenspace
		float ShipSize = 2 * ShipBase->GetMeshScale();
		float Distance = (TargetLocation - PlayerLocation).Size();
		float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * ViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.8f * Size, 400.0f) * FVector2D(1, 1);

		// Check if the mouse is there
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ShipBoxMin = ScreenPosition - ObjectSize / 2;
		FVector2D ShipBoxMax = ScreenPosition + ObjectSize / 2;
		bool Hovering = (MousePos.X >= ShipBoxMin.X && MousePos.Y >= ShipBoxMin.Y && MousePos.X <= ShipBoxMax.X && MousePos.Y <= ShipBoxMax.Y);

		// Draw the context menu
		if (Hovering && !FoundTargetUnderMouse && IsInteractive)
		{
			// Update state
			FoundTargetUnderMouse = true;
			ContextMenuPosition = ScreenPosition;

			// If ship, set data
			if (Ship)
			{
				if(Ship->IsStation())
				{
					ContextMenu->SetStation(Ship);
				}
				else
				{
					ContextMenu->SetShip(Ship);
				}
				if (Ship->GetDamageSystem()->IsAlive())
				{
					ContextMenu->Show();
				}
			}
		}

		// Draw the HUD designator
		else if ((Ship && Ship->GetDamageSystem()->IsAlive()) || !Ship)
		{
			float CornerSize = 8;

			// Draw designator corners
			FLinearColor Color = GetHostilityColor(PC, ShipBase);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0,     Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90,   Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180,  Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270,  Color);

			// Draw the status
			if (Ship && !Ship->IsStation() && ObjectSize.X > IconSize)
			{
				int32 NumberOfIcons = Ship->IsMilitary() ? 5 : 4;
				FVector2D StatusPos = ScreenPosition - ObjectSize / 2;
				StatusPos.X += 0.5 * (ObjectSize.X - NumberOfIcons * IconSize);
				StatusPos.Y -= (IconSize + 0.5 * CornerSize);
				DrawHUDDesignatorStatus(StatusPos, IconSize, Ship);
			}

			// Combat helper
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
			if (Ship && Ship->GetPlayerHostility() == EFlareHostility::Hostile && PlayerShip && PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
			{
				TArray<UFlareWeapon*> Weapons = PlayerShip->GetWeaponsSystem()->GetWeaponList();
				if (Weapons.Num() > 0)
				{
					float AmmoVelocity = Weapons[0]->GetAmmoVelocity();
					if (PC->ProjectWorldLocationToScreen(Ship->GetAimPosition(PlayerShip, AmmoVelocity, 0.0), ScreenPosition))
					{
						FLinearColor HUDAimHelperColor = GetHostilityColor(PC, Ship);
						DrawHUDIcon(ScreenPosition, IconSize, HUDAimHelperIcon, HUDAimHelperColor, true);
					}
				}
			}

			// Tell the HUD to draw the search marker only if we are outside this
			return (FVector2D::Distance(ScreenPosition, ViewportSize / 2) >= (ViewportSize.Size() / 3));
		}
	}

	// Dead ship
	if (Ship && !Ship->GetDamageSystem()->IsAlive())
	{
		return false;
	}

	return true;
}

void AFlareHUD::DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float DesignatorIconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor)
{
	ObjectSize = FMath::Max(ObjectSize, DesignatorIconSize * FVector2D::UnitVector);

	DrawTexture(HUDDesignatorCornerTexture,
		Position.X + (ObjectSize.X + DesignatorIconSize) * MainOffset.X / 2,
		Position.Y + (ObjectSize.Y + DesignatorIconSize) * MainOffset.Y / 2,
		DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1,
		HudColor,
		BLEND_Translucent, 1.0f, false,
		Rotation);
}

void AFlareHUD::DrawHUDDesignatorStatus(FVector2D Position, float DesignatorIconSize, AFlareSpacecraft* Ship)
{
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Temperature), HUDTemperatureIcon);
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Power), HUDPowerIcon);
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion), HUDPropulsionIcon);
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS), HUDRCSIcon);

	if (Ship->IsMilitary())
	{
		DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon), HUDWeaponIcon);
	}
}

FVector2D AFlareHUD::DrawHUDDesignatorStatusIcon(FVector2D Position, float DesignatorIconSize, float Health, UTexture2D* Texture)
{
	FLinearColor Color = FLinearColor(FColor::MakeRedToGreenColorFromScalar(Health)).Desaturate(0.05);
	Color.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
	DrawHUDIcon(Position, DesignatorIconSize, Texture, Color);
	return Position + DesignatorIconSize * FVector2D(1, 0);
}

void AFlareHUD::DrawHUDIcon(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, bool Center)
{
	if (Center)
	{
		Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	}
	DrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color);
}

void AFlareHUD::DrawHUDIconRotated(FVector2D Position, float DesignatorIconSize, UTexture2D* Texture, FLinearColor Color, float Rotation)
{
	Position -= (DesignatorIconSize / 2) * FVector2D::UnitVector;
	DrawTexture(Texture, Position.X, Position.Y, DesignatorIconSize, DesignatorIconSize, 0, 0, 1, 1, Color,
		EBlendMode::BLEND_Translucent, 1.0f, false, Rotation, FVector2D::UnitVector / 2);
}

FLinearColor AFlareHUD::GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraftPawn* Target)
{
	EFlareHostility::Type Hostility = Target->GetPlayerHostility();
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


/*----------------------------------------------------
	Menu interaction
----------------------------------------------------*/

void AFlareHUD::ToggleHUD()
{
	HUDVisible = !HUDVisible;
	SetHUDVisibility(HUDVisible);
}

void AFlareHUD::SetWheelMenu(bool State)
{
	if (State)
	{
		MouseMenu->Open();
	}
	else
	{
		MouseMenu->Close();
	}
}

void AFlareHUD::SetWheelCursorMove(FVector2D Move)
{
	MouseMenu->SetWheelCursorMove(Move);
}

void AFlareHUD::SetHUDVisibility(bool Visibility)
{
	Visibility = Visibility && !MenuIsOpen;

	FLOGV("AFlareHUD::SetHUDVisibility : new state is %d", Visibility);
	HUDMenu->SetVisibility(Visibility ? EVisibility::Visible : EVisibility::Collapsed);
	Notifier->SetVisibility(Visibility ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
}

void AFlareHUD::SetupMenu(FFlarePlayerSave& PlayerData)
{
	if (GEngine->IsValidLowLevel())
	{
		// HUD widget
		SAssignNew(OverlayContainer, SOverlay)

			// Context menu
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ContextMenu, SFlareContextMenu).OwnerHUD(this)
			];

		// Create menus
		SAssignNew(HUDMenu, SFlareHUDMenu).OwnerHUD(this);
		SAssignNew(MouseMenu, SFlareMouseMenu).OwnerHUD(this);
		SAssignNew(Dashboard, SFlareDashboard).OwnerHUD(this);
		SAssignNew(CompanyMenu, SFlareCompanyMenu).OwnerHUD(this);
		SAssignNew(ShipMenu, SFlareShipMenu).OwnerHUD(this);
		SAssignNew(StationMenu, SFlareStationMenu).OwnerHUD(this);
		SAssignNew(SectorMenu, SFlareSectorMenu).OwnerHUD(this);
		SAssignNew(Notifier, SFlareNotifier).OwnerHUD(this).Visibility(EVisibility::SelfHitTestInvisible);

		// Fade-to-black system
		SAssignNew(Fader, SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"));

		// Register menus at their Z-Index
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(HUDMenu.ToSharedRef()),          0);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MouseMenu.ToSharedRef()),        5);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(OverlayContainer.ToSharedRef()), 10);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Dashboard.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(StationMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Notifier.ToSharedRef()),         90);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),            100);

		// Setup menus
		Dashboard->Setup();
		CompanyMenu->Setup(PlayerData);
		ShipMenu->Setup();
		StationMenu->Setup();
		SectorMenu->Setup();

		// Setup extra menus
		ContextMenu->Hide();
		Fader->SetVisibility(EVisibility::Hidden);
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
		if (PC)
		{
			HUDMenu->SetTargetShip(PC->GetShipPawn());
		}

		// Visibility
		SetHUDVisibility(HUDVisible);
	}
}

void AFlareHUD::Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	if (Notifier.IsValid())
	{
		Notifier->Notify(Text, Info, Type, TargetMenu, TargetInfo);
	}
}

void AFlareHUD::OnTargetShipChanged()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	if (PC)
	{
		HUDMenu->SetTargetShip(PC->GetShipPawn());
		SetHUDVisibility(HUDVisible);
	}
}

void AFlareHUD::OpenMenu(EFlareMenu::Type Target, void* Data)
{
	MenuIsOpen = true;
	FadeOut();
	FadeTarget = Target;
	FadeTargetData = Data;
}

void AFlareHUD::CloseMenu(bool HardClose)
{
	if (MenuIsOpen)
	{
		if (HardClose)
		{
			ExitMenu();
		}
		else
		{
			OpenMenu(EFlareMenu::MENU_Exit);
		}
	}
	MenuIsOpen = false;
}

void AFlareHUD::SetInteractive(bool Status)
{
	IsInteractive = Status;
}


/*----------------------------------------------------
	Menu commands
----------------------------------------------------*/

void AFlareHUD::ProcessFadeTarget()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	switch (FadeTarget)
	{
		case EFlareMenu::MENU_Dashboard:
			OpenDashboard();
			break;

		case EFlareMenu::MENU_Company:
			InspectCompany(static_cast<UFlareCompany*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Ship:
			InspectShip(static_cast<IFlareSpacecraftInterface*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_ShipConfig:
			InspectShip(static_cast<IFlareSpacecraftInterface*>(FadeTargetData), true);
			break;

		case EFlareMenu::MENU_Sector:
			OpenSector();
			break;

		case EFlareMenu::MENU_Station:
			InspectStation(static_cast<IFlareSpacecraftInterface*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_Quit:
			PC->ConsoleCommand("quit");
			break;

		case EFlareMenu::MENU_Exit:
			ExitMenu();
			break;

		case EFlareMenu::MENU_None:
		default:
			break;
	}

	FadeTarget = EFlareMenu::MENU_None;
	FadeTargetData = NULL;
}

void AFlareHUD::OpenDashboard()
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	Dashboard->Enter();
}

void AFlareHUD::InspectCompany(UFlareCompany* Target)
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetCompany();
	}
	CompanyMenu->Enter(Target);
}

void AFlareHUD::InspectShip(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetShipPawn();
	}
	ShipMenu->Enter(Target, IsEditable);
}

void AFlareHUD::InspectStation(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	AFlareSpacecraft* PlayerShip = Cast<AFlarePlayerController>(GetOwner())->GetShipPawn();

	if (Target == NULL && PlayerShip && PlayerShip->GetNavigationSystem()->IsDocked())
	{
		Target = PlayerShip->GetNavigationSystem()->GetDockStation();
	}
	StationMenu->Enter(Target);
}

void AFlareHUD::OpenSector()
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	SectorMenu->Enter();
}

void AFlareHUD::ExitMenu()
{
	ResetMenu();
	SetMenuPawn(false);
	SetHUDVisibility(HUDVisible);
	OverlayContainer->SetVisibility(EVisibility::Visible);
}


/*----------------------------------------------------
	Menu management
----------------------------------------------------*/

void AFlareHUD::ResetMenu()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	Dashboard->Exit();
	CompanyMenu->Exit();
	ShipMenu->Exit();
	StationMenu->Exit();
	SectorMenu->Exit();
	HUDMenu->SetVisibility(EVisibility::Hidden);

	if (PC)
	{
		PC->GetMenuPawn()->ResetContent();
	}

	FadeIn();
	HUDMenu->SetTargetShip(PC->GetShipPawn());
	SetHUDVisibility(HUDVisible);
}

void AFlareHUD::FadeIn()
{
	FadeFromBlack = true;
	FadeTimer = 0;
}

void AFlareHUD::FadeOut()
{
	FadeFromBlack = false;
	FadeTimer = 0;
}

void AFlareHUD::SetMenuPawn(bool Status)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		if (Status)
		{
			PC->OnEnterMenu();
		}
		else
		{
			PC->OnExitMenu();
		}
	}
}


/*----------------------------------------------------
	Slate
----------------------------------------------------*/

const FSlateBrush* AFlareHUD::GetMenuIcon(EFlareMenu::Type MenuType)
{
	switch (MenuType)
	{
	case EFlareMenu::MENU_Dashboard:      return FFlareStyleSet::GetIcon("Dashboard");
	case EFlareMenu::MENU_Company:        return FFlareStyleSet::GetIcon("Company");
	case EFlareMenu::MENU_Ship:           return FFlareStyleSet::GetIcon("Ship");
	case EFlareMenu::MENU_ShipConfig:     return FFlareStyleSet::GetIcon("ShipUpgrade");
	case EFlareMenu::MENU_Station:        return FFlareStyleSet::GetIcon("Station");
	case EFlareMenu::MENU_Undock:         return FFlareStyleSet::GetIcon("Undock");
	case EFlareMenu::MENU_Sector:         return FFlareStyleSet::GetIcon("Sector");
	case EFlareMenu::MENU_Orbit:          return FFlareStyleSet::GetIcon("Universe");
	case EFlareMenu::MENU_Encyclopedia:   return FFlareStyleSet::GetIcon("Encyclopedia");
	case EFlareMenu::MENU_Help:           return FFlareStyleSet::GetIcon("Help");
	case EFlareMenu::MENU_Settings:       return FFlareStyleSet::GetIcon("Settings");
	case EFlareMenu::MENU_Quit:           return FFlareStyleSet::GetIcon("Quit");
	case EFlareMenu::MENU_Exit:           return FFlareStyleSet::GetIcon("Close");

	case EFlareMenu::MENU_None:
	default:
		return NULL;
	}
}

void AFlareHUD::ShowLoadingScreen()
{
	IFlareLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IFlareLoadingScreenModule>("FlareLoadingScreen");
	if (LoadingScreenModule)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

#undef LOCTEXT_NAMESPACE

