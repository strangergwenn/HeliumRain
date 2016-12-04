#include "Flare.h"
#include "FlareQuestCondition.h"
#include "FlareQuest.h"
#include "../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareQuestCondition"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestCondition::UFlareQuestCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Quest(NULL)
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
		if(!Condition->IsCompleted())
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

bool UFlareQuestCondition::IsCompleted()
{
	FLOG("ERROR: Not implemented IsCompleted");
	return false;
}

AFlareGame* UFlareQuestCondition::GetGame()
{
	return Quest->GetQuestManager()->GetGame();
}

AFlarePlayerController* UFlareQuestCondition::GetPC()
{
	return Quest->GetQuestManager()->GetGame()->GetPC();
}


/*----------------------------------------------------
	Flying ship class condition
----------------------------------------------------*/
UFlareQuestConditionFlyingShipClass::UFlareQuestConditionFlyingShipClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionFlyingShipClass* UFlareQuestConditionFlyingShipClass::Create(UFlareQuest* ParentQuest, FName ShipClassParam)
{
	UFlareQuestConditionFlyingShipClass*Condition = NewObject<UFlareQuestConditionFlyingShipClass>(ParentQuest, UFlareQuestConditionFlyingShipClass::StaticClass());
	Condition->Load(ParentQuest, ShipClassParam);
	return Condition;
}

void UFlareQuestConditionFlyingShipClass::Load(UFlareQuest* ParentQuest, FName ShipClassParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::FLY_SHIP);
	ShipClass = ShipClassParam;
}

bool UFlareQuestConditionFlyingShipClass::IsCompleted()
{
	if (GetPC()->GetShipPawn())
	{
		if (ShipClass == NAME_None)
		{
			// No specific ship required
			return true;
		}
		else if (ShipClass == GetPC()->GetPlayerShip()->GetDescription()->Identifier)
		{
			// The flyed ship is the right kind of ship
			return true;
		}
	}
	return false;
}

void UFlareQuestConditionFlyingShipClass::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlareSpacecraftDescription* SpacecraftDesc = GetGame()->GetSpacecraftCatalog()->Get(ShipClass);
	if (SpacecraftDesc)
	{
		AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = FText::Format(LOCTEXT("FlyShipFormat", "Fly a {0}-class ship"), SpacecraftDesc->Name);
		ObjectiveCondition.TerminalLabel = FText();
		ObjectiveCondition.Progress = 0;
		ObjectiveCondition.MaxProgress = 0;
		ObjectiveCondition.Counter = (Spacecraft && Spacecraft->GetDescription()->Identifier == ShipClass) ? 1 : 0;
		ObjectiveCondition.MaxCounter = 1;

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

#undef LOCTEXT_NAMESPACE
