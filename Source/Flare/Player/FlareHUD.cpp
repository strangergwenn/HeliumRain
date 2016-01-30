
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
	, IsDrawingCockpit(false)
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
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectedCornerObj(TEXT("/Game/Gameplay/HUD/TX_DesignatorCornerSelected.TX_DesignatorCornerSelected"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorSelectionObj (TEXT("/Game/Slate/Icons/TX_Icon_TargettingContextButton.TX_Icon_TargettingContextButton"));

	// Load content (status icons)
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDTemperatureIconObj     (TEXT("/Game/Slate/Icons/TX_Icon_Temperature.TX_Icon_Temperature"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPowerIconObj           (TEXT("/Game/Slate/Icons/TX_Icon_Power.TX_Icon_Power"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDPropulsionIconObj      (TEXT("/Game/Slate/Icons/TX_Icon_Propulsion.TX_Icon_Propulsion"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDRCSIconObj             (TEXT("/Game/Slate/Icons/TX_Icon_RCS.TX_Icon_RCS"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDHealthIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_LifeSupport.TX_Icon_LifeSupport"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDWeaponIconObj          (TEXT("/Game/Slate/Icons/TX_Icon_Shell.TX_Icon_Shell"));

	// Load content (font)
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontObj                (TEXT("/Game/Slate/Fonts/HudFont.HudFont"));
	static ConstructorHelpers::FObjectFinder<UFont>      HUDFontLargeObj           (TEXT("/Game/Slate/Fonts/HudFontLarge.HudFontLarge"));

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
	HUDDesignatorCornerSelectedTexture = HUDDesignatorSelectedCornerObj.Object;
	HUDDesignatorSelectionTexture = HUDDesignatorSelectionObj.Object;

	// Set content (status icons)
	HUDTemperatureIcon = HUDTemperatureIconObj.Object;
	HUDPowerIcon = HUDPowerIconObj.Object;
	HUDPropulsionIcon = HUDPropulsionIconObj.Object;
	HUDRCSIcon = HUDRCSIconObj.Object;
	HUDHealthIcon = HUDHealthIconObj.Object;
	HUDWeaponIcon = HUDWeaponIconObj.Object;

	// Set content (font)
	HUDFont = HUDFontObj.Object;
	HUDFontLarge = HUDFontLargeObj.Object;

	// Settings
	FocusDistance = 1000000;
	IconSize = 24;

	// Cockpit instruments
	TopInstrument =   FVector2D(20, 10);
	LeftInstrument =  FVector2D(20, 165);
	RightInstrument = FVector2D(20, 320);
	InstrumentSize =  FVector2D(380, 115);
	InstrumentLine =  FVector2D(0, 20);
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
		UpdateHUDVisibility();
		ContextMenu->Hide();
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

bool AFlareHUD::IsWheelMenuOpen() const
{
	return MouseMenu->IsOpen();
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
	AFlarePlayerController* PC = MenuManager->GetPC();

	FLOGV("AFlareHUD::UpdateHUDVisibility : new state is %d", NewVisibility);
	HUDMenu->SetVisibility((NewVisibility && !PC->UseCockpit) ? EVisibility::Visible : EVisibility::Collapsed);
	ContextMenu->SetVisibility(NewVisibility && !MenuManager->IsSwitchingMenu() ? EVisibility::Visible : EVisibility::Collapsed);
}

void AFlareHUD::DrawHUD()
{
	Super::DrawHUD();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && !PC->UseCockpit && HUDVisible)
	{
		CurrentViewportSize = ViewportSize;
		CurrentCanvas = Canvas;
		IsDrawingCockpit = false;
		DrawHUDInternal();
	}
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


/*----------------------------------------------------
	Cockpit HUD drawing
----------------------------------------------------*/

void AFlareHUD::DrawCockpitHUD(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->UseCockpit)
	{
		CurrentViewportSize = FVector2D(Width, Height);
		CurrentCanvas = TargetCanvas;
		IsDrawingCockpit = true;
		DrawHUDInternal();
	}
}

void AFlareHUD::DrawCockpitInstruments(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && PC->UseCockpit)
	{
		// Dynamic data
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		CurrentViewportSize = FVector2D(Width, Height);
		CurrentCanvas = TargetCanvas;

		// Draw instruments
		if (PlayerShip)
		{
			DrawCockpitSubsystems(PlayerShip);
			DrawCockpitEquipment(PlayerShip);
			DrawCockpitTarget(PlayerShip);
		}
	}
}

void AFlareHUD::DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip)
{
	// Data
	float CockpitIconSize = 20;
	FVector2D CurrentPos = RightInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 Temperature = PlayerShip->GetDamageSystem()->GetTemperature();
	FText TemperatureText = FText::Format(LOCTEXT("TemperatureFormat", "Hull Temperature: {0}K"), FText::AsNumber(Temperature));

	// Ship name
	FString FlyingText = PlayerShip->GetImmatriculation().ToString();
	FlareDrawText(FlyingText, CurrentPos, Theme.FriendlyColor, false, true);
	CurrentPos += 2 * InstrumentLine;

	// Ship status
	FlareDrawText(GetShipStatus(PlayerShip).ToString(), CurrentPos, Theme.FriendlyColor, false);
	CurrentPos += InstrumentLine;
	
	// Temperature text
	FLinearColor TemperatureColor = GetTemperatureColor(Temperature, PlayerShip->GetDamageSystem()->GetOverheatTemperature());
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
	if (PlayerShip->IsMilitary())
	{
		DrawCockpitSubsystemInfo(EFlareSubsystem::SYS_Weapon, CurrentPos);
	}

	// Ship icon
	int32 ShipIconSize = 80;
	UTexture2D* ShipIcon = Cast<UTexture2D>(PlayerShip->GetDescription()->MeshPreviewBrush.GetResourceObject());
	DrawHUDIcon(RightInstrument + FVector2D(InstrumentSize.X - ShipIconSize, 0), ShipIconSize, ShipIcon, Theme.FriendlyColor);
}

void AFlareHUD::DrawCockpitEquipment(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = LeftInstrument;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Military version
	if (PlayerShip->IsMilitary())
	{
		FText TitleText;
		FText InfoText;
		FLinearColor HealthColor = Theme.FriendlyColor;
		int32 CurrentWeapongroupIndex = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex();
		FFlareWeaponGroup* CurrentWeaponGroup = PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroup();
		FText DisarmText = LOCTEXT("WeaponsDisabled", "Standing down");;

		if (CurrentWeaponGroup)
		{
			float ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(CurrentWeapongroupIndex);
			HealthColor = GetHealthColor(ComponentHealth);

			// Get ammo count
			int32 RemainingAmmo = 0;
			for (int32 i = 0; i < CurrentWeaponGroup->Weapons.Num(); i++)
			{
				if (CurrentWeaponGroup->Weapons[i]->GetDamageRatio() <= 0.0f)
				{
					continue;
				}
				RemainingAmmo += CurrentWeaponGroup->Weapons[i]->GetCurrentAmmo();
			}

			// Final strings
			TitleText = CurrentWeaponGroup->Description->Name;
			InfoText = FText::Format(LOCTEXT("WeaponInfoFormat", "{0}x {1} - {2}%"),
				FText::AsNumber(CurrentWeaponGroup->Weapons.Num()),
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
		CurrentPos += 2 * InstrumentLine;

		// Weapon icon
		if (CurrentWeaponGroup)
		{
			int32 WeaponIconSize = 80;
			UTexture2D* WeaponIcon = Cast<UTexture2D>(CurrentWeaponGroup->Weapons[0]->GetDescription()->MeshPreviewBrush.GetResourceObject());
			DrawHUDIcon(LeftInstrument + FVector2D(InstrumentSize.X - WeaponIconSize, 0), WeaponIconSize, WeaponIcon, Theme.FriendlyColor);
		}

		// Weapon list
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetWeaponsSystem()->GetWeaponGroupList();
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			float WeaponHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(i);
			FText WeaponText = FText::Format(LOCTEXT("WeaponListInfoFormat", "{0}. {1} - {2}%"),
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

	}
}

void AFlareHUD::DrawCockpitTarget(AFlareSpacecraft* PlayerShip)
{
	FVector2D CurrentPos = TopInstrument + FVector2D(0, InstrumentSize.Y) - InstrumentLine;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Sector info
	UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();
	FText SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "Current sector : {0} ({1})"),
		CurrentSector->GetSectorName(),
		CurrentSector->GetSectorFriendlynessText(PlayerShip->GetCompany()));
	FlareDrawText(SectorText.ToString(), CurrentPos, Theme.FriendlyColor, false);
	CurrentPos += InstrumentLine;

	// Target info
	AFlareSpacecraft* TargetShip = PlayerShip->GetCurrentTarget();
	if (TargetShip)
	{
		FText ShipText = FText::Format(LOCTEXT("CurrentTargetFormat", "Current target : {0} ({1})"),
			FText::FromString(TargetShip->GetImmatriculation().ToString()),
			TargetShip->GetPlayerHostilityText());
		FlareDrawText(ShipText.ToString(), CurrentPos, Theme.FriendlyColor, false);
	}
}

void AFlareHUD::DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position)
{
	AFlareSpacecraft* PlayerShip = MenuManager->GetPC()->GetShipPawn();
	float ComponentHealth = PlayerShip->GetDamageSystem()->GetSubsystemHealth(Subsystem);

	FText SystemText = FText::Format(LOCTEXT("SubsystemInfoFormat", "{0}: {1}%"),
		IFlareSpacecraftDamageSystemInterface::GetSubsystemName(Subsystem),
		FText::AsNumber((int32)(100 * ComponentHealth)));

	// Drawing data
	UTexture2D* Icon = NULL;
	float CockpitIconSize = 20;
	FLinearColor HealthColor = GetHealthColor(ComponentHealth);
	
	// Icon
	switch (Subsystem)
	{
		case EFlareSubsystem::SYS_Temperature:
			Icon = HUDTemperatureIcon;
			break;
		case EFlareSubsystem::SYS_Propulsion:
			Icon = HUDPropulsionIcon;
			break;
		case EFlareSubsystem::SYS_RCS:
			Icon = HUDRCSIcon;
			break;
		case EFlareSubsystem::SYS_LifeSupport:
			Icon = HUDHealthIcon;
			break;
		case EFlareSubsystem::SYS_Power:
			Icon = HUDPowerIcon;
			break;
		case EFlareSubsystem::SYS_Weapon:
			Icon = HUDWeaponIcon;
			break;
	}

	DrawHUDIcon(Position, CockpitIconSize, Icon, HealthColor);
	FlareDrawText(SystemText.ToString(), Position + FVector2D(1.5 * CockpitIconSize, 0), HealthColor, false);
	Position += InstrumentLine;
}

FText AFlareHUD::GetShipStatus(AFlareSpacecraft* PlayerShip) const
{
	FText ModeText;
	FText AutopilotText;
	IFlareSpacecraftNavigationSystemInterface* Nav = PlayerShip->GetNavigationSystem();
	FFlareShipCommandData Command = Nav->GetCurrentCommand();

	if (Nav->IsDocked())
	{
		ModeText = LOCTEXT("Docked", "Docked");
	}
	else if (Command.Type == EFlareCommandDataType::CDT_Dock)
	{
		AFlareSpacecraft* Target = Cast<AFlareSpacecraft>(Command.ActionTarget);
		ModeText = FText::Format(LOCTEXT("DockingAtFormat", "Docking at {0}"), FText::FromName(Target->GetImmatriculation()));
	}
	else
	{
		ModeText = PlayerShip->GetWeaponsSystem()->GetWeaponModeInfo();
	}

	if (Nav->IsAutoPilot())
	{
		AutopilotText = LOCTEXT("Autopilot", " (Autopilot)");
	}

	return FText::Format(LOCTEXT("ShipInfoTextFormat", "{0} {1}"), ModeText, AutopilotText);
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FLinearColor AFlareHUD::GetTemperatureColor(float Current, float Max)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.FriendlyColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	float Distance = Current - 0.8f * Max;
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

inline static bool IsCloserToCenter(const FFlareScreenTarget& TargetA, const FFlareScreenTarget& TargetB)
{
	return (TargetA.DistanceFromScreenCenter < TargetB.DistanceFromScreenCenter);
}

void AFlareHUD::DrawHUDInternal()
{
	// Initial data and checks
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	UFlareSector* ActiveSector = PC->GetGame()->GetActiveSector();

	// So these are forbidden cases
	if (!ActiveSector || !PlayerShip || !PlayerShip->GetDamageSystem()->IsAlive() || MenuManager->IsMenuOpen() || MenuManager->IsSwitchingMenu() || IsWheelMenuOpen())
	{
		return;
	}
	
	// Iterate on all 'other' ships to show designators, markings, etc
	ScreenTargets.Empty();
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

		// Hide the context menu if nothing was found
		if (!FoundTargetUnderMouse || !IsInteractive)
		{
			ContextMenu->Hide();
		}
	}

	// Update HUD materials
	if (PlayerShip)
	{
		// Draw inertial vectors
		DrawSpeed(PC, PlayerShip, HUDReticleIcon, PlayerShip->GetSmoothedLinearVelocity() * 100, LOCTEXT("Forward", "FWD"), false);
		DrawSpeed(PC, PlayerShip, HUDBackReticleIcon, -PlayerShip->GetSmoothedLinearVelocity() * 100, LOCTEXT("Backward", "BWD"), true);

		// Draw nose
		if (!PlayerShip->GetStateManager()->IsExternalCamera())
		{
			DrawHUDIcon(CurrentViewportSize / 2, IconSize, PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN ? HUDAimIcon : HUDNoseIcon, HudColorNeutral, true);
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

				if (WorldToScreen(ObjectiveLocation, ScreenPosition))
				{
					if (IsInScreen(ScreenPosition))
					{
						// Draw icon
						DrawHUDIcon(ScreenPosition, IconSize, HUDObjectiveIcon, (Target->Active ? HudColorNeutral : InactiveColor), true);

						float Distance = (ObjectiveLocation - PlayerShip->GetActorLocation()).Size() / 100;
						FString ObjectiveText = FormatDistance(Distance);

						// Draw distance
						FVector2D CenterScreenPosition = ScreenPosition - CurrentViewportSize / 2 + FVector2D(0, IconSize);
						FlareDrawText(ObjectiveText, CenterScreenPosition, (Target->Active ? FLinearColor::White : InactiveTextColor));
					}

					// Tell the HUD to draw the search marker only if we are outside this
					ShouldDrawMarker = (FVector2D::Distance(ScreenPosition, CurrentViewportSize / 2) >= (CurrentViewportSize.GetMin() / 3));
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
			if (WorldToScreen(BombTarget, ScreenPosition))
			{
				// Icon
				DrawHUDIcon(ScreenPosition, IconSize, HUDBombAimIcon, HudColorNeutral, true);
			}
		}

		// Draw combat mouse pointer
		if (!PlayerShip->GetNavigationSystem()->IsAutoPilot() && (
		    PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_NONE
		 || PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN))
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
			DrawHUDIconRotated(CurrentViewportSize / 2 + MousePosDelta, IconSize, HUDCombatMouseIcon, PointerColor, MousePosDelta3D.Rotation().Yaw);
		}

		// Draw bombs
		for (int32 BombIndex = 0; BombIndex < ActiveSector->GetBombs().Num(); BombIndex++)
		{
			FVector2D ScreenPosition;
			AFlareBomb* Bomb = ActiveSector->GetBombs()[BombIndex];
			
			if (Bomb && WorldToScreen(Bomb->GetActorLocation(), ScreenPosition))
			{
				if (IsInScreen(ScreenPosition))
				{
					DrawHUDIcon(ScreenPosition, IconSize, HUDBombMarker, GetHostilityColor(PC, Bomb->GetFiringSpacecraft()) , true);
				}
			}
		}
	}

	// Sort screen targets
	ScreenTargets.Sort(&IsCloserToCenter);
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
	if (WorldToScreen(EndPoint, ScreenPosition) && SpeedMS > 1)
	{
		// Cap screen pos
		float ScreenBorderDistanceX = 100;
		float ScreenBorderDistanceY = 20;
		ScreenPosition.X = FMath::Clamp(ScreenPosition.X, ScreenBorderDistanceX, CurrentViewportSize.X - ScreenBorderDistanceX);
		ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, ScreenBorderDistanceY, CurrentViewportSize.Y - ScreenBorderDistanceY);

		// Label
		FString IndicatorText = Designation.ToString();
		FVector2D IndicatorPosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(42, 0);
		FlareDrawText(IndicatorText, IndicatorPosition);

		// Icon
		DrawHUDIcon(ScreenPosition, IconSize, Icon, HudColorNeutral, true);

		// Speed 
		FString VelocityText = FString::FromInt(Invert ? -SpeedMS : SpeedMS) + FString(" m/s");
		FVector2D VelocityPosition = ScreenPosition - CurrentViewportSize / 2 + FVector2D(42, 0);
		FlareDrawText(VelocityText, VelocityPosition);
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
		DrawHUDIconRotated(CurrentViewportSize / 2 + ScreenspacePosition, IconSize, HUDCombatMouseIcon, Color, Position3D.Rotation().Yaw);
	}
}

bool AFlareHUD::DrawHUDDesignator(AFlareSpacecraft* Spacecraft)
{
	// Calculation data
	FVector2D ScreenPosition;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = Spacecraft->GetActorLocation();

	if (WorldToScreen(TargetLocation, ScreenPosition))
	{
		// Compute apparent size in screenspace
		float ShipSize = 2 * Spacecraft->GetMeshScale();
		float Distance = (TargetLocation - PlayerLocation).Size();
		float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * CurrentViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.66f * Size, 300.0f) * FVector2D(1, 1);

		// Add to targets
		FFlareScreenTarget TargetData;
		TargetData.Spacecraft = Spacecraft;
		TargetData.DistanceFromScreenCenter = (ScreenPosition - CurrentViewportSize / 2).Size();
		ScreenTargets.Add(TargetData);

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
			bool Highlighted = (PlayerShip && Spacecraft == PlayerShip->GetCurrentTarget());
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0,     Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90,   Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180,  Color, Highlighted);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270,  Color, Highlighted);

			// Draw the target's distance
			FString DistanceText = FormatDistance(Distance / 100);
			FVector2D DistanceTextPosition = ScreenPosition - (CurrentViewportSize / 2) + FVector2D(-ObjectSize.X / 2, ObjectSize.Y / 2) + 3 * CornerSize * FVector2D::UnitVector;
			FlareDrawText(DistanceText, DistanceTextPosition, Color);

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

					if (InterceptTime > 0 && WorldToScreen(AmmoIntersectionLocation, ScreenPosition))
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
							FVector2D TimePosition = ScreenPosition - CurrentViewportSize / 2 - FVector2D(42,0);
							FlareDrawText(TimeText, TimePosition, HUDAimHelperColor);
						}
					}
				}
			}

			// Tell the HUD to draw the search marker only if we are outside this
			return (FVector2D::Distance(ScreenPosition, CurrentViewportSize / 2) >= (CurrentViewportSize.GetMin() / 3));
		}
	}

	// Dead ship
	if (!Spacecraft->GetDamageSystem()->IsAlive())
	{
		return false;
	}

	return true;
}

