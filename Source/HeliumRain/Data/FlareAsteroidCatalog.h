#pragma once

#include "../Flare.h"
#include "FlareAsteroidCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareAsteroidCatalog : public UDataAsset
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
