#pragma once

#include "../Flare.h"
#include "../Game/FlareSector.h"
#include "FlareSectorCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareSectorCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Sector data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareSectorDescription Data;

};
