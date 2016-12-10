
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuest.h"
#include "FlareQuestStep.h"
#include "FlareQuestCondition.h"
#include "FlareQuestAction.h"

#define LOCTEXT_NAMESPACE "FlareQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuest::UFlareQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  TrackObjectives(false)
{
}

/*----------------------------------------------------
	Save
----------------------------------------------------*/


void UFlareQuest::LoadInternal(UFlareQuestManager* Parent)
{
	QuestManager = Parent;
	QuestStatus = EFlareQuestStatus::AVAILABLE;
}

void UFlareQuest::Restore(const FFlareQuestProgressSave& Data)
{
	QuestData = Data;
	QuestStatus = EFlareQuestStatus::ACTIVE;


	for (UFlareQuestCondition* Condition: TriggerConditions)
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data.TriggerConditionsSave));
	}

	CurrentStep = NULL;

	// Init current step
	for(UFlareQuestStep* Step : Steps)
	{
		if(Data.SuccessfullSteps.Contains(Step->GetIdentifier()))
		{
			SuccessfullSteps.Add(Step);
		}
		else if(CurrentStep == NULL)
		{
			CurrentStep = Step;
			Step->Restore(Data.CurrentStepProgress);
		}

	}

	NextStep();
}

FFlareQuestProgressSave* UFlareQuest::Save()
{
	QuestData.SuccessfullSteps.Empty();
	QuestData.CurrentStepProgress.Empty();
	QuestData.TriggerConditionsSave.Empty();

	QuestData.QuestIdentifier = GetIdentifier();

	for(UFlareQuestStep* Step : SuccessfullSteps)
	{
		QuestData.SuccessfullSteps.Add(Step->GetIdentifier());
	}

	for (UFlareQuestCondition* Condition: TriggerConditions)
	{
		Condition->AddSave(QuestData.TriggerConditionsSave);
	}

	if(CurrentStep)
	{
		CurrentStep->Save(QuestData.CurrentStepProgress);
	}
	return &QuestData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareQuest::SetStatus(EFlareQuestStatus::Type Status)
{
	QuestStatus = Status;
}

void UFlareQuest::UpdateState()
{
	switch(QuestStatus)
	{
		case EFlareQuestStatus::AVAILABLE:
		{
			bool ConditionsStatus = UFlareQuestCondition::CheckConditions(TriggerConditions, true);
			if (ConditionsStatus)
			{
				Activate();
			}
			break;
		}
		case EFlareQuestStatus::ACTIVE:
		{
			if(!CurrentStep)
			{
				FLOGV("ERROR: Active quest '%s'without current Step", *Identifier.ToString());
				return;
			}

			CurrentStep->UpdateState();

			if (CurrentStep->IsFailed())
			{
				Fail();
			}
			else if (CurrentStep->IsCompleted())
			{
				EndStep();
			}

			UpdateObjectiveTracker();
			break;
		}
	}
}

void UFlareQuest::EndStep()
{
	SuccessfullSteps.Add(CurrentStep);
	FLOGV("Quest %s step %s end", *GetIdentifier().ToString(), *CurrentStep->GetIdentifier().ToString());

	//FText DoneText = LOCTEXT("DoneFormat", "{0} : Done");
	//SendQuestNotification(FText::Format(DoneText, StepDescription->Description), NAME_None);

	CurrentStep->PerformEndActions();

	CurrentStep = NULL;
	if (TrackObjectives)
	{
		QuestManager->GetGame()->GetPC()->CompleteObjective();
	}

	// Activate next step
	NextStep();
}

void UFlareQuest::NextStep()
{
	// Clear step progress

	CurrentStep = NULL;

	if (Steps.Num() == 0)
	{
		FLOGV("WARNING: The quest %s have no step", *GetIdentifier().ToString());
	}
	else
	{
		for(UFlareQuestStep* Step : Steps)
		{
			if (!SuccessfullSteps.Contains(Step))
			{
				CurrentStep = Step;

				FLOGV("Quest %s step %s begin", *GetIdentifier().ToString(), *Step->GetIdentifier().ToString());
				Step->Init();
				Step->PerformInitActions();

				FText MessageText = FormatTags(Step->GetStepDescription());
				SendQuestNotification(MessageText, FName(*(FString("quest-") + GetIdentifier().ToString() + "-message")));

				QuestManager->LoadCallbacks(this);
				UpdateState();
				return;
			}
		}
	}

	// All quest step are done. Success
	Success();
}

void UFlareQuest::Success()
{
	SetStatus(EFlareQuestStatus::SUCCESSFUL);
	UFlareQuestAction::PerformActions(SuccessActions);
	QuestManager->OnQuestSuccess(this);
}

void UFlareQuest::Fail()
{
	SetStatus(EFlareQuestStatus::FAILED);
	UFlareQuestAction::PerformActions(FailActions);
	QuestManager->OnQuestFail(this);
}

void UFlareQuest::Activate()
{
	SetStatus(EFlareQuestStatus::ACTIVE);
	// Activate next step
	NextStep();

	// Don't notify activation if quest is not active after first NextStep
	if (QuestStatus == EFlareQuestStatus::ACTIVE)
	{
		QuestManager->OnQuestActivation(this);
	}
}

FText UFlareQuest::FormatTags(FText Message)
{
	// Replace input tags
	bool Found = false;
	FString MessageString = Message.ToString();

	// input-axis
	do {
		Found = false;
		FString StartTag = TEXT("<input-axis:");
		FString EndTag = TEXT(">");

		int StartIndex = MessageString.Find(StartTag);
		if (StartIndex > 0)
		{
			int EndIndex = MessageString.Find(EndTag,ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex);
			if (EndIndex > 0)
			{
				// Tag found, replace it.
				Found = true;

				FString TagValue = MessageString.Mid(StartIndex + StartTag.Len(), EndIndex - (StartIndex + StartTag.Len()));

				FString AxisString;
				FString ScaleString;
				TagValue.Split(",", &AxisString, &ScaleString);
				float Scale = FCString::Atof(*ScaleString);
				FName AxisName = FName(*AxisString);

				FString Mapping;

				UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

				for (int32 i = 0; i < InputSettings->AxisMappings.Num(); i++)
				{
					FInputAxisKeyMapping Axis = InputSettings->AxisMappings[i];
					if (Axis.AxisName == AxisName && Axis.Scale == Scale && !Axis.Key.IsGamepadKey())
					{
						Mapping = Axis.Key.ToString();
					}
				}


				FString TagString = TEXT("<")+ Mapping + TEXT(">");

				MessageString = MessageString.Left(StartIndex) + TagString + MessageString.RightChop(EndIndex+1);
			}
			else
			{
				FLOGV("ERROR None closed tag at offset %d for quest %s: %s", StartIndex, *GetIdentifier().ToString(), *MessageString);
			}
		}

	} while(Found);

	// input-action
	do {
		Found = false;
		FString StartTag = TEXT("<input-action:");
		FString EndTag = TEXT(">");

		int StartIndex = MessageString.Find(StartTag);
		if (StartIndex > 0)
		{
			int EndIndex = MessageString.Find(EndTag,ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex);
			if (EndIndex > 0)
			{
				// Tag found, replace it.
				Found = true;

				FString TagValue = MessageString.Mid(StartIndex + StartTag.Len(), EndIndex - (StartIndex + StartTag.Len()));

				FName ActionName = FName(*TagValue);
				FString Mapping = AFlareMenuManager::GetKeyNameFromActionName(ActionName);

				FString TagString = TEXT("<")+ Mapping + TEXT(">");

				MessageString = MessageString.Left(StartIndex) + TagString + MessageString.RightChop(EndIndex+1);
			}
			else
			{
				FLOGV("ERROR None closed tag at offset %d for quest %s: %s", StartIndex, *GetIdentifier().ToString(), *MessageString);
			}
		}

	} while(Found);

	MessageString = MessageString.Replace(TEXT("<br>"), TEXT("\n"));

	return FText::FromString(MessageString);
}

void UFlareQuest::SendQuestNotification(FText Message, FName Tag, bool Pinned)
{
	FText Text = GetQuestName();
	FLOGV("UFlareQuest::SendQuestNotification : %s", *Message.ToString());
	QuestManager->GetGame()->GetPC()->Notify(Text, Message, Tag, EFlareNotification::NT_Quest, Pinned);
}


/*----------------------------------------------------
	Objective tracking
----------------------------------------------------*/

void UFlareQuest::StartObjectiveTracking()
{
	FLOG("UFlareQuest::StartObjectiveTracking");

	if (TrackObjectives)
	{
		return; // No change
	}

	FLOG("UFlareQuest::StartObjectiveTracking : tracking");
	TrackObjectives = true;
	QuestManager->GetGame()->GetPC()->CompleteObjective();
	UpdateObjectiveTracker();
}

void UFlareQuest::StopObjectiveTracking()
{
	FLOG("UFlareQuest::StopObjectiveTracking");

	if (!TrackObjectives)
	{
		return; // No change
	}

	FLOG("UFlareQuest::StopObjectiveTracking : not tracking anymore");
	TrackObjectives = false;
	QuestManager->GetGame()->GetPC()->CompleteObjective();
}


void UFlareQuest::UpdateObjectiveTracker()
{
	if (!TrackObjectives)
	{
		return;
	}

	FFlarePlayerObjectiveData Objective;
	if (CurrentStep)
	{
		Objective.StepsDone = GetSuccessfullStepCount();
		Objective.StepsCount = GetStepCount();
		Objective.Description = GetQuestDescription();
		Objective.Name = GetQuestName();

		CurrentStep->AddEndConditionObjectives(&Objective);
	}

	QuestManager->GetGame()->GetPC()->StartObjective(Objective.Name, Objective);
}

/*----------------------------------------------------
	Callback
----------------------------------------------------*/


TArray<EFlareQuestCallback::Type> UFlareQuest::GetCurrentCallbacks()
{
	TArray<EFlareQuestCallback::Type> Callbacks;

	// TODO Cache and return reference
	switch(QuestStatus)
	{
		case EFlareQuestStatus::AVAILABLE:
			// Use trigger conditions
			UFlareQuestCondition::AddConditionCallbacks(Callbacks, TriggerConditions);

			if (Callbacks.Contains(EFlareQuestCallback::TICK_FLYING))
			{
				FLOGV("WARNING: The quest %s need a TICK_FLYING callback as trigger", *GetIdentifier().ToString());
			}
			break;
		case EFlareQuestStatus::ACTIVE:
		 {
			// Use current step conditions
			if (CurrentStep)
			{
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetEnableConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetEndConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetFailConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetBlockConditions());
			}
			else
			{
				FLOGV("WARNING: The quest %s have no step", *GetIdentifier().ToString());
			}
			break;
		}
		default:
			// Don't add callback in others cases
			break;
	}

	return Callbacks;
}

