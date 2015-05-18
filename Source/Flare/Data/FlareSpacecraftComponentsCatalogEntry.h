#pragma once

#include "../Spacecrafts/FlareSpacecraftComponent.h"
#include "FlareSpacecraftComponentsCatalogEntry.generated.h"


UCLASS()
class UFlareSpacecraftComponentsCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareSpacecraftComponentDescription Data;


};
