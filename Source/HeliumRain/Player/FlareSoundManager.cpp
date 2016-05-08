
#include "../Flare.h"
#include "FlareSoundManager.h"
#include "FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareOrbitalEngine.h"
#include "AudioDevice.h"


#define LOCTEXT_NAMESPACE "UFlareSoundManager"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSoundManager::UFlareSoundManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Gameplay data
	ShipPawn = NULL;
	MusicVolume = 1.0f;
	MusicChanging = false;
	MusicDesiredTrack = EFlareMusicTrack::None;

	// Music track references
	static ConstructorHelpers::FObjectFinder<USoundCue> HomeMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> ExplorationMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> DangerMusicObk(TEXT("/Game/Master/Music/A_Danger_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> PacificMusicObj(TEXT("/Game/Master/Music/A_Exploration_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> CombatMusicObj(TEXT("/Game/Master/Music/A_Combat_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundCue> WarMusicObj(TEXT("/Game/Master/Music/A_Combat_Cue"));

	// Music track store
	MusicTracks.Add(NULL);
	MusicTracks.Add(HomeMusicObj.Object);
	MusicTracks.Add(ExplorationMusicObj.Object);
	MusicTracks.Add(DangerMusicObk.Object);
	MusicTracks.Add(PacificMusicObj.Object);
	MusicTracks.Add(CombatMusicObj.Object);
	MusicTracks.Add(WarMusicObj.Object);

	// Music sound
	MusicPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("MusicSound"));
	MusicPlayer.Sound->bAutoActivate = false;
	MusicPlayer.Sound->bAutoDestroy = false;
	MusicPlayer.PitchedFade = false;
	MusicPlayer.FadeSpeed = 5.0;
	MusicPlayer.Volume = 0;
	
	// Power sound
	PowerPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("PowerSound"));
	PowerPlayer.Sound->bAutoActivate = false;
	PowerPlayer.Sound->bAutoDestroy = false;
	PowerPlayer.PitchedFade = true;
	PowerPlayer.FadeSpeed = 0.3;
	PowerPlayer.Volume = 0;

	// Engine sound
	EnginePlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EnginePlayer.Sound->bAutoActivate = false;
	EnginePlayer.Sound->bAutoDestroy = false;
	EnginePlayer.PitchedFade = true;
	EnginePlayer.FadeSpeed = 2.0;
	EnginePlayer.Volume = 0;

	// RCS sound
	RCSPlayer.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("RCSSound"));
	RCSPlayer.Sound->bAutoActivate = false;
	RCSPlayer.Sound->bAutoDestroy = false;
	RCSPlayer.PitchedFade = true;
	RCSPlayer.FadeSpeed = 5.0;
	RCSPlayer.Volume = 0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSoundManager::Setup(AFlarePlayerController* Player)
{
	FLOG("UFlareSoundManager::Setup");
	PC = Player;

	// Attach all sounds
	if (Player)
	{
		USceneComponent* RootComponent = Player->GetRootComponent();

		MusicPlayer.Sound->AttachTo(RootComponent);
		MusicPlayer.Sound->SetSound(NULL);

		PowerPlayer.Sound->AttachTo(RootComponent);
		EnginePlayer.Sound->AttachTo(RootComponent);
		RCSPlayer.Sound->AttachTo(RootComponent);
	}
}

void UFlareSoundManager::SetMusicVolume(int32 Volume)
{
	FLOGV("UFlareSoundManager::SetMusicVolume %d", Volume);
	MusicVolume = FMath::Clamp(Volume / 10.0f, 0.0f, 1.0f);
	UpdatePlayer(MusicPlayer, 0, true);
}

void UFlareSoundManager::SetMasterVolume(int32 Volume)
{
	FLOGV("UFlareSoundManager::SetMasterVolume %d", Volume);
	float MasterVolume = FMath::Clamp(Volume / 10.0f, 0.0f, 1.0f);

	FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
	for (auto i = AudioDevice->SoundClasses.CreateIterator(); i; ++i)
	{
		USoundClass* SoundClass = i.Key();
		SoundClass->Properties.Volume = MasterVolume;
	}
}

void UFlareSoundManager::Update(float DeltaSeconds)
{
	// Update the ship if necessary
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
	if (PlayerShip != ShipPawn)
	{
		SetCurrentSpacecraft(PlayerShip);
	}

	// Music management
	if (MusicChanging)
	{
		if (MusicPlayer.Volume > 0)
		{
			UpdatePlayer(MusicPlayer, -DeltaSeconds);
		}
		else
		{
			SetDesiredMusicTrack();
		}
	}
	else if (MusicDesiredTrack == EFlareMusicTrack::None)
	{
		UpdatePlayer(MusicPlayer, -DeltaSeconds);
	}
	else
	{
		UpdatePlayer(MusicPlayer, +DeltaSeconds);
	}

	// Has a ship : update ship sounds
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
		UpdatePlayer(PowerPlayer,  (ShipPawn->GetDamageSystem()->IsPowered() && !ShipPawn->GetDamageSystem()->HasPowerOutage() ? 1 : -1) * DeltaSeconds);
		UpdatePlayer(EnginePlayer, (EngineAlpha > 0 ? EngineAlpha / EngineCount : -1)              * DeltaSeconds);
		UpdatePlayer(RCSPlayer,    (RCSAlpha > 0 ? RCSAlpha / RCSCount : -1)                       * DeltaSeconds);
	}

	// No ship : stop all ship sounds
	else
	{
		UpdatePlayer(PowerPlayer,  -DeltaSeconds);
		UpdatePlayer(EnginePlayer, -DeltaSeconds);
		UpdatePlayer(RCSPlayer,    -DeltaSeconds);
	}
}

