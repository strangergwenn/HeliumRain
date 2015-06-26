#pragma once

#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareAsteroidCatalog.generated.h"


UCLASS()
class UFlareAsteroidCatalog : public UDataAsset
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
