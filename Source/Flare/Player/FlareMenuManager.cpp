
#include "../Flare.h"
#include "FlareMenuManager.h"
#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../FlareLoadingScreen/FlareLoadingScreen.h"


#define LOCTEXT_NAMESPACE "FlareMenuManager"


/*----------------------------------------------------
	Setup
----------------------------------------------------*/

AFlareMenuManager::AFlareMenuManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, MenuIsOpen(false)
	, FadeDuration(0.25)
{
}

void AFlareMenuManager::BeginPlay()
{
	Super::BeginPlay();
	FadeIn();
}

void AFlareMenuManager::SetupMenu(FFlarePlayerSave& PlayerData)
{
	if (GEngine->IsValidLowLevel())
	{
		// Menus
		SAssignNew(Notifier, SFlareNotifier).MenuManager(this).Visibility(EVisibility::SelfHitTestInvisible);
		SAssignNew(MainMenu, SFlareMainMenu).MenuManager(this);
		SAssignNew(Dashboard, SFlareDashboard).MenuManager(this);
		SAssignNew(CompanyMenu, SFlareCompanyMenu).MenuManager(this);
		SAssignNew(ShipMenu, SFlareShipMenu).MenuManager(this);
		SAssignNew(StationMenu, SFlareStationMenu).MenuManager(this);
		SAssignNew(SectorMenu, SFlareSectorMenu).MenuManager(this);

		// Fader
		SAssignNew(Fader, SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FFlareStyleSet::Get().GetBrush("/Brushes/SB_Black"));
		Fader->SetVisibility(EVisibility::Hidden);

		// Register menus at their Z-Index
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Dashboard.ToSharedRef()),        50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(CompanyMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShipMenu.ToSharedRef()),         50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(StationMenu.ToSharedRef()),      50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(SectorMenu.ToSharedRef()),       50);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Notifier.ToSharedRef()),         90);
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(Fader.ToSharedRef()),            100);

		// Setup menus
		MainMenu->Setup();
		Dashboard->Setup();
		CompanyMenu->Setup(PlayerData);
		ShipMenu->Setup();
		StationMenu->Setup();
		SectorMenu->Setup();
	}
}


/*----------------------------------------------------
	Menu interaction
----------------------------------------------------*/

void AFlareMenuManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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

void AFlareMenuManager::OpenMenu(EFlareMenu::Type Target, void* Data)
{
	MenuIsOpen = true;
	FadeOut();
	FadeTarget = Target;
	FadeTargetData = Data;
}

void AFlareMenuManager::CloseMenu(bool HardClose)
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

bool AFlareMenuManager::IsMenuOpen() const
{
	return MenuIsOpen;
}

void AFlareMenuManager::ShowLoadingScreen()
{
	IFlareLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IFlareLoadingScreenModule>("FlareLoadingScreen");
	if (LoadingScreenModule)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void AFlareMenuManager::Notify(FText Text, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	if (Notifier.IsValid())
	{
		Notifier->Notify(Text, Info, Type, TargetMenu, TargetInfo);
	}
}

const FSlateBrush* AFlareMenuManager::GetMenuIcon(EFlareMenu::Type MenuType)
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
		case EFlareMenu::MENU_Quit:           return FFlareStyleSet::GetIcon("Quit");
		case EFlareMenu::MENU_Exit:           return FFlareStyleSet::GetIcon("Close");

		default:
			return NULL;
	}
}


/*----------------------------------------------------
	Menu management
----------------------------------------------------*/

void AFlareMenuManager::ResetMenu()
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

void AFlareMenuManager::FadeIn()
{
	FadeFromBlack = true;
	FadeTimer = 0;
}

void AFlareMenuManager::FadeOut()
{
	FadeFromBlack = false;
	FadeTimer = 0;
}

void AFlareMenuManager::ProcessFadeTarget()
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

		case EFlareMenu::MENU_FlyShip:
			FlyShip(static_cast<AFlareSpacecraft*>(FadeTargetData));
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

	// Reset everything
	FadeTargetData = NULL;
	FadeTarget = EFlareMenu::MENU_None;
	GetPC()->GetNavHUD()->UpdateHUDVisibility();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void AFlareMenuManager::OpenDashboard()
{
	ResetMenu();
	GetPC()->OnEnterMenu();

	Dashboard->Enter();
}

void AFlareMenuManager::InspectCompany(UFlareCompany* Target)
{
	ResetMenu();
	GetPC()->OnEnterMenu();

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetCompany();
	}
	CompanyMenu->Enter(Target);
}

void AFlareMenuManager::FlyShip(AFlareSpacecraft* Target)
{
	ExitMenu();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetOwner());
	if (PC && Target)
	{
		PC->FlyShip(Target);
		MenuIsOpen = false;
	}
}

void AFlareMenuManager::InspectShip(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	ResetMenu();
	GetPC()->OnEnterMenu();

	if (Target == NULL)
	{
		Target = Cast<AFlarePlayerController>(GetOwner())->GetShipPawn();
	}
	ShipMenu->Enter(Target, IsEditable);
}

void AFlareMenuManager::InspectStation(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	ResetMenu();
	GetPC()->OnEnterMenu();

	AFlareSpacecraft* PlayerShip = Cast<AFlarePlayerController>(GetOwner())->GetShipPawn();

	if (Target == NULL && PlayerShip && PlayerShip->GetNavigationSystem()->IsDocked())
	{
		Target = PlayerShip->GetNavigationSystem()->GetDockStation();
	}
	StationMenu->Enter(Target);
}

void AFlareMenuManager::OpenSector()
{
	ResetMenu();
	GetPC()->OnEnterMenu();
	SectorMenu->Enter();
}

void AFlareMenuManager::ExitMenu()
{
	ResetMenu();
	GetPC()->OnExitMenu();
}


#undef LOCTEXT_NAMESPACE

