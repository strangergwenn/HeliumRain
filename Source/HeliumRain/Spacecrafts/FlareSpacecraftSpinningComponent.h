#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftSpinningComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareSpacecraftSpinningComponent : public UFlareSpacecraftComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu = false) override;

	virtual void TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	// if true, will roll (not yaw or pitch)
	UPROPERTY(Category = Components, EditAnywhere, BlueprintReadOnly)
	bool RotationAxisRoll;

	// if true and not RotationAxisRoll, will yaw (not pitch)
	UPROPERTY(Category = Components, EditAnywhere, BlueprintReadOnly)
	bool RotationAxisYaw;

	// Rotation speed for the spinner
	UPROPERTY(Category = Components, EditAnywhere, BlueprintReadOnly)
	float RotationSpeed;

	// If true, will look for the sun
	UPROPERTY(Category = Components, EditAnywhere, BlueprintReadOnly)
	bool LookForSun;

	bool NeedTackerInit;
};
