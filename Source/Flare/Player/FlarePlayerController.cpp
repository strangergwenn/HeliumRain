
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
	, DustEffect(NULL)
	, Company(NULL)
	, CombatMode(false)
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_Dust"));
	DustEffectTemplate = DustEffectTemplateObj.Object;
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
	bShowMouseCursor = !CombatMode;

	// Spawn dust effects if they are not already here
	if (!DustEffect && ShipPawn)
	{
		DustEffect = UGameplayStatics::SpawnEmitterAttached(
			DustEffectTemplate,
			ShipPawn->GetRootComponent(),
			NAME_None);
	}

	// Update dust effects
	if (DustEffect && ShipPawn && !IsInMenu())
	{
		FVector ViewLocation;
		FRotator ViewRotation;

		GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewRotation.Normalize();
		ViewLocation += ViewRotation.RotateVector(500 * FVector(1, 0, 0));

		FVector Direction = ShipPawn->GetLinearVelocity();
		Direction.Normalize();

		DustEffect->SetWorldLocation(ViewLocation);
		DustEffect->SetVectorParameter("Direction", -Direction);
	}
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
		ResetMousePosition();
	}

	// If something changed...
	if (ExternalCamera != NewState || Force)
	{		
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
	if (ShipPawn)
	{
		ShipPawn->EnablePilot(true);
	}

	Possess(Ship);
	ShipPawn = Ship;
	CombatMode = false;
	SetExternalCamera(true, true);
	ShipPawn->EnablePilot(false);
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

void AFlarePlayerController::Notify(FText Text, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	Cast<AFlareHUD>(GetHUD())->Notify(Text, Type, TargetMenu, TargetInfo);
}

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

		if (CombatMode)
		{
			CombatMode = false;
			ShipPawn->SetCombatMode(false);
			ResetMousePosition();
		}
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

void AFlarePlayerController::ResetMousePosition()
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FViewport* Viewport = LocalPlayer->ViewportClient->Viewport;
		FVector2D ViewportSize = Viewport->GetSizeXY();
		Viewport->SetMouse(ViewportSize.X / 2, ViewportSize.Y / 2);
	}
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
	InputComponent->BindAction("TooglePilot", EInputEvent::IE_Released, this, &AFlarePlayerController::TogglePilot);

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
	if (ShipPawn->IsMilitary() && !IsInMenu())
	{
		CombatMode = !CombatMode;
		ShipPawn->SetCombatMode(CombatMode);
		SetExternalCamera(false, true);
		ResetMousePosition();
	}
}

void AFlarePlayerController::TogglePilot()
{
	FLOG("TooglePilot");
	ShipPawn->EnablePilot(!ShipPawn->IsPilotMode());
}

void AFlarePlayerController::Test1()
{
	Notify(FText::FromString("I am Test1"), EFlareNotification::NT_Trading, EFlareMenu::MENU_Dashboard);
}

void AFlarePlayerController::Test2()
{
	Notify(FText::FromString("I am Test2"));
}
