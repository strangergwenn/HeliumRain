
#pragma once

#include "Object.h"
#include "FlareCompany.generated.h"


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
	
	/** Paint color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPaintColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationLightColorIndex;

	/** Engine exhaust color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationEngineColorIndex;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};


class AFlareGame;
class IFlareShipInterface;
class IFlareStationInterface;


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

	/** Register a ship */
	virtual void Register(IFlareShipInterface* Ship);

	/** Register a station */
	virtual void Register(IFlareStationInterface* Station);

	/** Un-register a ship */
	virtual void Unregister(IFlareShipInterface* Ship);

	/** Un-register a station */
	virtual void Unregister(IFlareStationInterface* Station);


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the color of ship lights */
	inline void SetLightColorIndex(int32 Index);

	/** Set the color of ship paint */
	inline void SetPaintColorIndex(int32 Index);

	/** Set the color of engine exhausts */
	inline void SetEngineColorIndex(int32 Index);

	/** Set the pattern index for ship paint */
	inline void SetPatternIndex(int32 Index);

	/** Update all ships and stations */
	virtual void UpdateCompanyCustomization();

	/** Apply customization to a component's material */
	virtual void CustomizeModuleMaterial(UMaterialInstanceDynamic* Mat);

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
	TArray<IFlareShipInterface*>      CompanyShips;
	TArray<IFlareStationInterface*>   CompanyStations;


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

	inline int32 GetLightColorIndex() const
	{
		return CompanyData.CustomizationLightColorIndex;
	}

	inline int32 GetPaintColorIndex() const
	{
		return CompanyData.CustomizationPaintColorIndex;
	}

	inline int32 GetEngineColorIndex() const
	{
		return CompanyData.CustomizationEngineColorIndex;
	}

	inline int32 GetPatternIndex() const
	{
		return CompanyData.CustomizationPatternIndex;
	}

};
