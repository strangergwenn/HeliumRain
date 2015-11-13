#pragma once

#include "FlareFactoryCatalogEntry.h"
#include "../Economy/FlareFactory.h"
#include "FlareFactoryCatalog.generated.h"


UCLASS()
class FLARE_API UFlareFactoryCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Factories data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareFactoryCatalogEntry*> Factories;

};
