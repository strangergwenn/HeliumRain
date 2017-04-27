
#pragma once

#include "Object.h"
#include "FlareSaveReaderV1.generated.h"

class UFlareSaveGame;
struct FFlareTradeRouteSectorOperationSave;
struct FFlareFloatBuffer;

UCLASS()
class HELIUMRAIN_API UFlareSaveReaderV1: public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UFlareSaveGame* LoadGame(TSharedPtr< FJsonObject > GameObject);

protected:
	/*----------------------------------------------------
	  Loaders
	----------------------------------------------------*/

	void LoadPlayer(const TSharedPtr<FJsonObject> Object, FFlarePlayerSave* Data);
	void LoadQuest(const TSharedPtr<FJsonObject> Object, FFlareQuestSave* Data);
	void LoadQuestProgress(const TSharedPtr<FJsonObject> Object, FFlareQuestProgressSave* Data);
	void LoadGeneratedQuest(const TSharedPtr<FJsonObject> Object, FFlareGeneratedQuestSave* Data);

	void LoadQuestStepProgress(const TSharedPtr<FJsonObject> Object, FFlareQuestConditionSave* Data);

	void LoadCompanyDescription(const TSharedPtr<FJsonObject> Object, FFlareCompanyDescription* Data);
	void LoadWorld(const TSharedPtr<FJsonObject> Object, FFlareWorldSave* Data);


	void LoadCompany(const TSharedPtr<FJsonObject> Object, FFlareCompanySave* Data);

	void LoadSpacecraft(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftSave* Data);
	void LoadPilot(const TSharedPtr<FJsonObject> Object, FFlareShipPilotSave* Data);
	void LoadAsteroid(const TSharedPtr<FJsonObject> Object, FFlareAsteroidSave* Data);
	void LoadSpacecraftComponent(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentSave* Data);
	void LoadSpacecraftComponentTurret(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentTurretSave* Data);
	void LoadSpacecraftComponentWeapon(const TSharedPtr<FJsonObject> Object, FFlareSpacecraftComponentWeaponSave* Data);
	void LoadTurretPilot(const TSharedPtr<FJsonObject> Object, FFlareTurretPilotSave* Data);

	void LoadTradeOperation(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSectorOperationSave* Data);
	void LoadCargo(const TSharedPtr<FJsonObject> Object, FFlareCargoSave* Data);
	void LoadFactory(const TSharedPtr<FJsonObject> Object, FFlareFactorySave* Data);

	void LoadFleet(const TSharedPtr<FJsonObject> Object, FFlareFleetSave* Data);
	void LoadTradeRoute(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSave* Data);
	void LoadTradeRouteSector(const TSharedPtr<FJsonObject> Object, FFlareTradeRouteSectorSave* Data);
	void LoadSectorKnowledge(const TSharedPtr<FJsonObject> Object, FFlareCompanySectorKnowledge* Data);
	void LoadCompanyAI(const TSharedPtr<FJsonObject> Object, FFlareCompanyAISave* Data);
	void LoadCompanyReputation(const TSharedPtr<FJsonObject> Object, FFlareCompanyReputationSave* Data);


	void LoadSector(const TSharedPtr<FJsonObject> Object, FFlareSectorSave* Data);
	void LoadPeople(const TSharedPtr<FJsonObject> Object, FFlarePeopleSave* Data);
	void LoadBomb(const TSharedPtr<FJsonObject> Object, FFlareBombSave* Data);
	void LoadResourcePrice(const TSharedPtr<FJsonObject> Object, FFFlareResourcePrice* Data);
	void LoadTravel(const TSharedPtr<FJsonObject> Object, FFlareTravelSave* Data);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/



public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	void LoadInt32(TSharedPtr< FJsonObject > Object, FString Key, int32* Data);
	void LoadInt64(TSharedPtr< FJsonObject > Object, FString Key, int64* Data);
	void LoadFloat(TSharedPtr< FJsonObject > Object, FString Key, float* Data);
	void LoadFName(TSharedPtr< FJsonObject > Object, FString Key, FName* Data);
	void LoadFText(TSharedPtr< FJsonObject > Object, FString Key, FText* Data);
	void LoadFNameArray(TSharedPtr< FJsonObject > Object, FString Key, TArray<FName>* Data);
	void LoadFloatArray(TSharedPtr< FJsonObject > Object, FString Key, TArray<float>* Data);
	void LoadTransform(TSharedPtr< FJsonObject > Object, FString Key, FTransform* Data);
	bool LoadVector(TSharedPtr< FJsonObject > Object, FString Key, FVector* Data);
	void LoadRotator(TSharedPtr< FJsonObject > Object, FString Key, FRotator* Data);
	void LoadFloatBuffer(TSharedPtr< FJsonObject > Object, FString Key, FFlareFloatBuffer* Data);
	void LoadBundle(const TSharedPtr<FJsonObject> Object, FString Key, FFlareBundle* Data);




	template <typename EnumType>
	static FORCEINLINE EnumType LoadEnum(TSharedPtr< FJsonObject > Object, FString Key, const FString& EnumName)
	{
		FString DataString;
		if(Object->TryGetStringField(Key, DataString))
		{
			UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
			if(!Enum)
				{
				  return EnumType(0);
				}
			return (EnumType)Enum->GetIndexByName(FName(*DataString));
		}
		return EnumType(0);
	}
};
