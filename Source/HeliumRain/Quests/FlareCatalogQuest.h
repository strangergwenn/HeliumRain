#pragma once

#include "FlareQuest.h"
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


	//virtual FFlareQuestStepProgressSave* CreateStepProgressSave(const FFlareQuestConditionDescription* Condition);

	//virtual void GenerateConditionCollinearityObjective(FFlarePlayerObjectiveData* ObjectiveData, EFlareQuestCondition::Type ConditionType, float TargetCollinearity);


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
// TODO

/*inline FName GetIdentifier() const
{
	return QuestDescription->Identifier;
}*/

/*inline FText GetQuestName() const
{
	return QuestDescription->QuestName;
}*/


	 const FFlareQuestDescription*			CatalogDescription;

	 /*----------------------------------------------------
		 Internal getters
	 ----------------------------------------------------*/


	 const FFlareSharedQuestCondition* FindSharedCondition(FName SharedConditionIdentifier);

};
