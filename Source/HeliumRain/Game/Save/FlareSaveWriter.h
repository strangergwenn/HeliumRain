
#pragma once

#include "Object.h"
#include "../FlareSaveGame.h"
#include "FlareSaveWriter.generated.h"


struct FFlarePlayerSave;
struct FFlareQuestSave;
struct FFlareQuestProgressSave;
struct FFlareGeneratedQuestSave;
struct FFlareQuestConditionSave;

struct FFlareCompanyDescription;
struct FFlareWorldSave;

struct FFlareCompanySave;

struct FFlareSpacecraftSave;
struct FFlareShipPilotSave;
struct FFlareAsteroidSave;
struct FFlareSpacecraftComponentSave;
struct FFlareSpacecraftComponentTurretSave;
struct FFlareSpacecraftComponentWeaponSave;
struct FFlareTurretPilotSave;

struct FFlareCargoSave;
struct FFlareFactorySave;

struct FFlareFleetSave;
struct FFlareTradeRouteSave;
struct FFlareTradeRouteSectorSave;
struct FFlareTradeRouteSectorOperationSave;
struct FFlareCompanySectorKnowledge;
struct FFlareCompanyAISave;
struct FFlareCompanyReputationSave;

struct FFlareSectorSave;
struct FFlarePeopleSave;
struct FFlareBombSave;
struct FFFlareResourcePrice;
struct FFlareTravelSave;
struct FFlareFloatBuffer;



UCLASS()
class HELIUMRAIN_API UFlareSaveWriter: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	TSharedRef<FJsonObject> SaveGame(UFlareSaveGame* Data);

protected:
	/*----------------------------------------------------
	  Generator
	----------------------------------------------------*/

	TSharedRef<FJsonObject> SavePlayer(FFlarePlayerSave* Data);
	TSharedRef<FJsonObject> SaveQuest(FFlareQuestSave* Data);
	TSharedRef<FJsonObject> SaveQuestProgress(FFlareQuestProgressSave* Data);
	TSharedRef<FJsonObject> SaveGeneratedQuest(FFlareGeneratedQuestSave* Data);
	TSharedRef<FJsonObject> SaveQuestStepProgress(FFlareQuestConditionSave* Data);

	TSharedRef<FJsonObject> SaveCompanyDescription(FFlareCompanyDescription* Data);
	TSharedRef<FJsonObject> SaveWorld(FFlareWorldSave* Data);


	TSharedRef<FJsonObject> SaveCompany(FFlareCompanySave* Data);

	TSharedRef<FJsonObject> SaveSpacecraft(FFlareSpacecraftSave* Data);
	TSharedRef<FJsonObject> SavePilot(FFlareShipPilotSave* Data);
	TSharedRef<FJsonObject> SaveAsteroid(FFlareAsteroidSave* Data);
	TSharedRef<FJsonObject> SaveMeteorite(FFlareMeteoriteSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponent(FFlareSpacecraftComponentSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponentTurret(FFlareSpacecraftComponentTurretSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponentWeapon(FFlareSpacecraftComponentWeaponSave* Data);
	TSharedRef<FJsonObject> SaveTurretPilot(FFlareTurretPilotSave* Data);
	TSharedRef<FJsonObject> SaveStationConnection(FFlareConnectionSave* Data);

	TSharedRef<FJsonObject> SaveTradeOperation(FFlareTradeRouteSectorOperationSave* Data);
	TSharedRef<FJsonObject> SaveCargo(FFlareCargoSave* Data);
	TSharedRef<FJsonObject> SaveFactory(FFlareFactorySave* Data);
	TSharedRef<FJsonObject> SaveShipyardOrderQueue(FFlareShipyardOrderSave* Data);

	TSharedRef<FJsonObject> SaveFleet(FFlareFleetSave* Data);
	TSharedRef<FJsonObject> SaveTradeRoute(FFlareTradeRouteSave* Data);
	TSharedRef<FJsonObject> SaveTradeRouteSector(FFlareTradeRouteSectorSave* Data);
	TSharedRef<FJsonObject> SaveSectorKnowledge(FFlareCompanySectorKnowledge* Data);
	TSharedRef<FJsonObject> SaveCompanyAI(FFlareCompanyAISave* Data);
	TSharedRef<FJsonObject> SaveCompanyReputation(FFlareCompanyReputationSave* Data);


	TSharedRef<FJsonObject> SaveSector(FFlareSectorSave* Data);
	TSharedRef<FJsonObject> SavePeople(FFlarePeopleSave* Data);
	TSharedRef<FJsonObject> SaveBomb(FFlareBombSave* Data);
	TSharedRef<FJsonObject> SaveResourcePrice(FFFlareResourcePrice* Data);
	TSharedRef<FJsonObject> SaveFloatBuffer(FFlareFloatBuffer* Data);
	TSharedRef<FJsonObject> SaveBundle(FFlareBundle* Data);

	TSharedRef<FJsonObject> SaveTravel(FFlareTravelSave* Data);

	void SaveFloat(TSharedPtr< FJsonObject > Object, FString Key, float Data);



	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/



public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline static FString FormatInt32(int32 Data)
	{
		return FString::FromInt(Data);
	}

	inline static FString FormatInt64(int64 Data)
	{
		return FString::Printf(TEXT("%lld"), Data);
	}

	inline static FString FormatTransform(FTransform Data)
	{
		return FString::Printf(TEXT("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"),
				FixFloat(Data.GetRotation().X),
				FixFloat(Data.GetRotation().Y),
				FixFloat(Data.GetRotation().Z),
				FixFloat(Data.GetRotation().W),
				FixFloat(Data.GetTranslation().X),
				FixFloat(Data.GetTranslation().Y),
				FixFloat(Data.GetTranslation().Z),
				FixFloat(Data.GetScale3D().X),
				FixFloat(Data.GetScale3D().Y),
				FixFloat(Data.GetScale3D().Z));
	}

	inline static float FixFloat(float value)
	{
		if(FMath::IsNaN(value))
		{
			FLOG("WARNING: Fix NaN in code");
			return 0.f;
		}
		else if(!FMath::IsFinite(value))
		{
			FLOG("WARNING: Fix Inf in code");
			return 0.f;
		}
		return value;
	}

	inline static FString FormatVector(FVector Data)
	{
		return FString::Printf(TEXT("%f,%f,%f"),
				FixFloat(Data.X),
				FixFloat(Data.Y),
				FixFloat(Data.Z));
	}

	inline static FString FormatRotator(FRotator Data)
	{
		return FString::Printf(TEXT("%f,%f,%f"),
				FixFloat(Data.Pitch),
				FixFloat(Data.Yaw),
				FixFloat(Data.Roll));
	}

	template<typename TEnum>
	static FORCEINLINE FString FormatEnum(const FString& Name, TEnum Value)
	{
		const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!enumPtr)
		{
			return FString("Invalid");
		}

		return enumPtr->GetNameStringByIndex((int32)Value);
	}
};
