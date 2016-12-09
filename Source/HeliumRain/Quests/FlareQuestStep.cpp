
#include "Flare.h"
#include "FlareQuestStep.h"
#include "FlareQuestCondition.h"
#include "FlareQuestAction.h"
#include "../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareQuestStep"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestStep::UFlareQuestStep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



UFlareQuestStep* UFlareQuestStep::Create(UFlareQuest* Parent, FName Identifier, FText Description)
{
	UFlareQuestStep* Step = NewObject<UFlareQuestStep>(Parent, UFlareQuestStep::StaticClass());
	Step->Identifier = Identifier;
	Step->StepDescription = Description;

	return Step;
}


void UFlareQuestStep::UpdateState()
{
	FLOGV("Update step state %s Status=%d", *Identifier.ToString(), (Status+0));
	if(Status == EFlareQuestStepStatus::PENDING || Status == EFlareQuestStepStatus::FAILED || Status == EFlareQuestStepStatus::COMPLETED)
	{
		// Permanent state
		return;
	}

	if(Status == EFlareQuestStepStatus::DISABLED)
	{
		if (UFlareQuestCondition::CheckConditions(EnableConditions, true))
		{
			FLOG("Failed condition ko");
			Status = EFlareQuestStepStatus::ENABLED;
			UpdateState();
			return;
		}
	}
	else
	{
		// Enabled or blocked, check failed
		if (UFlareQuestCondition::CheckConditions(FailConditions, false))
		{
			FLOG("Failed condition ko");
			Status = EFlareQuestStepStatus::FAILED;
			return;
		}

		if(Status == EFlareQuestStepStatus::BLOCKED)
		{
			if (!UFlareQuestCondition::CheckConditions(BlockConditions, false))
			{
				FLOG("Enable condition ok");
				Status = EFlareQuestStepStatus::ENABLED;
				UpdateState();
				return;
			}
		}


		if(Status == EFlareQuestStepStatus::ENABLED)
		{
			if (UFlareQuestCondition::CheckConditions(BlockConditions, false))
			{
				FLOG("Block condition ko");
				Status = EFlareQuestStepStatus::BLOCKED;
				UpdateState();
				return;
			}

			if (UFlareQuestCondition::CheckConditions(EndConditions, false))
			{
				FLOG("End condition ok");
				Status = EFlareQuestStepStatus::COMPLETED;
				return;
			}
		}
	}
}

void UFlareQuestStep::Init()
{
	if (Status == EFlareQuestStepStatus::PENDING)
	{
		Status = EFlareQuestStepStatus::DISABLED;
	}
}

void UFlareQuestStep::Restore(const TArray<FFlareQuestConditionSave>& Data)
{
	for (UFlareQuestCondition* Condition: EnableConditions)
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: BlockConditions)
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: FailConditions)
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: EndConditions)
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}
}

void UFlareQuestStep::Save(TArray<FFlareQuestConditionSave>& Data)
{
	for (UFlareQuestCondition* Condition: EnableConditions)
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: BlockConditions)
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: FailConditions)
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: EndConditions)
	{
		Condition->AddSave(Data);
	}
}

void UFlareQuestStep::PerformInitActions()
{
	UFlareQuestAction::PerformActions(InitActions);
}

void UFlareQuestStep::PerformEndActions()
{
	UFlareQuestAction::PerformActions(EndActions);
}

void UFlareQuestStep::AddEndConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	for(UFlareQuestCondition* Condition: EndConditions)
	{
		Condition->AddConditionObjectives(ObjectiveData);
	}
}

#undef LOCTEXT_NAMESPACE
