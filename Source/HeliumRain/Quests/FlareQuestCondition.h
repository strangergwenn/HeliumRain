#pragma once

#include "FlareQuestManager.h"
#include "FlareQuestCondition.generated.h"

class UFlareQuest;
class AFlarePlayerController;
class AFlareGame;
class UFlareSimulatedSector;
struct FFlarePlayerObjectiveData;
struct FFlareBundle;

/** A quest Step condition */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestCondition: public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Restore(const FFlareBundle* Bundle)
	{
	}

protected:

	  void LoadInternal(UFlareQuest* ParentQuest, FName ConditionIdentifier = NAME_None)
	  {
		  Identifier = ConditionIdentifier;
		  Quest = ParentQuest;
	  }

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	int32                                   ConditionIndex;
	FText                       InitialLabel;
	FText						TerminalLabel;
	FName                       Identifier;
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

	static const FFlareBundle* GetStepConditionBundle(UFlareQuestCondition* Condition, const TArray<FFlareQuestStepProgressSave>& Data);

	FName GetIdentifier()
	{
		return Identifier;
	}

	AFlareGame* GetGame();
	AFlarePlayerController* GetPC();

};

//////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinCollinearVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinCollinearVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinearVelocity();


protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxCollinearVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxCollinearVelocity* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, float VelocityLimitParam);
	virtual void Restore(const FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinearVelocity();


protected:

	float VelocityLimit;
	bool HasInitialVelocity;
	float InitialVelocity;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinCollinear: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinCollinear* Create(UFlareQuest* ParentQuest, float CollinearLimitParam);
	void Load(UFlareQuest* ParentQuest, float CollinearLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinear();

protected:

	float CollinearLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxCollinear: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxCollinear* Create(UFlareQuest* ParentQuest, float CollinearLimitParam);
	void Load(UFlareQuest* ParentQuest, float CollinearLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetCollinear();

protected:

	float CollinearLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMinRotationVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMinRotationVelocity* Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVelocityInAxis();

protected:

	FVector LocalAxis;
	float AngularVelocityLimit;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionMaxRotationVelocity: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionMaxRotationVelocity* Create(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);
	void Load(UFlareQuest* ParentQuest, FVector LocalAxisParam, float AngularVelocityLimitParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	float GetVelocityInAxis();

protected:

	FVector LocalAxis;
	float AngularVelocityLimit;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionShipAlive: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionShipAlive* Create(UFlareQuest* ParentQuest, FName ShipIdentifierParam);
	void Load(UFlareQuest* ParentQuest, FName ShipIdentifierParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

	bool IsShipAlive();

protected:

	FName ShipIdentifier;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionQuestSuccessful: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionQuestSuccessful* Create(UFlareQuest* ParentQuest, UFlareQuest* QuestParam);
	void Load(UFlareQuest* ParentQuest, UFlareQuest* QuestParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareQuest* TargetQuest;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionQuestFailed: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionQuestFailed* Create(UFlareQuest* ParentQuest, UFlareQuest* QuestParam);
	void Load(UFlareQuest* ParentQuest, UFlareQuest* QuestParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareQuest* TargetQuest;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionFollowRelativeWaypoints: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionFollowRelativeWaypoints* Create(UFlareQuest* ParentQuest, TArray<FVector> VectorListParam);
	void Load(UFlareQuest* ParentQuest, TArray<FVector> VectorListParam);
	virtual void Restore(const FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	void Init();

	TArray<FVector> VectorList;

	bool IsInit;
	int32 CurrentProgression;
	FTransform InitialTransform;
};
