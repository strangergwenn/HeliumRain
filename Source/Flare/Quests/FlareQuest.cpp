
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuest.h"

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


void UFlareQuest::Load(const FFlareQuestDescription* Description)
{
	QuestManager = Cast<UFlareQuestManager>(GetOuter());
	QuestDescription = Description;
	QuestData.QuestIdentifier = QuestDescription->Identifier;
	QuestStatus = EFlareQuestStatus::AVAILABLE;
}

void UFlareQuest::Restore(const FFlareQuestProgressSave& Data)
{
	QuestData = Data;
	QuestStatus = EFlareQuestStatus::ACTIVE;

	// Init current step
	for(int StepIndex = 0; StepIndex < QuestDescription->Steps.Num(); StepIndex++)
	{
		if(!QuestData.SuccessfullSteps.Contains(QuestDescription->Steps[StepIndex].Identifier))
		{
			CurrentStepDescription = &QuestDescription->Steps[StepIndex];
			break;
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
			bool ConditionsStatus = CheckConditions(QuestDescription->Triggers);
			if(ConditionsStatus)
			{
				Activate();
			}
			break;
		}
		case EFlareQuestStatus::ACTIVE:
		{
			const FFlareQuestStepDescription* StepDescription = GetCurrentStepDescription();
			if(StepDescription)
			{
				bool StepEnabled = CheckConditions(StepDescription->EnabledConditions);
				if(StepEnabled)
				{
					bool StepFailed = CheckConditions(StepDescription->FailConditions);
					if(StepFailed)
					{
						Fail();
					}
					else
					{
						bool StepBlocked = CheckConditions(StepDescription->BlockConditions);
						if(!StepBlocked)
						{
							bool StepEnded = CheckConditions(StepDescription->EndConditions);
							if(StepEnded){
								// This step ended go to next step
								EndStep();
							}
						}
					}
				}
				UpdateObjectiveTracker();
			}
			break;
		}

	}
}

void UFlareQuest::EndStep()
{
	const FFlareQuestStepDescription* StepDescription = GetCurrentStepDescription();
	QuestData.SuccessfullSteps.Add(StepDescription->Identifier);
	FLOGV("Quest %s step %s end", *GetIdentifier().ToString(), *StepDescription->Identifier.ToString());

	SendQuestNotification(StepDescription->StepDescription + ": Done", NAME_None);

	PerformActions(StepDescription->EndActions);

	CurrentStepDescription = NULL;

	// Activate next step
	NextStep();
}

