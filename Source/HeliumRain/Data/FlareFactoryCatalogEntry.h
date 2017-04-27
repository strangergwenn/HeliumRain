#pragma once

#include "Engine/DataAsset.h"
#include "../Economy/FlareFactory.h"
#include "FlareFactoryCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareFactoryCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Factory data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareFactoryDescription Data;

};
