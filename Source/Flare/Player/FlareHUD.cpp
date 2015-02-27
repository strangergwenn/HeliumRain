
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
	, HUDTextRenderTarget(NULL)
{
	FadeTimer = FadeDuration;
	static ConstructorHelpers::FObjectFinder<UFont> HUDFontObj(TEXT("/Game/Master/FT_LatoBold"));
	static ConstructorHelpers::FObjectFinder<UMaterial> HUDHelpersMaterialObj(TEXT("/Game/Master/Materials/MT_HUDHelper"));
	static ConstructorHelpers::FObjectFinder<UMaterial> HUDTextMaterialMasterObj(TEXT("/Game/Master/Materials/MT_HUDGenerated"));
	HUDHelpersMaterialMaster = HUDHelpersMaterialObj.Object;
	HUDTextMaterialMaster = HUDTextMaterialMasterObj.Object;
	HUDFont = HUDFontObj.Object;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareHUD::BeginPlay()
{
	Super::BeginPlay();
	HUDHelpersMaterial = UMaterialInstanceDynamic::Create(HUDHelpersMaterialMaster, GetWorld());
	HUDTextMaterial = UMaterialInstanceDynamic::Create(HUDTextMaterialMaster, GetWorld());
}

void AFlareHUD::Tick(float DeltaSeconds)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	Super::Tick(DeltaSeconds);

	// In-game interactions with stations or other ships
	if (PC)
	{
		// Trace data
		FHitResult TraceHit;
		float      TraceDepth = 1000000;
		FVector    TraceOrigin;
		FVector    TraceDirection;

		// Deproject and trace the result against the world
		if (IsExternalCamera && PC->DeprojectMousePositionToWorld(TraceOrigin, TraceDirection))
		{
			GetWorld()->LineTraceSingle(TraceHit, TraceOrigin, TraceOrigin + TraceDepth * TraceDirection, ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams("ClickableTrace", true));

			// Update the context menu
			if (TraceHit.IsValidBlockingHit()
				&& TraceHit.Actor.IsValid()
				&& TraceHit.Actor->IsA(AFlareShipBase::StaticClass())
				&& TraceHit.Actor != PC->GetShipPawn())
			{
				if (!ContextMenu->IsVisible())
				{
					PC->ProjectWorldLocationToScreen(TraceHit.Component->GetComponentLocation(), ContextMenuPosition);
					ContextMenu->Show();

					// If station, set data
					AFlareStation* Station = Cast<AFlareStation>(TraceHit.Actor.Get());
					if (Station)
					{
						ContextMenu->SetStation(Station);
					}

					// If ship, set data
					AFlareShip* Ship = Cast<AFlareShip>(TraceHit.Actor.Get());
					if (Ship)
					{
						ContextMenu->SetShip(Ship);
					}
				}
			}

			// Close the context menu
			else if (ContextMenu->CanBeHidden())
			{
				ContextMenu->Hide();
			}
		}

		// Mouse control
		if (!ContextMenu->IsOpen())
		{
			FVector2D MousePos = PC->GetMousePosition();
			FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
			MousePos = 2 * ((MousePos - ViewportCenter) / ViewportSize);
			PC->MousePositionInput(MousePos);
		}
		else
		{
			PC->MousePositionInput(FVector2D::ZeroVector);
		}
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

	// Create render target for the text
	if (!HUDTextRenderTarget)
	{
		FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		HUDTextRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GetWorld(), UCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y);
		if (HUDTextRenderTarget)
		{
			HUDTextRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlareHUD::DrawHUDTextTarget);
			HUDTextMaterial->SetTextureParameterValue(TEXT("GeneratedTexture"), HUDTextRenderTarget);
		}
	}

	if (PC && !IsExternalCamera && !MenuIsOpen)
	{
		AFlareShip* Ship = PC->GetShipPawn();
		if (HUDHelpersMaterial && HUDTextMaterial && Ship)
		{
			// Get HUD data
			FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			int32 HelperScale = ViewportSize.Y;
			FRotator ShipAttitude = Ship->GetActorRotation();

			// Update helper
			HUDHelpersMaterial->SetVectorParameterValue(FName("Color"), PC->GetLightColor());
			HUDHelpersMaterial->SetScalarParameterValue(FName("Pitch"), -FMath::DegreesToRadians(ShipAttitude.Pitch));
			HUDHelpersMaterial->SetScalarParameterValue(FName("Yaw"), FMath::DegreesToRadians(ShipAttitude.Yaw));
			HUDHelpersMaterial->SetScalarParameterValue(FName("Roll"), FMath::DegreesToRadians(ShipAttitude.Roll));
			DrawMaterialSimple(HUDHelpersMaterial, ViewportSize.X / 2 - (HelperScale / 2), 0, HelperScale, HelperScale);

			// Update panel
			if (HUDTextRenderTarget)
			{
				HUDTextRenderTarget->UpdateResource();
				HUDTextMaterial->SetVectorParameterValue(FName("Color"), PC->GetLightColor());
				DrawMaterialSimple(HUDTextMaterial, 0, 0, ViewportSize.X, ViewportSize.Y);
			}
		}
	}
}

