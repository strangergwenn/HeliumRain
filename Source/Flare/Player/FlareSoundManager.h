#pragma once

#include "../Flare.h"
#include "FlareSoundManager.generated.h"


class AFlareSpacecraft;
class AFlarePlayerController;


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
};


/** Main ambient sound & music manager */
UCLASS()
class FLARE_API UFlareSoundManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
			Gameplay methods
	----------------------------------------------------*/

	/** Setup the sound controller */
	virtual void Setup(AFlarePlayerController* Player);
	
	/** Update the sound manager */
	virtual void Update(float DeltaSeconds);


protected:

	/*----------------------------------------------------
		Internal methods
	----------------------------------------------------*/
	
	/** Set the current spacecraft that the player uses */
	virtual void SetCurrentSpacecraft(AFlareSpacecraft* Ship);

	/** Update the sound state using fading */
	virtual void UpdateSound(FFlareSoundPlayer& Player, float VolumeDelta);

	/** Stop this sound without fading */
	virtual void StopSound(FFlareSoundPlayer& Player);

	
protected:

	/*----------------------------------------------------
		Gameplay data
	----------------------------------------------------*/

	/** Player controller */
	UPROPERTY()
	AFlarePlayerController*                  PC;

	/** Currently flown ship */
	UPROPERTY()
	AFlareSpacecraft*                        ShipPawn;
	
	// Engine sound node
	UPROPERTY()
	FFlareSoundPlayer                        EngineSound;

	// RCS sound node
	UPROPERTY()
	FFlareSoundPlayer                        RCSSound;

	// Power sound node
	UPROPERTY()
	FFlareSoundPlayer                        PowerSound;
	
};

