
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
	
	// Power sound
	PowerSound.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("PowerSound"));
	PowerSound.Sound->bAutoActivate = false;
	PowerSound.Sound->bAutoDestroy = false;
	PowerSound.FadeSpeed = 0.3;

	// Engine sound
	EngineSound.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EngineSound.Sound->bAutoActivate = false;
	EngineSound.Sound->bAutoDestroy = false;
	EngineSound.FadeSpeed = 2.0;

	// RCS sound
	RCSSound.Sound = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("RCSSound"));
	RCSSound.Sound->bAutoActivate = false;
	RCSSound.Sound->bAutoDestroy = false;
	RCSSound.FadeSpeed = 5.0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSoundManager::Setup(AFlarePlayerController* Player)
{
	PC = Player;
	FLOG("UFlareSoundManager::Setup");

	// Attach all sounds
	if (Player)
	{
		USceneComponent* RootComponent = Player->GetRootComponent();
		PowerSound.Sound->AttachTo(RootComponent);
		EngineSound.Sound->AttachTo(RootComponent);
		RCSSound.Sound->AttachTo(RootComponent);
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

	// Has a ship : update sound
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
		UpdateSound(PowerSound,  (ShipPawn->GetDamageSystem()->IsPowered() && !ShipPawn->GetDamageSystem()->HasPowerOutage() ? 1 : -1) * DeltaSeconds);
		UpdateSound(EngineSound, (EngineAlpha > 0 ? EngineAlpha / EngineCount : -1)              * DeltaSeconds);
		UpdateSound(RCSSound,    (RCSAlpha > 0 ? RCSAlpha / RCSCount : -1)                       * DeltaSeconds);
	}

	// No ship : stop all sounds
	else
	{
		StopSound(PowerSound);
		StopSound(EngineSound);
		StopSound(RCSSound);
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
		PowerSound.Sound->SetSound(ShipDescription ? ShipDescription->PowerSound : NULL);

		// Setup orbital engine sound
		FFlareSpacecraftComponentDescription* EngineDescription = Ship->GetOrbitalEngineDescription();
		EngineSound.Sound->SetSound(EngineDescription ? EngineDescription->EngineCharacteristics.EngineSound : NULL);

		// Setup RCS sound
		FFlareSpacecraftComponentDescription* RCSDescription = Ship->GetRCSDescription();
		RCSSound.Sound->SetSound(RCSDescription ? RCSDescription->EngineCharacteristics.EngineSound : NULL);
	}
}

void UFlareSoundManager::UpdateSound(FFlareSoundPlayer& Player, float VolumeDelta)
{
	float NewVolume = FMath::Clamp(Player.Volume + VolumeDelta * Player.FadeSpeed, 0.0f, 1.0f);
	if (NewVolume != Player.Volume)
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
			Player.Sound->SetVolumeMultiplier(NewVolume);
			Player.Sound->SetPitchMultiplier(0.5 + 0.5 * NewVolume);
		}
		Player.Volume = NewVolume;
	}
}

void UFlareSoundManager::StopSound(FFlareSoundPlayer& Player)
{
	Player.Sound->Stop();
	Player.Volume = 0;
}

#undef LOCTEXT_NAMESPACE
