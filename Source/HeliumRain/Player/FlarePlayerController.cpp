
#include "FlarePlayerController.h"
#include "../Flare.h"

#include "FlareHUD.h"
#include "FlareMenuPawn.h"
#include "FlareSoundManager.h"
#include "FlareMenuManager.h"
#include "FlareCockpitManager.h"

#include "../UI/Menus/FlareOrbitalMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/HUD/FlareMouseMenu.h"

#include "../Data/FlareCameraShakeCatalog.h"
#include "../Data/FlareCustomizationCatalog.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlarePlanetarium.h"
#include "../Game/FlareGameUserSettings.h"
#include "../Game/Planetarium/FlareSimulatedPlanetarium.h"
#include "../Game/AI/FlareCompanyAI.h"

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareTurret.h"
#include "../Spacecrafts/FlareShipPilot.h"
#include "../Spacecrafts/FlarePilotHelper.h"
#include "../Spacecrafts/FlareTurretPilot.h"

#include "GameFramework/InputSettings.h"

#include "Engine/PostProcessVolume.h"
#include "Engine/Canvas.h"
#include "Engine.h"
#include "EngineUtils.h"

#include "OnlineSubsystem.h"
#include "OnlineStats.h"
#include "OnlineIdentityInterface.h"
#include "OnlineAchievementsInterface.h"

#include "HighResScreenshot.h"
#include "SceneViewport.h"

DECLARE_CYCLE_STAT(TEXT("FlarePlayerTick ControlGroups"), STAT_FlarePlayerTick_ControlGroups, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlarePlayerTick Battle"), STAT_FlarePlayerTick_Battle, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlarePlayerTick Sound"), STAT_FlarePlayerTick_Sound, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "AFlarePlayerController"

#define SCANNABLES_RESEARCH_GAIN 100


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlarePlayerController::AFlarePlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, LowSpeedEffect(NULL)
	, HighSpeedEffect(NULL)
	, Company(NULL)
	, CompanyBanner(NULL)
	, BannerFont(NULL)
	, AchievementsAvailable(false)
	, WeaponSwitchTime(10.0f)
	, TimeSinceWeaponSwitch(0)
	, CombatZoomFOVRatio(0.4f)
	, WaitingForKey(false)
	, IsBusy(false)
{
	CheatClass = UFlareGameTools::StaticClass();
		
	// Mouse
	static ConstructorHelpers::FObjectFinder<UParticleSystem> LowSpeedEffectTemplateObj(TEXT("/Game/Master/Particles/SpeedEffects/PS_SpeedDust"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> HighSpeedEffectTemplateObj(TEXT("/Game/Master/Particles/SpeedEffects/PS_SpeedDust_Travel"));
	LowSpeedEffectTemplate = LowSpeedEffectTemplateObj.Object;
	HighSpeedEffectTemplate = HighSpeedEffectTemplateObj.Object;
	DefaultMouseCursor = EMouseCursor::Default;

	// Camera shakes
	static ConstructorHelpers::FObjectFinder<UFlareCameraShakeCatalog> CameraShakeCatalogObj(TEXT("/Game/Gameplay/Catalog/CameraShakeCatalog"));
	CameraShakeCatalog = CameraShakeCatalogObj.Object;

	// Font
	static ConstructorHelpers::FObjectFinder<UFont> BannerFontObj(TEXT("/Game/Slate/Fonts/StationFont.StationFont"));
	BannerFont = BannerFontObj.Object;

	// Sound data
	static ConstructorHelpers::FObjectFinder<USoundCue> NotificationInfoSoundObj(TEXT("/Game/Sound/Game/A_NotificationInfo"));
	static ConstructorHelpers::FObjectFinder<USoundCue> NotificationCombatSoundObj(TEXT("/Game/Sound/Game/A_NotificationCombat"));
	static ConstructorHelpers::FObjectFinder<USoundCue> NotificationQuestSoundObj(TEXT("/Game/Sound/Game/A_NotificationQuest"));
	static ConstructorHelpers::FObjectFinder<USoundCue> NotificationTradingSoundObj(TEXT("/Game/Sound/Game/A_NotificationEconomy"));
	static ConstructorHelpers::FObjectFinder<USoundCue> CrashSoundObj(TEXT("/Game/Sound/Impacts/A_Collision"));
	static ConstructorHelpers::FObjectFinder<USoundCue> MissileWarningSoundObj(TEXT("/Game/Sound/Game/A_MissileWarning"));
	
	// Sound
	NotificationInfoSound = NotificationInfoSoundObj.Object;
	NotificationCombatSound = NotificationCombatSoundObj.Object;
	NotificationQuestSound = NotificationQuestSoundObj.Object;
	NotificationTradingSound = NotificationTradingSoundObj.Object;
	CrashSound = CrashSoundObj.Object;
	MissileWarningSound = MissileWarningSoundObj.Object;	

	// Gameplay
	QuickSwitchNextOffset = 0;
	HasCurrentObjective = false;
	RightMousePressed = false;
	LastBattleState.Init();

	// Setup
	ShipPawn = NULL;
	PlayerShip = NULL;

	// Default colors
	CompanyData.CustomizationBasePaintColor = FLinearColor(0.147932,0.167765,0.185); //3
	CompanyData.CustomizationPaintColor = FLinearColor(0.952941, 0.043137, 0.007843); //7
	CompanyData.CustomizationOverlayColor = FLinearColor(0.894444,0.892754,1.0); //4
	CompanyData.CustomizationLightColor = FLinearColor(0.0,0.3,1.0); //13
	CompanyData.CustomizationPatternIndex = 1;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlarePlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnableCheats();
	
	// Get online subsystem
	OnlineSub = IOnlineSubsystem::Get();
	FCHECK(OnlineSub);
	FCHECK(OnlineSub->GetIdentityInterface().IsValid());

	// Get online user
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	UserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);
	FCHECK(UserId->IsValid());

	// Query achievements
	IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
	Achievements->QueryAchievements(*UserId.Get(), FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &AFlarePlayerController::OnQueryAchievementsComplete));

	// Get settings, ensure settings added in updates have the default value
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	FCHECK(MyGameSettings);
	MyGameSettings->EnsureConsistency();

	// Apply settings
	FString GammaCommand = FString::Printf(TEXT("gamma %f"), MyGameSettings->Gamma);
	GEngine->GameViewport->ConsoleCommand(GammaCommand);
	UseCockpit = MyGameSettings->UseCockpit;
	PauseGameInMenus = MyGameSettings->PauseGameInMenus;
	SetUseMotionBlur(MyGameSettings->UseMotionBlur);

	// Gamepad controls
	SetupGamepad();

	// Cockpit
	SetupCockpit();

	// Menu manager
	SetupMenu();
	MenuManager->OpenMenu(EFlareMenu::MENU_Main);

	// Company banner
	CompanyBanner = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 512, 128);
	FCHECK(CompanyBanner);
	CompanyBanner->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlarePlayerController::DrawBanner);
	CompanyBanner->ClearColor = FLinearColor::Black;
	CompanyBanner->UpdateResource();

	// Sound manager
	SoundManager = NewObject<UFlareSoundManager>(this, UFlareSoundManager::StaticClass());
	if (SoundManager)
	{
		SoundManager->Setup(this);
		SoundManager->SetMusicVolume(MyGameSettings->MusicVolume);
		SoundManager->SetMasterVolume(MyGameSettings->MasterVolume);
	}
}

void AFlarePlayerController::PlayerTick(float DeltaSeconds)
{
	Super::PlayerTick(DeltaSeconds);
	AFlareHUD* HUD = GetNavHUD();
	TimeSinceWeaponSwitch += DeltaSeconds;
		
	// We are flying
	if (ShipPawn)
	{
		// Ship groups
		HUD->SetInteractive(ShipPawn->GetStateManager()->IsWantContextMenu());
		{
			SCOPE_CYCLE_COUNTER(STAT_FlarePlayerTick_ControlGroups);

			UFlareSimulatedSector* Sector = ShipPawn->GetParent()->GetCurrentSector();
			GetTacticManager()->ResetControlGroups(Sector);
		}
		
		// Battle state
		if (GetGame()->GetActiveSector())
		{
			SCOPE_CYCLE_COUNTER(STAT_FlarePlayerTick_Battle);

			FFlareSectorBattleState BattleState = GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorBattleState(GetCompany());
			if (BattleState != LastBattleState)
			{
				LastBattleState = BattleState;
				UpdateMusicTrack(BattleState);
			}
		}

		// Exhaust kick
		if (ShipPawn->GetNavigationSystem()->IsUsingOrbitalEngines())
		{
			ClientPlayCameraShake(CameraShakeCatalog->Exhaust);
		}
	}

	// Update FOV
	PlayerCameraManager->SetFOV(GetCurrentFOV());

	// Mouse cursor
	bool NewShowMouseCursor = true;
	if ((!MenuManager->IsUIOpen() && ShipPawn && !ShipPawn->GetStateManager()->IsWantCursor()) || GetNavHUD()->IsWheelMenuOpen())
	{
		NewShowMouseCursor = false;
	}

	if (NewShowMouseCursor != bShowMouseCursor)
	{
		FLOGV("AFlarePlayerController::PlayerTick : New mouse cursor state is %d", NewShowMouseCursor);

		// Set mouse state
		bShowMouseCursor = NewShowMouseCursor;
		
		// Set focus
		if (NewShowMouseCursor)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);

			auto& App = FSlateApplication::Get();
			FVector2D CursorPos = App.GetCursorPos();
			App.SetCursorPos(CursorPos + FVector2D(0, 1));
			App.OnMouseMove();
			App.SetCursorPos(CursorPos);
			App.OnMouseMove();
		}
		else
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}
	
	// Update speed effects
	if (ShipPawn && !IsInMenu())
	{
		// Get ship velocity & direction
		FVector ViewLocation;
		FRotator ViewRotation;
		FVector ShipVelocity = ShipPawn->GetLinearVelocity();
		FVector ShipDirection = ShipVelocity;
		ShipDirection.Normalize();
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		
		// Spawn dust effects if they are not already here
		if (!LowSpeedEffect)
		{
			LowSpeedEffect = UGameplayStatics::SpawnEmitterAtLocation(this, LowSpeedEffectTemplate, ViewLocation, FRotator::ZeroRotator, false);
			FCHECK(LowSpeedEffect);
		}
		if (!HighSpeedEffect)
		{
			HighSpeedEffect = UGameplayStatics::SpawnEmitterAtLocation(this, HighSpeedEffectTemplate, ViewLocation, FRotator::ZeroRotator, false);
			FCHECK(HighSpeedEffect);
		}

		// Update location
		ViewLocation += ShipDirection.Rotation().RotateVector(5000 * FVector(1, 0, 0));
		LowSpeedEffect->SetWorldLocation(ViewLocation);
		HighSpeedEffect->SetWorldLocation(ViewLocation);

		// Update effects
		if (GetPlayerFleet()->IsTraveling())
		{
			// Get the stellar velocity, and multiply it if the player is flying faster than that in reverse
			FVector StellarVelocity = GetGame()->GetPlanetarium()->GetStellarDustVelocity();
			float ShipColinearity = FVector::DotProduct(StellarVelocity.GetSafeNormal(), ShipDirection);
			if (ShipColinearity > 0)
			{
				StellarVelocity = (1 + ShipColinearity) * StellarVelocity;
			}

			// Low speed effect
			LowSpeedEffect->SetFloatParameter("SpawnCount", 0);
			LowSpeedEffect->SetColorParameter("Intensity", FVector::ZeroVector);
			LowSpeedEffect->SetColorParameter("Direction", FVector::ZeroVector);

			// High speed effect
			HighSpeedEffect->SetFloatParameter("SpawnCount", 1.0f);
			HighSpeedEffect->SetColorParameter("Intensity", FLinearColor::White);
			HighSpeedEffect->SetVectorParameter("Direction", StellarVelocity);
			HighSpeedEffect->SetVectorParameter("Size", FVector(1, 1, 1));
		}
		else
		{
			float VelocityFactor = FMath::Clamp(ShipVelocity.Size() / 100.0f, 0.0f, 1.0f);
			FLinearColor Intensity = FLinearColor::White * VelocityFactor;

			// Low speed effect
			LowSpeedEffect->SetFloatParameter("SpawnCount", VelocityFactor);
			LowSpeedEffect->SetColorParameter("Intensity", Intensity);
			LowSpeedEffect->SetVectorParameter("Direction", -ShipDirection);
			LowSpeedEffect->SetVectorParameter("Size", FVector(1, VelocityFactor, 1));

			// High speed effect
			HighSpeedEffect->SetFloatParameter("SpawnCount", 0);
			HighSpeedEffect->SetColorParameter("Intensity", FVector::ZeroVector);
		}
	}
	else if (LowSpeedEffect && HighSpeedEffect)
	{
		// Low speed effect
		LowSpeedEffect->SetFloatParameter("SpawnCount", 0);
		LowSpeedEffect->SetColorParameter("Intensity", FVector::ZeroVector);
		LowSpeedEffect->SetColorParameter("Direction", FVector::ZeroVector);

		// High speed effect
		HighSpeedEffect->SetFloatParameter("SpawnCount", 0);
		HighSpeedEffect->SetColorParameter("Intensity", FVector::ZeroVector);
	}

	// Sound
	if (SoundManager)
	{
		SCOPE_CYCLE_COUNTER(STAT_FlarePlayerTick_Sound);
		SoundManager->Update(DeltaSeconds);
	}

	if (ShipPawn && GetPlayerShip()->GetCurrentSector())
	{
		CheckSectorStateChanges(GetPlayerShip()->GetCurrentSector());
	}
}

