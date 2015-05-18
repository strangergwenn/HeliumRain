
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
		
	/** Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

	/** Short name */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ShortName;

	/** Save identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;


	/** Money money money / Always funny / In a rich men's world */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 Money;


	/** Base color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationBasePaintColorIndex;
	
	/** Paint color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPaintColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationOverlayColorIndex;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};


class AFlareGame;
class IFlareShipInterface;

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
	virtual void Register(IFlareShipInterface* Ship);

	/** Un-register a ship or a station*/
	virtual void Unregister(IFlareShipInterface* Ship);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	/** Check if we are friend or for toward the player */
	virtual EFlareHostility::Type GetPlayerHostility() const;

	/** Check if we are friend or foe toward this target company */
	virtual EFlareHostility::Type GetHostility(UFlareCompany* TargetCompany) const;


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the color of engine exhausts */
	inline void SetBasePaintColorIndex(int32 Index);

	/** Set the color of ship paint */
	inline void SetPaintColorIndex(int32 Index);

	/** Set the color of ship lights */
	inline void SetOverlayColorIndex(int32 Index);

	/** Set the pattern index for ship paint */
	inline void SetPatternIndex(int32 Index);

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
	FFlareCompanySave                 CompanyData;
	TArray<IFlareShipInterface*>   CompanyStations;
	TArray<IFlareShipInterface*>      CompanyShips;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Cast<AFlareGame>(GetOuter()->GetWorld()->GetAuthGameMode());
	}

	inline FName GetIdentifier() const
	{
		return CompanyData.Identifier;
	}

	inline FString GetCompanyName() const
	{
		return CompanyData.Name;
	}

	inline FName GetShortName() const
	{
		return CompanyData.ShortName;
	}

	inline int32 GetBasePaintColorIndex() const
	{
		return CompanyData.CustomizationBasePaintColorIndex;
	}

	inline int32 GetPaintColorIndex() const
	{
		return CompanyData.CustomizationPaintColorIndex;
	}

	inline int32 GetOverlayColorIndex() const
	{
		return CompanyData.CustomizationOverlayColorIndex;
	}

	inline int32 GetPatternIndex() const
	{
		return CompanyData.CustomizationPatternIndex;
	}

	inline TArray<IFlareShipInterface*>& GetCompanyStations()
	{
		return CompanyStations;
	}

	inline TArray<IFlareShipInterface*>& GetCompanyShips()
	{
		return CompanyShips;
	}

};
