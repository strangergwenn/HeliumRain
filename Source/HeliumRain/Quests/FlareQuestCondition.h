#pragma once

#include "FlareQuestCondition.generated.h"

struct FFlarePlayerObjectiveData;

/** A quest Step condition */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestCondition: public UObject
{
	GENERATED_UCLASS_BODY()

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	int32                                   ConditionIndex;
	FText                       InitialLabel;
	FText						TerminalLabel;
	TArray<EFlareQuestCallback::Type> Callbacks;

public:

	/*----------------------------------------------------
	 Getters
	----------------------------------------------------*/

	virtual bool IsCompleted(bool EmptyResult);

	int32 GetConditionIndex()
	{
		return ConditionIndex;
	}

	FText GetInitialLabel()
	{
		return InitialLabel;
	}

	FText GetTerminalLabel()
	{
		return TerminalLabel;
	}

	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	virtual TArray<EFlareQuestCallback::Type>& GetConditionCallbacks()
	{
		return Callbacks;
	}

	static bool CheckConditions(TArray<UFlareQuestCondition*>& Conditions, bool EmptyResult);

	static void AddConditionCallbacks(TArray<EFlareQuestCallback::Type>& Callbacks, const TArray<UFlareQuestCondition*>& Conditions);


};