float AFlarePlayerController::VerticalToHorizontalFOV(float VerticalFOV) const
{
	float AspectRatio = GetNavHUD()->GetViewportSize().X / GetNavHUD()->GetViewportSize().Y;
	float VerticalFOVRadians = FMath::DegreesToRadians(VerticalFOV);
	float HorizontalFOVRadians = FMath::Atan(FMath::Tan(VerticalFOVRadians / 2) * AspectRatio);
	return 2 * FMath::RadiansToDegrees(HorizontalFOVRadians);
}

float AFlarePlayerController::GetMaxVerticalFOV() const
{
	return 90.0f;
}

float AFlarePlayerController::GetMinVerticalFOV() const
{
	return 60.0f;
}

float AFlarePlayerController::GetMinFOV() const
{
	return VerticalToHorizontalFOV(GetMinVerticalFOV());
}

float AFlarePlayerController::GetNormalFOV() const
{
	int VerticalFOV = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->VerticalFOV;
	return VerticalToHorizontalFOV(VerticalFOV);
}

float AFlarePlayerController::GetCurrentFOV() const
{
	// Zooming just decreases FOV
	int VerticalFOV = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->VerticalFOV;
	if (ShipPawn)
	{
		VerticalFOV *= FMath::Lerp(1.0f, CombatZoomFOVRatio, ShipPawn->GetStateManager()->GetCombatZoomAlpha());
	}

	return VerticalToHorizontalFOV(VerticalFOV);
}

void AFlarePlayerController::SetExternalCamera(bool NewState)
{
	if (ShipPawn)
	{
		// Close main overlay if open outside menu
		if (!MenuManager->IsMenuOpen())
		{
			MenuManager->CloseMainOverlay();
		}

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
	FCHECK(Ship);

	if (ShipPawn == Ship)
	{
		// Already flying this ship
		// Fly the new ship
		if (PossessNow)
		{
			Possess(Ship);
		}

		return;
	}

	// Reset the current ship to autopilot, unless it's a freighter on a player-issued order
	if (ShipPawn)
	{
		ShipPawn->ResetCurrentTarget();

		if (ShipPawn->IsMilitary())
		{
			ShipPawn->GetStateManager()->EnablePilot(true);
		}
		else if (!ShipPawn->GetNavigationSystem()->IsAutoPilot())
		{
			ShipPawn->GetStateManager()->EnablePilot(true);
		}
	}

	// Fly the new ship
	if (PossessNow)
	{
		Ship->ResetCurrentTarget();
		Possess(Ship);
	}

	// Setup everything
	ShipPawn = Ship;
	SetExternalCamera(false);
	//ShipPawn->GetNavigationSystem()->AbortAllCommands();
	ShipPawn->GetStateManager()->EnablePilot(false);
	ShipPawn->GetWeaponsSystem()->DeactivateWeapons();
	CockpitManager->OnFlyShip(ShipPawn);
	
	// Combat groups
	GetTacticManager()->SetCurrentShipGroup(EFlareCombatGroup::AllMilitary);
	GetTacticManager()->ResetShipGroup(EFlareCombatTactic::ProtectMe);

	// Set player ship
	SetPlayerShip(ShipPawn->GetParent());
	if (GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnFlyShip(Ship);
	}

	// Update HUD
	GetNavHUD()->OnTargetShipChanged();
	SetSelectingWeapon();
}

void AFlarePlayerController::ExitShip()
{
	ShipPawn = NULL;
}

void AFlarePlayerController::PrepareForExit()
{
	PlayerShip = NULL;

	if (IsInMenu())
	{
		MenuManager->CloseMenu(true);
	}
}


/*----------------------------------------------------
	Data management
----------------------------------------------------*/

void AFlarePlayerController::SetCompanyDescription(const FFlareCompanyDescription& SaveCompanyData)
{
	CompanyData = SaveCompanyData;

	// Load decorator
	UFlareCustomizationCatalog* CustomizationCatalog = GetGame()->GetCustomizationCatalog();
	BackgroundDecorator = &CustomizationCatalog->PaintPatterns[SaveCompanyData.CustomizationPatternIndex].BackgroundDecorator;
}

void AFlarePlayerController::Load(const FFlarePlayerSave& SavePlayerData)
{
	// Load data
	PlayerData = SavePlayerData;
	Company = GetGame()->GetGameWorld()->FindCompany(PlayerData.CompanyIdentifier);
	PlayerFleet = GetGame()->GetGameWorld()->FindFleet(PlayerData.PlayerFleetIdentifier);

	// Load player emblem
	UFlareCustomizationCatalog* CustomizationCatalog = GetGame()->GetCustomizationCatalog();
	CompanyData.Emblem = CustomizationCatalog->GetEmblem(PlayerData.PlayerEmblemIndex);
	CompanyBanner->UpdateResource();
}

void AFlarePlayerController::OnLoadComplete()
{
	Company->UpdateCompanyCustomization();
}

void AFlarePlayerController::OnSectorActivated(UFlareSector* ActiveSector)
{
	FLOGV("AFlarePlayerController::OnSectorActivated %s", *ActiveSector->GetSimulatedSector()->GetData()->Identifier.ToString());
	bool CandidateFound = false;

	// Last flown ship
	if (PlayerShip && PlayerShip->IsActive())
	{
		// Disable pilot during the switch
		PlayerShip->GetActive()->GetStateManager()->EnablePilot(false);
		FlyShip(PlayerShip->GetActive(), false);
	}
	else
	{
		FLOG("AFlarePlayerController::OnSectorActivated no candidate");
		SwitchToNextShip(true);
	}

	UpdateMusicTrack(FFlareSectorBattleState().Init());
}

void AFlarePlayerController::OnSectorDeactivated()
{
	FLOG("AFlarePlayerController::OnSectorDeactivated");

	// Reset the ship
	CockpitManager->OnStopFlying();
	if (ShipPawn)
	{
		ShipPawn->ResetCurrentTarget();
		ShipPawn = NULL;
	}

	// Reset states
	LastBattleState.Init();
}

void AFlarePlayerController::UpdateMusicTrack(FFlareSectorBattleState NewBattleState)
{
	if (NewBattleState.HasDanger)
	{
		if (NewBattleState.WantFight())
		{
			UFlareSimulatedSector* Sector = GetGame()->GetActiveSector()->GetSimulatedSector();
			if (Sector->GetSectorShips().Num() > 15)
			{
				FLOG("AFlarePlayerController::UpdateMusicTrack : battle");
				SoundManager->RequestMusicTrack(EFlareMusicTrack::Battle);
			}
			else
			{
				FLOG("AFlarePlayerController::UpdateMusicTrack : combat");
				SoundManager->RequestMusicTrack(EFlareMusicTrack::Combat);
			}
		}
		else
		{
			FLOG("AFlarePlayerController::UpdateMusicTrack : battle lost");
			SoundManager->RequestMusicTrack(EFlareMusicTrack::Danger);
		}
	}
	else
	{
		if (GetPlayerFleet()->IsTraveling() || !GetGame()->GetActiveSector())
		{
			FLOG("AFlarePlayerController::UpdateMusicTrack : travel");
			SoundManager->RequestMusicTrack(FMath::RandBool() ? EFlareMusicTrack::Travel1 : EFlareMusicTrack::Travel2);
		}
		else
		{
			EFlareMusicTrack::Type LevelMusic = GetGame()->GetActiveSector()->GetSimulatedSector()->GetDescription()->LevelTrack;
			if (LevelMusic != EFlareMusicTrack::None)
			{
				FLOG("AFlarePlayerController::UpdateMusicTrack : using level music");
				SoundManager->RequestMusicTrack(LevelMusic);
			}
			else if (FMath::RandBool())
			{
				FLOG("AFlarePlayerController::UpdateMusicTrack : ambient1");
				SoundManager->RequestMusicTrack(EFlareMusicTrack::Ambient1);
			}
			else
			{
				FLOG("AFlarePlayerController::UpdateMusicTrack : ambient2");
				SoundManager->RequestMusicTrack(EFlareMusicTrack::Ambient2);
			}
		}
	}
}

void AFlarePlayerController::Save(FFlarePlayerSave& SavePlayerData, FFlareCompanyDescription& SaveCompanyData)
{
	SavePlayerData = PlayerData;
	SaveCompanyData = CompanyData;
}

void AFlarePlayerController::SetCompany(UFlareCompany* NewCompany)
{
	Company = NewCompany;
}

void AFlarePlayerController::SetPlayerShip(UFlareSimulatedSpacecraft* NewPlayerShip)
{
	PlayerData.LastFlownShipIdentifier = NewPlayerShip->GetImmatriculation();
	PlayerShip = NewPlayerShip;
}

void AFlarePlayerController::SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType)
{
	GetNavHUD()->SignalHit(HitSpacecraft, DamageType);
}

