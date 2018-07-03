#pragma once

#include "../Flare.h"
#include "../Game/FlareGameTypes.h"
#include "Engine/DataAsset.h"
#include "FlareScannableCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareScannableCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Scannable data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareScannableDescription Data;

};
