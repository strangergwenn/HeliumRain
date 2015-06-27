
#include "../Flare.h"
#include "FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareOrbitalEngine.h"
#include "../Spacecrafts/FlareShell.h"
#include "EngineUtils.h"


#define LOCTEXT_NAMESPACE "AFlarePlayerController"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlayerController::AFlarePlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, DustEffect(NULL)
	, Company(NULL)
	, WeaponSwitchTime(10.0f)
	, TimeSinceWeaponSwitch(0)
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
	QuickSwitchNextOffset = 0;
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
	AFlareHUD* HUD = Cast<AFlareHUD>(GetHUD());
	TimeSinceWeaponSwitch += DeltaSeconds;

	if (ShipPawn)
	{
		Cast<AFlareHUD>(GetHUD())->SetInteractive(ShipPawn->GetStateManager()->IsWantContextMenu());
	}

	// Mouse cursor
	bool NewShowMouseCursor = !HUD->IsWheelOpen() ;
	if (!HUD->IsMenuOpen() && ShipPawn && !ShipPawn->GetStateManager()->IsWantCursor())
	{
		NewShowMouseCursor = false;
	}

	if (NewShowMouseCursor != bShowMouseCursor)
	{
		// Set the mouse status
		FLOGV("AFlarePlayerController::PlayerTick : New mouse cursor state is %d", NewShowMouseCursor);
		bShowMouseCursor = NewShowMouseCursor;

		ResetMousePosition();

#if PLATFORM_LINUX
		// This code fix missing cursor bug but cause focus regression.
		if (NewShowMouseCursor)
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
#endif

		// Force focus to UI
		if (NewShowMouseCursor || HUD->IsWheelOpen())
		{
			FInputModeGameAndUI InputMode;
			SetInputMode(InputMode);

			if (!NewShowMouseCursor)
			{
				ULocalPlayer* LocalPlayer = Cast< ULocalPlayer >( Player );

				UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport();
				TSharedPtr<SViewport> ViewportWidget = GameViewportClient->GetGameViewportWidget();
				if (ViewportWidget.IsValid())
				{
					TSharedRef<SViewport> ViewportWidgetRef = ViewportWidget.ToSharedRef();
					LocalPlayer->GetSlateOperations().UseHighPrecisionMouseMovement(ViewportWidgetRef);
				}
			}
		}

		// Force focus to game
		else
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}

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
		UpdateSound(PowerSound,  (ShipPawn->GetDamageSystem()->IsPowered() && !ShipPawn->GetDamageSystem()->HasPowerOutage() ? 1 : -1) * PowerSoundFadeSpeed  * DeltaSeconds, PowerSoundVolume);
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

void AFlarePlayerController::SetExternalCamera(bool NewState)
{
	if (ShipPawn)
	{
		// No internal camera when docked
		if (ShipPawn && ShipPawn->GetNavigationSystem()->IsDocked())
		{
			NewState = true;
		}

		// Send the camera order to the ship
		ShipPawn->GetStateManager()->SetExternalCamera(NewState);
	}
}

void AFlarePlayerController::FlyShip(AFlareSpacecraft* Ship, bool PossessNow)
{
	// Reset the current ship to auto
	if (ShipPawn)
	{
		ShipPawn->GetStateManager()->EnablePilot(true);
	}

	// Fly the new ship
	if(PossessNow)
	{
		Possess(Ship);
	}
	ShipPawn = Ship;
	SetExternalCamera(false);
	ShipPawn->GetStateManager()->EnablePilot(false);
	ShipPawn->GetWeaponsSystem()->DeactivateWeapons();

	// Setup power sound
	FFlareSpacecraftDescription* ShipDescription = Ship->GetDescription();
	if (ShipDescription)
	{
		PowerSound->SetSound(ShipDescription->PowerSound);
	}
	else
	{
		PowerSound->SetSound(NULL);
	}

	// Setup orbital engine sound
	FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
	if (EngineDescription)
	{
		EngineSound->SetSound(EngineDescription->EngineCharacteristics.EngineSound);
	}
	else
	{
		EngineSound->SetSound(NULL);
	}

	// Setup RCS sound
	FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
	if (RCSDescription)
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
		FText Text = FText::FromString(LOCTEXT("Flying", "Now flying").ToString() + " " + FString(*Ship->GetName()));
		FText Info = LOCTEXT("FlyingInfo", "You can switch to nearby ships with N.");
		Notify(Text, Info, EFlareNotification::NT_Help);
		SetSelectingWeapon();
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
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(*ActorItr);
		if (Ship && Ship->GetName() == PlayerData.CurrentShipName)
		{
			FLOGV("AFlarePlayerController::PossessCurrentShip : Found player ship '%s'", *PlayerData.CurrentShipName);
			ShipPawn = Ship;
			break;
		}
	}

	// Possess the ship
	if (ShipPawn)
	{
		FlyShip(ShipPawn);
	}

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

