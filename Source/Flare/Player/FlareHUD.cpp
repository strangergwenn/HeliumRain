
#include "../Flare.h"
#include "FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"


#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, CombatMouseRadius(100)
	, HUDVisible(true)
{
	// Load content (general icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDReticleIconObj         (TEXT("/Game/Gameplay/HUD/TX_Reticle.TX_Reticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBackReticleIconObj     (TEXT("/Game/Gameplay/HUD/TX_BackReticle.TX_BackReticle"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimIconObj             (TEXT("/Game/Gameplay/HUD/TX_Aim.TX_Aim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombAimIconObj         (TEXT("/Game/Gameplay/HUD/TX_BombAim.TX_BombAim"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDBombMarkerObj          (TEXT("/Game/Gameplay/HUD/TX_BombMarker.TX_BombMarker"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAimHelperIconObj       (TEXT("/Game/Gameplay/HUD/TX_AimHelper.TX_AimHelper"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDNoseIconObj            (TEXT("/Game/Gameplay/HUD/TX_Nose.TX_Nose"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDObjectiveIconObj       (TEXT("/Game/Gameplay/HUD/TX_Objective.TX_Objective"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCombatMouseIconObj     (TEXT("/Game/Gameplay/HUD/TX_CombatCursor.TX_CombatCursor"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorCornerObj    (TEXT("/Game/Gameplay/HUD/TX_DesignatorCorner.TX_DesignatorCorner"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectionObj (TEXT("/Game/Slate/Icons/TX_Icon_TargettingContextButton.TX_Icon_TargettingContextButton"));

	// Load content (status icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDTemperatureIconObj     (TEXT("/Game/Slate/Icons/TX_Icon_Temperature.TX_Icon_Temperature"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPowerIconObj           (TEXT("/Game/Slate/Icons/TX_Icon_Power.TX_Icon_Power"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPropulsionIconObj      (TEXT("/Game/Slate/Icons/TX_Icon_Propulsion.TX_Icon_Propulsion"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHealthIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_LifeSupport.TX_Icon_LifeSupport"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDWeaponIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_Shell.TX_Icon_Shell"));

	// Load content (font)
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontObj                (TEXT("/Game/Slate/Fonts/HudFont.HudFont"));

	// Set content (general icons)
	HUDReticleIcon = HUDReticleIconObj.Object;
	HUDBackReticleIcon = HUDBackReticleIconObj.Object;
	HUDAimIcon = HUDAimIconObj.Object;
	HUDBombAimIcon = HUDBombAimIconObj.Object;
	HUDBombMarker = HUDBombMarkerObj.Object;
	HUDAimHelperIcon = HUDAimHelperIconObj.Object;
	HUDNoseIcon = HUDNoseIconObj.Object;
	HUDObjectiveIcon = HUDObjectiveIconObj.Object;
	HUDCombatMouseIcon = HUDCombatMouseIconObj.Object;
	HUDDesignatorCornerTexture = HUDDesignatorCornerObj.Object;
	HUDDesignatorSelectionTexture = HUDDesignatorSelectionObj.Object;

	// Set content (status icons)
	HUDTemperatureIcon = HUDTemperatureIconObj.Object;
	HUDPowerIcon = HUDPowerIconObj.Object;
	HUDPropulsionIcon = HUDPropulsionIconObj.Object;
	HUDHealthIcon = HUDHealthIconObj.Object;
	HUDWeaponIcon = HUDWeaponIconObj.Object;

	// Set content (font)
	HUDFont = HUDFontObj.Object;

	// Settings
	FocusDistance = 1000000;
	IconSize = 24;
}

void AFlareHUD::BeginPlay()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get HUD colors
	HudColorNeutral = Theme.NeutralColor;
	HudColorFriendly = Theme.FriendlyColor;
	HudColorEnemy = Theme.EnemyColor;
	HudColorObjective = Theme.ObjectiveColor;
	HudColorNeutral.A = Theme.DefaultAlpha;
	HudColorFriendly.A = Theme.DefaultAlpha;
	HudColorEnemy.A = Theme.DefaultAlpha;
	HudColorObjective.A = Theme.DefaultAlpha;

	Super::BeginPlay();
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

		// Setup extra menus
		ContextMenu->Hide();
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
		UpdateHUDVisibility();
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

void AFlareHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

	// Mouse control
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && !MouseMenu->IsOpen())
	{
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
		MousePos = 2 * ((MousePos - ViewportCenter) / ViewportSize);
		PC->MousePositionInput(MousePos);
	}
}

void AFlareHUD::OnTargetShipChanged()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->GetShipPawn())
	{
		HUDMenu->SetTargetShip(PC->GetShipPawn());
		UpdateHUDVisibility();
	}
}

void AFlareHUD::UpdateHUDVisibility()
{
	bool NewVisibility = HUDVisible && !MenuManager->IsMenuOpen();

	FLOGV("AFlareHUD::UpdateHUDVisibility : new state is %d", NewVisibility);
	HUDMenu->SetVisibility(NewVisibility ? EVisibility::Visible : EVisibility::Collapsed);
	ContextMenu->SetVisibility(NewVisibility && !MenuManager->IsSwitchingMenu() ? EVisibility::Visible : EVisibility::Collapsed);
}


/*----------------------------------------------------
	HUD drawing
----------------------------------------------------*/

void AFlareHUD::DrawHUD()
{
	Super::DrawHUD();

	// Initial data and checks
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();

	if (!ActiveSector || !HUDVisible || !PlayerShip || !PlayerShip->GetDamageSystem()->IsAlive())
	{
		return;
	}

	// Draw designators and context menu
	if (!MenuManager->IsMenuOpen() && !MenuManager->IsSwitchingMenu() && !IsMouseMenuOpen())
	{
		// Draw ship designators and markers
		FoundTargetUnderMouse = false;

		for (int SpacecraftIndex = 0; SpacecraftIndex < ActiveSector->GetSpacecrafts().Num(); SpacecraftIndex ++)
		{
			AFlareSpacecraft* Spacecraft = ActiveSector->GetSpacecrafts()[SpacecraftIndex];
			if (Spacecraft != PlayerShip)
			{
				// Draw designators
				bool ShouldDrawSearchMarker;
				if (PC->LineOfSightTo(Spacecraft))
				{
					ShouldDrawSearchMarker = DrawHUDDesignator(Spacecraft);
				}
				else
				{
					ShouldDrawSearchMarker = Spacecraft->GetDamageSystem()->IsAlive();
				}

				// Draw search markers
				if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE
				 && !PlayerShip->GetStateManager()->IsExternalCamera()
				 && ShouldDrawSearchMarker)
				{
					DrawSearchArrow(Spacecraft->GetActorLocation(), GetHostilityColor(PC, Spacecraft), FocusDistance);
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
	if (PC && PlayerShip && !MenuManager->IsMenuOpen() && !MenuManager->IsSwitchingMenu() && !IsMouseMenuOpen())
	{
		// Draw inertial vectors
		DrawSpeed(PC, PlayerShip, HUDReticleIcon, PlayerShip->GetSmoothedLinearVelocity() * 100, LOCTEXT("Forward", "FWD"), false);
		DrawSpeed(PC, PlayerShip, HUDBackReticleIcon, -PlayerShip->GetSmoothedLinearVelocity() * 100, LOCTEXT("Backward", "BWD"), true);

		// Draw nose
		if (!PlayerShip->GetStateManager()->IsExternalCamera())
		{
			DrawHUDIcon(ViewportSize / 2, IconSize, PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN ? HUDAimIcon : HUDNoseIcon, HudColorNeutral, true);
		}

		// Draw objective
		if (PC->HasObjective() && PC->GetCurrentObjective()->Data.TargetList.Num() > 0)
		{
			FVector2D ScreenPosition;

			for (int TargetIndex = 0; TargetIndex < PC->GetCurrentObjective()->Data.TargetList.Num(); TargetIndex++)
			{

				const FFlarePlayerObjectiveTarget* Target = &PC->GetCurrentObjective()->Data.TargetList[TargetIndex];
				FVector ObjectiveLocation = Target->Location;
				FLinearColor InactiveColor = HudColorNeutral;
				InactiveColor.A = 0.25;
				FColor InactiveTextColor = FColor::White;
				InactiveTextColor.A = 64;

				bool ShouldDrawMarker = false;

				if (PC->ProjectWorldLocationToScreen(ObjectiveLocation, ScreenPosition))
				{
					if (IsInScreen(ScreenPosition))
					{

						// Draw icon
						DrawHUDIcon(ScreenPosition, IconSize, HUDObjectiveIcon, (Target->Active ? HudColorNeutral : InactiveColor), true);

						float Distance = (ObjectiveLocation - PlayerShip->GetActorLocation()).Size() / 100;
						FString ObjectiveText = FormatDistance(Distance);

						// Draw distance
						FVector2D CenterScreenPosition = ScreenPosition - ViewportSize / 2 + FVector2D(0, IconSize);
						DrawTextShaded(ObjectiveText, CenterScreenPosition, (Target->Active ? FLinearColor::White : InactiveTextColor));
					}

					// Tell the HUD to draw the search marker only if we are outside this
					ShouldDrawMarker = (FVector2D::Distance(ScreenPosition, ViewportSize / 2) >= (ViewportSize.GetMin() / 3));
				}
				else
				{
					ShouldDrawMarker = true;
				}

				if (ShouldDrawMarker && !PlayerShip->GetStateManager()->IsExternalCamera() && Target->Active)
				{
					DrawSearchArrow(ObjectiveLocation, HudColorObjective);
				}
			}
		}

		// Draw bomb marker
		if (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB)
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
			if (PC->ProjectWorldLocationToScreen(BombTarget, ScreenPosition))
			{
				// Icon
				DrawHUDIcon(ScreenPosition, IconSize, HUDBombAimIcon, HudColorNeutral, true);
			}
		}

		// Draw combat mouse pointer
		if (PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
		{
			// Compute clamped mouse position
			FVector2D MousePosDelta = CombatMouseRadius * PlayerShip->GetStateManager()->GetPlayerMouseOffset();
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

		// Draw bombs
		for (int32 BombIndex = 0; BombIndex < ActiveSector->GetBombs().Num(); BombIndex++)
		{
			FVector2D ScreenPosition;
			AFlareBomb* Bomb = ActiveSector->GetBombs()[BombIndex];
			
			if (Bomb && PC->ProjectWorldLocationToScreen(Bomb->GetActorLocation(), ScreenPosition))
			{
				if (IsInScreen(ScreenPosition))
				{
					DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, GetHostilityColor(PC, Bomb->GetFiringSpacecraft()) , true);
				}
			}
		}
	}
}

FString AFlareHUD::FormatDistance(float Distance)
{
	if (Distance < 1000)
	{
		return FString::FromInt(Distance) + FString(" m");
	}
	else
	{
		int Kilometers = ((int) Distance)/1000;
		if (Kilometers < 10)
		{
			int Hectometer = ((int)(Distance - Kilometers * 1000)) / 100;
			return FString::FromInt(Kilometers) +"." + FString::FromInt(Hectometer)+ FString(" km");
		}
		else
		{
			return FString::FromInt(Kilometers) + FString(" km");
		}
	}
}

void AFlareHUD::DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation, bool Invert)
{
	// Get HUD data
	FVector2D ScreenPosition;
	int32 SpeedMS = (Speed.Size() + 10.) / 100.0f;

	// Draw inertial vector
	FVector EndPoint = Object->GetActorLocation() + FocusDistance * Speed;
	if (PC->ProjectWorldLocationToScreen(EndPoint, ScreenPosition) && SpeedMS > 1)
	{
		if (IsInScreen(ScreenPosition))
		{
			// Label
			FString IndicatorText = Designation.ToString();
			FVector2D IndicatorPosition = ScreenPosition - ViewportSize / 2 - FVector2D(42, 0);
			DrawTextShaded(IndicatorText, IndicatorPosition);

			// Icon
			DrawHUDIcon(ScreenPosition, IconSize, Icon, HudColorNeutral, true);

			// Speed 
			FString VelocityText = FString::FromInt(Invert ? -SpeedMS : SpeedMS) + FString(" m/s");
			FVector2D VelocityPosition = ScreenPosition - ViewportSize / 2 + FVector2D(42, 0);
			DrawTextShaded(VelocityText, VelocityPosition);
		}
	}
}

void AFlareHUD::DrawSearchArrow(FVector TargetLocation, FLinearColor Color, float MaxDistance)
{
	// Data
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* Ship = PC->GetShipPawn();
	FVector Direction = TargetLocation - Ship->GetActorLocation();

	if (Direction.Size() < MaxDistance)
	{
		// Compute position
		FVector LocalDirection = Ship->GetRootComponent()->GetComponentToWorld().GetRotation().Inverse().RotateVector(Direction);
		FVector2D ScreenspacePosition = FVector2D(LocalDirection.Y, -LocalDirection.Z);
		ScreenspacePosition.Normalize();
		ScreenspacePosition *= 1.2 * CombatMouseRadius;

		// Draw
		FVector Position3D = FVector(ScreenspacePosition.X, ScreenspacePosition.Y, 0);
		DrawHUDIconRotated(ViewportSize / 2 + ScreenspacePosition, IconSize, HUDCombatMouseIcon, Color, Position3D.Rotation().Yaw);
	}
}

bool AFlareHUD::DrawHUDDesignator(AFlareSpacecraft* Spacecraft)
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
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * ViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.66f * Size, 500.0f) * FVector2D(1, 1);

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
		if (Hovering && !FoundTargetUnderMouse && IsInteractive)
		{
			// Update state
			FoundTargetUnderMouse = true;
			ContextMenuPosition = ScreenPosition;

			ContextMenu->SetSpacecraft(Spacecraft);
			if (Spacecraft->GetDamageSystem()->IsAlive())
			{
				ContextMenu->Show();
			}
		}

		// Draw the HUD designator
		else if (Spacecraft->GetDamageSystem()->IsAlive())
		{
			float CornerSize = 8;
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
			FVector2D CenterPos = ScreenPosition - ObjectSize / 2;
			FLinearColor Color = GetHostilityColor(PC, Spacecraft);

			// Draw designator corners
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0,     Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90,   Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180,  Color);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270,  Color);

			// Draw the target's distance
			FString DistanceText = FormatDistance(Distance / 100);
			FVector2D DistanceTextPosition = ScreenPosition - (ViewportSize / 2) + FVector2D(-ObjectSize.X / 2, ObjectSize.Y / 2) + 3 * CornerSize * FVector2D::UnitVector;
			DrawTextShaded(DistanceText, DistanceTextPosition, Color);

			// Draw the status
			if (!Spacecraft->IsStation() && ObjectSize.X > IconSize)
			{
				int32 NumberOfIcons = Spacecraft->IsMilitary() ? 3 : 2;
				FVector2D StatusPos = CenterPos;
				StatusPos.X += 0.5 * (ObjectSize.X - NumberOfIcons * IconSize);
				StatusPos.Y -= (IconSize + 0.5 * CornerSize);
				DrawHUDDesignatorStatus(StatusPos, IconSize, Spacecraft);
			}

			// Target selection info
			int32 SelectionSize = FMath::Min(48, (int32)ObjectSize.X);
			if (Spacecraft == PlayerShip->GetWeaponsSystem()->GetActiveWeaponTarget())
			{
				DrawHUDIcon(ScreenPosition, SelectionSize, HUDDesignatorSelectionTexture, Color, true);
			}
			
			// Combat helper
			if (Spacecraft->GetPlayerHostility() == EFlareHostility::Hostile && PlayerShip && PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
			{
				FFlareWeaponGroup* WeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
				if (WeaponGroup)
				{
					float AmmoVelocity = WeaponGroup->Weapons[0]->GetAmmoVelocity();
					FVector AmmoIntersectionLocation;
					float InterceptTime = Spacecraft->GetAimPosition(PlayerShip, AmmoVelocity, 0.0, &AmmoIntersectionLocation);

					if (InterceptTime > 0 && PC->ProjectWorldLocationToScreen(AmmoIntersectionLocation, ScreenPosition))
					{
						// Get some more data
						FLinearColor HUDAimHelperColor = GetHostilityColor(PC, Spacecraft);
						EFlareWeaponGroupType::Type WeaponType = PlayerShip->GetWeaponsSystem()->GetActiveWeaponType();
						bool FighterTargettingLarge = WeaponType == EFlareWeaponGroupType::WG_GUN && Spacecraft->GetSize() == EFlarePartSize::L;
						bool BomberTargettingSmall = WeaponType == EFlareWeaponGroupType::WG_BOMB && Spacecraft->GetSize() == EFlarePartSize::S;
						bool BomberTargettingLarge = WeaponType == EFlareWeaponGroupType::WG_BOMB && Spacecraft->GetSize() == EFlarePartSize::L;

						// Draw helper if it makes sense
						if (!FighterTargettingLarge && !BomberTargettingSmall)
						{
							DrawHUDIcon(ScreenPosition, IconSize, HUDAimHelperIcon, HUDAimHelperColor, true);
						}

						// Bomber UI
						if (BomberTargettingLarge)
						{
							// Time display
							FString TimeText = FString::FromInt(InterceptTime) + FString(".") + FString::FromInt( (InterceptTime - (int) InterceptTime ) *10) + FString(" s");
							FVector2D TimePosition = ScreenPosition - ViewportSize / 2 - FVector2D(42,0);
							DrawTextShaded(TimeText, TimePosition, HUDAimHelperColor);
						}
					}
				}
			}

			// Tell the HUD to draw the search marker only if we are outside this
			return (FVector2D::Distance(ScreenPosition, ViewportSize / 2) >= (ViewportSize.GetMin() / 3));
		}
	}

	// Dead ship
	if (!Spacecraft->GetDamageSystem()->IsAlive())
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
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion), HUDPropulsionIcon);
	Position = DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_LifeSupport), HUDHealthIcon);

	if (Ship->IsMilitary())
	{
		DrawHUDDesignatorStatusIcon(Position, DesignatorIconSize, Ship->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon), HUDWeaponIcon);
	}
}