void UFlareQuest::NextStep()
{
	// Clear step progress
	CurrentStepDescription = NULL;
	QuestData.CurrentStepProgress.Empty();

	if (QuestDescription->Steps.Num() == 0)
	{
		FLOGV("WARNING: The quest %s have no step", *GetIdentifier().ToString());
		return;
	}

	// Find first not done step
	for(int StepIndex = 0; StepIndex < QuestDescription->Steps.Num(); StepIndex++)
	{
		if(!QuestData.SuccessfullSteps.Contains(QuestDescription->Steps[StepIndex].Identifier))
		{
			CurrentStepDescription = &QuestDescription->Steps[StepIndex];
			FLOGV("Quest %s step %s begin", *GetIdentifier().ToString(), *CurrentStepDescription->Identifier.ToString());
			PerformActions(CurrentStepDescription->InitActions);
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
	PerformActions(QuestDescription->SuccessActions);
	QuestManager->OnQuestSuccess(this);
}

void UFlareQuest::Fail()
{
	SetStatus(EFlareQuestStatus::FAILED);
	PerformActions(QuestDescription->FailActions);
	QuestManager->OnQuestFail(this);
}

void UFlareQuest::Activate()
{
	SetStatus(EFlareQuestStatus::ACTIVE);
	QuestManager->OnQuestActivation(this);
	// Activate next step
	NextStep();
}

bool UFlareQuest::CheckConditions(const TArray<FFlareQuestConditionDescription>& Conditions)
{
	if (Conditions.Num() == 0)
	{
		return false;
	}

	for (int ConditionIndex = 0; ConditionIndex < Conditions.Num(); ConditionIndex++)
	{
		if (!CheckCondition(&Conditions[ConditionIndex]))
		{
			return false;
		}
	}

	return true;
}

bool UFlareQuest::CheckCondition(const FFlareQuestConditionDescription* Condition)
{
	bool Status = false;

	switch(Condition->Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->Identifier1);
			if(SharedCondition)
			{
				Status = CheckConditions(SharedCondition->Conditions);
			}
			break;
		}
		case EFlareQuestCondition::FLYING_SHIP:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				if (Condition->Identifier1 == NAME_None)
				{
					// No specific ship required
					Status = true;
				}
				else if (Condition->Identifier1 == QuestManager->GetGame()->GetPC()->GetShipPawn()->GetDescription()->Identifier)
				{
					// The flyed ship is the right kind of ship
					Status = true;
				}
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEAR_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEAR_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEARITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				if (Spacecraft->GetLinearVelocity().IsNearlyZero())
				{
					Status = false;
				}
				else
				{
					Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector()) > Condition->FloatParam1);
				}
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_COLLINEARITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				if (Spacecraft->GetLinearVelocity().IsNearlyZero())
				{
					Status = false;
				}
				else
				{
					Status = (FVector::DotProduct(Spacecraft->GetLinearVelocity().GetUnsafeNormal(), Spacecraft->GetFrontVector()) < Condition->FloatParam1);
				}
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_PITCH_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Y > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_PITCH_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Y < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_YAW_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Z > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_YAW_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.Z < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MIN_ROLL_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.X > Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_MAX_ROLL_VELOCITY:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				FVector WorldAngularVelocity = Spacecraft->Airframe->GetPhysicsAngularVelocity();
				FVector LocalAngularVelocity = Spacecraft->Airframe->GetComponentToWorld().Inverse().GetRotation().RotateVector(WorldAngularVelocity);
				Status = (LocalAngularVelocity.X < Condition->FloatParam1);
			}
			break;
		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				AFlareSpacecraft* Spacecraft = QuestManager->GetGame()->GetPC()->GetShipPawn();
				// TODO
				FLOG("TODO: SHIP_FOLLOW_RELATIVE_WAYPOINTS");
			}
			break;
		case EFlareQuestCondition::SHIP_ALIVE:
			if (QuestManager->GetGame()->GetPC()->GetShipPawn())
			{
				Status = QuestManager->GetGame()->GetPC()->GetShipPawn()->GetDamageSystem()->IsAlive();
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


void UFlareQuest::PerformActions(const TArray<FFlareQuestActionDescription>& Actions)
{
	for (int ActionIndex = 0; ActionIndex < Actions.Num(); ActionIndex++)
	{
		PerformAction(&Actions[ActionIndex]);
	}

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
		for (int i = 0; i < Action->MessagesParameter.Num(); i++)
		{
			//Replace tags in quests text
			FString MessageString = FormatTags(Action->MessagesParameter[i].Text);

			SendQuestNotification(MessageString, FName(*(FString("quest-")+GetIdentifier().ToString()+"-message")));
		}
		break;
	default:
		FLOGV("ERROR: PerformAction not implemented for action type %d", (int)(Action->Type +0));
		break;
	}

}

