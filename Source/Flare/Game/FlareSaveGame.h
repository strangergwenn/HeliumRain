#pragma once

#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraftInterface.h"
#include "../Spacecrafts/FlareBomb.h"
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
	TArray<FFlareSpacecraftSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> StationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareBombSave> BombData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	int32 CurrentImmatriculationIndex;

	UPROPERTY(VisibleAnywhere, Category = Save)
	int32 CurrentImmatriculationNameIndex;

};

