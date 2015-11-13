#pragma once

#include "../Quests/FlareQuest.h"
#include "FlareQuestCatalogEntry.generated.h"


UCLASS()
class FLARE_API UFlareQuestCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Quest data */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareQuestDescription Data;

};