void UFlareQuest::OnTick(float DeltaSeconds)
{
	UpdateState();
}

void UFlareQuest::OnFlyShip(AFlareSpacecraft* Ship)
{
	UpdateState();
}


void UFlareQuest::OnQuestStatusChanged(UFlareQuest* Quest)
{
	UpdateState();
}

void UFlareQuest::OnSectorActivation(UFlareSimulatedSector* Sector)
{
	UpdateState();
}

void UFlareQuest::OnSectorVisited(UFlareSimulatedSector* Sector)
{
	UpdateState();
}



/*----------------------------------------------------
	Getters
----------------------------------------------------*/

FText UFlareQuest::GetStatusText() const
{
	switch (QuestStatus)
	{
		case EFlareQuestStatus::AVAILABLE:    return LOCTEXT("QuestAvailable", "Available");   break;
		case EFlareQuestStatus::ACTIVE:       return LOCTEXT("QuestActive", "Active");         break;
		case EFlareQuestStatus::SUCCESSFUL:   return LOCTEXT("QuestCompleted", "Completed");   break;
		case EFlareQuestStatus::ABANDONNED:   return LOCTEXT("QuestAbandonned", "Abandonned"); break;
		case EFlareQuestStatus::FAILED:       return LOCTEXT("QuestFailed", "Failed");         break;
		default:                              return LOCTEXT("QuestUnknown", "Unknown");       break;
	}
}

#undef LOCTEXT_NAMESPACE
