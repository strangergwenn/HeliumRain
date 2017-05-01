#pragma once

#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraftComponent.h"
#include "Engine/DataAsset.h"
#include "FlareSpacecraftComponentsCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareSpacecraftComponentsCatalogEntry : public UDataAsset
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
