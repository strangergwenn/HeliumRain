#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareFleet.h"
#include "FlareTradeRoute.h"
#include "FlareGameTypes.generated.h"

/** Possible menu targets */
UENUM()
namespace EFlareMenu
{
	enum Type
	{
		MENU_None,

		// Boot menus
		MENU_Main,
		MENU_NewGame,

		// Special "menus" for async transitions
		MENU_CreateGame,
		MENU_LoadGame,
		MENU_FlyShip,
		MENU_Travel,
		MENU_ReloadSector,
		MENU_FastForwardSingle,
		MENU_GameOver,

		// Main gameplay menus
		MENU_Story,
		MENU_Company,
		MENU_Fleet,
		MENU_Quest,
		MENU_Ship,
		MENU_ShipConfig,
		MENU_Station,
		MENU_Sector,
		MENU_Trade,
		MENU_TradeRoute,
		MENU_Orbit,
		MENU_Leaderboard,
		MENU_ResourcePrices,
		MENU_WorldEconomy,

		// Support menus
		MENU_Settings,
		MENU_Credits,
		MENU_Quit
	};
}


/** Menu parameter structure storing commands + data for async processing */
struct FFlareMenuParameterData
{
	FFlareMenuParameterData()
		: Company(NULL)
		, Factory(NULL)
		, Fleet(NULL)
		, Quest(NULL)
		, Route(NULL)
		, Sector(NULL)
		, Spacecraft(NULL)
		, Travel(NULL)
		, Resource(NULL)
		, ScenarioIndex(0)
		, PlayerEmblemIndex(0)
	{}

	class UFlareCompany*                        Company;
	class UFlareFactory*                        Factory;
	class UFlareFleet*                          Fleet;
	class UFlareQuest*                          Quest;
	class UFlareTradeRoute*                     Route;
	class UFlareSimulatedSector*                Sector;
	class UFlareSimulatedSpacecraft*            Spacecraft;
	class UFlareTravel*                         Travel;
	struct FFlareResourceDescription*           Resource;

	struct FFlareCompanyDescription*            CompanyDescription;
	int32                                       ScenarioIndex;
	int32                                       PlayerEmblemIndex;
	bool                                        PlayTutorial;
};

/** Hostility status */
UENUM()
namespace EFlareHostility
{
	enum Type
	{
		Hostile,
		Neutral,
		Friendly,
		Owned
	};
}

/** Hostility status */
UENUM()
namespace EFlareBudget
{
	enum Type
	{
		Military,
		Station,
		Technology,
		Trade,
		None
	};
}

/** Combat groups */
UENUM()
namespace EFlareCombatGroup
{
	enum Type
	{
		AllMilitary,
		Capitals,
		Fighters,
		Civilan
	};
}

/** Resource price context */
UENUM()
namespace EFlareResourcePriceContext
{
	enum Type
	{
		Default, /** Default price */
		FactoryInput, /** Price selling to a factory needing the resource */
		FactoryOutput, /** Price buying the resource to a factory */
		ConsumerConsumption, /** Price selling to a the people */
		MaintenanceConsumption, /** Price selling to company using maintenance */
	};
}

/** Combat tactics */
UENUM()
namespace EFlareCombatTactic
{
	enum Type
	{
		ProtectMe,
		AttackMilitary,
		AttackStations,
		AttackCivilians,
		StandDown
	};
}

/** Sector knowledge status */
UENUM()
namespace EFlareSectorKnowledge
{
	enum Type
	{
		Unknown, /** The existence of this sector is unknown */
		Known, /** The sector is visible on the map but its content is unknown */
		Visited /** The sector is visited, all static structure are visible */
	};
}

/** Company sector knowledge data */
USTRUCT()
struct FFlareCompanySectorKnowledge
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName SectorIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareSectorKnowledge::Type> Knowledge;
};

/** Company reputation save data */
USTRUCT()
struct FFlareCompanyReputationSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	float Reputation;

};