FString UFlareQuest::FormatTags(FString Message)
{
	FString MessageString = Message;

	// Replace input tags
	bool Found = false;

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

				FString Mapping;

				UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

				for (int32 i = 0; i < InputSettings->ActionMappings.Num(); i++)
				{
					FInputActionKeyMapping Action = InputSettings->ActionMappings[i];
					if (Action.ActionName == ActionName&& !Action.Key.IsGamepadKey())
					{
						Mapping = Action.Key.ToString();
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

	MessageString = MessageString.Replace(TEXT("<br>"), TEXT("\n"));

	return MessageString;
}



void UFlareQuest::SendQuestNotification(FString Message, FName Tag)
{
	FText Text = FText::FromString(GetQuestName());
	FText Info = FText::FromString(Message);
	float Duration = 5 + Message.Len() / 10.0f;
	FLOGV("Send quest notification: %s", *Message);
	QuestManager->GetGame()->GetPC()->Notify(Text, Info, Tag, EFlareNotification::NT_Quest, Duration);
}

/*----------------------------------------------------
	Objective tracking
----------------------------------------------------*/

void UFlareQuest::StartObjectiveTracking()
{
	if(TrackObjectives)
	{
		return; // No change
	}

	TrackObjectives = true;
	UpdateObjectiveTracker();
}

void UFlareQuest::StopObjectiveTracking()
{
	if(!TrackObjectives)
	{
		return; // No change
	}

	TrackObjectives = false;
	QuestManager->GetGame()->GetPC()->CompleteObjective();
}


void UFlareQuest::UpdateObjectiveTracker()
{
	if(!TrackObjectives)
	{
		return;
	}

	QuestManager->GetGame()->GetPC()->CompleteObjective();

	FText Name = FText::FromString(GetQuestName());
	FText Infos = FText::FromString("");
	if(GetCurrentStepDescription())
	{
		Infos = FText::FromString(GetCurrentStepDescription()->StepDescription);
	}

	QuestManager->GetGame()->GetPC()->StartObjective(Name, Infos);
	QuestManager->GetGame()->GetPC()->SetObjectiveProgress(1.0);
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
			AddConditionCallbacks(Callbacks, QuestDescription->Triggers);

			if(Callbacks.Contains(EFlareQuestCallback::TICK_FLYING))
			{
				FLOGV("WARNING: The quest %s need a TICK_FLYING callback as trigger", *GetIdentifier().ToString());
			}
			break;
		case EFlareQuestStatus::ACTIVE:
		 {
			// Use current step conditions
			const FFlareQuestStepDescription* StepDescription = GetCurrentStepDescription();
			if(StepDescription)
			{
				AddConditionCallbacks(Callbacks, StepDescription->EnabledConditions);
				AddConditionCallbacks(Callbacks, StepDescription->FailConditions);
				AddConditionCallbacks(Callbacks, StepDescription->BlockConditions);
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

void UFlareQuest::AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<FFlareQuestConditionDescription>& Conditions)
{
	for (int ConditionIndex = 0; ConditionIndex < Conditions.Num(); ConditionIndex++)
	{
		TArray<EFlareQuestCallback::Type> ConditionCallbacks = GetConditionCallbacks(&Conditions[ConditionIndex]);
		for (int CallbackIndex = 0; CallbackIndex < ConditionCallbacks.Num(); CallbackIndex++)
		{
			Callbacks.AddUnique(ConditionCallbacks[CallbackIndex]);
		}
	}
}

TArray<EFlareQuestCallback::Type> UFlareQuest::GetConditionCallbacks(const FFlareQuestConditionDescription* Condition)
{
	TArray<EFlareQuestCallback::Type> Callbacks;

	switch(Condition->Type)
	{
		case EFlareQuestCondition::SHARED_CONDITION:
		{
			const FFlareSharedQuestCondition* SharedCondition = FindSharedCondition(Condition->Identifier1);
			if(SharedCondition)
			{
				AddConditionCallbacks(Callbacks, SharedCondition->Conditions);
			}
			break;
		}
		case EFlareQuestCondition::FLYING_SHIP:
			Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
			break;
		case EFlareQuestCondition::SHIP_MIN_COLLINEAR_VELOCITY:
		case EFlareQuestCondition::SHIP_MAX_COLLINEAR_VELOCITY:
		case EFlareQuestCondition::SHIP_MIN_COLLINEARITY:
		case EFlareQuestCondition::SHIP_MAX_COLLINEARITY:
		case EFlareQuestCondition::SHIP_MIN_PITCH_VELOCITY:
		case EFlareQuestCondition::SHIP_MAX_PITCH_VELOCITY:
		case EFlareQuestCondition::SHIP_MIN_YAW_VELOCITY:
		case EFlareQuestCondition::SHIP_MAX_YAW_VELOCITY:
		case EFlareQuestCondition::SHIP_MIN_ROLL_VELOCITY:
		case EFlareQuestCondition::SHIP_MAX_ROLL_VELOCITY:
		case EFlareQuestCondition::SHIP_FOLLOW_RELATIVE_WAYPOINTS:
		case EFlareQuestCondition::SHIP_ALIVE:
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


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareSharedQuestCondition* UFlareQuest::FindSharedCondition(FName SharedConditionIdentifier)
{
	for (int SharedConditionIndex = 0; SharedConditionIndex < QuestDescription->SharedConditions.Num(); SharedConditionIndex++)
	{
		if(QuestDescription->SharedConditions[SharedConditionIndex].Identifier == SharedConditionIdentifier)
		{
			return &QuestDescription->SharedConditions[SharedConditionIndex];
		}
	}

	FLOGV("ERROR: The quest %s don't have shared condition named %s", *GetIdentifier().ToString(), *SharedConditionIdentifier.ToString());

	return NULL;
}


#undef LOCTEXT_NAMESPACE