void AFlarePlayerController::SpacecraftHit(EFlarePartSize::Type WeaponSize)
{
	EFlarePartSize::Type ShipSize = PlayerShip->GetDescription()->Size;

	if (ShipSize == EFlarePartSize::S && WeaponSize == EFlarePartSize::L)
	{
		ClientPlayCameraShake(CameraShakeCatalog->HitHeavy);
	}
	else if (ShipSize == WeaponSize)
	{
		ClientPlayCameraShake(CameraShakeCatalog->HitNormal);
	}
}

void AFlarePlayerController::SpacecraftCrashed()
{
	ClientPlaySound(CrashSound);

	if (PlayerShip->GetDescription()->Size == EFlarePartSize::S)
	{
		ClientPlayCameraShake(CameraShakeCatalog->ImpactS);
	}
	else
	{
		ClientPlayCameraShake(CameraShakeCatalog->ImpactL);
	}
}

void AFlarePlayerController::MissileFired(AFlareBomb* Bomb)
{
	ClientPlaySound(MissileWarningSound);
	
	Notify(LOCTEXT("MissileFiredAtPlayer", "Incoming missile !"),
		LOCTEXT("MissileFiredAtPlayerInfo", "A missile has been fired at your ship !"),
		FName(*Bomb->GetName()),
		EFlareNotification::NT_MilitarySilent);
}

void AFlarePlayerController::PlayLocalizedSound(USoundCue* Sound, FVector WorldLocation, USceneComponent* Component)
{
	if (MenuManager->IsMenuOpen())
	{
		ClientPlaySound(Sound);
	}
	else if (Component != nullptr)
	{


		UGameplayStatics::SpawnSoundAttached(Sound, Component, NAME_None, WorldLocation, FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, WorldLocation);
	}
}

void AFlarePlayerController::Clean()
{
	PlayerData.UUID = NAME_None;
	PlayerData.ScenarioId = 0;
	PlayerData.CompanyIdentifier = NAME_None;
	PlayerData.LastFlownShipIdentifier = NAME_None;

	ShipPawn = NULL;
	PlayerShip = NULL;
	Company = NULL;

	HasCurrentObjective = false;

	QuickSwitchNextOffset = 0;
	TimeSinceWeaponSwitch = 0;

	LastBattleState.Init();
	LastSectorBattleStates.Empty();

	MenuManager->FlushNotifications();
}

bool AFlarePlayerController::IsScannableUnlocked(FName Identifier)
{
	if (PlayerData.UnlockedScannables.Find(Identifier) != INDEX_NONE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AFlarePlayerController::UnlockScannable(FName Identifier)
{
	FLOGV("AFlarePlayerController::UnlockScannable : unlocking '%s'", *Identifier.ToString());

	Company->GiveResearch(SCANNABLES_RESEARCH_GAIN);

	Notify(LOCTEXT("ScannableUnlocked", "New research data"),
		LOCTEXT("ScannableUnlockedInfo", "Target analyzis revealed valuable data for technology research."),
		FName("scannable-unlocked"),
		EFlareNotification::NT_Info,
		false,
		EFlareMenu::MENU_Technology);

	PlayerData.UnlockedScannables.AddUnique(Identifier);
}


/*----------------------------------------------------
	Gamepad
----------------------------------------------------*/

void RemoveGamepadActionMapping(UInputSettings* InputSettings, FName ActionName)
{
	TArray<FInputActionKeyMapping> Bindings;
	InputSettings->GetActionMappingByName(ActionName, Bindings);

	for (FInputActionKeyMapping& Bind : Bindings)
	{
		if (Bind.Key.ToString().StartsWith("Gamepad"))
		{
			InputSettings->RemoveActionMapping(Bind);
		}
	}
}

void RemoveGamepadAxisMapping(UInputSettings* InputSettings, FName AxisName)
{
	TArray<FInputAxisKeyMapping> Bindings;
	InputSettings->GetAxisMappingByName(AxisName, Bindings);

	for (FInputAxisKeyMapping& Bind : Bindings)
	{
		if (Bind.Key.ToString().StartsWith("Gamepad"))
		{
			InputSettings->RemoveAxisMapping(Bind);
		}
	}
}

void AddGamepadActionMapping(UInputSettings* InputSettings, FName ActionName, FName KeyName)
{
	FInputActionKeyMapping Bind;
	Bind.ActionName = ActionName;
	Bind.Key = FKey(KeyName);
	InputSettings->AddActionMapping(Bind);
}

void AddGamepadAxisMapping(UInputSettings* InputSettings, FName AxisName, FName KeyName, float Scale)
{
	FInputAxisKeyMapping Bind;
	Bind.AxisName = AxisName;
	Bind.Key = FKey(KeyName);
	Bind.Scale = Scale;
	InputSettings->AddAxisMapping(Bind);
}

void AFlarePlayerController::SetupGamepad()
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());

	// Remove previous gamepad mappings (#931)
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	RemoveGamepadAxisMapping(InputSettings, "GamepadMoveVerticalInput");
	RemoveGamepadAxisMapping(InputSettings, "GamepadThrustInput");
	RemoveGamepadAxisMapping(InputSettings, "NormalRollInput");

	// Add gamepad mappings
	AddGamepadAxisMapping(InputSettings, "GamepadMoveHorizontalInput", "Gamepad_LeftX", 1.0f);
	AddGamepadAxisMapping(InputSettings, "GamepadMoveVerticalInput", "Gamepad_LeftY", -1.0f);
	AddGamepadAxisMapping(InputSettings, "GamepadYawInput", "Gamepad_RightX", 1.0f);
	AddGamepadAxisMapping(InputSettings, "GamepadPitchInput", "Gamepad_RightY", -1.0f);
	AddGamepadActionMapping(InputSettings, "NextTarget", "Gamepad_DPad_Right");
	AddGamepadActionMapping(InputSettings, "PreviousWeapon", "Gamepad_DPad_Down");
	AddGamepadActionMapping(InputSettings, "PreviousTarget", "Gamepad_DPad_Left");
	AddGamepadActionMapping(InputSettings, "NextWeapon", "Gamepad_DPad_Up");
	AddGamepadActionMapping(InputSettings, "BackMenu", "Gamepad_FaceButton_Right");
	AddGamepadActionMapping(InputSettings, "EnterMenu", "Gamepad_FaceButton_Bottom");
	AddGamepadActionMapping(InputSettings, "QuickSwitch", "Gamepad_FaceButton_Left");
	AddGamepadActionMapping(InputSettings, "ToggleMenu", "Gamepad_Special_Left");
	AddGamepadActionMapping(InputSettings, "Wheel", "Gamepad_Special_Right");

	// Clear saved controls that will be overridden by profile settings
	RemoveGamepadAxisMapping(InputSettings, "GamepadThrustInput");
	RemoveGamepadAxisMapping(InputSettings, "NormalRollInput");
	RemoveGamepadActionMapping(InputSettings, "CombatZoom");
	RemoveGamepadActionMapping(InputSettings, "StartFire");
	RemoveGamepadActionMapping(InputSettings, "FindTarget");

	// Left handed profile for rear buttons
	if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_LeftHanded)
	{
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_LeftTrigger", 1.0f);
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_RightTrigger", -1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_LeftThumbstick", -1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_RightThumbstick", 1.0f);

		AddGamepadActionMapping(InputSettings, "CombatZoom", "Gamepad_RightShoulder");
		AddGamepadActionMapping(InputSettings, "StartFire", "Gamepad_LeftShoulder");
		AddGamepadActionMapping(InputSettings, "FindTarget", "Gamepad_FaceButton_Top");
	}

	// Profile for playing without thumbsticks
	else if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_RollWithShoulders)
	{
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_LeftTrigger", -1.0f);
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_RightTrigger", 1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_LeftShoulder", -1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_RightShoulder", 1.0f);

		AddGamepadActionMapping(InputSettings, "CombatZoom", "Gamepad_LeftThumbstick");
		AddGamepadActionMapping(InputSettings, "StartFire", "Gamepad_FaceButton_Top");
		AddGamepadActionMapping(InputSettings, "FindTarget", "Gamepad_RightThumbstick");
	}

	// Default & turn-with-left-stick profiles
	else
	{
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_LeftShoulder", -1.0f);
		AddGamepadAxisMapping(InputSettings, "GamepadThrustInput", "Gamepad_RightShoulder", 1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_LeftThumbstick", -1.0f);
		AddGamepadAxisMapping(InputSettings, "NormalRollInput", "Gamepad_RightThumbstick", 1.0f);

		AddGamepadActionMapping(InputSettings, "CombatZoom", "Gamepad_LeftTrigger");
		AddGamepadActionMapping(InputSettings, "StartFire", "Gamepad_RightTrigger");
		AddGamepadActionMapping(InputSettings, "FindTarget", "Gamepad_FaceButton_Top");
	}

	// Save inputs
	InputSettings->SaveKeyMappings();
}


/*----------------------------------------------------
	Menus
----------------------------------------------------*/

void AFlarePlayerController::Notify(FText Title, FText Info, FName Tag, EFlareNotification::Type Type, bool Pinned, EFlareMenu::Type TargetMenu, FFlareMenuParameterData TargetInfo)
{
	FLOGV("AFlarePlayerController::Notify : '%s'", *Title.ToString());

	// Notify
	if (MenuManager->Notify(Title, Info, Tag, Type, Pinned, TargetMenu, TargetInfo))
	{
		// Play sound
		USoundCue* NotifSound = NULL;
		switch (Type)
		{
			case EFlareNotification::NT_Info:            NotifSound = NotificationInfoSound;      break;
			case EFlareNotification::NT_Military:        NotifSound = NotificationCombatSound;    break;
			case EFlareNotification::NT_MilitarySilent:  NotifSound = NULL;                       break;
			case EFlareNotification::NT_Quest:	         NotifSound = NotificationQuestSound;     break;
			case EFlareNotification::NT_NewQuest:        NotifSound = NotificationQuestSound;     break;
			case EFlareNotification::NT_Economy:         NotifSound = NotificationTradingSound;   break;
		}

		if (NotifSound)
		{
			MenuManager->GetPC()->ClientPlaySound(NotifSound);
		}
	}
}

void AFlarePlayerController::SetupCockpit()
{
	if (!CockpitManager)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = Instigator;
		SpawnInfo.ObjectFlags |= RF_Transient;
		CockpitManager = GetWorld()->SpawnActor<AFlareCockpitManager>(AFlareCockpitManager::StaticClass(), SpawnInfo);
	}
	CockpitManager->SetupCockpit(this);
}

void AFlarePlayerController::SetupMenu()
{
	// Save the default pawn
	APawn* DefaultPawn = GetPawn();

	// Get the menu pawn
	TArray<AActor*> MenuPawnCandidates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlareMenuPawn::StaticClass(), MenuPawnCandidates);
	if (MenuPawnCandidates.Num())
	{
		MenuPawn = Cast<AFlareMenuPawn>(MenuPawnCandidates.Last());
	}
	FCHECK(MenuPawn);

	// Put the menu pawn at an arbitrarily large location
	FVector ActorSpawnLocation(1000000 * FVector(-1, -1, -1));
	MenuPawn->SetActorLocation(ActorSpawnLocation);
	Possess(MenuPawn);
	
	// Spawn the menu manager
	BackgroundDecorator = FFlareStyleSet::Get().GetBrush("/Brushes/SB_DefaultBackgroundDecorator");
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;
	MenuManager = GetWorld()->SpawnActor<AFlareMenuManager>(AFlareMenuManager::StaticClass(), SpawnInfo);

	// Setup menus and HUD
	MenuManager->SetupMenu();
	GetNavHUD()->Setup(MenuManager);

	// Destroy the old pawn
	if (DefaultPawn)
	{
		DefaultPawn->Destroy();
	}
}

