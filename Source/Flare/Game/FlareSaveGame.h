#pragma once

#include "../Player/FlarePlayerController.h"
#include "../Ships/FlareShipInterface.h"
#include "FlareCompany.h"
#include "FlareSaveGame.generated.h"


UCLASS()
class UFlareSaveGame : public USaveGame
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	UPROPERTY(VisibleAnywhere, Category = Save)
	FFlarePlayerSave PlayerData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareShipSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareStationSave> StationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	int32 CurrentImmatriculationIndex;

};

