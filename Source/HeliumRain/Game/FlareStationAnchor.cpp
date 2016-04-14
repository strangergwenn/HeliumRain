
#include "../Flare.h"
#include "FlareStationAnchor.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareStationAnchor::AFlareStationAnchor(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent0"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);

		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = FName("Helium Rain");
		ArrowComponent->SpriteInfo.DisplayName = NSLOCTEXT("FlareStationAnchor", "StationAnchor", "Station Anchor");
		ArrowComponent->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA
}
