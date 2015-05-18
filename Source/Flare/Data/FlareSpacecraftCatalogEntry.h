#pragma once

#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareSpacecraftCatalogEntry.generated.h"


UCLASS()
class UFlareSpacecraftCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Ship data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareSpacecraftDescription Data;

};
