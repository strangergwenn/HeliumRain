#pragma once

#include <functional>
#include "FlareQuestAction.generated.h"

struct FFlareResourceDescription;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
class UFlareQuest;
class UFlareCompany;

/** A quest Step action */
UCLASS(abstract)
class HELIUMRAIN_API UFlareQuestAction: public UObject
{
	GENERATED_UCLASS_BODY()


public:
	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void Perform();

	static void PerformActions(const TArray<UFlareQuestAction*>& Actions);
protected:

	void LoadInternal(UFlareQuest* ParentQuest)
	{
		Quest = ParentQuest;
	}

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareQuest* Quest;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/




};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionDiscoverSector: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionDiscoverSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual void Perform();

	inline UFlareSimulatedSector* GetSector() const
	{
		return Sector;
	}

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionVisitSector: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionVisitSector* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionPrintMessage: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionPrintMessage* Create(UFlareQuest* ParentQuest, FText MessageParam);
	void Load(UFlareQuest* ParentQuest, FText MessageParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	FText Message;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionGiveMoney: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionGiveMoney* Create(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int64 AmountParam);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int64 AmountParam);

	virtual void Perform();

	inline int64 GetAmount() const
	{
		return Amount;
	}

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareCompany* FromCompany;
	UFlareCompany* ToCompany;
	int64 Amount;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionGiveResearch: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionGiveResearch* Create(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int32 AmountParam);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* FromCompanyParam, UFlareCompany* ToCompanyParam, int32 AmountParam);

	virtual void Perform();

	inline int64 GetAmount() const
	{
		return Amount;
	}

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareCompany* FromCompany;
	UFlareCompany* ToCompany;
	int64 Amount;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionReputationChange: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionReputationChange* Create(UFlareQuest* ParentQuest, UFlareCompany* CompanyParam, int64 AmountParam);
	void Load(UFlareQuest* ParentQuest, UFlareCompany* CompanyParam, int64 AmountParam);

	virtual void Perform();

	inline int64 GetAmount() const
	{
		return Amount;
	}

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareCompany* Company;
	int64 Amount;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionTakeResources: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionTakeResources* Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam, FName ResourceIdentifierParam, int32 CountParam);
	void Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* SectorParam, FName ResourceIdentifierParam, int32 CountParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector* Sector;
	FFlareResourceDescription* Resource;
	int32 Count;
};


//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestActionGeneric: public UFlareQuestAction
{
	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	static UFlareQuestActionGeneric* Create(UFlareQuest* ParentQuest, std::function<void ()> PerfomFuncParam);
	void Load(UFlareQuest* ParentQuest, std::function<void ()> PerfomFuncParam);

	virtual void Perform();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	std::function<void ()> PerfomFunc;
};
