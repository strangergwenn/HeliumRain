#pragma once

#include "FlareQuestCondition.generated.h"

class UFlareQuest;
class AFlarePlayerController;
class AFlareGame;
class UFlareSimulatedSector;
struct FFlarePlayerObjectiveData;

/** A quest Step condition */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestCondition: public UObject
{
	GENERATED_UCLASS_BODY()

protected:

	  void LoadInternal(UFlareQuest* ParentQuest)
	  {
		  Quest = ParentQuest;
	  }

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	int32                                   ConditionIndex;
	FText                       InitialLabel;
	FText						TerminalLabel;
	TArray<EFlareQuestCallback::Type> Callbacks;

	UFlareQuest* Quest;
public:

	/*----------------------------------------------------
	 Getters
	----------------------------------------------------*/

	virtual bool IsCompleted();

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

	AFlareGame* GetGame();
	AFlarePlayerController* GetPC();

};


UCLASS()
class HELIUMRAIN_API UFlareQuestConditionFlyingShipClass: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionFlyingShipClass* Create(UFlareQuest* ParentQuest, FName ShipClassParam);
	void Load(UFlareQuest* ParentQuest, FName ShipClassParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	FName ShipClass;
};

UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSectorActive: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionSectorActive* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* Sector;
};

UCLASS()
class HELIUMRAIN_API UFlareQuestConditionSectorVisited: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionSectorVisited* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* Sector;
};
