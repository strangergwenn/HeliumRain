#pragma once

#include "../FlareQuest.h"
#include "../FlareQuestCondition.h"
#include "FlareHistoryQuest.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareQuestPendulum: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionDockAtType: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:

	static UFlareQuestConditionDockAtType* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	UFlareSimulatedSector* TargetSector;
	FName TargetStationType;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionVisitSector: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionVisitSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	UFlareSimulatedSector* TargetSector;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionWaypoints: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionWaypoints* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:

	void Init();

	TArray<FVector> VectorList;

	bool IsInit;
	int32 CurrentProgression;
	UFlareSimulatedSector* TargetSector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestStationCount : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestStationCount* Create(UFlareQuest* ParentQuest, FName StationIdentifier, int32 Count);
	void Load(UFlareQuest* ParentQuest, FName StationIdentifier, int32 Count);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 GetStationCount();

	FName TargetStationIdentifier;
	int32 TargetStationCount;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestBringResource : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestBringResource* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 GetResourceCount();
	UFlareSimulatedSector* TargetSector;
	FName TargetResourceIdentifier;
	int32 TargetResourceCount;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareCompanyMaxCombatValue: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()

public:
	static UFlareCompanyMaxCombatValue* Create(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:
	UFlareCompany* TargetCompany;
	int32 TargetArmyPoints;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestMinSectorStationCount : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestMinSectorStationCount* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, int32 Count);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, int32 Count);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:

	UFlareSimulatedSector* TargetSector;
	int32 TargetStationCount;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionWait: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionWait* Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, int32 Duration);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, int32 Duration);
	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
	virtual FText GetInitialLabel();

protected:

	void Init();

	bool IsInit;
	int32 TargetDuration;
	int64 StartDate;
};
