
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "FlareCatalogQuest.h"

#define LOCTEXT_NAMESPACE "FlareCatalogQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCatalogQuest::UFlareCatalogQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCatalogQuest::Load(const FFlareQuestDescription* Description)
{
	// TODO
}


/*----------------------------------------------------
	Internal getters
----------------------------------------------------*/
const FFlareSharedQuestCondition* UFlareCatalogQuest::FindSharedCondition(FName SharedConditionIdentifier)
{
	for (int SharedConditionIndex = 0; SharedConditionIndex < QuestDescription->SharedConditions.Num(); SharedConditionIndex++)
	{
		if (QuestDescription->SharedConditions[SharedConditionIndex].Identifier == SharedConditionIdentifier)
		{
			return &QuestDescription->SharedConditions[SharedConditionIndex];
		}
	}

	FLOGV("ERROR: The quest %s doesn't have shared condition named %s", *GetIdentifier().ToString(), *SharedConditionIdentifier.ToString());

	return NULL;
}
#undef LOCTEXT_NAMESPACE