void AFlarePlayerController::Notify(FText Title, FText Info, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo)
{
	FLOGV("AFlarePlayerController::Notify : '%s'", *Title.ToString());
	Cast<AFlareHUD>(GetHUD())->Notify(Title, Info, Type, TargetMenu, TargetInfo);
}

void AFlarePlayerController::SetupMenu()
{
	// Spawn the menu pawn at an arbitrarily large location
	FVector SpawnLocation(5000000 * FVector(1, 1, 1));
	MenuPawn = GetWorld()->SpawnActor<AFlareMenuPawn>(GetGame()->GetMenuPawnClass(), SpawnLocation, FRotator::ZeroRotator);

	// Setup HUD
	AFlareHUD* HUD = Cast<AFlareHUD>(GetHUD());
	if (HUD)
	{
		// Signal the menu to setup as well
		HUD->SetupMenu(PlayerData);
	}
}

void AFlarePlayerController::OnEnterMenu()
{
	if (!IsInMenu())
	{
		ClientPlaySound(OnSound);
		Possess(MenuPawn);

		//Pause all gameplay actors
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if(SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode())
			{
				FLOGV("Pause %s", *SpacecraftCandidate->GetName());
				SpacecraftCandidate->SetPause(true);
			}

			AFlareBomb* BombCandidate = Cast<AFlareBomb>(*ActorItr);
			if(BombCandidate)
			{
				FLOGV("Pause %s", *BombCandidate->GetName());
				BombCandidate->SetPause(true);
			}

			AFlareShell* ShellCandidate = Cast<AFlareShell>(*ActorItr);
			if(ShellCandidate)
			{
				FLOGV("Pause %s", *ShellCandidate->GetName());
				ShellCandidate->SetPause(true);
			}
		}
	}
}

void AFlarePlayerController::OnExitMenu()
{
	if (IsInMenu())
	{
		ClientPlaySound(OffSound);
		Possess(ShipPawn);

		//Unpause all gameplay actors
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* SpacecraftCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if(SpacecraftCandidate && !SpacecraftCandidate->IsPresentationMode())
			{
				FLOGV("UnPause %s", *SpacecraftCandidate->GetName());
				SpacecraftCandidate->SetPause(false);
			}

			AFlareBomb* BombCandidate = Cast<AFlareBomb>(*ActorItr);
			if(BombCandidate)
			{
				FLOGV("UnPause %s", *BombCandidate->GetName());
				BombCandidate->SetPause(false);
			}

			AFlareShell* ShellCandidate = Cast<AFlareShell>(*ActorItr);
			if(ShellCandidate)
			{
				FLOGV("UnPause %s", *ShellCandidate->GetName());
				ShellCandidate->SetPause(false);
			}
		}
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
	auto& App = FSlateApplication::Get();
	FVector2D CursorPos = App.GetCursorPos();
	App.SetCursorPos(CursorPos + FVector2D(0, 1));
	App.OnMouseMove();
	App.SetCursorPos(CursorPos);
	App.OnMouseMove();
	App.SetAllUserFocusToGameViewport();
}

void AFlarePlayerController::SetSelectingWeapon()
{
	TimeSinceWeaponSwitch = 0;
}

