
#include "FlareQuestStep.h"
#include "Flare.h"
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
	Step->EnableCondition = UFlareQuestConditionAndGroup::Create(Parent, true);
	Step->BlockCondition = UFlareQuestConditionOrGroup::Create(Parent, false);
	Step->FailCondition = UFlareQuestConditionOrGroup::Create(Parent, false);
	Step->EndCondition = UFlareQuestConditionAndGroup::Create(Parent, true);

	return Step;
}


void UFlareQuestStep::UpdateState()
{
	if(Status == EFlareQuestStepStatus::PENDING || Status == EFlareQuestStepStatus::FAILED || Status == EFlareQuestStepStatus::COMPLETED)
	{
		// Permanent state
		return;
	}

	if(Status == EFlareQuestStepStatus::DISABLED)
	{
		if (EnableCondition == NULL || EnableCondition->IsCompleted())
		{
			Status = EFlareQuestStepStatus::ENABLED;
			UpdateState();
			return;
		}
	}
	else
	{
		// Enabled or blocked, check failed
		if (FailCondition != NULL && FailCondition->IsCompleted())
		{
			Status = EFlareQuestStepStatus::FAILED;
			return;
		}

		if(Status == EFlareQuestStepStatus::BLOCKED)
		{
			if (BlockCondition == NULL || !BlockCondition->IsCompleted())
			{
				Status = EFlareQuestStepStatus::ENABLED;
				UpdateState();
				return;
			}
		}


		if(Status == EFlareQuestStepStatus::ENABLED)
		{
			if (BlockCondition != NULL && BlockCondition->IsCompleted())
			{
				Status = EFlareQuestStepStatus::BLOCKED;
				UpdateState();
				return;
			}

			if (EndCondition == NULL || EndCondition->IsCompleted())
			{
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
	for (UFlareQuestCondition* Condition: EnableCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: BlockCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: FailCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}

	for (UFlareQuestCondition* Condition: EndCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data));
	}
}

void UFlareQuestStep::Save(TArray<FFlareQuestConditionSave>& Data)
{
	for (UFlareQuestCondition* Condition: EnableCondition->GetAllConditions())
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: BlockCondition->GetAllConditions())
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: FailCondition->GetAllConditions())
	{
		Condition->AddSave(Data);
	}

	for (UFlareQuestCondition* Condition: EndCondition->GetAllConditions())
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
	for(UFlareQuestCondition* Condition: EndCondition->GetAllConditions(false))
	{
		Condition->AddConditionObjectives(ObjectiveData);
	}
}

void UFlareQuestStep::SetupStepIndexes(int32 Index)
{
	StepIndex = Index;

	int32 ConditionIndex;

	ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: EnableCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}

	ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: BlockCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}

	ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: FailCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}

	ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: EndCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}
}

#undef LOCTEXT_NAMESPACE
