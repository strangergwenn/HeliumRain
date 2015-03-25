
#include "../Flare.h"
#include "FlareHUD.h"
#include "../Player/FlarePlayerController.h"
#include "../Ships/FlareShipInterface.h"
#include "../Stations/FlareStationInterface.h"
#include "../FlareLoadingScreen/FlareLoadingScreen.h"


#define LOCTEXT_NAMESPACE "FlareHUD"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareHUD::AFlareHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, MenuIsOpen(false)
	, FadeDuration(0.15)
	, HUDHelpersMaterial(NULL)
{
	// Load content
	static ConstructorHelpers::FObjectFinder<UMaterial> HUDHelpersMaterialObj(TEXT("/Game/Gameplay/HUD/MT_HUDHelper"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDDesignatorCornerObj(TEXT("/Game/Gameplay/HUD/TX_DesignatorCorner.TX_DesignatorCorner"));

	// Set content
	HUDHelpersMaterialMaster = HUDHelpersMaterialObj.Object;
	HUDDesignatorCornerTexture = HUDDesignatorCornerObj.Object;

	// Dynamic data
	FadeTimer = FadeDuration;
	HudColor = FLinearColor::White;
	HudColor.A = 0.7;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareHUD::BeginPlay()
{
	Super::BeginPlay();
	HUDHelpersMaterial = UMaterialInstanceDynamic::Create(HUDHelpersMaterialMaster, GetWorld());
}

void AFlareHUD::Tick(float DeltaSeconds)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	Super::Tick(DeltaSeconds);

	// Mouse control
	if (PC)
	{
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
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
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());

	// Draw designators and context menu
	if (!MenuIsOpen)
	{
		// Draw designators
		FoundTargetUnderMouse = false;
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareShipBase* ShipBase = Cast<AFlareShipBase>(*ActorItr);
			if (PC && ShipBase && ShipBase != PC->GetShipPawn() && ShipBase != PC->GetMenuPawn())
			{
				if (PC->LineOfSightTo(ShipBase))
				{
					DrawHUDDesignator(ShipBase);
				}
			}
		}

		// Hide the context menu
		if (!FoundTargetUnderMouse || !IsExternalCamera)
		{
			ContextMenu->Hide();
		}
	}

	// Update HUD materials
	if (PC && !IsExternalCamera && !MenuIsOpen)
	{
		AFlareShip* Ship = PC->GetShipPawn();
		if (HUDHelpersMaterial && Ship)
		{
			// Get HUD data
			FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			int32 HelperScale = ViewportSize.Y;
			FRotator ShipAttitude = Ship->GetActorRotation();

			// Update helper
			HUDHelpersMaterial->SetVectorParameterValue(FName("Color"), PC->GetOverlayColor());
			HUDHelpersMaterial->SetScalarParameterValue(FName("Pitch"), -FMath::DegreesToRadians(ShipAttitude.Pitch));
			HUDHelpersMaterial->SetScalarParameterValue(FName("Yaw"), FMath::DegreesToRadians(ShipAttitude.Yaw));
			HUDHelpersMaterial->SetScalarParameterValue(FName("Roll"), FMath::DegreesToRadians(ShipAttitude.Roll));
			DrawMaterialSimple(HUDHelpersMaterial, ViewportSize.X / 2 - (HelperScale / 2), 0, HelperScale, HelperScale);
		}
	}
}

void AFlareHUD::DrawHUDDesignator(AFlareShipBase* ShipBase)
{
	// Calculation data
	FVector2D ScreenPosition;
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	FVector PlayerLocation = PC->GetShipPawn()->GetActorLocation();
	FVector TargetLocation = ShipBase->GetActorLocation();

	if (PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition))
	{
		// Compute apparent size in screenspace
		float ShipSize = 2 * ShipBase->GetMeshScale();
		float Distance = (TargetLocation - PlayerLocation).Size();
		float ApparentAngle = FMath::RadiansToDegrees(FMath::Atan(ShipSize / Distance));
		float Size = (ApparentAngle / PC->PlayerCameraManager->GetFOVAngle()) * ViewportSize.X;
		FVector2D ObjectSize = FMath::Min(0.8f * Size, 500.0f) * FVector2D(1, 1);

		// Check if the mouse is there
		FVector2D MousePos = PC->GetMousePosition();
		FVector2D ShipBoxMin = ScreenPosition - ObjectSize / 2;
		FVector2D ShipBoxMax = ScreenPosition + ObjectSize / 2;
		bool Hovering = (MousePos.X >= ShipBoxMin.X && MousePos.Y >= ShipBoxMin.Y && MousePos.X <= ShipBoxMax.X && MousePos.Y <= ShipBoxMax.Y);

		// Draw the context menu
		if (Hovering && !FoundTargetUnderMouse && IsExternalCamera)
		{
			// Update state
			FoundTargetUnderMouse = true;
			ContextMenuPosition = ScreenPosition;
			ContextMenu->Show();

			// If station, set data
			AFlareStation* Station = Cast<AFlareStation>(ShipBase);
			if (Station)
			{
				ContextMenu->SetStation(Station);
			}

			// If ship, set data
			AFlareShip* Ship = Cast<AFlareShip>(ShipBase);
			if (Ship)
			{
				ContextMenu->SetShip(Ship);
			}
		}
		else
		{
			// Draw designator
			float CornerSize = 16;
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, -1), 0);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(-1, +1), -90);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, +1), -180);
			DrawHUDDesignatorCorner(ScreenPosition, ObjectSize, CornerSize, FVector2D(+1, -1), -270);
		}
	}
}

