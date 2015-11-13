#pragma once

#include "../Economy/FlareResource.h"
#include "FlareResourceCatalogEntry.generated.h"


UCLASS()
class FLARE_API UFlareResourceCatalogEntry : public UDataAsset
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