void AFlareHUD::DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float DesignatorIconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Highlighted)
{
	float ScaledDesignatorIconSize = DesignatorIconSize * (Highlighted ? 2 : 1);
	ObjectSize = FMath::Max(ObjectSize, ScaledDesignatorIconSize * FVector2D::UnitVector);

	FlareDrawTexture(Highlighted ? HUDDesignatorCornerSelectedTexture : HUDDesignatorCornerTexture,
		Position.X + (ObjectSize.X + DesignatorIconSize) * MainOffset.X / 2,
		Position.Y + (ObjectSize.Y + DesignatorIconSize) * MainOffset.Y / 2,
		ScaledDesignatorIconSize, ScaledDesignatorIconSize, 0, 0, 1, 1,
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
		UFont* Font = Large ? HUDFontLarge : HUDFont;

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

		// Drawing
		{
			FCanvasTextItem ShadowItem(FVector2D(X - 1, Y - 3), FText::FromString(Text), Font, FLinearColor::Black);
			ShadowItem.Scale = FVector2D(1, 1.4);
			CurrentCanvas->DrawItem(ShadowItem);
		}
		{
			FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
			TextItem.Scale = FVector2D(1, 1);
			CurrentCanvas->DrawItem(TextItem);
		}
	}
}

void AFlareHUD::FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor Color, EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
	if (CurrentCanvas && Texture)
	{
		FCanvasTileItem TileItem(FVector2D(ScreenX, ScreenY), Texture->Resource, FVector2D(ScreenW, ScreenH) * Scale, FVector2D(TextureU, TextureV), FVector2D(TextureU + TextureUWidth, TextureV + TextureVHeight), Color);
		TileItem.Rotation = FRotator(0, Rotation, 0);
		TileItem.PivotPoint = RotPivot;
		if (bScalePosition)
		{
			TileItem.Position *= Scale;
		}
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(BlendMode);
		CurrentCanvas->DrawItem(TileItem);
	}
}

