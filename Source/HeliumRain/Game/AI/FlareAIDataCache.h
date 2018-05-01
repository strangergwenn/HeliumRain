#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"
#include "FlareAIDataCache.generated.h"


class UFlareSimulatedSector;
struct FFlareResourceDescription;

UCLASS()
class HELIUMRAIN_API UFlareAIDataCache : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public API
	----------------------------------------------------*/

	/** Load the company behavior */
	virtual void Load(AFlareGame* GameParam);

protected:
	UPROPERTY()
	AFlareGame* Game;


public:



	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	int64 GetInputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource);

	int64 GetOutputResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription const* Resource);

	AFlareGame* GetGame() const
	{
		return Game;
	}
};