void AFlarePlayerController::OnEnterMenu()
{
	if (!IsInMenu())
	{
		Possess(MenuPawn);

		// Pause all gameplay actors
		SetWorldPause(true);
		MenuPawn->SetActorHiddenInGame(false);
	}

	GetGame()->GetPlanetarium()->SetActorHiddenInGame(true);
	GetNavHUD()->UpdateHUDVisibility();
}

void AFlarePlayerController::OnExitMenu()
{
	if (IsInMenu())
	{
		// Quit menu
		MenuPawn->SetActorHiddenInGame(true);

		// Unpause all gameplay actors
		SetWorldPause(false);

		// Fly the ship
		Possess(ShipPawn);
		GetNavHUD()->OnTargetShipChanged();
		CockpitManager->OnFlyShip(ShipPawn);
		SetSelectingWeapon();
	}

	if (GetNavHUD())
	{
		GetNavHUD()->UpdateHUDVisibility();
	}
	GetGame()->GetPlanetarium()->SetActorHiddenInGame(false);
}

void AFlarePlayerController::SetWorldPause(bool Pause)
{
	FLOGV("AFlarePlayerController::SetWorldPause world %d", Pause);

	if (PauseGameInMenus && GetGame()->GetActiveSector())
	{
		GetGame()->SetWorldPause(Pause);
		GetGame()->GetActiveSector()->SetPause(Pause);
	}
}

UFlareFleet* AFlarePlayerController::GetPlayerFleet()
{
	if (PlayerFleet)
	{
		return PlayerFleet;
	}
	else if (GetShipPawn())
	{
		return GetShipPawn()->GetParent()->GetCurrentFleet();
	}
	else
	{
		return NULL;
	}
}

bool AFlarePlayerController::SwitchToNextShip(bool Instant)
{
	FLOG("AFlarePlayerController::SwitchToNextShip");

	if (GetGame()->GetActiveSector() && Company)
	{
		TArray<AFlareSpacecraft*> CompanyShips = GetGame()->GetActiveSector()->GetCompanyShips(Company);

		if (CompanyShips.Num())
		{
			FText CantFlyReasons;
			int32 OffsetIndex = 0;
			int32 QuickSwitchOffset = QuickSwitchNextOffset;
			AFlareSpacecraft* SeletedCandidate = NULL;
			FFlareSectorBattleState BattleState = GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorBattleState(Company);

			// First loop in military armed alive ships
			if (BattleState.InBattle)
			{
				for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
				{
					OffsetIndex = (ShipIndex + QuickSwitchOffset) % CompanyShips.Num();
					AFlareSpacecraft* Candidate = CompanyShips[OffsetIndex];

					if (Candidate && Candidate != ShipPawn && Candidate->GetParent()->CanBeFlown(CantFlyReasons) && Candidate->GetParent()->CanFight())
					{
						SeletedCandidate = Candidate;
						break;
					}
				}
			}

			// If not, loop in all alive ships
			if (!SeletedCandidate)
			{
				for (int32 ShipIndex = 0; ShipIndex < CompanyShips.Num(); ShipIndex++)
				{
					OffsetIndex = (ShipIndex + QuickSwitchOffset) % CompanyShips.Num();
					AFlareSpacecraft* Candidate = CompanyShips[OffsetIndex];
					if (Candidate && Candidate != ShipPawn && Candidate->GetParent()->CanBeFlown(CantFlyReasons))
					{
						SeletedCandidate = Candidate;
						break;
					}
				}
			}

			// Switch to the found ship
			if (SeletedCandidate)
			{
				FLOGV("AFlarePlayerController::SwitchToNextShip : found new ship %s", *SeletedCandidate->GetImmatriculation().ToString());
				QuickSwitchNextOffset = OffsetIndex + 1;
				// Disable pilot during the switch
				SeletedCandidate->GetStateManager()->EnablePilot(false);

				if (Instant)
				{
					FlyShip(SeletedCandidate, false);
				}
				else
				{
					FFlareMenuParameterData Data;
					Data.Spacecraft = SeletedCandidate->GetParent();
					MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
				}

				return true;
			}
			else
			{
				FLOG("AFlarePlayerController::SwitchToNextShip : no ship found");
			}
		}
		else
		{
			FLOG("AFlarePlayerController::SwitchToNextShip : no ships in company !");
		}
	}
	else
	{
		FLOG("AFlarePlayerController::SwitchToNextShip : no sector or company !");
	}

	return false;
}

void AFlarePlayerController::GetPlayerShipThreatStatus(bool& IsTargeted, bool& IsFiredUpon, bool& CollidingSoon, bool& ExitingSoon, bool& LowHealth, UFlareSimulatedSpacecraft*& Threat) const
{
	IsTargeted = false;
	IsFiredUpon = false;
	Threat = NULL;

	AFlareSpacecraft* DockSpacecraft = NULL;
	FFlareDockingParameters DockParams;
	FText DockInfo;

	float MaxSDangerDistance = 250000;
	float MaxLDangerDistance = 500000;

	if (GetShipPawn() && GetGame()->GetActiveSector())
	{
		// Threats
		TArray<AFlareSpacecraft*>& Ships = GetGame()->GetActiveSector()->GetShips();
		for (AFlareSpacecraft* Ship : Ships)
		{
			bool IsDangerous = false, IsFiring = false;
			float ShipDistance = (Ship->GetActorLocation() - GetShipPawn()->GetActorLocation()).Size();
			
			// Small ship
			if (ShipDistance < MaxSDangerDistance && Ship->GetDescription()->Size == EFlarePartSize::S)
			{
				IsDangerous = (Ship->GetPilot()->GetPilotTarget().Is(GetShipPawn()));
				IsFiring = IsDangerous && Ship->GetPilot()->IsWantFire();
			}

			// Large ship
			else if (ShipDistance < MaxLDangerDistance && Ship->GetDescription()->Size == EFlarePartSize::L)
			{
				for (auto Weapon : Ship->GetWeaponsSystem()->GetWeaponList())
				{
					UFlareTurret* Turret = Cast<UFlareTurret>(Weapon);
					FCHECK(Turret);

					if (Turret->GetTurretPilot()->GetTurretTarget().Is(GetShipPawn()))
					{
						IsDangerous = true;
						if (Turret->GetTurretPilot()->IsWantFire())
						{
							IsFiring = true;
						}
					}
				}
			}

			// Confirm this ship is working, then flag it
			if (IsDangerous && !Ship->GetParent()->GetDamageSystem()->IsDisarmed() && !Ship->GetParent()->GetDamageSystem()->IsUncontrollable())
			{
				// Is threat
				IsTargeted = true;
				if (!Threat)
				{
					Threat = Ship->GetParent();
				}

				// Is active threat
				if (IsFiring)
				{
					IsFiredUpon = true;
					Threat = Ship->GetParent();
				}
			}
		}

		// Bomb alarm
		if (!IsFiredUpon)
		{
			for (AFlareBomb* Bomb : GetGame()->GetActiveSector()->GetBombs())
			{
				if (Bomb->GetTargetSpacecraft() == GetShipPawn() && Bomb->IsActive() && Bomb->GetDistanceTo(GetShipPawn()) < 200000)
				{
					IsFiredUpon = true;
					break;
				}
			}
		}

		// Low health
		UFlareSimulatedSpacecraftDamageSystem* DamageSystem = GetShipPawn()->GetParent()->GetDamageSystem();
		LowHealth = (DamageSystem->IsCrewEndangered() || DamageSystem->IsUncontrollable()) && DamageSystem->IsAlive();

		// Collision
		UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
		bool NoCollisionRisk = (MyGameSettings->UseAnticollision || MenuManager->IsUIOpen());
		bool DockingInProgress = GetShipPawn()->GetManualDockingProgress(DockSpacecraft, DockParams, DockInfo);
		bool LowSpeed = GetShipPawn()->GetLinearVelocity().Size() < 20;
		bool SafeDocking = DockingInProgress && LowSpeed;

		// Other helpers
		CollidingSoon = PilotHelper::IsAnticollisionImminent(GetShipPawn(), GetShipPawn()->GetAgressiveAnticollisionTime(), 200.f) && !NoCollisionRisk && !SafeDocking;
		ExitingSoon = PilotHelper::IsSectorExitImminent(GetShipPawn(), 15.0f);
	}
}

void AFlarePlayerController::CheckSectorStateChanges(UFlareSimulatedSector* Sector)
{
	FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(GetCompany());
	FText BattleStateText = Sector->GetSectorBattleStateText(GetCompany());


	FFlareSectorBattleState LastSectorBattleState;
	LastSectorBattleState.Init();
	if (LastSectorBattleStates.Contains(Sector))
	{
		LastSectorBattleState = LastSectorBattleStates[Sector];
	}

	// Ignore same states
	if (LastSectorBattleState == BattleState)
	{
		return;
	}

	// Fight in progress with player fleet
	else if (BattleState.HasDanger && Sector == GetPlayerShip()->GetCurrentSector())
	{
		FString NotificationIdStr;
		NotificationIdStr += "battle-state-fight-";
		NotificationIdStr += Sector->GetIdentifier().ToString();
		FName NotificationId(*NotificationIdStr);

		if(BattleState.InFight)
		{
			BattleStateText = FText::Format(LOCTEXT("BattleStateFightFormat", "Your personal fleet is engaged in battle in {0} !"),
											Sector->GetSectorName());
		}
		else if(BattleState.BattleWon)
		{
			BattleStateText = FText::Format(LOCTEXT("BattleStateWonFormat", "Your personal fleet won a battle in {0} !"),
											Sector->GetSectorName());
		}
		else
		{
			BattleStateText = FText::Format(LOCTEXT("BattleStateLostFormat", "Your personal fleet lost a battle in {0} !"),
											Sector->GetSectorName());
		}

		FFlareMenuParameterData Data;
		Data.Sector = Sector;
		Notify(LOCTEXT("BattleStateFight", "Battle"),
			BattleStateText,
			NotificationId,
			EFlareNotification::NT_Military,
			false,
			EFlareMenu::MENU_Sector,
			Data);
	}

	// Battle in progress and ignore victory states that are boring for the player
	else if (BattleState.HasDanger && BattleState.InBattle && !(BattleState.BattleWon || BattleState.ActiveFightWon))
	{
		FString NotificationIdStr;
		NotificationIdStr += "battle-state-in-progress-";
		NotificationIdStr += Sector->GetIdentifier().ToString();
		FName NotificationId(*NotificationIdStr);

		FFlareMenuParameterData Data;
		Data.Sector = Sector;
		Notify(LOCTEXT("BattleStateInProgress", "Battle"),
			FText::Format(LOCTEXT("BattleStateInProgressFormat", "{0} in {1}"),
				BattleStateText,
				Sector->GetSectorName()),
			NotificationId,
			EFlareNotification::NT_Military,
			false,
			EFlareMenu::MENU_Sector,
			Data);
	}

	if (LastSectorBattleStates.Contains(Sector))
	{
		LastSectorBattleStates[Sector] = BattleState;
	}
	else
	{
		LastSectorBattleStates.Add(Sector, BattleState);
	}
}

