#pragma once

#include "../FlareQuest.h"
#include "FlareTutorialQuest.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialFlying: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};


UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialNavigation: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};

UCLASS()
class HELIUMRAIN_API UFlareQuestTutorialContracts: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:

	/** Load the quest from description file */
	virtual void Load(UFlareQuestManager* Parent);
	static UFlareQuest* Create(UFlareQuestManager* Parent);
};


// Conditions


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialGetContrat: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialGetContrat* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialOpenMenu: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialOpenMenu* Create(UFlareQuest* ParentQuest, EFlareMenu::Type Menu);
	void Load(UFlareQuest* ParentQuest, EFlareMenu::Type Menu);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	EFlareMenu::Type MenuType;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialAcceptQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialAcceptQuest* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialTrackQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialTrackQuest* Create(UFlareQuest* ParentQuest);
	void Load(UFlareQuest* ParentQuest);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);

protected:
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialFinishQuest: public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialFinishQuest* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 Count);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 Count);

	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	int32 QuestCount;
	int32 CurrentProgression;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialCommandDock : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialCommandDock* Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* TargetParam);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	UFlareSimulatedSpacecraft* Target;
	bool Completed;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestConditionTutorialTargetSpacecraft : public UFlareQuestCondition
{
	GENERATED_UCLASS_BODY()


public:

	static UFlareQuestConditionTutorialTargetSpacecraft* Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* Spacecraft,  float Duration);
	void Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, UFlareSimulatedSpacecraft* Spacecraft, float Duration);

	virtual void Restore(const FFlareBundle* Bundle);
	virtual void Save(FFlareBundle* Bundle);

	virtual bool IsCompleted();
	virtual void OnEvent(FFlareBundle& Bundle);
	virtual void AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData);
protected:
	float TargetDuration;
	UFlareSimulatedSpacecraft* TargetSpacecraft;
	float LastTargetChangeTimestamp;
	int32 CurrentProgression;
};
