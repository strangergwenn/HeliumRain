
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
{
	DefaultMouseCursor = EMouseCursor::Default;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlarePlayerController::BeginPlay()
{
	// Setup player stuff
	Super::BeginPlay();
	SetupMenu();

	// Load our ship
	PossessCurrentShip();
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
	else
	{
		// Keep param original value
	}

	if (ExternalCamera != NewState || Force)
	{
		bShowMouseCursor = true;
		
		// Send the camera order to the ship
		if (ShipPawn)
		{
			ShipPawn->SetExternalCamera(NewState);
		}
		Cast<AFlareHUD>(GetHUD())->SetIsExternalCamera(NewState);
		ExternalCamera = NewState;
	}
}

void AFlarePlayerController::FlyShip(AFlareShip* Ship)
{
	Possess(Ship);
	ShipPawn = Ship;
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
}

void AFlarePlayerController::Save(FFlarePlayerSave& Data)
{
	PlayerData.CurrentShipName = ShipPawn->GetName();
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

	// At this point, no save file was available, create a first company & ship
	if (!ShipPawn)
	{
		Company = GetGame()->CreateCompany("Weyland-Yutani");
		ShipPawn = GetGame()->CreateShip(FName("ship-ghoul"));
		ShipPawn->SetOwnerCompany(Company);
	}

	// Possess the ship
	FlyShip(ShipPawn);

	// Destroy the old pawn
	if (DefaultPawn)
	{
		DefaultPawn->Destroy();
	}
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
	Customization
----------------------------------------------------*/

void AFlarePlayerController::CustomizeModuleMaterial(UMaterialInstanceDynamic* Mat)
{
	// Get data from storage
	AFlareGame* Game = GetGame();
	FLinearColor PaintColor = Game->GetCustomizationCatalog()->GetColor(PlayerData.CustomizationPaintColorIndex);
	FLinearColor LightColor = Game->GetCustomizationCatalog()->GetColor(PlayerData.CustomizationLightColorIndex);
	UTexture2D* Pattern = Game->GetCustomizationCatalog()->GetPattern(PlayerData.CustomizationPatternIndex);

	// Apply settings to the material instance
	Mat->SetVectorParameterValue("PaintColor", PaintColor);
	Mat->SetVectorParameterValue("GlowColor", NormalizeColor(LightColor));
	Mat->SetTextureParameterValue("PaintPattern", Pattern);
}

void AFlarePlayerController::CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat)
{
	FLinearColor EngineColor = GetGame()->GetCustomizationCatalog()->GetColor(PlayerData.CustomizationEngineColorIndex);
	Mat->SetVectorParameterValue("GlowColor", NormalizeColor(EngineColor));
}

void AFlarePlayerController::SetCustomizationPaintColorIndex(int32 Index)
{
	PlayerData.CustomizationPaintColorIndex = Index;
	UpdateShipCustomization();
}

void AFlarePlayerController::SetCustomizationLightColorIndex(int32 Index)
{
	PlayerData.CustomizationLightColorIndex = Index;
	UpdateShipCustomization();
}

void AFlarePlayerController::SetCustomizationEngineColorIndex(int32 Index)
{
	PlayerData.CustomizationEngineColorIndex = Index;
	UpdateShipCustomization();
}

void AFlarePlayerController::SetCustomizationPatternIndex(int32 Index)
{
	PlayerData.CustomizationPatternIndex = Index;
	UpdateShipCustomization();
}

void AFlarePlayerController::UpdateShipCustomization()
{
	// Current player ship
	if (ShipPawn)
	{
		ShipPawn->UpdateCustomization();
	}

	// Current menu pawn
	if (MenuPawn)
	{
		MenuPawn->UpdateCustomization();
	}
}

FLinearColor AFlarePlayerController::NormalizeColor(FLinearColor Col) const
{
	return FLinearColor(FVector(Col.R, Col.G, Col.B) / Col.GetLuminance());
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlarePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ToggleCamera", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCamera);
	InputComponent->BindAction("ToggleMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleMenu);

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