bool AFlarePlayerController::IsSelectingWeapon() const
{
	return (TimeSinceWeaponSwitch < WeaponSwitchTime);
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

	InputComponent->BindAction("Wheel", EInputEvent::IE_Pressed, this, &AFlarePlayerController::WheelPressed);
	InputComponent->BindAction("Wheel", EInputEvent::IE_Released, this, &AFlarePlayerController::WheelReleased);

	InputComponent->BindAxis("MouseInputX", this, &AFlarePlayerController::MouseInputX);
	InputComponent->BindAxis("MouseInputY", this, &AFlarePlayerController::MouseInputY);


	InputComponent->BindAction("Test1", EInputEvent::IE_Released, this, &AFlarePlayerController::Test1);
	InputComponent->BindAction("Test2", EInputEvent::IE_Released, this, &AFlarePlayerController::Test2);
}

void AFlarePlayerController::MousePositionInput(FVector2D Val)
{
	if (ShipPawn)
	{
		ShipPawn->GetStateManager()->SetPlayerMousePosition(Val);
	}
}

void AFlarePlayerController::ToggleCamera()
{
	if(ShipPawn)
	{
		SetExternalCamera(!ShipPawn->GetStateManager()->IsExternalCamera());
	}
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
	if (ShipPawn && ShipPawn->IsMilitary() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
	{
		FLOG("AFlarePlayerController::ToggleCombat");
		ShipPawn->GetWeaponsSystem()->ToogleWeaponActivation();
		SetExternalCamera(false);
	}
}

void AFlarePlayerController::TogglePilot()
{
	bool NewState = !ShipPawn->GetStateManager()->IsPilotMode();
	FLOGV("AFlarePlayerController::TooglePilot : new state is %d", NewState);
	ShipPawn->GetStateManager()->EnablePilot(NewState);
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
		AFlareSpacecraft* SeletedCandidate = NULL;

		// First loop in military armed alive ships
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			OffsetIndex = (i + QuickSwitchOffset) % CompanyShips.Num();
			AFlareSpacecraft* Candidate = Cast<AFlareSpacecraft>(CompanyShips[OffsetIndex]);

			if (Candidate && Candidate != ShipPawn
			 && Candidate->GetDamageSystem()->IsAlive()
			 && Candidate->IsMilitary() 
			 && Candidate->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon) > 0)
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
				AFlareSpacecraft* Candidate = Cast<AFlareSpacecraft>(CompanyShips[OffsetIndex]);
				if (Candidate && Candidate != ShipPawn && Candidate->GetDamageSystem()->IsAlive())
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
			QuickSwitchNextOffset = OffsetIndex + 1;
			Cast<AFlareHUD>(GetHUD())->OpenMenu(EFlareMenu::MENU_FlyShip, SeletedCandidate);
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

void AFlarePlayerController::MouseInputX(float Val)
{
	if (Cast<AFlareHUD>(GetHUD())->IsMenuOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->YawInput(Val);
		}
		return;
	}

	if (Cast<AFlareHUD>(GetHUD())->IsWheelOpen())
	{
		Cast<AFlareHUD>(GetHUD())->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		ShipPawn->YawInput(Val);
	}
}

void AFlarePlayerController::MouseInputY(float Val)
{
	if (Cast<AFlareHUD>(GetHUD())->IsMenuOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->PitchInput(Val);
		}
		return;
	}

	if (Cast<AFlareHUD>(GetHUD())->IsWheelOpen())
	{
		Cast<AFlareHUD>(GetHUD())->SetWheelCursorMove(FVector2D(0, -Val));
	}
	else if (ShipPawn)
	{
		ShipPawn->PitchInput(Val);
	}
}

void AFlarePlayerController::Test1()
{
	Notify(FText::FromString("The cake is a lie"), FText::FromString("This is a test of the explanation system."), EFlareNotification::NT_Trading, EFlareMenu::MENU_Dashboard);
}

void AFlarePlayerController::Test2()
{
	Notify(FText::FromString("I am a beautiful butterfly"), FText::FromString("This is a longer, more full of explanation test of the explanation system."));
}


/*----------------------------------------------------
	Wheel menu
----------------------------------------------------*/

