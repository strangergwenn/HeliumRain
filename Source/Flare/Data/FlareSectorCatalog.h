#pragma once

#include "../Game/FlareSimulatedSector.h"
#include "FlareSectorCatalog.generated.h"


UCLASS()
class UFlareSectorCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Sectors data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSectorCelestialBodyDescription> OrbitalBodies;

};
