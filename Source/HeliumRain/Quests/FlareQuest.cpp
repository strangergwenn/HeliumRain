
#include "FlareQuest.h"
#include "Flare.h"
#include "../Game/FlareGame.h"
#include "../Player/FlarePlayerController.h"
#include "FlareQuestStep.h"
#include "FlareQuestCondition.h"
#include "FlareQuestAction.h"

#include "GameFramework/InputSettings.h"

#define LOCTEXT_NAMESPACE "FlareQuest"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuest::UFlareQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  TrackObjectives(false),
	  Client(NULL)
{
	Accepted = false;
	QuestData.AvailableDate = 0;
	QuestData.AcceptationDate = 0;
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/


void UFlareQuest::LoadInternal(UFlareQuestManager* Parent)
{
	QuestManager = Parent;
	QuestStatus = EFlareQuestStatus::PENDING;

	TriggerCondition = UFlareQuestConditionAndGroup::Create(this, true);
	ExpirationCondition = UFlareQuestConditionOrGroup::Create(this, false);
}

void UFlareQuest::Restore(const FFlareQuestProgressSave& Data)
{
	QuestData = Data;
	QuestStatus = Data.Status;


	for (UFlareQuestCondition* Condition: TriggerCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data.TriggerConditionsSave));
	}

	for (UFlareQuestCondition* Condition: ExpirationCondition->GetAllConditions())
	{
		Condition->Restore(UFlareQuestCondition::GetStepConditionBundle(Condition, Data.ExpirationConditionsSave));
	}

	CurrentStep = NULL;

	// Init current step
	for(UFlareQuestStep* Step : Steps)
	{
		if(Data.SuccessfullSteps.Contains(Step->GetIdentifier()))
		{
			SuccessfullSteps.Add(Step);
			Step->SetStatus(EFlareQuestStepStatus::COMPLETED);
		}
		else if(CurrentStep == NULL)
		{
			CurrentStep = Step;
			Step->Restore(Data.CurrentStepProgress);
		}

	}

	NextStep(true);
}

FFlareQuestProgressSave* UFlareQuest::Save()
{
	QuestData.SuccessfullSteps.Empty();
	QuestData.CurrentStepProgress.Empty();
	QuestData.TriggerConditionsSave.Empty();
	QuestData.ExpirationConditionsSave.Empty();

	QuestData.QuestIdentifier = GetIdentifier();
	QuestData.Status = QuestStatus;

	for(UFlareQuestStep* Step : SuccessfullSteps)
	{
		QuestData.SuccessfullSteps.Add(Step->GetIdentifier());
	}

	for (UFlareQuestCondition* Condition: TriggerCondition->GetAllConditions())
	{
		Condition->AddSave(QuestData.TriggerConditionsSave);
	}

	for (UFlareQuestCondition* Condition: ExpirationCondition->GetAllConditions())
	{
		Condition->AddSave(QuestData.ExpirationConditionsSave);
	}

	if(CurrentStep)
	{
		CurrentStep->Save(QuestData.CurrentStepProgress);
	}
	return &QuestData;
}

