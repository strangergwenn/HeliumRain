#pragma once

#include "../Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "Engine/DataAsset.h"
#include "FlareOrbitalMap.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareOrbitalMap : public UDataAsset
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
