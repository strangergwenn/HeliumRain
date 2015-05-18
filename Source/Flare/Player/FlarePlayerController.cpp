
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Ships/FlareShip.h"
#include "../Ships/FlareOrbitalEngine.h"
#include "EngineUtils.h"


#define LOCTEXT_NAMESPACE "AFlarePlayerController"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlayerController::AFlarePlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, DustEffect(NULL)
	, Company(NULL)
	, CombatMode(false)
{
	// Mouse
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_Dust"));
	DustEffectTemplate = DustEffectTemplateObj.Object;
	DefaultMouseCursor = EMouseCursor::Default;

	// Sounds
	static ConstructorHelpers::FObjectFinder<USoundCue> OnSoundObj(TEXT("/Game/Master/Sound/A_Beep_On"));
	static ConstructorHelpers::FObjectFinder<USoundCue> OffSoundObj(TEXT("/Game/Master/Sound/A_Beep_Off"));
	OnSound = OnSoundObj.Object;
	OffSound = OffSoundObj.Object;

	// Power sound
	PowerSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("PowerSound"));
	PowerSound->AttachTo(RootComponent);
	PowerSound->bAutoActivate = false;
	PowerSound->bAutoDestroy = false;
	PowerSoundFadeSpeed = 0.3;

	// Engine sound
	EngineSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EngineSound->AttachTo(RootComponent);
	EngineSound->bAutoActivate = false;
	EngineSound->bAutoDestroy = false;
	EngineSoundFadeSpeed = 2.0;

	// RCS sound
	RCSSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("RCSSound"));
	RCSSound->AttachTo(RootComponent);
	RCSSound->bAutoActivate = false;
	RCSSound->bAutoDestroy = false;
	RCSSoundFadeSpeed = 5.0;
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


void AFlarePlayerController::PlayerTick(float DeltaSeconds)
{
	Super::PlayerTick(DeltaSeconds);
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

	// Sound management
	if (ShipPawn)
	{
		// Engine values
		float RCSAlpha = 0;
		float EngineAlpha = 0;
		int32 RCSCount = 0;
		int32 EngineCount = 0;

		// Check all engines for engine alpha values
		TArray<UActorComponent*> Engines = ShipPawn->GetComponentsByClass(UFlareEngine::StaticClass());
		for (int32 EngineIndex = 0; EngineIndex < Engines.Num(); EngineIndex++)
		{
			UFlareEngine* Engine = Cast<UFlareEngine>(Engines[EngineIndex]);
			if (Engine->IsA(UFlareOrbitalEngine::StaticClass()))
			{
				EngineAlpha += Engine->GetEffectiveAlpha();
				EngineCount++;
			}
			else
			{
				RCSAlpha += Engine->GetEffectiveAlpha();
				RCSCount++;
			}
		}

		// Update sounds
		UpdateSound(PowerSound,  (ShipPawn->IsPowered() && !ShipPawn->HasPowerOutage() ? 1 : -1) * PowerSoundFadeSpeed  * DeltaSeconds, PowerSoundVolume);
		UpdateSound(EngineSound, (EngineAlpha > 0 ? EngineAlpha / EngineCount : -1)              * EngineSoundFadeSpeed * DeltaSeconds, EngineSoundVolume);
		UpdateSound(RCSSound,    (RCSAlpha > 0 ? RCSAlpha / RCSCount : -1)                       * RCSSoundFadeSpeed    * DeltaSeconds, RCSSoundVolume);
	}
	else
	{
		PowerSound->Stop();
		EngineSound->Stop();
		RCSSound->Stop();
		PowerSoundVolume = 0;
		EngineSoundVolume = 0;
		RCSSoundVolume = 0;
	}
}

