#pragma once

#include "../Flare.h"
#include "../Economy/FlareResource.h"
#include "Engine/DataAsset.h"
#include "FlareResourceCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareResourceCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Factory data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareResourceDescription Data;

};
