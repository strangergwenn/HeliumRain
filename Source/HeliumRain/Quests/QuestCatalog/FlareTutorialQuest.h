#pragma once

#include "../FlareQuest.h"
#include "FlareTutorialQuest.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialFlying: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialNavigation: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};
