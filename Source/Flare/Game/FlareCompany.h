
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

};


UCLASS()
class FLARE_API UFlareCompany : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Load the company from a save file */
	virtual void Load(const FFlareCompanySave& Data);

	/** Save the company to a save file */
	virtual FFlareCompanySave* Save();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareCompanySave         CompanyData;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline FName GetIdentifier() const
	{
		return CompanyData.Identifier;
	}
	
};
