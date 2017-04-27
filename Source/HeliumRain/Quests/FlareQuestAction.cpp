
#include "FlareQuestAction.h"
#include "Flare.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareCompany.h"

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
		Sector->GetGame()->GetPC()->DiscoverSector(Sector, false, true);
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
		Sector->GetGame()->GetPC()->DiscoverSector(Sector, true, true);
	}
	else
	{
		FLOGV("ERROR in UFlareQuestActionVisitSector::Perform : invalid sector to visit for quest %s", *Quest->GetIdentifier().ToString());
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

/*----------------------------------------------------
	Give money action
----------------------------------------------------*/
UFlareQuestActionGiveMoney::UFlareQuestActionGiveMoney(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestActionGiveMoney* UFlareQuestActionGiveMoney::Create(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int64 AmountParam)
{
	UFlareQuestActionGiveMoney* Action = NewObject<UFlareQuestActionGiveMoney>(ParentQuest, UFlareQuestActionGiveMoney::StaticClass());
	Action->Load(ParentQuest, FromCompanyParam, ToCompanyParam, AmountParam);
	return Action;
}

void UFlareQuestActionGiveMoney::Load(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int64 AmountParam)
{
	LoadInternal(ParentQuest);
	FromCompany = FromCompanyParam;
	ToCompany = ToCompanyParam;
	Amount = AmountParam;
}

void UFlareQuestActionGiveMoney::Perform()
{
	FromCompany->TakeMoney(Amount, true);
	ToCompany->GiveMoney(Amount);
}

/*----------------------------------------------------
	Give research action
----------------------------------------------------*/
UFlareQuestActionGiveResearch::UFlareQuestActionGiveResearch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestActionGiveResearch* UFlareQuestActionGiveResearch::Create(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int32 AmountParam)
{
	UFlareQuestActionGiveResearch* Action = NewObject<UFlareQuestActionGiveResearch>(ParentQuest, UFlareQuestActionGiveResearch::StaticClass());
	Action->Load(ParentQuest, FromCompanyParam, ToCompanyParam, AmountParam);
	return Action;
}

void UFlareQuestActionGiveResearch::Load(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int32 AmountParam)
{
	LoadInternal(ParentQuest);
	FromCompany = FromCompanyParam;
	ToCompany = ToCompanyParam;
	Amount = AmountParam;
}

void UFlareQuestActionGiveResearch::Perform()
{
	ToCompany->GiveResearch(Amount);
}

/*----------------------------------------------------
	Reputation change action
----------------------------------------------------*/
UFlareQuestActionReputationChange::UFlareQuestActionReputationChange(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestActionReputationChange* UFlareQuestActionReputationChange::Create(UFlareQuest* ParentQuest, UFlareCompany* CompanyParam, int64 AmountParam)
{
	UFlareQuestActionReputationChange* Action = NewObject<UFlareQuestActionReputationChange>(ParentQuest, UFlareQuestActionReputationChange::StaticClass());
	Action->Load(ParentQuest, CompanyParam, AmountParam);
	return Action;
}

void UFlareQuestActionReputationChange::Load(UFlareQuest* ParentQuest, UFlareCompany* CompanyParam, int64 AmountParam)
{
	LoadInternal(ParentQuest);
	Company = CompanyParam;
	Amount = AmountParam;
}

void UFlareQuestActionReputationChange::Perform()
{
	Company->GivePlayerReputation(Amount);
}


#undef LOCTEXT_NAMESPACE
