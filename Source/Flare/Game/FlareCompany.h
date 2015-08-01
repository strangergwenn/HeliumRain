
#pragma once

#include "Object.h"
#include "FlareCompany.generated.h"


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

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> HostileCompanies;



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

	/** Save the company to a save file */
	virtual FFlareCompanySave* Save();

	/** Register a ship or a station*/
	virtual void Register(UFlareSimulatedSpacecraft* Ship);

	/** Un-register a ship or a station*/
	virtual void Unregister(UFlareSimulatedSpacecraft* Ship);


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Check if we are friend or for toward the player */
	virtual EFlareHostility::Type GetPlayerHostility() const;

	/** Check if we are friend or foe toward this target company */
	virtual EFlareHostility::Type GetHostility(const UFlareCompany* TargetCompany) const;

	virtual void SetHostilityTo(const UFlareCompany* TargetCompany, bool Hostile);

	/** Get an info string for this company save */
	virtual FText GetInfoText(bool Minimized);


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
	TArray<UFlareSimulatedSpacecraft*>      CompanyStations;
	TArray<UFlareSimulatedSpacecraft*>      CompanyShips;

	AFlareGame*                             Game;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
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

};