void AFlarePlayerController::UpdateSound(UAudioComponent* SoundComp, float VolumeDelta, float& CurrentVolume)
{
	float NewVolume = FMath::Clamp(CurrentVolume + VolumeDelta, 0.0f, 1.0f);
	if (NewVolume != CurrentVolume)
	{
		if (NewVolume == 0)
		{
			SoundComp->Stop();
		}
		else if (CurrentVolume == 0)
		{
			SoundComp->Play();
		}
		else
		{
			SoundComp->SetVolumeMultiplier(NewVolume);
			SoundComp->SetPitchMultiplier(0.5 + 0.5 * NewVolume);
		}
		CurrentVolume = NewVolume;
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
	// Reset the current ship to auto
	if (ShipPawn)
	{
		ShipPawn->EnablePilot(true);
	}

	// Fly the new ship
	Possess(Ship);
	ShipPawn = Ship;
	CombatMode = false;
	SetExternalCamera(true, true);
	ShipPawn->EnablePilot(false);

	QuickSwitchNextOffset = 0;

	FFlareShipDescription* ShipDescription = Ship->GetDescription();
	if(ShipDescription)
	{
		PowerSound->SetSound(ShipDescription->PowerSound);
	}
	else
	{
		PowerSound->SetSound(NULL);
	}

	FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
	if(EngineDescription)
	{
		EngineSound->SetSound(EngineDescription->EngineCharacteristics.EngineSound);
	}
	else
	{
		EngineSound->SetSound(NULL);
	}

	FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
	if(RCSDescription)
	{
		RCSSound->SetSound(RCSDescription->EngineCharacteristics.EngineSound);
	}
	else
	{
		RCSSound->SetSound(NULL);
	}

	// Inform the player
	if (Ship)
	{
		FString Text = LOCTEXT("Flying", "Now flying").ToString();
		Notify(FText::FromString(Text + " " + FString(*Ship->GetName())));
	}


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
	FLOGV("AFlarePlayerController::Notify : '%s'", *Text.ToString());
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
		ClientPlaySound(OnSound);
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
		ClientPlaySound(OffSound);
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
	InputComponent->BindAction("ToggleHUD", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleHUD);
	InputComponent->BindAction("QuickSwitch", EInputEvent::IE_Released, this, &AFlarePlayerController::QuickSwitch);

	InputComponent->BindAction("Test1", EInputEvent::IE_Released, this, &AFlarePlayerController::Test1);
	InputComponent->BindAction("Test2", EInputEvent::IE_Released, this, &AFlarePlayerController::Test2);
}

void AFlarePlayerController::MousePositionInput(FVector2D Val)
{
	if (ShipPawn && !CombatMode)
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
	if (ShipPawn && ShipPawn->IsMilitary() && !ShipPawn->IsDocked() && !IsInMenu())
	{
		FLOGV("AFlarePlayerController::ToggleCombat : new state is %d", !CombatMode);
		CombatMode = !CombatMode;
		ShipPawn->SetCombatMode(CombatMode);
		SetExternalCamera(false, true);
		if(!CombatMode)
		{
			ResetMousePosition();
		}
	}
}

void AFlarePlayerController::TogglePilot()
{
	bool NewState = !ShipPawn->IsPilotMode();
	FLOGV("AFlarePlayerController::TooglePilot : new state is %d", NewState);
	ShipPawn->EnablePilot(NewState);
}

void AFlarePlayerController::ToggleHUD()
{
	if (!IsInMenu())
	{
		FLOG("AFlarePlayerController::ToggleHUD");
		Cast<AFlareHUD>(GetHUD())->ToggleHUD();
	}
	else
	{
		FLOG("AFlarePlayerController::ToggleHUD : don't do it in menus");
	}
}

void AFlarePlayerController::QuickSwitch()
{
	FLOG("AFlarePlayerController::QuickSwitch");

	TArray<IFlareSpacecraftInterface*>& CompanyShips = Company->GetCompanyShips();

	if (CompanyShips.Num())
	{
		int32 QuickSwitchOffset = QuickSwitchNextOffset;
		int32 OffsetIndex = 0;
		AFlareShip* SeletedCandidate = NULL;

		// First loop in military armed alive ships
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			OffsetIndex = (i + QuickSwitchOffset) % CompanyShips.Num();
			AFlareShip* Candidate = Cast<AFlareShip>(CompanyShips[OffsetIndex]);
			if (Candidate && Candidate != ShipPawn && Candidate->IsAlive() && Candidate->IsMilitary() && Candidate->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
			{
				SeletedCandidate = Candidate;
				break;
			}
		}

		// If not, loop in all alive ships
		if (!SeletedCandidate)
		{
			for (int32 i = 0; i < CompanyShips.Num(); i++)
			{
				OffsetIndex = (i + QuickSwitchOffset) % CompanyShips.Num();
				AFlareShip* Candidate = Cast<AFlareShip>(CompanyShips[OffsetIndex]);
				if (Candidate && Candidate != ShipPawn && Candidate->IsAlive())
				{
					SeletedCandidate = Candidate;
					break;
				}
			}
		}

		// Switch to the found ship
		if (SeletedCandidate)
		{
			FLOG("AFlarePlayerController::QuickSwitch : found new ship");
			FlyShip(SeletedCandidate);
			QuickSwitchNextOffset = OffsetIndex + 1;
			Cast<AFlareHUD>(GetHUD())->OnTargetShipChanged();
		}
		else
		{
			FLOG("AFlarePlayerController::QuickSwitch : no ship found");
		}
	}
	else
	{
		FLOG("AFlarePlayerController::QuickSwitch : no ships in company !");
	}
}

void AFlarePlayerController::Test1()
{
	Notify(FText::FromString("I am Test1"), EFlareNotification::NT_Trading, EFlareMenu::MENU_Dashboard);
}

void AFlarePlayerController::Test2()
{
	Notify(FText::FromString("I am Test2"));
}


#undef LOCTEXT_NAMESPACE
