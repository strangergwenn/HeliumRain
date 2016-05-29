#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareFleet.h"
#include "FlareTradeRoute.h"
#include "FlareGameTypes.generated.h"

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
