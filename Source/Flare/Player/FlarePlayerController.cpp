
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Ships/FlareShip.h"
#include "../Stations/FlareStation.h"
#include "EngineUtils.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlayerController::AFlarePlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Company(NULL)
	, CombatMode(false)
{
	DefaultMouseCursor = EMouseCursor::Default;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlarePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Load our ship
	PossessCurrentShip();

	// Menus
	SetupMenu();
	SetExternalCamera(false);
}


void AFlarePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AFlarePlayerController::SetExternalCamera(bool NewState, bool Force)
{
	// No internal camera when docked
	if (ShipPawn && ShipPawn->IsDocked())
	{
		NewState = true;
	}

	// Abort combat if we are going to external
	if (NewState && CombatMode)
	{
		CombatMode = false;
		ShipPawn->SetCombatMode(false);
	}

	// If something changed...
	if (ExternalCamera != NewState || Force)
	{
		// TODO COMBAT MODE : force focus / mouse appearance when changing focus
		bShowMouseCursor = !CombatMode;
		
		// Send the camera order to the ship
		if (ShipPawn)
		{
			ShipPawn->SetExternalCamera(NewState);
		}

		// Update camera 
		Cast<AFlareHUD>(GetHUD())->SetInteractive(!CombatMode);
		ExternalCamera = NewState;
	}
}

void AFlarePlayerController::FlyShip(AFlareShip* Ship)
{
	Possess(Ship);
	ShipPawn = Ship;
	CombatMode = false;
	SetExternalCamera(true, true);
}

void AFlarePlayerController::PrepareForExit()
{
	if (IsInMenu())
	{
		Cast<AFlareHUD>(GetHUD())->CloseMenu(true);
	}
}


/*----------------------------------------------------
	Data management
----------------------------------------------------*/

void AFlarePlayerController::Load(const FFlarePlayerSave& Data)
{
	PlayerData = Data;
	Company = GetGame()->FindCompany(Data.CompanyIdentifier);
}

void AFlarePlayerController::Save(FFlarePlayerSave& Data)
{
	if (ShipPawn)
	{
		PlayerData.CurrentShipName = ShipPawn->GetName();
	}
	Data = PlayerData;
}

void AFlarePlayerController::PossessCurrentShip()
{
	// Save the default pawn
	APawn* DefaultPawn = GetPawn();
	ShipPawn = NULL;

	// Look for the ship in the game
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareShip* Ship = Cast<AFlareShip>(*ActorItr);
		if (Ship && Ship->GetName() == PlayerData.CurrentShipName)
		{
			FLOGV("AFlarePlayerController::PossessCurrentShip : Found player ship '%s'", *PlayerData.CurrentShipName);
			ShipPawn = Ship;
			break;
		}
	}

	// Possess the ship
	FlyShip(ShipPawn);

	// Destroy the old pawn
	if (DefaultPawn)
	{
		DefaultPawn->Destroy();
	}
}

void AFlarePlayerController::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}


/*----------------------------------------------------
	Menus
----------------------------------------------------*/

void AFlarePlayerController::SetupMenu()
{
	// Spawn the menu pawn at an arbitrarily large location
	FVector SpawnLocation(5000000 * FVector(1, 1, 1));
	MenuPawn = GetWorld()->SpawnActor<AFlareMenuPawn>(GetGame()->GetMenuPawnClass(), SpawnLocation, FRotator::ZeroRotator);

	// Signal the menu to setup as well
	Cast<AFlareHUD>(GetHUD())->SetupMenu(PlayerData);
}

void AFlarePlayerController::OnEnterMenu()
{
	if (!IsInMenu())
	{
		Possess(MenuPawn);
		bShowMouseCursor = true;
	}
}

void AFlarePlayerController::OnExitMenu()
{
	if (IsInMenu())
	{
		Possess(ShipPawn);
		SetExternalCamera(false);
	}
}

bool AFlarePlayerController::IsInMenu()
{
	return (GetPawn() == MenuPawn);
}

FVector2D AFlarePlayerController::GetMousePosition()
{
	FVector2D Result;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D ScreenPosition;
		LocalPlayer->ViewportClient->GetMousePosition(Result);
	}

	return Result;
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlarePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ToggleCamera", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCamera);
	InputComponent->BindAction("ToggleMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleMenu);
	InputComponent->BindAction("ToggleCombat", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCombat);

	InputComponent->BindAction("Test1", EInputEvent::IE_Released, this, &AFlarePlayerController::Test1);
	InputComponent->BindAction("Test2", EInputEvent::IE_Released, this, &AFlarePlayerController::Test2);
}

void AFlarePlayerController::MousePositionInput(FVector2D Val)
{
	if (ShipPawn)
	{
		ShipPawn->MousePositionInput(Val);
	}
}

void AFlarePlayerController::ToggleCamera()
{
	SetExternalCamera(!ExternalCamera);
}

void AFlarePlayerController::ToggleMenu()
{
	if (IsInMenu())
	{
		Cast<AFlareHUD>(GetHUD())->CloseMenu();
	}
	else
	{
		Cast<AFlareHUD>(GetHUD())->OpenMenu(EFlareMenu::MENU_Dashboard);
	}
}

void AFlarePlayerController::ToggleCombat()
{
	if (ShipPawn->IsMilitary())
	{
		CombatMode = !CombatMode;
		ShipPawn->SetCombatMode(CombatMode);
		SetExternalCamera(false, true);
	}
}

void AFlarePlayerController::Test1()
{
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFlareStation* TargetStation = Cast<AFlareStation>(*ActorItr);
		if (TargetStation)
		{
			ShipPawn->DockAt(TargetStation);
		}
	}
}

void AFlarePlayerController::Test2()
{
	ShipPawn->Undock();
}
