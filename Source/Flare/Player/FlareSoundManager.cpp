
#include "../Flare.h"
#include "FlareSoundManager.h"
#include "FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareOrbitalEngine.h"


#define LOCTEXT_NAMESPACE "UFlareSoundManager"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSoundManager::UFlareSoundManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Data
	ShipPawn = NULL;

	//// Power sound
	PowerSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("PowerSound"));
	PowerSound->bAutoActivate = false;
	PowerSound->bAutoDestroy = false;
	PowerSoundFadeSpeed = 0.3;

	// Engine sound
	EngineSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EngineSound->bAutoActivate = false;
	EngineSound->bAutoDestroy = false;
	EngineSoundFadeSpeed = 2.0;

	// RCS sound
	RCSSound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("RCSSound"));
	RCSSound->bAutoActivate = false;
	RCSSound->bAutoDestroy = false;
	RCSSoundFadeSpeed = 5.0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSoundManager::Setup(AFlarePlayerController* Player)
{
	PC = Player;

	// Attach all sounds
	if (Player)
	{
		USceneComponent* RootComponent = Player->GetRootComponent();
		PowerSound->AttachTo(RootComponent);
		EngineSound->AttachTo(RootComponent);
		RCSSound->AttachTo(RootComponent);
	}
}

void UFlareSoundManager::Update(float DeltaSeconds)
{
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

	// No ship : stop all sounds
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

void UFlareSoundManager::SetCurrentSpacecraft(AFlareSpacecraft* Ship)
{
	ShipPawn = Ship;

	if (Ship)
	{
		// Setup power sound
		FFlareSpacecraftDescription* ShipDescription = Ship->GetDescription();
		PowerSound->SetSound(ShipDescription ? ShipDescription->PowerSound : NULL);

		// Setup orbital engine sound
		FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
		EngineSound->SetSound(EngineDescription ? EngineDescription->EngineCharacteristics.EngineSound : NULL);

		// Setup RCS sound
		FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
		RCSSound->SetSound(RCSDescription ? RCSDescription->EngineCharacteristics.EngineSound : NULL);
	}
}


/*----------------------------------------------------
	Internal methods
----------------------------------------------------*/

void UFlareSoundManager::UpdateSound(UAudioComponent* SoundComp, float VolumeDelta, float& CurrentVolume)
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

#undef LOCTEXT_NAMESPACE