void AFlareHUD::DrawHUDTextTarget(UCanvas* Cnv, int32 Width, int32 Height)
{
	Cnv->Canvas->Clear(FLinearColor::Black);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC)
	{
		AFlareShip* Ship = PC->GetShipPawn();
		if (Ship)
		{
			TArray<UFlareWeapon*>& WeaponList = Ship->GetWeaponList();

			// Weapons
			PrintText(Cnv, FString("MAIN GUNS"), 200, 150);
			PrintText(Cnv, FString("ARMED"), 300, 150);
			for (int32 i = 0; i < WeaponList.Num(); i++)
			{
				PrintText(Cnv, FString("GUN ") + FString::FromInt(i) + (WeaponList[i]->isFiring() ? " (!)" : ""), 200, 200 + 30 * i);
				PrintText(Cnv, FString::FromInt(WeaponList[i]->GetCurrentAmmo()) + " / " + FString::FromInt(WeaponList[i]->GetMaxAmmo()) , 300, 200 + 30 * i);
			}

			// Ship location
			FVector ShipLocation = Ship->GetActorLocation() / 100;
			FString LocationDescription = FString::Printf(TEXT("\n\n%ldm\n%ldm\n%ldm"), (long)ShipLocation.X, (long)ShipLocation.Y, (long)ShipLocation.Z);
			PrintText(Cnv, Ship->GetName() + FString("\n\nX\nY\nZ"), Width - 300, 150);
			PrintText(Cnv, LocationDescription, Width - 200, 150);
		}
	}
}

void AFlareHUD::PrintText(UCanvas* Cnv, FString Text, int32 X, int32 Y)
{
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), HUDFont, FLinearColor::White);
	TextItem.Scale *= FVector2D((float)Cnv->SizeY / 2048, (float)Cnv->SizeY / 2048);
	Cnv->DrawItem(TextItem);
}


/*----------------------------------------------------
	Menu interaction
----------------------------------------------------*/

void AFlareHUD::SetupMenu(FFlarePlayerSave& PlayerData)
{
	if (GEngine->IsValidLowLevel())
	{
		// HUD widgets
		SAssignNew(HUDContainer, SOverlay)

			// Context menu
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ContextMenu, SFlareContextMenu).OwnerHUD(this)
			];

		// Create menus
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
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(HUDContainer.ToSharedRef()),    10);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Dashboard.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),     50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(StationMenu.ToSharedRef()),     50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),           100);

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
	HUDContainer->SetVisibility(EVisibility::Hidden);

	Dashboard->Enter();
}

void AFlareHUD::InspectCompany(UFlareCompany* Target)
{
	ResetMenu();
	SetMenuPawn(true);
	HUDContainer->SetVisibility(EVisibility::Hidden);

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
	HUDContainer->SetVisibility(EVisibility::Hidden);

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
	HUDContainer->SetVisibility(EVisibility::Hidden);

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
	HUDContainer->SetVisibility(EVisibility::Hidden);

	SectorMenu->Enter();
}

void AFlareHUD::ExitMenu()
{
	ResetMenu();
	SetMenuPawn(false);
	HUDContainer->SetVisibility(EVisibility::Visible);
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

