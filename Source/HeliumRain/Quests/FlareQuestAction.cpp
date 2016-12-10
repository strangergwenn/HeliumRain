
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

UFlareQuestActionDiscoverSector* UFlareQuestActionDiscoverSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	UFlareQuestActionDiscoverSector* Action = NewObject<UFlareQuestActionDiscoverSector>(ParentQuest, UFlareQuestActionDiscoverSector::StaticClass());
	Action->Load(ParentQuest, SectorParam);
	return Action;
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

/*----------------------------------------------------
	Visit sector action
----------------------------------------------------*/
UFlareQuestActionVisitSector::UFlareQuestActionVisitSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestActionVisitSector* UFlareQuestActionVisitSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	UFlareQuestActionVisitSector* Action = NewObject<UFlareQuestActionVisitSector>(ParentQuest, UFlareQuestActionVisitSector::StaticClass());
	Action->Load(ParentQuest, SectorParam);
	return Action;
}

void UFlareQuestActionVisitSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam)
{
	LoadInternal(ParentQuest);
	Sector = SectorParam;
}

void UFlareQuestActionVisitSector::Perform()
{
	if (Sector)
	{
		Sector->GetGame()->GetPC()->GetCompany()->VisitSector(Sector);
	}
	else
	{
		FLOGV("ERROR in UFlareQuestActionVisitSector::Perform : invalid sector to discover for quest %s", *Quest->GetIdentifier().ToString());
	}
}

/*----------------------------------------------------
	Print message action
----------------------------------------------------*/
UFlareQuestActionPrintMessage::UFlareQuestActionPrintMessage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestActionPrintMessage* UFlareQuestActionPrintMessage::Create(UFlareQuest* ParentQuest, FText MessageParam)
{
	UFlareQuestActionPrintMessage* Action = NewObject<UFlareQuestActionPrintMessage>(ParentQuest, UFlareQuestActionPrintMessage::StaticClass());
	Action->Load(ParentQuest, MessageParam);
	return Action;
}

void UFlareQuestActionPrintMessage::Load(UFlareQuest* ParentQuest, FText MessageParam)
{
	LoadInternal(ParentQuest);
	Message = MessageParam;
}

void UFlareQuestActionPrintMessage::Perform()
{
	FText MessageText = Quest->FormatTags(Message);
	Quest->SendQuestNotification(MessageText, FName(*(FString("quest-") + Quest->GetIdentifier().ToString() + "-message")));
}

#undef LOCTEXT_NAMESPACE