bool AFlareHUD::IsInScreen(FVector2D ScreenPosition) const
{
	int32 ScreenBorderDistance = 100;

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

bool AFlareHUD::WorldToScreen(FVector World, FVector2D& Screen)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	// Cockpit projection
	if (IsDrawingCockpit)
	{
		FVector WorldDirection = World - PC->GetShipPawn()->GetActorLocation();
		FVector ScreenDirection = PC->GetShipPawn()->WorldToLocal(WorldDirection);
		ScreenDirection += FVector(10000, 0, 0); // Cockpit is supposed to be spherical around -100

		// FoV calculation. The vertical FoV is scaled by an arbitrary factor because UV adjustments were made for horizontal.
		float HorizontalFOV = PC->PlayerCameraManager->GetFOVAngle();
		float VerticalFOV = (ViewportSize.Y / ViewportSize.X) * HorizontalFOV * 0.79;

		FVector ScreenRotation = ScreenDirection.Rotation().Euler();
		if (FMath::Abs(ScreenRotation.Z) <= HorizontalFOV && FMath::Abs(ScreenRotation.Y) <= VerticalFOV)
		{
			Screen.X = (ScreenRotation.Z / HorizontalFOV) * CurrentViewportSize.X;
			Screen.Y = (-ScreenRotation.Y / VerticalFOV) * CurrentViewportSize.Y;
			Screen += CurrentViewportSize / 2;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Flat screen projection
	else
	{
		return PC->ProjectWorldLocationToScreen(World, Screen);
	}
}


#undef LOCTEXT_NAMESPACE