void AFlarePlayerController::DiscoverSector(UFlareSimulatedSector* Sector, bool MarkedAsVisited, bool NotifyPlayer)
{
	// Discover, visit if needed

	if (MarkedAsVisited)
	{
		if(GetCompany()->IsVisitedSector(Sector))
		{
			// Already visited
			return;
		}
		GetCompany()->VisitSector(Sector);
	}
	else
	{
		if(GetCompany()->IsKnownSector(Sector))
		{
			// Already known
			return;
		}
		GetCompany()->DiscoverSector(Sector);
	}

	// Refresh menus
	if (MenuManager->IsMenuOpen())
	{
		if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
		{
			MenuManager->GetOrbitMenu()->Enter();
		}
		else if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Sector)
		{
			MenuManager->GetSectorMenu()->Enter(Sector);
		}
	}

	// Notify
	if (NotifyPlayer)
	{
		FFlareMenuParameterData Data;
		Data.Sector = Sector;

		Notify(
			LOCTEXT("DiscoveredSector", "New sector discovered"),
			FText::Format(
				LOCTEXT("DiscoveredSectorInfoFormat", "You have discovered a new sector, {0}."),
				Sector->GetSectorName()),
			"discover-sector",
			EFlareNotification::NT_Info,
			false,
			EFlareMenu::MENU_Sector,
			Data);
	}
}

void AFlarePlayerController::SetAchievementProgression(FName Name, float CompletionRatio)
{
	if (AchievementsAvailable)
	{
		FLOGV("AFlarePlayerController::SetAchievementProgression : setting %s to %f completion for user %s",
			*Name.ToString(), CompletionRatio, *UserId->GetHexEncodedString());

		IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
		FOnlineAchievementsWritePtr WriteObject = MakeShareable(new FOnlineAchievementsWrite());
		FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
		WriteObject->SetFloatStat(Name, CompletionRatio);
		Achievements->WriteAchievements(*UserId, WriteObjectRef);
	}
	else
	{
		FLOG("AFlarePlayerController::SetAchievementProgression : achievements are unavailable");
	}	
}

void AFlarePlayerController::IncrementAchievementProgression(FName Name, float AddedRatio)
{
	if (AchievementsAvailable)
	{
		FLOGV("AFlarePlayerController::IncrementAchievementProgression : incrementing %s with %f completion for user %s",
			*Name.ToString(), AddedRatio, *UserId->GetHexEncodedString());

		IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
		FOnlineAchievementsWritePtr WriteObject = MakeShareable(new FOnlineAchievementsWrite());
		FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
		WriteObject->IncrementFloatStat(Name, AddedRatio);
		Achievements->WriteAchievements(*UserId, WriteObjectRef);
	}
	else
	{
		FLOG("AFlarePlayerController::IncrementAchievementProgression : achievements are unavailable");
	}
}

void AFlarePlayerController::ClearAchievementProgression()
{
#if !UE_BUILD_SHIPPING
	if (AchievementsAvailable)
	{
		FLOGV("AFlarePlayerController::ClearAchievementProgression : clearing achievements for user %s", *UserId->GetHexEncodedString());

		IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
		Achievements->ResetAchievements(*UserId);
	}
	else
#endif // !UE_BUILD_SHIPPING
	{
		FLOG("AFlarePlayerController::ClearAchievementProgression : can't clear achievements");
	}
}

