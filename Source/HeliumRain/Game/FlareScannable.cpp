
#include "HeliumRain/Game/FlareScannable.h"
#include "../Flare.h"

#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareScannable::AFlareScannable(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	Root = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = Root;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

bool AFlareScannable::IsActive()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	return !PC->IsScannableUnlocked(ScannableIdentifier);
}

void  AFlareScannable::Unlock()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());

	PC->UnlockScannable(ScannableIdentifier);

	OnUnlocked();
}