/** Company AI save data */
USTRUCT()
struct FFlareCompanyAISave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName ConstructionProjectStationDescriptionIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	FName ConstructionProjectSectorIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	FName ConstructionProjectStationIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> ConstructionShipsIdentifiers;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> ConstructionStaticShipsIdentifiers;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 ConstructionProjectNeedCapacity;

	int64 BudgetTechnology;
	int64 BudgetMilitary;
	int64 BudgetStation;
	int64 BudgetTrade;

	/* Modify AttackThreshold */
	float Caution;
};


/** Game save data */
USTRUCT()
struct FFlareCompanySave
{
	GENERATED_USTRUCT_BODY()

	/** Save identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Index of the company description in the catalog, or -1 if player */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CatalogIdentifier;

	/** Money money money / Always funny / In a rich men's world */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 Money;

	/** Hostile companies */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> HostileCompanies;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> StationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> DestroyedSpacecraftData;

	/** Company fleets */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareFleetSave> Fleets;

	/** Company trade routes */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTradeRouteSave> TradeRoutes;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 FleetImmatriculationIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 TradeRouteImmatriculationIndex;

	/** List of known or visited sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanySectorKnowledge> SectorsKnowledge;

	/** Company AI data */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareCompanyAISave AI;

	/** List of others companies's reputation */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanyReputationSave> CompaniesReputation;

	/** Value of all company assets */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 CompanyValue;

	/** Date of last tribute given to the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastTributeDate;

	/** Date of last peace with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastPeaceDate;

	/** Date of last war with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastWarDate;

	/* Modify reputation to this company */
	float Shame;
};

/** Game save data */
USTRUCT()
struct FFlareFloatBuffer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int32 MaxSize;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 WriteIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<float> Values;


	void Init(int32 Size);

	void Resize(int32 Size);

	void Append(float NewValue);

	float GetValue(int32 Age);

	float GetMean(int32 StartAge, int32 EndAge);
};


USTRUCT()
struct FVectorArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FVector> Entries;
};

/** Generic storage system */
USTRUCT()
struct FFlareBundle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, float> FloatValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, int32> Int32Values;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FTransform> TransformValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FVectorArray> VectorArrayValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FName> NameValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FString> StringValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> Tags;

	bool HasFloat(FName Key) const;
	bool HasInt32(FName Key) const;
	bool HasTransform(FName Key) const;
	bool HasVectorArray(FName Key) const;
	bool HasName(FName Key) const;
	bool HasString(FName Key) const;
	bool HasTag(FName Tag) const;


	float GetFloat(FName Key, float Default = 0.f) const;
	int32 GetInt32(FName Key, int32 Default = 0) const;
	FTransform GetTransform(FName Key, const FTransform Default = FTransform::Identity) const;
	TArray<FVector> GetVectorArray(FName Key) const;
	FName GetName(FName Key) const;
	FString GetString(FName Key) const;

	void PutFloat(FName Key, float Value);
	void PutInt32(FName Key, int32 Value);
	void PutTransform(FName Key, const FTransform Value);
	void PutVectorArray(FName Key, const TArray<FVector> Value);
	void PutName(FName Key, FName Value);
	void PutString(FName Key, FString Value);
	void PutTag(FName Tag);

	void Clear();
};

/** Incoming event description */
USTRUCT()
struct FFlareIncomingEvent
{
	GENERATED_USTRUCT_BODY()

	/** Event text */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Text;

	/** Days until event */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 RemainingDuration;
};

UCLASS()
class HELIUMRAIN_API UFlareGameTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Get the group name */
	static FText GetCombatGroupDescription(EFlareCombatGroup::Type Type);

	/** Get the tactic name */
	static FText GetCombatTacticDescription(EFlareCombatTactic::Type Type);

	/** Get the icon for this combat group */
	static const FSlateBrush* GetCombatGroupIcon(EFlareCombatGroup::Type Type);


};