void AFlarePlayerController::OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		FLOG("AFlarePlayerController::OnQueryAchievementsComplete");

		TArray<FOnlineAchievement> PlayerAchievements;
		IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
		if (Achievements->GetCachedAchievements(*UserId.Get(), PlayerAchievements) != EOnlineCachedResult::Success || PlayerAchievements.Num() == 0)
		{
			FLOG("AFlarePlayerController::OnQueryAchievementsComplete : no achievements available");
		}
		else
		{
			AchievementsAvailable = true;

			for (int32 Idx = 0; Idx < PlayerAchievements.Num(); ++Idx)
			{
				FLOGV("AFlarePlayerController::OnQueryAchievementsComplete : Achievement %d : %s",
					Idx, *PlayerAchievements[Idx].ToDebugString());
			}
		}
	}
	else
	{
		FLOG("AFlarePlayerController::OnQueryAchievementsComplete : failed to query achievements");
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

void AFlarePlayerController::SetSelectingWeapon()
{
	TimeSinceWeaponSwitch = 0;
}

bool AFlarePlayerController::IsSelectingWeapon() const
{
	return (TimeSinceWeaponSwitch < WeaponSwitchTime);
}

bool AFlarePlayerController::IsTyping() const
{
	const auto& App = FSlateApplication::Get();
	TSharedPtr<SWidget> Widget = App.GetUserFocusedWidget(0);

	if (Widget.IsValid() && Widget.Get()->GetTypeAsString() == "SEditableText")
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AFlarePlayerController::NotifyDockingResult(bool Success, UFlareSimulatedSpacecraft* Target)
{
	if (Success)
	{
		if (Target->GetActive())
		{
			ShipPawn->SetCurrentTarget(Target->GetActive());
		}

		Notify(
			LOCTEXT("DockingGranted", "Docking granted"),
			FText::Format(
				LOCTEXT("DockingGrantedInfoFormat", "Your ship is now automatically docking at {0}. Using manual controls will abort docking."),
				UFlareGameTools::DisplaySpacecraftName(Target)),
			"docking-granted",
			EFlareNotification::NT_Info,
			false);

		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("start-docking").PutName("target", Target->GetImmatriculation()));
	}
	else
	{
		Notify(
			LOCTEXT("DockingDenied", "Docking denied"),
			FText::Format(LOCTEXT("DockingDeniedInfoFormat", "{0} denied your docking request"), UFlareGameTools::DisplaySpacecraftName(Target)),
			"docking-denied",
			EFlareNotification::NT_Info,
			false);
	}
}

void AFlarePlayerController::NotifyDockingComplete(AFlareSpacecraft* DockStation, bool TellUser)
{
	if (!ShipPawn->GetStateManager()->IsExternalCamera())
	{
		SetExternalCamera(true);
	}

	// Reload if we were in a real menu
	EFlareMenu::Type CurrentMenu = MenuManager->GetCurrentMenu();
	if (CurrentMenu != EFlareMenu::MENU_None && CurrentMenu != EFlareMenu::MENU_ReloadSector && CurrentMenu != EFlareMenu::MENU_FastForwardSingle && CurrentMenu != EFlareMenu::MENU_GameOver)
	{
		MenuManager->Reload();
	}

	if (TellUser)
	{
		Notify(
			LOCTEXT("DockingSuccess", "Docking successful"),
			FText::Format(LOCTEXT("DockingSuccessInfoFormat", "Your ship is now docked at {0}"), UFlareGameTools::DisplaySpacecraftName(DockStation->GetParent())),
			"docking-success",
			EFlareNotification::NT_Info);
	}
}

bool AFlarePlayerController::ConfirmFastForward(FSimpleDelegate OnConfirmed, FSimpleDelegate OnCanceled, bool Automatic)
{
	FText ConfirmFFTitleText = LOCTEXT("ConfirmBattleTitle", "BATTLE IN PROGRESS");
	FText ConfirmFFDetailsText;


	// Check for battle
	for (int32 SectorIndex = 0; SectorIndex < GetCompany()->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = GetCompany()->GetKnownSectors()[SectorIndex];

		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(GetCompany());

		if (BattleState.InBattle)
		{
			if (BattleState.InActiveFight)
			{
				ConfirmFFDetailsText = FText::Format(LOCTEXT("ConfirmBattleFormat", "{0}Battle in progress in {1}. Ships will fight and may be lost.\n"),
					ConfirmFFDetailsText, Sector->GetSectorName());
			}
			else if (!BattleState.InFight && !BattleState.BattleWon)
			{
				ConfirmFFTitleText = LOCTEXT("ConfirmBattleLostTitle", "BATTLE LOST");

				if (BattleState.RetreatPossible)
				{
					ConfirmFFDetailsText = FText::Format(LOCTEXT("ConfirmBattleLostRetreatFormat", "{0}Battle lost in {1}. Ships can still retreat.\n"),
						ConfirmFFDetailsText, Sector->GetSectorName());
				}
				else
				{
					if(BattleState.FriendlyControllableShipCount > 0 ||
							(BattleState.FriendlyStationCount> 0 && BattleState.FriendlyStationInCaptureCount == 0))
					{
						ConfirmFFDetailsText = FText::Format(LOCTEXT("ConfirmBattleLostNoRetreatFormat", "{0}Battle lost in {1}. Ships cannot retreat and may be lost.\n"),
							ConfirmFFDetailsText, Sector->GetSectorName());
					}
				}
			}
		}
	}

	if(ConfirmFFDetailsText.ToString().Len() > 0)
	{
		// Is battle
		ConfirmFFTitleText = LOCTEXT("ConfirmBattleTitle", "BATTLE IN PROGRESS");
	}
	else if (Automatic)
	{
		if(GetGame()->GetGameWorld()->GetIncomingEvents().Num() == 0)
		{
			ConfirmFFTitleText = LOCTEXT("ConfirmNoIncomingEventTitle", "NO INCOMING EVENT");
			ConfirmFFDetailsText = LOCTEXT("ConfirmNoIncomingEventFormat", "You may have to stop the fast forward by clicking the button again.\n");
		}
	}

	// Notify when a battle is happening
	if (ConfirmFFDetailsText.ToString().Len())
	{
		MenuManager->Confirm(ConfirmFFTitleText, ConfirmFFDetailsText, OnConfirmed, OnCanceled);
		return false;
	}
	else
	{
		return true;
	}
}

void AFlarePlayerController::NotifyExitSector()
{
	GetMenuManager()->Confirm(
		LOCTEXT("ExitSector", "GO BACK TO SECTOR ?"),
		LOCTEXT("ExitSectorDescription", "Your ship went too far from the sector origin. Hit confirm to navigate back to it and wait one day.\n You can also cancel the return and find a new destination here."),
		FSimpleDelegate::CreateUObject(this, &AFlarePlayerController::OnConfirmBackToSector));
}

void AFlarePlayerController::OnConfirmBackToSector()
{
	if (ConfirmFastForward(FSimpleDelegate::CreateUObject(this, &AFlarePlayerController::SimulateConfirmed), FSimpleDelegate(), false))
	{
		SimulateConfirmed();
	}
}


/*----------------------------------------------------
	Objectives
----------------------------------------------------*/

void AFlarePlayerController::StartObjective(FText Name, FFlarePlayerObjectiveData Data)
{
	HasCurrentObjective = true;
	CurrentObjective = Data;
}

void AFlarePlayerController::CompleteObjective()
{
	HasCurrentObjective = false;
}

bool AFlarePlayerController::HasObjective() const
{
	return HasCurrentObjective;
}

const FFlarePlayerObjectiveData* AFlarePlayerController::GetCurrentObjective() const
{
	return (HasCurrentObjective ? &CurrentObjective : NULL);
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlarePlayerController::DrawBanner(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	if (TargetCanvas && GetCompany())
	{
		FText Text = FText::FromString(GetCompany()->GetShortName().ToString().ToUpper());
		UFlareCustomizationCatalog* CustomizationCatalog = GetGame()->GetCustomizationCatalog();
		const UTexture2D* Icon = Cast<UTexture2D>(CustomizationCatalog->GetEmblemBrush(PlayerData.PlayerEmblemIndex)->GetResourceObject());
		
		// Centering
		float XL, YL;
		TargetCanvas->TextSize(BannerFont, Text.ToString(), XL, YL);
		float X = 0.25f * Width - 0.5f * XL;
		float Y = 0.50f * Height - 0.5f * YL;
		
		// Drawing
		FCanvasTextItem TextItem(FVector2D(X, Y), Text, BannerFont, UFlareSpacecraftComponent::NormalizeColor(CompanyData.CustomizationBasePaintColor));
		TextItem.Scale = FVector2D(1, 1);
		TargetCanvas->DrawItem(TextItem);
		
		// Top line
		int32 LineHeight = Height / 5;
		FVector2D Start = FVector2D(0, 0.1f * Height);
		FVector2D End = FVector2D(Width, 0.1f * Height);
		FCanvasLineItem TopLineItem(Start, End);
		TopLineItem.SetColor(UFlareSpacecraftComponent::NormalizeColor(CompanyData.CustomizationLightColor));
		TopLineItem.LineThickness = LineHeight;
		TargetCanvas->DrawItem(TopLineItem);

		// Bottom line
		Start = FVector2D(0, 0.9f * Height);
		End = FVector2D(Width, 0.9f * Height);
		FCanvasLineItem BottomLineItem(Start, End);
		BottomLineItem.SetColor(UFlareSpacecraftComponent::NormalizeColor(CompanyData.CustomizationPaintColor));
		BottomLineItem.LineThickness = LineHeight;
		TargetCanvas->DrawItem(BottomLineItem);
		
		// Icon
		FCanvasTileItem TileItem(
			FVector2D(0.7f * Width, 0.2f * Height), 
			Icon->Resource,
			FVector2D(0.6 * Height, 0.6 * Height),
			FVector2D(0, 0),
			FVector2D(0 + 1, 0 + 1),
			UFlareSpacecraftComponent::NormalizeColor(CompanyData.CustomizationOverlayColor));
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Translucent);
		TargetCanvas->DrawItem(TileItem);
	}
}

void AFlarePlayerController::SetBasePaintColor(FLinearColor Color)
{
	CompanyData.CustomizationBasePaintColor = Color;

	if (Company)
	{
		Company->UpdateCompanyCustomization();
	}
	CompanyBanner->UpdateResource();
}

void AFlarePlayerController::SetPaintColor(FLinearColor Color)
{
	CompanyData.CustomizationPaintColor = Color;

	if (Company)
	{
		Company->UpdateCompanyCustomization();
	}
	CompanyBanner->UpdateResource();
}

void AFlarePlayerController::SetOverlayColor(FLinearColor Color)
{
	CompanyData.CustomizationOverlayColor = Color;

	if (Company)
	{
		Company->UpdateCompanyCustomization();
	}
	CompanyBanner->UpdateResource();
}

void AFlarePlayerController::SetLightColor(FLinearColor Color)
{
	CompanyData.CustomizationLightColor = Color;

	if (Company)
	{
		Company->UpdateCompanyCustomization();
	}
	CompanyBanner->UpdateResource();
}

void AFlarePlayerController::SetPatternIndex(int32 Index)
{
	CompanyData.CustomizationPatternIndex = Index;

	UFlareCustomizationCatalog* CustomizationCatalog = GetGame()->GetCustomizationCatalog();
	BackgroundDecorator = &CustomizationCatalog->PaintPatterns[Index].BackgroundDecorator;

	if (Company)
	{
		Company->UpdateCompanyCustomization();
	}
	CompanyBanner->UpdateResource();
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlarePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Main keys
	InputComponent->BindAction("ToggleCamera", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCamera);
	InputComponent->BindAction("ToggleMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleMenu);
	InputComponent->BindAction("ToggleOverlay", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleOverlay);
	InputComponent->BindAction("EnterMenu", EInputEvent::IE_Pressed, this, &AFlarePlayerController::EnterMenuDown);
	InputComponent->BindAction("EnterMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::EnterMenuUp);
	InputComponent->BindAction("BackMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::BackMenu);
	InputComponent->BindAction("Simulate", EInputEvent::IE_Released, this, &AFlarePlayerController::Simulate);
	InputComponent->BindAction("ToggleCombat", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleCombat);
	InputComponent->BindAction("EnablePilot", EInputEvent::IE_Released, this, &AFlarePlayerController::EnablePilot);
	InputComponent->BindAction("DisengagePilot", EInputEvent::IE_Released, this, &AFlarePlayerController::DisengagePilot);
	InputComponent->BindAction("ToggleHUD", EInputEvent::IE_Released, this, &AFlarePlayerController::ToggleHUD);
	InputComponent->BindAction("QuickSwitch", EInputEvent::IE_Released, this, &AFlarePlayerController::QuickSwitch);
#if !UE_BUILD_SHIPPING
	InputComponent->BindAction("TogglePerformance", EInputEvent::IE_Released, this, &AFlarePlayerController::TogglePerformance);
#endif

	// Menus
	InputComponent->BindAction("ShipMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::ShipMenu);
	InputComponent->BindAction("SectorMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::SectorMenu);
	InputComponent->BindAction("OrbitMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::OrbitMenu);
	InputComponent->BindAction("EconomyMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::EconomyMenu);
	InputComponent->BindAction("LeaderboardMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::LeaderboardMenu);
	InputComponent->BindAction("CompanyMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::CompanyMenu);
	InputComponent->BindAction("FleetMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::FleetMenu);
	InputComponent->BindAction("TechnologyMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::TechnologyMenu);
	InputComponent->BindAction("QuestMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::QuestMenu);
	InputComponent->BindAction("MainMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::MainMenu);
	InputComponent->BindAction("SettingsMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::SettingsMenu);
	InputComponent->BindAction("TradeMenu", EInputEvent::IE_Released, this, &AFlarePlayerController::TradeMenu);

	// Hotkeys
	InputComponent->BindAction("SpacecraftKey1", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey1);
	InputComponent->BindAction("SpacecraftKey2", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey2);
	InputComponent->BindAction("SpacecraftKey3", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey3);
	InputComponent->BindAction("SpacecraftKey4", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey4);
	InputComponent->BindAction("SpacecraftKey5", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey5);
	InputComponent->BindAction("SpacecraftKey6", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey6);
	InputComponent->BindAction("SpacecraftKey7", EInputEvent::IE_Released, this, &AFlarePlayerController::SpacecraftKey7);

	// Mouse
	InputComponent->BindAction("Wheel", EInputEvent::IE_Pressed, this, &AFlarePlayerController::WheelPressed);
	InputComponent->BindAction("Wheel", EInputEvent::IE_Released, this, &AFlarePlayerController::WheelReleased);
	InputComponent->BindAxis("MouseInputX", this, &AFlarePlayerController::MouseInputX);
	InputComponent->BindAxis("MouseInputY", this, &AFlarePlayerController::MouseInputY);

	// Gamepad
	InputComponent->BindAxis("GamepadMoveHorizontalInput", this, &AFlarePlayerController::GamepadLeftStickX);
	InputComponent->BindAxis("GamepadMoveVerticalInput", this, &AFlarePlayerController::GamepadLeftStickY);
	InputComponent->BindAxis("GamepadYawInput", this, &AFlarePlayerController::GamepadRightStickX);
	InputComponent->BindAxis("GamepadPitchInput", this, &AFlarePlayerController::GamepadRightStickY);
	InputComponent->BindAxis("GamepadThrustInput", this, &AFlarePlayerController::GamepadThrustInput);

	// Joystick
	InputComponent->BindAxis("JoystickMoveHorizontalInput", this, &AFlarePlayerController::JoystickMoveHorizontalInput);
	InputComponent->BindAxis("JoystickMoveVerticalInput", this, &AFlarePlayerController::JoystickMoveVerticalInput);
	InputComponent->BindAxis("JoystickYawInput", this, &AFlarePlayerController::JoystickYawInput);
	InputComponent->BindAxis("JoystickPitchInput", this, &AFlarePlayerController::JoystickPitchInput);

	// Hack for right mouse button triggering drag in external camera
	InputComponent->BindAction("RightMouseButton", EInputEvent::IE_Pressed, this, &AFlarePlayerController::RightMouseButtonPressed);
	InputComponent->BindAction("RightMouseButton", EInputEvent::IE_Released, this, &AFlarePlayerController::RightMouseButtonReleased);

	// Others
	InputComponent->BindAction("HighResShot", EInputEvent::IE_Released, this, &AFlarePlayerController::TakeHighResScreenshot);
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
	if (ShipPawn && ShipPawn->GetParent()->GetDamageSystem()->IsAlive() && !MenuManager->IsMenuOpen() && !IsTyping())
	{
		SetExternalCamera(!ShipPawn->GetStateManager()->IsExternalCamera());
		ClientPlaySound(GetSoundManager()->TickSound);
	}
}

void AFlarePlayerController::ToggleMenu()
{
	if (GetGame()->IsLoadedOrCreated() && ShipPawn && GetGame()->GetActiveSector() && !IsTyping() && !MenuManager->IsFading())
	{
		if (MenuManager->IsMenuOpen())
		{
			MenuManager->CloseMenu();
			MenuManager->CloseMainOverlay();
			ClientPlaySound(GetSoundManager()->NegativeClickSound);
		}
		else if (MenuManager->IsOverlayOpen())
		{
			MenuManager->CloseMainOverlay();
		}
		else
		{
			MenuManager->OpenMainOverlay();
			ClientPlaySound(GetSoundManager()->TickSound);
		}
	}
}

void AFlarePlayerController::ToggleOverlay()
{
	if (GetGame()->IsLoadedOrCreated() && ShipPawn && GetGame()->GetActiveSector() && !MenuManager->IsMenuOpen() && !IsTyping())
	{
		// Close mouse menu
		if (GetNavHUD()->IsWheelMenuOpen())
		{
			GetNavHUD()->SetWheelMenu(false, false);
		}

		// Toggle overlay
		if (MenuManager->IsOverlayOpen())
		{
			MenuManager->CloseMainOverlay();
		}
		else if (!MenuManager->IsUIOpen())
		{
			MenuManager->OpenMainOverlay();
			ClientPlaySound(GetSoundManager()->TickSound);
		}
	}
}

void AFlarePlayerController::EnterMenuDown()
{
	auto& App = FSlateApplication::Get();
	TSharedPtr<FGenericWindow> GenWindow;
	App.OnMouseDown(GenWindow, EMouseButtons::Left);
}

void AFlarePlayerController::EnterMenuUp()
{
	auto& App = FSlateApplication::Get();
	App.OnMouseUp(EMouseButtons::Left);
	App.SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
}

void AFlarePlayerController::BackMenu()
{
	if (!IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::BackMenu");
		MenuManager->Back();
	}
}

void AFlarePlayerController::SetBusy(bool Busy)
{
	FLOGV("AFlarePlayerController::SetBusy : %d", Busy);
	IsBusy = Busy;
}

void AFlarePlayerController::Simulate()
{
	if (!GetGame()->IsSkirmish()
	 && GetGame()->IsLoadedOrCreated() && !GetNavHUD()->IsWheelMenuOpen() && !IsTyping()
	 && GetGame()->IsLoadedOrCreated() && !MenuManager->IsSwitchingMenu() && !GetNavHUD()->IsWheelMenuOpen() && !IsTyping())
	{
		FLOG("AFlarePlayerController::Simulate");
		bool CanGoAhead = ConfirmFastForward(FSimpleDelegate::CreateUObject(this, &AFlarePlayerController::SimulateConfirmed), FSimpleDelegate(), false);
		if (CanGoAhead)
		{
			SimulateConfirmed();
		}
	}
}

void AFlarePlayerController::SimulateConfirmed()
{
	// Menu version : simple synchronous code
	if (MenuManager->IsMenuOpen())
	{
		FLOG("AFlarePlayerController::SimulateConfirmed : synchronous");

		// Do the FF and reload
		GetGame()->DeactivateSector();
		MenuManager->GetGame()->GetGameWorld()->Simulate();
		MenuManager->Reload();
		GetGame()->ActivateCurrentSector();

		// Notify date
		MenuManager->GetPC()->Notify(LOCTEXT("NewDate", "A day passed by..."),
			UFlareGameTools::GetDisplayDate(MenuManager->GetGame()->GetGameWorld()->GetDate()),
			FName("new-date-ff"));
	}

	// Asynchronous mode when flying
	else
	{
		FLOG("AFlarePlayerController::SimulateConfirmed : asynchronous");
		MenuManager->OpenMenu(EFlareMenu::MENU_FastForwardSingle);
	}
}

#if !UE_BUILD_SHIPPING
void AFlarePlayerController::TogglePerformance()
{
	GetNavHUD()->TogglePerformance();
}
#endif

void AFlarePlayerController::SetWaitingForKey(bool State)
{
	WaitingForKey = State;
}

void AFlarePlayerController::ShipMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::ShipMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Ship);
	}
}

void AFlarePlayerController::SectorMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::SectorMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Sector);
	}
}

void AFlarePlayerController::OrbitMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::OrbitMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Orbit);
	}
}

void AFlarePlayerController::EconomyMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::EconomyMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_WorldEconomy);
	}
}

void AFlarePlayerController::LeaderboardMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::LeaderboardMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Leaderboard);
	}
}

