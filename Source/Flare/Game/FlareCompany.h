
#pragma once

#include "Object.h"
#include "FlareFleet.h"
#include "FlareSimulatedSector.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "FlareCompany.generated.h"


class UFlareFleet;

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
	int32 Money;

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

	UPROPERTY(EditAnywhere, Category = Save)
	int32 FleetImmatriculationIndex;

	/** List of known or visited sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanySectorKnowledge> SectorsKnowledge;
};


/** Catalog data */
USTRUCT()
struct FFlareCompanyDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Name;

	/** Short name */
	UPROPERTY(EditAnywhere, Category = Company)
	FName ShortName;

	/** Emblem */
	UPROPERTY(EditAnywhere, Category = Company)
	UTexture2D* Emblem;

	/** Base color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationBasePaintColorIndex;
	
	/** Paint color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPaintColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationOverlayColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationLightColorIndex;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};


class AFlareGame;
class UFlareSimulatedSpacecraft;

UCLASS()
class FLARE_API UFlareCompany : public UObject
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


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Check if we are friend or for toward the player */
	virtual EFlareHostility::Type GetPlayerHostility() const;

	/** Check if we are friend or foe toward this target company */
	virtual EFlareHostility::Type GetHostility(const UFlareCompany* TargetCompany) const;

	/** Set whether this company is hostile to an other company */
	virtual void SetHostilityTo(const UFlareCompany* TargetCompany, bool Hostile);

	/** Get an info string for this company */
	virtual FText GetShortInfoText();

	/** Create a new fleet in a sector */
	virtual UFlareFleet* CreateFleet(FString FleetName, UFlareSimulatedSector* FleetSector);

	/** Destroy a fleet */
	virtual void RemoveFleet(UFlareFleet* Fleet);

	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	/** Set a sector discovered */
	virtual void DiscoverSector(UFlareSimulatedSector* Sector);

	/** Set a sector visited */
	virtual void VisitSector(UFlareSimulatedSector* Sector);


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/
	
	/** Update all ships and stations */
	virtual void UpdateCompanyCustomization();

	/** Apply customization to a component's material */
	virtual void CustomizeComponentMaterial(UMaterialInstanceDynamic* Mat);

	/** Apply customization to a component's special effect material */
	virtual void CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat);
	
	/** Normalize a color */
	FLinearColor NormalizeColor(FLinearColor Col) const;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	const FFlareCompanyDescription*         CompanyDescription;
	FFlareCompanySave                       CompanyData;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyStations;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanyShips;

	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>      CompanySpacecrafts;

	UPROPERTY()
	TArray<UFlareFleet*>                    CompanyFleets;

	AFlareGame*                             Game;
	TArray<UFlareSimulatedSector*>          KnownSectors;
	TArray<UFlareSimulatedSector*>          VisitedSectors;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

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
		check(CompanyDescription);
		return CompanyDescription->Name;
	}

	inline FName GetShortName() const
	{
		check(CompanyDescription);
		return CompanyDescription->ShortName;
	}

	const FSlateBrush* GetEmblem() const;

	inline int32 GetBasePaintColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationBasePaintColorIndex;
	}

	inline int32 GetPaintColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationPaintColorIndex;
	}

	inline int32 GetOverlayColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationOverlayColorIndex;
	}

	inline int32 GetLightColorIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationLightColorIndex;
	}

	inline int32 GetPatternIndex() const
	{
		check(CompanyDescription);
		return CompanyDescription->CustomizationPatternIndex;
	}

	inline int32 GetMoney() const
	{
		return CompanyData.Money;
	}

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

	inline TArray<UFlareSimulatedSector*>& GetKnownSectors()
	{
		return KnownSectors;
	}

	inline TArray<UFlareSimulatedSector*>& GetVisitedSectors()
	{
		return VisitedSectors;
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

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation);

	bool HasVisitedSector(const UFlareSimulatedSector* Sector) const;

};
