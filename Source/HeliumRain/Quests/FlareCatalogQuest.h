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
	virtual void Load(const FFlareQuestDescription* Description);

protected:

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

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


	 const FFlareQuestDescription*			QuestDescription;
	 const FFlareQuestStepDescription*		CurrentStepDescription;

	 /*----------------------------------------------------
		 Internal getters
	 ----------------------------------------------------*/
	 inline const FFlareQuestDescription* GetQuestDescription() const
	 {
		 return QuestDescription;
	 }


	 const FFlareSharedQuestCondition* FindSharedCondition(FName SharedConditionIdentifier);

};
