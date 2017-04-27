#pragma once

#include "../Flare.h"
#include "../Game/FlareGameTypes.h"
#include "FlareTechnologyCatalogEntry.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareTechnologyCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Technology data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareTechnologyDescription Data;

};