void UFlareQuest::SetupIndexes()
{
	int32 StepIndex = 0;
	for(UFlareQuestStep* Step : GetSteps())
	{
		Step->SetupStepIndexes(StepIndex++);
	}

	// Setup condition indexes
	int32 ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: TriggerCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}

	// Setup condition indexes
	ConditionIndex = 0;
	for (UFlareQuestCondition* Condition: ExpirationCondition->GetAllConditions())
	{
		Condition->SetConditionIndex(ConditionIndex++);
	}
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
		case EFlareQuestStatus::PENDING:
		{
			bool ConditionsStatus = TriggerCondition->IsCompleted();
			if (ConditionsStatus)
			{
				MakeAvailable();
				UpdateState();
			}
			break;
		}
		case EFlareQuestStatus::AVAILABLE:
		{
			bool ConditionsStatus = ExpirationCondition->IsCompleted();
			if (ConditionsStatus)
			{
				Abandon(true);
				break;
			}

			if (IsAccepted() || HasAutoAccept())
			{
				MakeOngoing();
				UpdateState();
			}
			break;
		}
		case EFlareQuestStatus::ONGOING:
		{
			if(!CurrentStep)
			{
				FLOGV("ERROR: Ongoing quest '%s'without current Step", *Identifier.ToString());
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

void UFlareQuest::NextStep(bool Silent)
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

				if (!Silent)
				{
					FText MessageText = FormatTags(Step->GetStepDescription());
					SendQuestNotification(MessageText, GetQuestNotificationTag(), GetQuestCategory() == EFlareQuestCategory::TUTORIAL);
				}

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
	if (QuestStatus == EFlareQuestStatus::SUCCESSFUL)
	{
		// Already successful
		return;
	}

	QuestManager->GetGame()->GetPC()->GetMenuManager()->ClearNotifications(GetQuestNotificationTag());
	SetStatus(EFlareQuestStatus::SUCCESSFUL);
	UFlareQuestAction::PerformActions(SuccessActions);
	QuestManager->OnQuestSuccess(this);
}

void UFlareQuest::Fail()
{
	if (QuestStatus == EFlareQuestStatus::FAILED)
	{
		// Already failed
		return;
	}

	QuestManager->GetGame()->GetPC()->GetMenuManager()->ClearNotifications(GetQuestNotificationTag());

	SetStatus(EFlareQuestStatus::FAILED);
	UFlareQuestAction::PerformActions(FailActions);
	QuestManager->OnQuestFail(this, true);
}

void UFlareQuest::Abandon(bool Expired)
{
	if (QuestStatus == EFlareQuestStatus::ABANDONED)
	{
		// Already abandoned
		return;
	}

	QuestManager->GetGame()->GetPC()->GetMenuManager()->ClearNotifications(GetQuestNotificationTag());

	SetStatus(EFlareQuestStatus::ABANDONED);
	if(!Expired)
	{
		UFlareQuestAction::PerformActions(FailActions);
	}
	QuestManager->OnQuestFail(this, Expired ? false : true);
}

void UFlareQuest::MakeAvailable()
{
	if (QuestStatus == EFlareQuestStatus::AVAILABLE)
	{
		// Already available
		return;
	}

	QuestData.AvailableDate = QuestManager->GetGame()->GetGameWorld()->GetDate();
	SetStatus(EFlareQuestStatus::AVAILABLE);

	if (QuestStatus == EFlareQuestStatus::AVAILABLE)
	{
		QuestManager->OnQuestAvailable(this);
	}
}

void UFlareQuest::Accept()
{
	Accepted = true;
	UpdateState();
}

void UFlareQuest::MakeOngoing()
{
	if (QuestStatus == EFlareQuestStatus::ONGOING)
	{
		// Already ongoing
		return;
	}

	QuestData.AcceptationDate = QuestManager->GetGame()->GetGameWorld()->GetDate();
	SetStatus(EFlareQuestStatus::ONGOING);
	// Activate next step
	NextStep();

	// Don't notify ongoing if quest is not ongoing after first NextStep
	if (QuestStatus == EFlareQuestStatus::ONGOING)
	{
		QuestManager->OnQuestOngoing(this);
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

	FFlareMenuParameterData Data;
	Data.Quest = this;

	QuestManager->GetGame()->GetPC()->Notify(Text, Message, Tag, EFlareNotification::NT_Quest, Pinned, EFlareMenu::MENU_Quest, Data);
}

bool UFlareQuest::HasAutoAccept()
{
	return GetQuestCategory() == EFlareQuestCategory::TUTORIAL;
}

bool UFlareQuest::IsActive()
{
	return QuestStatus == EFlareQuestStatus::PENDING || QuestStatus == EFlareQuestStatus::AVAILABLE || QuestStatus == EFlareQuestStatus::ONGOING;
}

UFlareSimulatedSector* UFlareQuest::FindSector(FName SectorIdentifier)
{
	UFlareSimulatedSector* Sector = QuestManager->GetGame()->GetGameWorld()->FindSector(SectorIdentifier);

	if(!Sector)
	{
		FLOGV("ERROR: Fail to find sector '%s' for quest '%s'",
		  *SectorIdentifier.ToString(),
		  *GetIdentifier().ToString());
	}
	return Sector;
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
	Callbacks
----------------------------------------------------*/

TArray<UFlareQuestCondition*> UFlareQuest::GetCurrentConditions()
{
	TArray<UFlareQuestCondition*> Conditions;

	switch(QuestStatus)
	{
		case EFlareQuestStatus::PENDING:
			// Use trigger conditions
			Conditions += TriggerCondition->GetAllConditions();
			break;
		case EFlareQuestStatus::ONGOING:
		 {
			// Use current step conditions
			if (CurrentStep)
			{
				Conditions += CurrentStep->GetEnableCondition()->GetAllConditions();
				Conditions += CurrentStep->GetEndCondition()->GetAllConditions();
				Conditions += CurrentStep->GetFailCondition()->GetAllConditions();
				Conditions += CurrentStep->GetBlockCondition()->GetAllConditions();
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

	return Conditions;
}

TArray<EFlareQuestCallback::Type> UFlareQuest::GetCurrentCallbacks()
{
	TArray<EFlareQuestCallback::Type> Callbacks;

	// TODO Cache and return reference
	switch(QuestStatus)
	{
		case EFlareQuestStatus::PENDING:
			// Use trigger conditions
			UFlareQuestCondition::AddConditionCallbacks(Callbacks, TriggerCondition->GetAllConditions());

			if (Callbacks.Contains(EFlareQuestCallback::TICK_FLYING))
			{
				FLOGV("WARNING: The quest %s need a TICK_FLYING callback as trigger", *GetIdentifier().ToString());
			}
			break;
		case EFlareQuestStatus::AVAILABLE:
			// Use expiration conditions
			UFlareQuestCondition::AddConditionCallbacks(Callbacks, ExpirationCondition->GetAllConditions());
			break;
		case EFlareQuestStatus::ONGOING:
		 {
			// Use current step conditions
			if (CurrentStep)
			{
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetEnableCondition()->GetAllConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetEndCondition()->GetAllConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetFailCondition()->GetAllConditions());
				UFlareQuestCondition::AddConditionCallbacks(Callbacks, CurrentStep->GetBlockCondition()->GetAllConditions());
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

void UFlareQuest::OnTradeDone(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 Quantity)
{
	for (UFlareQuestCondition* Condition : GetCurrentConditions())
	{
		Condition->OnTradeDone(SourceSpacecraft, DestinationSpacecraft, Resource, Quantity);
	}
}

void UFlareQuest::OnSpacecraftDestroyed(UFlareSimulatedSpacecraft* Spacecraft, bool Uncontrollable, UFlareCompany* Source)
{
	for (UFlareQuestCondition* Condition : GetCurrentConditions())
	{
		Condition->OnSpacecraftDestroyed(Spacecraft, Uncontrollable, Source);
	}
}

void UFlareQuest::OnSpacecraftCaptured(UFlareSimulatedSpacecraft* CapturedSpacecraftBefore, UFlareSimulatedSpacecraft* CapturedSpacecraftAfter)
{
	for (UFlareQuestCondition* Condition : GetCurrentConditions())
	{
		Condition->OnSpacecraftCaptured(CapturedSpacecraftBefore, CapturedSpacecraftAfter);
	}
}

void UFlareQuest::OnTravelStarted(UFlareTravel* Travel)
{
	for (UFlareQuestCondition* Condition : GetCurrentConditions())
	{
		Condition->OnTravelStarted(Travel);
	}
}

void UFlareQuest::OnEvent(FFlareBundle& Bundle)
{
	for (UFlareQuestCondition* Condition : GetCurrentConditions())
	{
		Condition->OnEvent(Bundle);
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

FText UFlareQuest::GetQuestReward()
{
	TArray<UFlareQuestAction*> Actions = GetSuccessActions();

	FString Result;

	for (auto Action : Actions)
	{
		UFlareQuestActionGiveMoney* MoneyAction = Cast<UFlareQuestActionGiveMoney>(Action);
		if (MoneyAction)
		{
			Result += FText::Format(LOCTEXT("QuestRewardMoneyFormat", "\u2022 Payment of {0} credits\n"), FText::AsNumber(UFlareGameTools::DisplayMoney(MoneyAction->GetAmount()))).ToString();
		}

		UFlareQuestActionGiveResearch* ResearchAction = Cast<UFlareQuestActionGiveResearch>(Action);
		if (ResearchAction)
		{
			Result += FText::Format(LOCTEXT("QuestRewardResearchFormat", "\u2022 Gain of {0} research\n"), FText::AsNumber(ResearchAction->GetAmount())).ToString();
		}

		UFlareQuestActionDiscoverSector* SectorAction = Cast<UFlareQuestActionDiscoverSector>(Action);
		if (SectorAction)
		{
			Result += FText::Format(LOCTEXT("QuestRewardSectorFormat", "\u2022 Coordinates to sector {0}\n"), SectorAction->GetSector()->GetSectorName()).ToString();
		}

		UFlareQuestActionReputationChange* ReputationAction = Cast<UFlareQuestActionReputationChange>(Action);
		if (ReputationAction)
		{
			Result += FText::Format(LOCTEXT("QuestRewardReputationFormat", "\u2022 Gain of {0} reputation\n"), FText::AsNumber(ReputationAction->GetAmount())).ToString();
		}
	}

	if (Result.Len() == 0)
	{
		if (Actions.Num() == 0)
		{
			return LOCTEXT("QuestRewardNone", "\u2022 No rewards");
		}
		else
		{
			return LOCTEXT("QuestRewardUnknown", "\u2022 Unknown rewards");
		}
	}

	return FText::FromString(Result);
}

FText UFlareQuest::GetQuestPenalty()
{
	TArray<UFlareQuestAction*> Actions = GetFailActions();

	FString Result;

	for (auto Action : Actions)
	{
		UFlareQuestActionGiveMoney* MoneyAction = Cast<UFlareQuestActionGiveMoney>(Action);
		if (MoneyAction)
		{
			Result += FText::Format(LOCTEXT("QuestPenaltyMoneyFormat", "\u2022 Fine of {0} credits\n"), FText::AsNumber(UFlareGameTools::DisplayMoney(-MoneyAction->GetAmount()))).ToString();
		}

		UFlareQuestActionReputationChange* ReputationAction = Cast<UFlareQuestActionReputationChange>(Action);
		if (ReputationAction)
		{
			Result += FText::Format(LOCTEXT("QuestPenaltyReputationFormat", "\u2022 Loss of {0} reputation\n"), FText::AsNumber(-ReputationAction->GetAmount())).ToString();
		}
	}

	if (Result.Len() == 0)
	{
		if (Actions.Num() == 0)
		{
			return LOCTEXT("QuestPenaltyNone", "\u2022 No penalties");
		}
		else
		{
			return LOCTEXT("QuestPenaltyUnknown", "\u2022 Unknown penalties");
		}
	}

	return FText::FromString(Result);
}

FText UFlareQuest::GetQuestExpiration()
{
	FString Result;

	for (auto Condition : ExpirationCondition->GetAllConditions())
	{
		if (Result.Len())
		{
			Result += "\n";
		}
		Result += LOCTEXT("QuestExpirationSymbol", "\u2022 ").ToString() + Condition->GetInitialLabel().ToString();
	}

	if (Result.Len())
	{
		return FText::FromString(Result);
	}
	else
	{
		return LOCTEXT("QuestExpirationNone", "\u2022 This contract never expires.");
	}
}

TArray<UFlareQuestCondition*> UFlareQuest::GetGlobalFailConditions()
{
	TArray<UFlareQuestCondition*> GlobalFailConditions;

	for(UFlareQuestStep* Step : Steps)
	{
		TArray<UFlareQuestCondition*> StepFailConditions = Step->GetFailCondition()->GetAllConditions();

		if(Step == Steps[0])
		{
			GlobalFailConditions.Append(StepFailConditions);
		}
		else
		{
			TArray<UFlareQuestCondition*> ConditionsToRemove;

			// Remove missing conditions
			for (auto Condition : GlobalFailConditions)
			{
				if(!StepFailConditions.Contains(Condition))
				{
					ConditionsToRemove.Add(Condition);
				}
			}

			for (auto Condition : ConditionsToRemove)
			{
				GlobalFailConditions.Remove(Condition);
			}
		}
	}
	return GlobalFailConditions;
}


FText UFlareQuest::GetQuestFailure()
{
	FString Result;

	TArray<UFlareQuestCondition*> FailConditions;
	// TODO: uncomment if you want failure to be constant instead of exact
	//TArray<UFlareQuestCondition*> FailConditions = GetGlobalFailConditions();

	// TODO: comment if you want failure to be constant instead of exact
	if(CurrentStep)
	{
		for(auto Condition: CurrentStep->GetFailCondition()->GetAllConditions())
		{
			FailConditions.AddUnique(Condition);
		}
	}
	else
	{
		FailConditions = GetGlobalFailConditions();
	}

	for (auto Condition : FailConditions)
	{
		if (Result.Len())
		{
			Result += "\n";
		}
		Result += LOCTEXT("QuestFailureSymbol", "\u2022 ").ToString() + Condition->GetInitialLabel().ToString();
	}

	if (Result.Len())
	{
		return FText::FromString(Result);
	}
	else
	{
		return LOCTEXT("QuestFailureNone", "\u2022 This contract cannot fail.");
	}
}

FName UFlareQuest::GetQuestNotificationTag() const
{
	return FName(*(FString("quest-") + GetIdentifier().ToString() + "-message"));
}

FText UFlareQuest::GetStatusText() const
{
	switch (QuestStatus)
	{
		case EFlareQuestStatus::PENDING:      return LOCTEXT("QuestPending", "Pending");       break;
		case EFlareQuestStatus::AVAILABLE:    return LOCTEXT("QuestAvailable", "Available");   break;
		case EFlareQuestStatus::ONGOING:       return LOCTEXT("QuestOngoing", "Ongoing");         break;
		case EFlareQuestStatus::SUCCESSFUL:   return LOCTEXT("QuestCompleted", "Completed");   break;
		case EFlareQuestStatus::ABANDONED:    return LOCTEXT("QuestAbandoned", "Abandoned");   break;
		case EFlareQuestStatus::FAILED:       return LOCTEXT("QuestFailed", "Failed");         break;
		default:                              return LOCTEXT("QuestUnknown", "Unknown");       break;
	}
}

#undef LOCTEXT_NAMESPACE
