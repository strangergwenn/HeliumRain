#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareFleet.h"
#include "FlareTradeRoute.h"
#include "FlareGameTypes.generated.h"


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

/** Fleet save data */
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
	uint64 Money;

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
};

UCLASS()
class FLARE_API UFlareGameTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:
};