void AFlarePlayerController::CompanyMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::CompanyMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Company);
	}
}

void AFlarePlayerController::FleetMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::FleetMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Fleet);
	}
}

void AFlarePlayerController::TechnologyMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::TechnologyMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Technology);
	}
}

void AFlarePlayerController::QuestMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !GetGame()->IsSkirmish() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::QuestMenu");
		MenuManager->ToggleMenu(EFlareMenu::MENU_Quest);
	}
}

void AFlarePlayerController::MainMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping() && !MenuManager->IsFading() && MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Main)
	{
		FLOG("AFlarePlayerController::MainMenu");
		MenuManager->OpenMenu(EFlareMenu::MENU_Main);
	}
}

void AFlarePlayerController::SettingsMenu()
{
	if (!IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::SettingsMenu");
		if (GetGame()->IsLoadedOrCreated())
		{
			MenuManager->ToggleMenu(EFlareMenu::MENU_Settings);
		}
		else if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Settings)
		{
			MenuManager->OpenMenu(EFlareMenu::MENU_Settings);
		}
	}
}

void AFlarePlayerController::TradeMenu()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping() && !MenuManager->IsFading())
	{
		FLOG("AFlarePlayerController::TradeMenu");

		// Is a battle in progress ?
		bool IsBattleInProgress = false;
		if (GetPlayerShip()->GetCurrentSector())
		{
			IsBattleInProgress = GetPlayerShip()->GetCurrentSector()->IsPlayerBattleInProgress();
		}

		// Trade if possible
		if (ShipPawn && ShipPawn->GetNavigationSystem()->IsDocked() && !IsBattleInProgress && ShipPawn->GetParent()->GetDescription()->CargoBayCount > 0)
		{
			StartTrading();
		}
	}
}


void AFlarePlayerController::SpacecraftKey1()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(1);
		}
		else
		{
			ShipPawn->DeactivateWeapon();
		}
	}
}

void AFlarePlayerController::SpacecraftKey2()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(2);
		}
		else
		{
			ShipPawn->ActivateWeaponGroup1();
		}
	}
}

void AFlarePlayerController::SpacecraftKey3()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(3);
		}
		else
		{
			ShipPawn->ActivateWeaponGroup2();
		}
	}
}

void AFlarePlayerController::SpacecraftKey4()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(4);
		}
		else
		{
			ShipPawn->ActivateWeaponGroup3();
		}
	}
}

void AFlarePlayerController::SpacecraftKey5()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(5);
		}
	}
}

void AFlarePlayerController::SpacecraftKey6()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(6);
		}
	}
}

void AFlarePlayerController::SpacecraftKey7()
{
	if (GetGame()->IsLoadedOrCreated() && !IsTyping())
	{
		if (IsInMenu())
		{
			MenuManager->SpacecraftInfoHotkey(7);
		}
	}
}

void AFlarePlayerController::ToggleCombat()
{
	if (ShipPawn && ShipPawn->GetParent()->IsMilitary() && !IsTyping() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
	{
		FLOG("AFlarePlayerController::ToggleCombat");
		ShipPawn->GetWeaponsSystem()->ToggleWeaponActivation();
		if (ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_MISSILE || ShipPawn->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			SetExternalCamera(false);
		}
	}
}

void AFlarePlayerController::EnablePilot()
{
	if (ShipPawn && !IsTyping() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
	{
		FLOG("AFlarePlayerController::EnablePilot");

		bool EnableRegularPilot = true;

		// Try docking at target station
		if (GetCompany()->IsTechnologyUnlocked("auto-docking"))
		{
			AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
			if (TargetSpacecraft && TargetSpacecraft->IsStation())
			{
				bool DockingConfirmed = ShipPawn->GetNavigationSystem()->DockAt(TargetSpacecraft);
				NotifyDockingResult(DockingConfirmed, TargetSpacecraft->GetParent());
				if (DockingConfirmed)
				{
					EnableRegularPilot = false;
				}
			}
		}

		// Else use pilot
		if (EnableRegularPilot)
		{
			ShipPawn->GetStateManager()->EnablePilot(true);
			Notify(
				LOCTEXT("AutopilotEngaged", "Autopilot engaged"),
				LOCTEXT("AutopilotEngagedInfo", "Your ship is now flying on its own. Use any manual input to regain control."),
				"autopilot-engaged",
				EFlareNotification::NT_Info,
				false);
		}
	}
}

void AFlarePlayerController::DisengagePilot()
{
	if (ShipPawn && !IsTyping() && !ShipPawn->GetNavigationSystem()->IsDocked() && !IsInMenu())
	{
		FLOG("AFlarePlayerController::DisengagePilot");
		ShipPawn->GetStateManager()->EnablePilot(false);
	}
}

void AFlarePlayerController::ToggleHUD()
{
	if (!IsInMenu() && !IsTyping())
	{
		FLOG("AFlarePlayerController::ToggleHUD");
		GetNavHUD()->ToggleHUD();
	}
	else
	{
		FLOG("AFlarePlayerController::ToggleHUD : don't do it in menus");
	}
}

void AFlarePlayerController::QuickSwitch()
{
	if (!MenuManager->IsMenuOpen() && !IsTyping())
	{
		SwitchToNextShip(false);

		if (GetGame()->GetQuestManager())
		{
			GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("quick-switch"));
		}
	}
}

void AFlarePlayerController::MouseInputX(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (MenuManager->IsUIOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->YawInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn && !RightMousePressed)
	{
		ShipPawn->YawInput(Val);
	}
}

void AFlarePlayerController::MouseInputY(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (MenuManager->IsUIOpen())
	{
		if (MenuPawn)
		{
			MenuPawn->PitchInput(Val);
		}
		return;
	}

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, -Val));
	}
	else if (ShipPawn && !RightMousePressed)
	{
		ShipPawn->PitchInput(Val);
	}
}

void AFlarePlayerController::GamepadLeftStickX(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_TurnWithLeftStick)
		{
			ShipPawn->GamepadYawInput(Val);
		}
		else
		{
			ShipPawn->GamepadMoveHorizontalInput(Val);
		}
	}
}

void AFlarePlayerController::GamepadLeftStickY(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, Val));
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(0, Val));
	}
	else if (ShipPawn)
	{
		if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_TurnWithLeftStick)
		{
			ShipPawn->GamepadPitchInput(-Val);
		}
		else
		{
			ShipPawn->GamepadMoveVerticalInput(-Val);
		}
	}
}

void AFlarePlayerController::GamepadRightStickX(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (MenuManager->IsMenuOpen())
	{
		// Do nothing, scroll mode
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_TurnWithLeftStick)
		{
			ShipPawn->GamepadMoveHorizontalInput(Val);
		}
		else
		{
			ShipPawn->GamepadYawInput(Val);
		}
	}
}

void AFlarePlayerController::GamepadRightStickY(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, -Val));
	}
	else if (MenuManager->IsMenuOpen())
	{
		if (!WaitingForKey)
		{
			auto& App = FSlateApplication::Get();
			App.OnMouseWheel(Val);
		}
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(0, -Val));
	}
	else if (ShipPawn)
	{
		if (MyGameSettings->GamepadProfileLayout == EFlareGamepadLayout::GL_TurnWithLeftStick)
		{
			ShipPawn->GamepadMoveVerticalInput(Val);
		}
		else
		{
			ShipPawn->GamepadPitchInput(Val);
		}
	}
}

void AFlarePlayerController::GamepadThrustInput(float Val)
{
	if (ShipPawn && !MenuManager->IsUIOpen())
	{
		ShipPawn->GamepadThrustInput(-Val);
	}
}


void AFlarePlayerController::JoystickMoveHorizontalInput(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (MenuManager->IsMenuOpen())
	{
		// Do nothing, scroll mode
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		ShipPawn->JoystickMoveHorizontalInput(Val);
	}
}

void AFlarePlayerController::JoystickMoveVerticalInput(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, -Val));
	}
	else if (MenuManager->IsMenuOpen())
	{
		if (!WaitingForKey)
		{
			auto& App = FSlateApplication::Get();
			App.OnMouseWheel(Val);
		}
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(0, -Val));
	}
	else if (ShipPawn)
	{
		ShipPawn->JoystickMoveVerticalInput(Val);
	}
}

void AFlarePlayerController::JoystickYawInput(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(Val, 0));
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(Val, 0));
	}
	else if (ShipPawn)
	{
		ShipPawn->JoystickYawInput(Val);
	}
}

void AFlarePlayerController::JoystickPitchInput(float Val)
{
	UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
	Val *= MyGameSettings->InputSensitivity;

	if (GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelCursorMove(FVector2D(0, Val));
	}
	else if (MenuManager->IsUIOpen())
	{
		MenuManager->JoystickCursorMove(FVector2D(0, Val));
	}
	else if (ShipPawn)
	{
		ShipPawn->JoystickPitchInput(Val);
	}
}


void AFlarePlayerController::RightMouseButtonPressed()
{
	RightMousePressed = true;
}

void AFlarePlayerController::RightMouseButtonReleased()
{
	RightMousePressed = false;
}

