#pragma once

#include "HeliumRain/Data/FlareQuestCatalogEntry.h"
#include "FlareQuestCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareQuestCatalog : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Sectors data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareQuestCatalogEntry*> Quests;

};