void AFlareHUD::DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float CornerSize, FVector2D MainOffset, float Rotation)
{
	DrawTexture(HUDDesignatorCornerTexture,
		Position.X + (ObjectSize.X + CornerSize) * MainOffset.X / 2,
		Position.Y + (ObjectSize.Y + CornerSize) * MainOffset.Y / 2,
		CornerSize, CornerSize, 0, 0, 1, 1,
		HudColor,
		BLEND_Translucent, 1.0f, false,
		Rotation);
}


/*----------------------------------------------------
	Menu interaction
----------------------------------------------------*/

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
		SAssignNew(Dashboard, SFlareDashboard).OwnerHUD(this);
		SAssignNew(CompanyMenu, SFlareCompanyMenu).OwnerHUD(this);
		SAssignNew(ShipMenu, SFlareShipMenu).OwnerHUD(this);
		SAssignNew(StationMenu, SFlareStationMenu).OwnerHUD(this);
		SAssignNew(SectorMenu, SFlareSectorMenu).OwnerHUD(this);

		// Fade-to-black system
		SAssignNew(Fader, SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"));

		// Register menus at their Z-Index
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(HUDMenu.ToSharedRef()),          0);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(OverlayContainer.ToSharedRef()), 10);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Dashboard.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(StationMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),       50);
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

void AFlareHUD::SetIsExternalCamera(bool Status)
{
	IsExternalCamera = Status;
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
			InspectShip(static_cast<IFlareShipInterface*>(FadeTargetData));
			break;

		case EFlareMenu::MENU_ShipConfig:
			InspectShip(static_cast<IFlareShipInterface*>(FadeTargetData), true);
			break;

		case EFlareMenu::MENU_Sector:
			OpenSector();
			break;

		case EFlareMenu::MENU_Station:
			InspectStation(static_cast<IFlareStationInterface*>(FadeTargetData));
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

void AFlareHUD::InspectShip(IFlareShipInterface* Target, bool IsEditable)
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

void AFlareHUD::InspectStation(IFlareStationInterface* Target, bool IsEditable)
{
	ResetMenu();
	SetMenuPawn(true);
	OverlayContainer->SetVisibility(EVisibility::Hidden);

	AFlareShip* PlayerShip = Cast<AFlarePlayerController>(GetOwner())->GetShipPawn();

	if (Target == NULL && PlayerShip && PlayerShip->IsDocked())
	{
		Target = PlayerShip->GetDockStation();
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
	HUDMenu->SetVisibility(EVisibility::Visible);
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
			HUDMenu->SetTargetShip(PC->GetShipPawn());
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