void AFlarePlayerController::TakeHighResScreenshot()
{
	if (!MenuManager->IsMenuOpen() && !IsTyping())
	{
		if (GetHighResScreenshotConfig().FHighResScreenshotConfig::ParseConsoleCommand(FString::SanitizeFloat(2.f), *GLog))
		{
			if (GEngine->GameViewport->Viewport->TakeHighResScreenShot())
			{
				FLOG("AFlarePlayerController::TakeHighResScreenShot done");
				ClientPlaySound(GetSoundManager()->BellSound);
			}
			else
			{
				FLOG("AFlarePlayerController::TakeHighResScreenShot fail");
			}
		}
		else
		{
			FLOG("AFlarePlayerController::TakeHighResScreenShot fail configuration");
		}

	}
}


/*----------------------------------------------------
	Wheel menu
----------------------------------------------------*/

void AFlarePlayerController::WheelPressed()
{
	// Force close the overlay
	if (MenuManager->IsOverlayOpen() && !MenuManager->IsMenuOpen())
	{
		MenuManager->CloseMainOverlay();
	}

	if (GetGame()->IsLoadedOrCreated() && MenuManager && !MenuManager->IsMenuOpen() && !GetNavHUD()->IsWheelMenuOpen())
	{
		TSharedPtr<SFlareMouseMenu> MouseMenu = GetNavHUD()->GetMouseMenu();

		// Setup mouse menu
		MouseMenu->ClearWidgets();
		MouseMenu->AddDefaultWidget("Mouse_Nothing", LOCTEXT("Cancel", "Cancel"));

		// Is a battle in progress ?
		bool IsBattleInProgress = false;
		if (GetPlayerShip()->GetCurrentSector())
		{
			IsBattleInProgress = GetPlayerShip()->GetCurrentSector()->IsPlayerBattleInProgress();
		}

		// Docked controls
		if (ShipPawn->GetNavigationSystem()->IsDocked())
		{
			AFlareSpacecraft* Target = ShipPawn->GetNavigationSystem()->GetDockStation();

			if (!IsBattleInProgress)
			{
				// Buy ship / details
				if (Target->GetParent()->IsShipyard())
				{
					MouseMenu->AddWidget("Mouse_Fly", LOCTEXT("Buy ship", "Buy ship"),
						FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::InspectTargetSpacecraft));
				}
				else
				{
					FText Text = FText::Format(LOCTEXT("InspectDockedFormat", "Details for {0}"), UFlareGameTools::DisplaySpacecraftName(Target->GetParent()));
					MouseMenu->AddWidget(Target->GetParent()->IsStation() ? "Mouse_Inspect_Station" : "Mouse_Inspect_Ship", Text,
						FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::InspectTargetSpacecraft));
				}

				// Trade if possible
				if (ShipPawn->GetParent()->GetDescription()->CargoBayCount > 0)
				{
					MouseMenu->AddWidget("Trade_Button", LOCTEXT("Trade", "Trade"),
						FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::StartTrading));
				}
			}

			// Undock
			MouseMenu->AddWidget("Undock_Button", LOCTEXT("Undock", "Undock"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UndockShip));

			// Upgrade if possible
			AFlareSpacecraft* DockStation = ShipPawn->GetNavigationSystem()->GetDockStation();
			if (!IsBattleInProgress
			 && ShipPawn->GetParent()->GetCurrentSector()->CanUpgrade(ShipPawn->GetParent()->GetCompany())
			 && ShipPawn->GetNavigationSystem()->IsDocked() && DockStation->GetParent()->HasCapability(EFlareSpacecraftCapability::Upgrade))
			{
				MouseMenu->AddWidget("ShipUpgrade_Button", LOCTEXT("Upgrade", "Upgrade"),
				FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::UpgradeShip));
			}
		}

		// Flying controls
		else
		{
			// Targeting
			AFlareSpacecraft* Target = ShipPawn->GetCurrentTarget().SpacecraftTarget;
			if (Target)
			{
				FText Text;

				// Civilian controls
				if (!IsBattleInProgress)
				{
					// Inspect
					Text = FText::Format(LOCTEXT("InspectTargetFormat", "Details for {0}"), UFlareGameTools::DisplaySpacecraftName(Target->GetParent()));
					MouseMenu->AddWidget(Target->GetParent()->IsStation() ? "Mouse_Inspect_Station" : "Mouse_Inspect_Ship", Text,
						FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::InspectTargetSpacecraft));

					// Look at
					Text = FText::Format(LOCTEXT("LookAtTargetFormat", "Focus on {0}"), UFlareGameTools::DisplaySpacecraftName(Target->GetParent()));
					MouseMenu->AddWidget("Mouse_LookAt", Text, FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::LookAtTargetSpacecraft));

					// Dock
					if (Target->GetDockingSystem()->HasCompatibleDock(GetShipPawn())
						&& GetCompany()->IsTechnologyUnlocked("auto-docking")
						&& Target->GetParent()->GetCompany()->GetPlayerWarState() >= EFlareHostility::Neutral)
					{
						Text = FText::Format(LOCTEXT("DockAtTargetFormat", "Dock at {0}"), UFlareGameTools::DisplaySpacecraftName(Target->GetParent()));
						MouseMenu->AddWidget("Mouse_DockAt", Text, FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::DockAtTargetSpacecraft));
					}
				}

				// Fly
				if (Target->GetParent()->GetCompany() == GetCompany() && !Target->GetParent()->IsStation()
					&& Target->GetParent()->GetCurrentFleet() == GetPlayerFleet())
				{
					Text = FText::Format(LOCTEXT("FlyTargetFormat", "Fly {0}"), UFlareGameTools::DisplaySpacecraftName(Target->GetParent()));
					MouseMenu->AddWidget("Mouse_Fly", Text, FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::FlyTargetSpacecraft));
				}
			}

			// Fleet controls
			if (ShipPawn->IsMilitary())
			{
				MouseMenu->AddWidget("Mouse_ProtectMe", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::ProtectMe),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::ProtectMe));
				MouseMenu->AddWidget("Mouse_AttackAll", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackMilitary),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackMilitary));
				MouseMenu->AddWidget("Mouse_AttackStations", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackStations),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackStations));
				MouseMenu->AddWidget("Mouse_AttackCivilians", UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::AttackCivilians),
					FFlareMouseMenuClicked::CreateUObject(this, &AFlarePlayerController::SetTacticForCurrentGroup, EFlareCombatTactic::AttackCivilians));
			}
		}

		GetNavHUD()->SetWheelMenu(true);
	}
}

void AFlarePlayerController::WheelReleased()
{
	if (GetGame()->IsLoadedOrCreated() && MenuManager && !MenuManager->IsUIOpen() && GetNavHUD()->IsWheelMenuOpen())
	{
		GetNavHUD()->SetWheelMenu(false);
	}
}

void AFlarePlayerController::InspectTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
		if (ShipPawn->GetNavigationSystem()->IsDocked())
		{
			TargetSpacecraft = ShipPawn->GetNavigationSystem()->GetDockStation();
		}

		if (TargetSpacecraft)
		{
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft->GetParent();
			MenuManager->OpenMenu(TargetSpacecraft->IsStation() ? EFlareMenu::MENU_Station : EFlareMenu::MENU_Ship, Data);
		}
	}
}

void AFlarePlayerController::FlyTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
		if (TargetSpacecraft)
		{
			// Disable pilot during the switch
			TargetSpacecraft->GetStateManager()->EnablePilot(false);
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft->GetParent();
			MenuManager->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
		}
	}
}

void AFlarePlayerController::DockAtTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
		if (TargetSpacecraft)
		{
			bool DockingConfirmed = ShipPawn->GetNavigationSystem()->DockAt(TargetSpacecraft);
			NotifyDockingResult(DockingConfirmed, TargetSpacecraft->GetParent());
		}
	}
}

void AFlarePlayerController::SetTacticForCurrentGroup(EFlareCombatTactic::Type Tactic)
{
	GetTacticManager()->SetTacticForCurrentShipGroup(Tactic);
}

void AFlarePlayerController::MatchSpeedWithTargetSpacecraft()
{
	if (ShipPawn)
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
		if (TargetSpacecraft)
		{
			DisengagePilot();
			ShipPawn->BrakeToVelocity(TargetSpacecraft->GetLinearVelocity());
		}
	}
}

void AFlarePlayerController::LookAtTargetSpacecraft()
{
	if (ShipPawn && !ShipPawn->GetNavigationSystem()->IsDocked())
	{
		AFlareSpacecraft* TargetSpacecraft = ShipPawn->GetCurrentTarget().SpacecraftTarget;
		if (TargetSpacecraft)
		{
			FVector TargetDirection = (TargetSpacecraft->GetActorLocation() - ShipPawn->GetActorLocation());
			DisengagePilot();
			ShipPawn->GetNavigationSystem()->PushCommandRotation(TargetDirection, FVector(1, 0, 0));
		}
	}
}

void AFlarePlayerController::UpgradeShip()
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = ShipPawn->GetParent();
	MenuManager->OpenMenu(EFlareMenu::MENU_ShipConfig, Data);
}

void AFlarePlayerController::UndockShip()
{
	if (ShipPawn)
	{
		ShipPawn->GetNavigationSystem()->Undock();
	}
}

void AFlarePlayerController::StartTrading()
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = ShipPawn->GetParent();
	MenuManager->OpenMenu(EFlareMenu::MENU_Trade, Data);
}


/*----------------------------------------------------
	Config
----------------------------------------------------*/

void AFlarePlayerController::SetUseCockpit(bool New)
{
	UseCockpit = New;
	CockpitManager->SetupCockpit(this);
	SetExternalCamera(false);
}

void AFlarePlayerController::SetUseMotionBlur(bool New)
{
	UseMotionBlur = New;
	GetGame()->GetPostProcessVolume()->Settings.bOverride_MotionBlurAmount = true;
	GetGame()->GetPostProcessVolume()->Settings.MotionBlurAmount = UseMotionBlur ? 0.3f : 0.0f;
}

void AFlarePlayerController::SetPauseGameInMenus(bool New)
{
	// Unpause if we are disabling the option
	if (New == false)
	{
		SetWorldPause(false);
	}

	PauseGameInMenus = New;

	// Pause if we are setting the option
	if (New == true)
	{
		SetWorldPause(true);
	}
}

void AFlarePlayerController::SetMusicVolume(int32 New)
{
	SoundManager->SetMusicVolume(New);
}

void AFlarePlayerController::SetMasterVolume(int32 New)
{
	SoundManager->SetMasterVolume(New);
}


/*----------------------------------------------------
	Getters for game classes
----------------------------------------------------*/

UFlareSimulatedSpacecraft* AFlarePlayerController::GetPlayerShip()
{
	UFlareSimulatedSpacecraft* Result = NULL;
	UFlareWorld* GameWorld = GetGame()->GetGameWorld();

	if (PlayerShip)
	{
		Result = PlayerShip;
	}
	else if (ShipPawn && ShipPawn->GetParent())
	{
		Result = ShipPawn->GetParent();
	}
	else if (GameWorld)
	{
		Result = GameWorld->FindSpacecraft(PlayerData.LastFlownShipIdentifier);
	}

	return Result;
}

AFlareGame* AFlarePlayerController::GetGame() const
{
	return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
}

AFlareHUD* AFlarePlayerController::GetNavHUD() const
{
	return Cast<AFlareHUD>(GetHUD());
}

#undef LOCTEXT_NAMESPACE
