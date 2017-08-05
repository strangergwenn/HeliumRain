
#pragma once

#include "Object.h"
#include "FlareFleet.h"
#include "FlareGameTypes.h"
#include "FlareSimulatedSector.h"
#include "AI/FlareCompanyAI.h"
#include "AI/FlareTacticManager.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "FlareCompany.generated.h"


class UFlareFleet;
class UFlareCompanyAI;


class AFlareGame;
class UFlareSimulatedSpacecraft;

UCLASS()
class HELIUMRAIN_API UFlareCompany : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the company from a save file */
	virtual void Load(const FFlareCompanySave& Data);

	/** Post Load to perform task needing sectors to be loaded */
	virtual void PostLoad();

	/** Save the company to a save file */
	virtual FFlareCompanySave* Save();

	/** Spawn a simulated spacecraft from save data */
	virtual UFlareSimulatedSpacecraft* LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData);

	/** Load a fleet from save */
	virtual UFlareFleet* LoadFleet(const FFlareFleetSave& FleetData);

	/** Load a trade route from save */
	virtual UFlareTradeRoute* LoadTradeRoute(const FFlareTradeRouteSave& TradeRouteData);


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void SimulateAI();

	virtual void TickAI();


	/** Check if we are friend or for toward the player */
	virtual EFlareHostility::Type GetPlayerHostility() const;

	/** Check if we are friend or foe toward this target company */
	virtual EFlareHostility::Type GetHostility(const UFlareCompany* TargetCompany) const;

	/** Check if we are friend or foe toward this target company. Hostile if at least one company is hostile */
	virtual EFlareHostility::Type GetPlayerWarState() const;

	/** Check if we are friend or foe toward this target company.  Hostile if at least one company is hostile */
	virtual EFlareHostility::Type GetWarState(const UFlareCompany* TargetCompany) const;

	bool IsAtWar(const UFlareCompany* TargetCompany) const;

	void ResetLastPeaceDate();
	void SetLastWarDate();
	void ClearLastWarDate();
	void ResetLastTributeDate();

	/** Set whether this company is hostile to an other company */
	virtual void SetHostilityTo(UFlareCompany* TargetCompany, bool Hostile);


	/** Get an info string for this company */
	virtual FText GetShortInfoText();

	/** Get a new fleet color */
	FLinearColor PickFleetColor() const;

	/** Create a new fleet in a sector */
	virtual UFlareFleet* CreateFleet(FText FleetName, UFlareSimulatedSector* FleetSector);

	virtual UFlareFleet* CreateAutomaticFleet(UFlareSimulatedSpacecraft * Spacecraft);

	/** Destroy a fleet */
	virtual void RemoveFleet(UFlareFleet* Fleet);

	/** Change fleet order in list */
	void MoveFleetUp(UFlareFleet* Fleet);

	/** Change fleet order in list */
	void MoveFleetDown(UFlareFleet* Fleet);

	/** Create a new trade route */
	virtual UFlareTradeRoute* CreateTradeRoute(FText TradeRouteName);

	/** Destroy a trade route */
	virtual void RemoveTradeRoute(UFlareTradeRoute* TradeRoute);


	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	/** Set a sector discovered */
	virtual void DiscoverSector(UFlareSimulatedSector* Sector);

	/** Set a sector visited */
	virtual void VisitSector(UFlareSimulatedSector* Sector);


	/** Take a money amount from the company */
	virtual bool TakeMoney(int64 Amount, bool AllowDepts = false);

	/** Give a money amount to the company. In cents */
	virtual void GiveMoney(int64 Amount);

	/** Give a research amount to the company */
	virtual void GiveResearch(int64 Amount);
	
	virtual void GivePlayerReputation(float Amount, float Max = -200);

	void GivePlayerReputationToOthers(float Amount);

	/** Compute how much will be necessary to reset reputation with Company */
	int64 GetTributeCost(UFlareCompany* Company);

	/** Make peace with Company in echange for money */
	void PayTribute(UFlareCompany* Company, bool AllowDepts = false);

	void GiveShame(float ShameGain);

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/
	
	/** Update all ships and stations */
	virtual void UpdateCompanyCustomization();

	/** Apply customization to a component's material */
	virtual void CustomizeMaterial(UMaterialInstanceDynamic* Mat);

	/** Setup the emblem for this company, using the company colors */
	void SetupEmblem();

	/** Get the company emblem */
	const FSlateBrush* GetEmblem() const;


	/*----------------------------------------------------
		Technology
	----------------------------------------------------*/

	/** Check if a technology has been unlocked and is used */
	bool IsTechnologyUnlocked(FName Identifier) const;

	/** Check if a technology can be unlocked */
	bool IsTechnologyAvailable(FName Identifier, FText& Reason, bool IgnoreCost=false) const;

	/** Get the current technology cost */
	int32 GetTechnologyCost(const FFlareTechnologyDescription* Technology) const;

	/** Get the current technology level */
	int32 GetTechnologyLevel() const;

	/** Get the current amount of science */
	int32 GetResearchAmount() const;

	/** Get the current amount of science */
	int32 GetResearchSpent() const;

	/** Get the total amount of science */
	int32 GetResearchValue() const;

	/** Unlock a technology */
	void UnlockTechnology(FName Identifier, bool FromSave = false, bool Force = false);

	bool HasStationTechnologyUnlocked() const;
	
	bool IsTechnologyUnlockedStation(const FFlareSpacecraftDescription* Description) const;

	bool IsTechnologyUnlockedPart(const FFlareSpacecraftComponentDescription* Description) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	const FFlareCompanyDescription*         CompanyDescription;
	FFlareCompanySave                       CompanyData;

	UPROPERTY()
	UFlareCompanyAI*                         CompanyAI;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyStations;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyShips;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanySpacecrafts;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyDestroyedSpacecrafts;

	UPROPERTY()
	TArray<UFlareFleet*>                    CompanyFleets;

	UPROPERTY()
	UFlareTacticManager*                    TacticManager;

	UPROPERTY()
	TArray<UFlareTradeRoute*>               CompanyTradeRoutes;

	UPROPERTY()
	UMaterialInstanceDynamic*               CompanyEmblem;

	UPROPERTY()
	FSlateBrush                             CompanyEmblemBrush;

	AFlareGame*                             Game;
	TArray<UFlareSimulatedSector*>          KnownSectors;
	TArray<UFlareSimulatedSector*>          VisitedSectors;

	int32                                   ResearchAmount;
	TMap<FName, FFlareTechnologyDescription*> UnlockedTechnologies;

	mutable struct CompanyValue						CompanyValueCache;
	mutable bool									CompanyValueCacheValid;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/


	void InvalidateCompanyValueCache()
	{
		CompanyValueCacheValid = false;
	}

	/** Get the hostility text */
	FText GetPlayerHostilityText() const;

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline FName GetIdentifier() const
	{
		return CompanyData.Identifier;
	}

	inline const FFlareCompanyDescription* GetDescription() const
	{
		return CompanyDescription;
	}

	inline FText GetCompanyName() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->Name;
	}

	inline FName GetShortName() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->ShortName;
	}

	inline FLinearColor GetBasePaintColor() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->CustomizationBasePaintColor;
	}

	inline FLinearColor GetPaintColor() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->CustomizationPaintColor;
	}

	inline FLinearColor GetOverlayColor() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->CustomizationOverlayColor;
	}

	inline FLinearColor GetLightColor() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->CustomizationLightColor;
	}

	inline int32 GetPatternIndex() const
	{
		FCHECK(CompanyDescription);
		return CompanyDescription->CustomizationPatternIndex;
	}

	inline int64 GetMoney() const
	{
		return CompanyData.Money;
	}

	const struct CompanyValue& GetCompanyValue(UFlareSimulatedSector* SectorFilter = NULL, bool IncludeIncoming = true) const;

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanyStations()
	{
		return CompanyStations;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanyShips()
	{
		return CompanyShips;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetCompanySpacecrafts()
	{
		return CompanySpacecrafts;
	}

	inline TArray<UFlareFleet*>& GetCompanyFleets()
	{
		return CompanyFleets;
	}

	inline TArray<UFlareTradeRoute*>& GetCompanyTradeRoutes()
	{
		return CompanyTradeRoutes;
	}

	inline TArray<UFlareSimulatedSector*>& GetKnownSectors()
	{
		return KnownSectors;
	}

	inline TArray<UFlareSimulatedSector*>& GetVisitedSectors()
	{
		return VisitedSectors;
	}

	inline bool IsKnownSector(UFlareSimulatedSector* Sector) const
	{
		return (KnownSectors.Find(Sector) != INDEX_NONE);
	}

	inline bool IsVisitedSector(UFlareSimulatedSector* Sector) const
	{
		if(Sector->IsTravelSector())
		{
			return true;
		}
		return (VisitedSectors.Find(Sector) != INDEX_NONE);
	}

	UFlareFleet* FindFleet(FName Identifier) const
	{
		for (int i = 0; i < CompanyFleets.Num(); i++)
		{
			if (CompanyFleets[i]->GetIdentifier() == Identifier)
			{
				return CompanyFleets[i];
			}
		}
		return NULL;
	}

	UFlareTradeRoute* FindTradeRoute(FName Identifier) const
	{
		for (int i = 0; i < CompanyTradeRoutes.Num(); i++)
		{
			if (CompanyTradeRoutes[i]->GetIdentifier() == Identifier)
			{
				return CompanyTradeRoutes[i];
			}
		}
		return NULL;
	}

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation, bool Destroyed = false);

	bool HasVisitedSector(const UFlareSimulatedSector* Sector) const;

	float GetPlayerReputation()
	{
		return CompanyData.PlayerReputation;
	}

	inline UFlareCompanyAI* GetAI()
	{
		return CompanyAI;
	}

	inline UFlareTacticManager* GetTacticManager()
	{
		return TacticManager;
	}

	inline int64 GetLastPeaceDate()
	{
		return CompanyData.PlayerLastPeaceDate;
	}

	inline int64 GetLastWarDate()
	{
		return CompanyData.PlayerLastWarDate;
	}

	inline int64 GetLastTributeDate()
	{
		return CompanyData.PlayerLastTributeDate;
	}

	inline float GetShame()
	{
		return CompanyData.Shame;
	}

	TArray<UFlareCompany*> GetOtherCompanies(bool Shuffle = false);

	float GetConfidenceLevel(UFlareCompany* ReferenceCompany, TArray<UFlareCompany*>& Allies);

	float ComputeCompanyDiplomaticWeight(bool WithShame);

	bool WantWarWith(UFlareCompany* TargetCompany);

	bool AtWar();

	int32 GetTransportCapacity();

	bool HasKnowResourceInput(FFlareResourceDescription* Resource);

	bool HasKnowResourceOutput(FFlareResourceDescription* Resource);

	int32 GetWarCount(UFlareCompany* ExcludeCompany) const;

	bool IsPlayerCompany() const;

};
