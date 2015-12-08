#pragma once

#include "../Flare.h"
#include "FlareSoundManager.generated.h"


class AFlareSpacecraft;
class AFlarePlayerController;


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
	
	/** Set the current spacecraft that the player uses */
	virtual void SetCurrentSpacecraft(AFlareSpacecraft* Ship);


protected:

	/*----------------------------------------------------
		Internal methods
	----------------------------------------------------*/

	/** Update the sound state using fading */
	virtual void UpdateSound(UAudioComponent* SoundComp, float VolumeDelta, float& CurrentVolume);

	
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
	UAudioComponent*                         EngineSound;

	// RCS sound node
	UPROPERTY()
	UAudioComponent*                         RCSSound;

	// Power sound node
	UPROPERTY()
	UAudioComponent*                         PowerSound;

	// Sound data
	float                                    EngineSoundFadeSpeed;
	float                                    RCSSoundFadeSpeed;
	float                                    PowerSoundFadeSpeed;
	float                                    EngineSoundVolume;
	float                                    RCSSoundVolume;
	float                                    PowerSoundVolume;
	

};

