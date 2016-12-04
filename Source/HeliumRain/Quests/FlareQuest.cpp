
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
		if(Data.SuccessfullSteps.Contains(Identifier))
		{
			SuccessfullSteps.Add(Step);

		}
		else if(CurrentStep == NULL)
		{
			CurrentStep = Step;
			Step->Restore(Data.CurrentStepProgress);
		}

	}
}

FFlareQuestProgressSave* UFlareQuest::Save()
{
	// TODO Save
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
	QuestData.CurrentStepProgress.Empty();

	if (Steps.Num() == 0)
	{
		FLOGV("WARNING: The quest %s have no step", *GetIdentifier().ToString());
		return;
	}

	for(UFlareQuestStep* Step : Steps)
	{
		if (!SuccessfullSteps.Contains(Step))
		{
			CurrentStep = Step;

			FLOGV("Quest %s step %s begin", *GetIdentifier().ToString(), *Step->GetIdentifier().ToString());
			Step->Init();
			Step->PerformInitActions();


			// Notify message only when it's different than previous step
			if (Step != CurrentStep)
			{
				FText MessageText = FormatTags(Step->GetStepDescription());
				SendQuestNotification(MessageText, FName(*(FString("quest-") + GetIdentifier().ToString() + "-message")));
			}

			QuestManager->LoadCallbacks(this);
			UpdateState();
			return;
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
	QuestManager->OnQuestActivation(this);
}

/*
bool UFlareQuest::CheckCondition(const FFlareQuestConditionDescription* Condition, bool EmptyResult)
{
	bool Status = false;

	switch(Condition->Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->Identifier1);
			if (SharedCondition)
			{
				Status = CheckConditions(SharedCondition->Conditions, EmptyResult);
			}
			break;
		}



		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();

				FFlareQuestStepProgressSave* ProgressSave = GetCurrentStepProgressSave(Condition);

				if (!ProgressSave)
				{
					ProgressSave = CreateStepProgressSave(Condition);
					ProgressSave->CurrentProgression = 0;
					ProgressSave->InitialTransform = Spacecraft->Airframe->GetComponentTransform();
				}

				FVector InitialLocation = ProgressSave->InitialTransform.GetTranslation();
				FVector RelativeTargetLocation = Condition->VectorListParam[ProgressSave->CurrentProgression] * 100;
				FVector WorldTargetLocation = InitialLocation + ProgressSave->InitialTransform.GetRotation().RotateVector(RelativeTargetLocation);


				float MaxDistance = Condition->FloatListParam[ProgressSave->CurrentProgression] * 100;


				if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
				{
					// Nearing the target
					if (ProgressSave->CurrentProgression + 2 <= Condition->VectorListParam.Num())
					{
						// Progress.
						ProgressSave->CurrentProgression++;

						FText WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");

						SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(Condition->VectorListParam.Num() - ProgressSave->CurrentProgression)),
											  FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")));
					}
					else
					{
						// All waypoint reach
						Status = true;
					}

				}
			}
			break;

		case EFlareQuestCondition::QUEST_SUCCESSFUL:
			Status = QuestManager->IsQuestSuccesfull(Condition->Identifier1);
			break;
		case EFlareQuestCondition::QUEST_FAILED:
			Status = QuestManager->IsQuestFailed(Condition->Identifier1);
			break;
		default:
			FLOGV("ERROR: CheckCondition not implemented for condition type %d", (int)(Condition->Type +0));
			break;
	}

	return Status;
}

void UFlareQuest::PerformAction(const FFlareQuestActionDescription* Action)
{
	switch (Action->Type) {
	case EFlareQuestAction::DISCOVER_SECTOR:
	{
		UFlareSimulatedSector* Sector = QuestManager->GetGame()->GetGameWorld()->FindSector(Action->Identifier1);
		if (Sector)
		{
			QuestManager->GetGame()->GetPC()->GetCompany()->DiscoverSector(Sector);
		}
		else
		{
			FLOGV("ERROR in PerformAction : unknown sector to discover %s for quest %s", *Action->Identifier1.ToString(), *GetIdentifier().ToString());
		}
		break;
	}
	case EFlareQuestAction::VISIT_SECTOR:
	{
		UFlareSimulatedSector* Sector = QuestManager->GetGame()->GetGameWorld()->FindSector(Action->Identifier1);
		if (Sector)
		{
			QuestManager->GetGame()->GetPC()->GetCompany()->VisitSector(Sector);
		}
		else
		{
			FLOGV("ERROR in PerformAction : unknown sector to visit %s for quest %s", *Action->Identifier1.ToString(), *GetIdentifier().ToString());
		}
		break;
	}
	case EFlareQuestAction::PRINT_MESSAGE:

		// Replace tags in quests text
		{
			FText MessageText = FormatTags(Action->MessagesParameter);
			SendQuestNotification(MessageText, FName(*(FString("quest-") + GetIdentifier().ToString() + "-message")));
		}
		break;

	default:
		FLOGV("ERROR: PerformAction not implemented for action type %d", (int)(Action->Type +0));
		break;
	}

}
*/
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

void UFlareQuest::SendQuestNotification(FText Message, FName Tag)
{
	FText Text = GetQuestName();
	FLOGV("UFlareQuest::SendQuestNotification : %s", *Message.ToString());
	QuestManager->GetGame()->GetPC()->Notify(Text, Message, Tag, EFlareNotification::NT_Quest, true);
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

	FLOG("UFlareQuest::StartObjectiveTracking : not tracking anymore");
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

/*
void UFlareQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData, const TArray<FFlareQuestConditionDescription>& Conditions)
{
	AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
	FVector WorldAngularVelocity = Spacecraft ? Spacecraft->Airframe->GetPhysicsAngularVelocity() : FVector(0, 0, 0);
	FVector LocalAngularVelocity = Spacecraft ? Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity) : FVector(0, 0, 0);

	for (int ConditionIndex = 0; ConditionIndex < Conditions.Num(); ConditionIndex++)
	{
		const FFlareQuestConditionDescription* Condition = &Conditions[ConditionIndex];

		switch (Condition->Type) {
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->Identifier1);
			if (SharedCondition)
			{
				AddConditionObjectives(ObjectiveData, SharedCondition->Conditions);
			}
			break;
		}


		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
		{
			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = LOCTEXT("FollowWaypoints", "Fly to waypoints");
			ObjectiveCondition.TerminalLabel = FText::GetEmpty();
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = Condition->VectorListParam.Num();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = Condition->VectorListParam.Num();

			// It need navigation point. Get current point coordinate.
			FFlareQuestStepProgressSave* ProgressSave = GetCurrentStepProgressSave(Condition);

			if (ProgressSave)
			{
				ObjectiveCondition.Counter = ProgressSave->CurrentProgression;
				ObjectiveCondition.Progress = ProgressSave->CurrentProgression;
				for (int TargetIndex = 0; TargetIndex < Condition->VectorListParam.Num(); TargetIndex++)
				{
					if (TargetIndex < ProgressSave->CurrentProgression)
					{
						// Don't show old target
						continue;
					}
					FFlarePlayerObjectiveTarget ObjectiveTarget;
					ObjectiveTarget.Actor = NULL;
					ObjectiveTarget.Active = (ProgressSave->CurrentProgression == TargetIndex);
					ObjectiveTarget.Radius = Condition->FloatListParam[TargetIndex];

					FVector InitialLocation = ProgressSave->InitialTransform.GetTranslation();
					FVector RelativeTargetLocation = Condition->VectorListParam[TargetIndex] * 100; // In cm
					FVector WorldTargetLocation = InitialLocation + ProgressSave->InitialTransform.GetRotation().RotateVector(RelativeTargetLocation);

					ObjectiveTarget.Location = WorldTargetLocation;
					ObjectiveData->TargetList.Add(ObjectiveTarget);
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			break;
		}



		default:
			FLOGV("ERROR: UpdateObjectiveTracker not implemented for condition type %d", (int)(Condition->Type +0));
			break;
		}
	}
}
*/

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

/*
TArray<EFlareQuestCallback::Type> UFlareQuest::GetConditionCallbacks(const FFlareQuestConditionDescription* Condition)
{

	TArray<EFlareQuestCallback::Type> Callbacks;
	switch(Condition->Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->Identifier1);
			if (SharedCondition)
			{
				AddConditionCallbacks(Callbacks, SharedCondition->Conditions);
			}
			break;
		}




		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:

			Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
			break;
		case EFlareQuestCondition::QUEST_SUCCESSFUL:
		case EFlareQuestCondition::QUEST_FAILED:
			Callbacks.AddUnique(EFlareQuestCallback::QUEST);
			break;
		default:
			FLOGV("ERROR: GetConditionCallbacks not implemented for condition type %d", (int)(Condition->Type +0));
			break;
	}

	return Callbacks;
}
*/

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

/*
FFlareQuestStepProgressSave* UFlareQuest::CreateStepProgressSave(const FFlareQuestConditionDescription* Condition)
{
	FName SaveId = FName(*FString::FromInt(Condition->Type + 0));
	if (Condition->ConditionIdentifier != NAME_None)
	{
		SaveId = Condition->ConditionIdentifier;
	}

	FFlareQuestStepProgressSave NewSave;
	NewSave.ConditionIdentifier = SaveId;
	NewSave.CurrentProgression = 0;
	NewSave.InitialTransform = FTransform::Identity;
	QuestData.CurrentStepProgress.Add(NewSave);
	return &QuestData.CurrentStepProgress.Last();
}
*/
#undef LOCTEXT_NAMESPACE
