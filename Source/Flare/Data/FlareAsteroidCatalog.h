#pragma once

#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareAsteroidCatalog.generated.h"


UCLASS()
class FLARE_API UFlareAsteroidCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Asteroid data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UStaticMesh*> Asteroids;

};
