
#include "Flare.h"
#include "FlareQuestAction.h"
#include "../Game/FlareSimulatedSector.h"

#define LOCTEXT_NAMESPACE "FlareQuestAction"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestAction::UFlareQuestAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestAction::Perform()
{
	FLOG("ERROR: Not implemented perform action");
}

void UFlareQuestAction::PerformActions(const TArray<UFlareQuestAction*>& Actions)
{
	for(UFlareQuestAction* Action: Actions)
	{
		Action->Perform();
	}
}

/*----------------------------------------------------
	Discover sector action
----------------------------------------------------*/
UFlareQuestActionDiscoverSector::UFlareQuestActionDiscoverSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestActionDiscoverSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	LoadInternal(ParentQuest);
	Sector = SectorParam;
}

void UFlareQuestActionDiscoverSector::Perform()
{
	if (Sector)
	{
		Sector->GetGame()->GetPC()->GetCompany()->DiscoverSector(Sector);
	}
	else
	{
		FLOGV("ERROR in UFlareQuestActionDiscoverSector::Perform : invalid sector to discover for quest %s", *Quest->GetIdentifier().ToString());
	}
}

#undef LOCTEXT_NAMESPACE
