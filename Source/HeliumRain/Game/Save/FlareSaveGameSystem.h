
#pragma once

#include "Object.h"
#include "FlareSaveGameSystem.generated.h"

class UFlareSaveGame;

UCLASS()
class HELIUMRAIN_API UFlareSaveGameSystem: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Interface
	----------------------------------------------------*/


	virtual bool DoesSaveGameExist(const FString SaveName);


	virtual bool SaveGame(const FString SaveName, UFlareSaveGame* SaveData);

	virtual UFlareSaveGame* LoadGame(const FString SaveName);


	virtual bool DeleteGame(const FString SaveName);

protected:


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	FCriticalSection SaveLock;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

   /** Get the path to save game file for the given name, a platform _may_ be able to simply override this and no other functions above */
   virtual FString GetSaveGamePath(const FString SaveName);

};
