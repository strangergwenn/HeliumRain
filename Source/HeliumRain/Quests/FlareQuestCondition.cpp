#include "Flare.h"
#include "FlareQuestCondition.h"
#include "../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareQuestCondition"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestCondition::UFlareQuestCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UFlareQuestCondition::CheckConditions(TArray<UFlareQuestCondition*>& Conditions, bool EmptyResult)
{
	if (Conditions.Num() == 0)
	{
		return EmptyResult;
	}

	for(UFlareQuestCondition* Condition: Conditions)
	{
		if(!Condition->IsCompleted(EmptyResult))
		{
			return false;
		}
	}

	return true;
}


void UFlareQuestCondition::AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<UFlareQuestCondition*>& Conditions)
{
	for (UFlareQuestCondition* Condition : Conditions)
	{
		TArray<EFlareQuestCallback::Type> ConditionCallbacks = Condition->GetConditionCallbacks();
		for (int CallbackIndex = 0; CallbackIndex < ConditionCallbacks.Num(); CallbackIndex++)
		{
			Callbacks.AddUnique(ConditionCallbacks[CallbackIndex]);
		}
	}
}

void UFlareQuestCondition::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FLOG("ERROR: Not implemented AddConditionObjectives")
}

bool UFlareQuestCondition::IsCompleted(bool EmptyResult)
{
	FLOG("ERROR: Not implemented IsCompleted");
	return EmptyResult;
}

#undef LOCTEXT_NAMESPACE