FVector2D AFlareHUD::DrawHUDDesignatorStatusIcon(FVector2D Position, float DesignatorIconSize, float Health, UTexture2D* Texture)
{
	if (Health < 0.95f)
	{
		FLinearColor Color = FFlareStyleSet::GetHealthColor(Health);
		Color.A = FFlareStyleSet::GetDefaultTheme().DefaultAlpha;
		DrawHUDIcon(Position, DesignatorIconSize, Texture, Color);
	}

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

void AFlareHUD::DrawTextShaded(FString Text, FVector2D Position, FLinearColor Color)
{
	DrawText(Text, Position - FVector2D(1, 3), HUDFont, FVector2D(1, 1.4), FColor::Black);
	DrawText(Text, Position, HUDFont, FVector2D::UnitVector, Color.ToFColor(true));
}

bool AFlareHUD::IsInScreen(FVector2D ScreenPosition) const
{
	int32 ScreenBorderDistance = 100;

	if (ScreenPosition.X > ViewportSize.X - ScreenBorderDistance || ScreenPosition.X < ScreenBorderDistance
	 || ScreenPosition.Y > ViewportSize.Y - ScreenBorderDistance || ScreenPosition.Y < ScreenBorderDistance)
	{
		return false;
	}
	else
	{
		return true;
	}
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


#undef LOCTEXT_NAMESPACE