void AFlarePlayerController::WheelPressed()
{
	AFlareHUD* HUD = Cast<AFlareHUD>(GetHUD());
	if (HUD && !HUD->IsMenuOpen() && !HUD->IsWheelOpen())
	{
		// Setup mouse menu
		HUD->GetMouseMenu()->ClearWidgets();
		HUD->GetMouseMenu()->AddDefaultWidget("Mouse_Nothing", LOCTEXT("Cancel", "Cancel"));

		// Docked controls
		if (ShipPawn->GetNavigationSystem()->IsDocked())
		{
			HUD->GetMouseMenu()->AddWidget("ShipUpgrade", LOCTEXT("Upgrade", "Upgrade"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UpgradeShip));
			HUD->GetMouseMenu()->AddWidget("Undock", LOCTEXT("Undock", "Undock"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UndockShip));
		}

		// Flying controls
		else
		{
			if (ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_BOMB)
			{
				HUD->GetMouseMenu()->AddWidget("Mouse_Align", LOCTEXT("Align", "Forward"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::AlignToSpeed));
			}

			HUD->GetMouseMenu()->AddWidget("Mouse_Brake", LOCTEXT("Brake", "Brake"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::Brake));

			if (ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_BOMB)
			{
				HUD->GetMouseMenu()->AddWidget("Mouse_Reverse", LOCTEXT("Backward", "Backward"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::AlignToReverse));
			}

			AFlareSpacecraft* Nearest = GetNearestSpacecraft(true);
			if (Nearest)
			{
				FText Text = FText::FromString(LOCTEXT("MatchSpeed", "Match speed with ").ToString() + Nearest->GetName());
				HUD->GetMouseMenu()->AddWidget("Mouse_MatchSpeed", Text,
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::MatchSpeedWithNearestSpacecraft));
			}
			else
			{
				HUD->GetMouseMenu()->AddWidget("Mouse_LookAt", LOCTEXT("FindNearest", "Look at nearest spacecraft"),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::LookAtNearestSpacecraft));
			}

		}

		HUD->SetWheelMenu(true);
	}
}

void AFlarePlayerController::WheelReleased()
{
	Cast<AFlareHUD>(GetHUD())->SetWheelMenu(false);
}

void AFlarePlayerController::AlignToSpeed()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->FaceForward();
	}
}

void AFlarePlayerController::AlignToReverse()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->FaceBackward();
	}
}

void AFlarePlayerController::Brake()
{
	if (ShipPawn)
	{
		ShipPawn->ForceManual();
		ShipPawn->Brake();
	}
}

AFlareSpacecraft* AFlarePlayerController::GetNearestSpacecraft(bool OnScreenRequired)
{
	AFlareSpacecraft* TargetSpacecraft = NULL;
	if (ShipPawn)
	{
		float TargetDistance = 0;
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AFlareSpacecraft* ShipBase = Cast<AFlareSpacecraft>(*ActorItr);
			if (ShipBase && ShipBase != ShipPawn)
			{
				float Distance = (ShipBase->GetActorLocation() - ShipPawn->GetActorLocation()).Size();
				if (!TargetSpacecraft || Distance < TargetDistance)
				{
					FVector2D ScreenPosition;
					if (!OnScreenRequired || ProjectWorldLocationToScreen(ShipBase->GetActorLocation(), ScreenPosition))
					{
						TargetSpacecraft = ShipBase;
						TargetDistance = Distance;
					}
				}
			}
		}
	}

	if (TargetSpacecraft)
	{
		FLOGV("AFlarePlayerController::GetNearestSpacecraft : Found %s", *TargetSpacecraft->GetName());
	}
	return TargetSpacecraft;
}

void AFlarePlayerController::MatchSpeedWithNearestSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = GetNearestSpacecraft(true);
		if (TargetSpacecraft)
		{
			ShipPawn->ForceManual();
			ShipPawn->BrakeToVelocity(TargetSpacecraft->GetLinearVelocity());
		}
	}
}

void AFlarePlayerController::LookAtNearestSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = GetNearestSpacecraft(false);
		if (TargetSpacecraft)
		{
			FVector TargetDirection = (TargetSpacecraft->GetActorLocation() - ShipPawn->GetActorLocation());
			ShipPawn->ForceManual();
			ShipPawn->GetNavigationSystem()->PushCommandRotation(TargetDirection, FVector(1, 0, 0));
		}
	}
}

void AFlarePlayerController::UpgradeShip()
{
	Cast<AFlareHUD>(GetHUD())->OpenMenu(EFlareMenu::MENU_ShipConfig);
}

void AFlarePlayerController::UndockShip()
{
	if (ShipPawn)
	{
		ShipPawn->GetNavigationSystem()->Undock();
	}
}


#undef LOCTEXT_NAMESPACE