void UFlareSoundManager::RequestMusicTrack(EFlareMusicTrack::Type NewTrack)
{
	FLOGV("UFlareSoundManager::RequestMusicTrack : requested %d", (int32)(NewTrack - EFlareMusicTrack::None));
	if (NewTrack != MusicDesiredTrack)
	{
		FLOG("UFlareSoundManager::RequestMusicTrack : switching");
		MusicDesiredTrack = NewTrack;
		MusicChanging = true;
	}
}


/*----------------------------------------------------
	Internal methods
----------------------------------------------------*/

void UFlareSoundManager::SetCurrentSpacecraft(AFlareSpacecraft* Ship)
{
	ShipPawn = Ship;
	FLOG("UFlareSoundManager::SetCurrentSpacecraft : new spacecraft");

	if (Ship)
	{
		// Setup power sound
		FFlareSpacecraftDescription* ShipDescription = Ship->GetDescription();
		PowerPlayer.Sound->SetSound(ShipDescription ? ShipDescription->PowerSound : NULL);

		// Setup orbital engine sound
		FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
		EnginePlayer.Sound->SetSound(EngineDescription ? EngineDescription->EngineCharacteristics.EngineSound : NULL);

		// Setup RCS sound
		FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
		RCSPlayer.Sound->SetSound(RCSDescription ? RCSDescription->EngineCharacteristics.EngineSound : NULL);
	}
}

void UFlareSoundManager::SetDesiredMusicTrack()
{
	int32 TrackIndex = (int32)(MusicDesiredTrack - EFlareMusicTrack::None);
	FLOGV("UFlareSoundManager::SetDesiredMusicTrack : starting %d", TrackIndex);
	check(TrackIndex >= 0 && TrackIndex < MusicTracks.Num() - 1);

	MusicPlayer.Sound->SetSound(MusicTracks[TrackIndex]);
	MusicChanging = false;
}

void UFlareSoundManager::UpdatePlayer(FFlareSoundPlayer& Player, float VolumeDelta, bool Force)
{
	float NewVolume = FMath::Clamp(Player.Volume + VolumeDelta * Player.FadeSpeed, 0.0f, 1.0f);
	if (NewVolume != Player.Volume || Force)
	{
		if (NewVolume == 0)
		{
			Player.Sound->Stop();
		}
		else if (Player.Volume == 0)
		{
			Player.Sound->Play();
		}
		else
		{
			Player.Sound->SetVolumeMultiplier((Player.Sound == MusicPlayer.Sound) ? MusicVolume * NewVolume : NewVolume);
			Player.Sound->SetPitchMultiplier(Player.PitchedFade ? 0.5f + 0.5f * NewVolume : 1.0f);
		}
		Player.Volume = NewVolume;
	}
	else if (Player.Sound == MusicPlayer.Sound && Player.Sound->VolumeMultiplier != MusicVolume * NewVolume)
	{
		Player.Sound->SetVolumeMultiplier(MusicVolume * NewVolume);
	}
}

void UFlareSoundManager::StopPlayer(FFlareSoundPlayer& Player)
{
	Player.Sound->Stop();
	Player.Volume = 0;
}

#undef LOCTEXT_NAMESPACE
