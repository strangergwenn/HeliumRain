
#pragma once

#include "Object.h"
#include "FlareSaveWriter.generated.h"

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
	TSharedRef<FJsonObject> SaveQuestStepProgress(FFlareQuestStepProgressSave* Data);

	TSharedRef<FJsonObject> SaveCompanyDescription(FFlareCompanyDescription* Data);
	TSharedRef<FJsonObject> SaveWorld(FFlareWorldSave* Data);


	TSharedRef<FJsonObject> SaveCompany(FFlareCompanySave* Data);

	TSharedRef<FJsonObject> SaveSpacecraft(FFlareSpacecraftSave* Data);
	TSharedRef<FJsonObject> SavePilot(FFlareShipPilotSave* Data);
	TSharedRef<FJsonObject> SaveAsteroid(FFlareAsteroidSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponent(FFlareSpacecraftComponentSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponentTurret(FFlareSpacecraftComponentTurretSave* Data);
	TSharedRef<FJsonObject> SaveSpacecraftComponentWeapon(FFlareSpacecraftComponentWeaponSave* Data);
	TSharedRef<FJsonObject> SaveTurretPilot(FFlareTurretPilotSave* Data);

	TSharedRef<FJsonObject> SaveCargo(FFlareCargoSave* Data);
	TSharedRef<FJsonObject> SaveFactory(FFlareFactorySave* Data);

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
	TSharedRef<FJsonObject> SaveTravel(FFlareTravelSave* Data);



	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/



public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline FString FormatInt32(int32 Data)
	{
		return FString::FromInt(Data);
	}

	inline FString FormatInt64(int64 Data)
	{
		return FString::Printf(TEXT("%lld"), Data);
	}

	inline FString FormatTransform(FTransform Data)
	{
		return FString::Printf(TEXT("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"),
				Data.GetRotation().X,
				Data.GetRotation().Y,
				Data.GetRotation().Z,
				Data.GetRotation().W,
				Data.GetTranslation().X,
				Data.GetTranslation().Y,
				Data.GetTranslation().Z,
				Data.GetScale3D().X,
				Data.GetScale3D().Y,
				Data.GetScale3D().Z);
	}

	inline FString FormatVector(FVector Data)
	{
		return FString::Printf(TEXT("%f,%f,%f"),
				Data.X,
				Data.Y,
				Data.Z);
	}

	inline FString FormatRotator(FRotator Data)
	{
		return FString::Printf(TEXT("%f,%f,%f"),
				Data.Pitch,
				Data.Yaw,
				Data.Roll);
	}

	template<typename TEnum>
	static FORCEINLINE FString FormatEnum(const FString& Name, TEnum Value)
	{
		const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!enumPtr)
		{
			return FString("Invalid");
		}

		return enumPtr->GetEnumName((int32)Value);
	}
};
