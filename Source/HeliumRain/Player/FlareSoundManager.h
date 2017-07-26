#pragma once

#include "../Flare.h"

#include "Sound/SoundCue.h"

#include "FlareSoundManager.generated.h"


class AFlareSpacecraft;
class AFlarePlayerController;


/** Music tracks */
UENUM()
namespace EFlareMusicTrack
{
	enum Type
	{
		None,
		Intro,
		Menu,
		Ambient1,
		Ambient2,
		Travel1,
		Travel2,
		Exploration,
		Danger,
		Combat,
		Battle,
		Anomaly1,
		Anomaly2,
		Daedalus1,
		Daedalus2
	};
}


/** Ambient sound player with fading */
USTRUCT()
struct FFlareSoundPlayer
{
	GENERATED_USTRUCT_BODY()

	/** Sound component */
	UPROPERTY()
	class UAudioComponent*                   Sound;

	/** Fade speed */
	float                                    FadeSpeed;

	/** Volume */
	float                                    Volume;

	/** Should this sound change pitch with the volume ? */
	bool                                     PitchedFade;
};


/** Main ambient sound & music manager */
UCLASS()
class HELIUMRAIN_API UFlareSoundManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
			Gameplay methods
	----------------------------------------------------*/

	/** Setup the sound controller */
	void Setup(AFlarePlayerController* Player);
	
	/** Update the sound manager */
	void Update(float DeltaSeconds);

	/** Set the music volume to use */
	void SetMusicVolume(int32 Volume);

	/** Set the master volume to use */
	void SetMasterVolume(int32 Volume);

	/** Require a specific music track */
	void RequestMusicTrack(EFlareMusicTrack::Type NewTrack);


protected:

	/*----------------------------------------------------
		Internal methods
	----------------------------------------------------*/
	
	/** Set the current spacecraft that the player uses */
	void SetCurrentSpacecraft(AFlareSpacecraft* Ship);

	/** Start playing the requested music */
	void SetDesiredMusicTrack();

	/** Update the effects volume */
	void UpdateEffectsVolume(float DeltaSeconds);

	/** Update the sound state using fading */
	void UpdatePlayer(FFlareSoundPlayer& Player, float VolumeDelta);

	/** Stop this sound without fading */
	void StopPlayer(FFlareSoundPlayer& Player);

	
protected:

	/*----------------------------------------------------
		Gameplay data
	----------------------------------------------------*/

	// Player controller
	UPROPERTY()
	AFlarePlayerController*                  PC;

	// Currently flown ship
	UPROPERTY()
	AFlareSpacecraft*                        ShipPawn;

	// Master sound class
	UPROPERTY()
	USoundClass*                             MasterSoundClass;

	// Music sound class
	UPROPERTY()
	USoundClass*                             MusicSoundClass;

	// Effects sound class
	UPROPERTY()
	USoundClass*                             EffectsSoundClass;

	// Master sound mix
	UPROPERTY()
	USoundMix*                               MasterSoundMix;

	float                                    MasterVolume;
	float                                    MusicVolume;
	float                                    EffectsVolume;


	/*----------------------------------------------------
		Music
	----------------------------------------------------*/

	// Music store
	UPROPERTY()
	TArray<USoundCue*>                       MusicTracks;

	// Track switching system
	EFlareMusicTrack::Type                   MusicDesiredTrack;
	bool                                     MusicChanging;

	// Music sound node
	UPROPERTY()
	FFlareSoundPlayer                        MusicPlayer;


	/*----------------------------------------------------
		Sound players
	----------------------------------------------------*/

	// Engine sound node
	UPROPERTY()
	FFlareSoundPlayer                        EnginePlayer;

	// RCS sound node
	UPROPERTY()
	FFlareSoundPlayer                        RCSPlayer;

	// Power sound node
	UPROPERTY()
	FFlareSoundPlayer                        PowerPlayer;

	// Target warning sound node
	UPROPERTY()
	FFlareSoundPlayer                        TargetWarningPlayer;

	// Attack warning sound node
	UPROPERTY()
	FFlareSoundPlayer                        AttackWarningPlayer;

	// Health warning sound node
	UPROPERTY()
	FFlareSoundPlayer                        HealthWarningPlayer;

	// Collision warning sound node
	UPROPERTY()
	FFlareSoundPlayer                        CollisionWarningPlayer;

	// Exit warning sound node
	UPROPERTY()
	FFlareSoundPlayer                        SectorExitWarningPlayer;
	

	/*----------------------------------------------------
		Sound data
	----------------------------------------------------*/

	// Warning sound on targeting
	UPROPERTY()
	USoundCue*                               TargetWarningSound;

	// Warning sound on attack
	UPROPERTY()
	USoundCue*                               AttackWarningSound;

	// Warning sound on low health
	UPROPERTY()
	USoundCue*                               HealthWarningSound;

	// Warning sound on low health (heavy ships)
	UPROPERTY()
	USoundCue*                               HealthWarningHeavySound;

	// Warning sound on collision risk
	UPROPERTY()
	USoundCue*                               CollisionWarningSound;

	// Warning sound on sector exit
	UPROPERTY()
	USoundCue*                               SectorExitWarningSound;
	 

public:

	/*----------------------------------------------------
		Sound library
	----------------------------------------------------*/

	/** Negative sound */
	UPROPERTY()
	USoundCue*                               NegativeClickSound;

	/** Info sound */
	UPROPERTY()
	USoundCue*                               InfoSound;

	/** Ticking sound */
	UPROPERTY()
	USoundCue*                               TickSound;

	/** Ticking sound */
	UPROPERTY()
	USoundCue*                               BellSound;
	
	/** Deletion sound */
	UPROPERTY()
	USoundCue*                               DeleteSound;

};

