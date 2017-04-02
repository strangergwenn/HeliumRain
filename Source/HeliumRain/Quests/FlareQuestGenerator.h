#pragma once
#include "Game/FlareGameTypes.h"
#include "FlareQuestManager.h"
#include "FlareQuestGenerator.generated.h"

class AFlareGame;
class UFlareQuestManager;
class UFlareQuest;
class UFlareSimulatedSector;
class UFlareQuestGenerated;

/** Quest generator */
UCLASS()
class HELIUMRAIN_API UFlareQuestGenerator: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Load(UFlareQuestManager* Parent, const FFlareQuestSave& Data);

	void LoadQuests(const FFlareQuestSave& Data);

	virtual void Save(FFlareQuestSave& Data);

	/*----------------------------------------------------
		Quest generation
	----------------------------------------------------*/

	static FText GeneratePersonName();

	void GenerateIdentifer(FName QuestClass, FFlareBundle& Data);

	void GenerateSectorQuest(UFlareSimulatedSector* Sector);

	void GenerateMilitaryQuests();

	void RegisterQuest(UFlareQuestGenerated* Quest);

	bool FindUniqueTag(FName Tag);

	FName GenerateVipTag(UFlareSimulatedSpacecraft* SourceSpacecraft);

	FName GenerateTradeTag(UFlareSimulatedSpacecraft* SourceSpacecraft, FFlareResourceDescription* Resource);

	FName GenerateDefenseTag(UFlareSimulatedSector* Sector, UFlareCompany* OwnerCompany, UFlareCompany* HostileCompany);

protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UFlareQuestManager*						QuestManager;

	AFlareGame*                             Game;

	int64                                   NextQuestIndex;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
	}
protected:
	UPROPERTY()
	TArray<UFlareQuestGenerated*>	                 GeneratedQuests;
};


UCLASS()
class HELIUMRAIN_API UFlareQuestGenerated: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);

	void AddGlobalFailCondition(UFlareQuestCondition* Condition);
	void SetupQuestGiver(UFlareCompany* Company, bool AddWarCondition);
	void SetupGenericReward(const FFlareBundle& Data);

	static void CreateGenericReward(FFlareBundle& Data, int64 QuestValue);

	FFlareBundle* GetInitData() {
		return &InitData;
	}

	FName GetQuestClass() {
		return QuestClass;
	}

protected:
	FName QuestClass;
	FFlareBundle InitData;
	UFlareQuestGenerator* QuestGenerator;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedVipTransport: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "vip-transport"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceSale: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-sale"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourcePurchase: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-purchase"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceTrade: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-trade"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedStationDefense: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "station-defense"; }

	/** Load the quest from description file */
	virtual void Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany);
};
