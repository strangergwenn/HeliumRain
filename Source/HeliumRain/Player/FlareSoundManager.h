#pragma once

#include "../Flare.h"
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
		Exploration,
		Danger,
		Pacific,
		Combat,
		War,
		Menu
	};
}


/** Ambient sound player with fading */
USTRUCT()
struct FFlareSoundPlayer
{
	GENERATED_USTRUCT_BODY()

	/** Sound component */
	UPROPERTY()
	UAudioComponent*                         Sound;

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

	/** Update the sound state using fading */
	void UpdatePlayer(FFlareSoundPlayer& Player, float VolumeDelta, bool Force = false);

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

	// Master sound mix
	UPROPERTY()
	USoundMix*                               MasterSoundMix;

	float                                    MusicVolume;


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

};

