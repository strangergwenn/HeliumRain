#pragma once

#include "HeliumRain/Quests/FlareQuest.h"
#include "../Data/FlareQuestCatalogEntry.h"
#include "FlareCatalogQuest.generated.h"

struct FFlareQuestStepDescription;




/** Process of a question instance for a company */
UCLASS()
class HELIUMRAIN_API UFlareCatalogQuest: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent, const FFlareQuestDescription* Description);

protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	TArray<UFlareQuestCondition*> GenerateCatalogCondition(const FFlareQuestConditionDescription& ConditionDescription);

	UFlareQuestAction* GenerateCatalogAction(const FFlareQuestActionDescription& ActionDescription);

	UFlareQuestStep* GenerateCatalogStep(const FFlareQuestStepDescription& StepDescription);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	 const FFlareQuestDescription*			CatalogDescription;

	 /*----------------------------------------------------
		 Internal getters
	 ----------------------------------------------------*/


	 const FFlareSharedQuestCondition* FindSharedCondition(FName SharedConditionIdentifier);

};
