#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareScannable.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareScannable : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/
	
	/** Check if the player has unlocked this scannable */
	bool IsActive();

	/** Scan this target and unlock its returns */
	void OnScanned();


public:

	/*----------------------------------------------------
		Properties
	----------------------------------------------------*/
	
	/** Root */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	USceneComponent*                              Root;

	/** Scannable ID */
	UPROPERTY(EditAnywhere)
	FName                                         ScannableIdentifier;


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	FName GetScannableIdentifier() const
	{
		return ScannableIdentifier;
	}


};
