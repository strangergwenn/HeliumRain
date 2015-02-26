#pragma once

#include "FlareEngine.h"
#include "FlareRCS.generated.h"


/** Possible values for RCS thrust */
UENUM()
namespace ERCSCapability
{
	enum Type
	{
		Thrust,
		Brake,
		TurnRight,
		TurnLeft,
		TurnUp,
		TurnDown,
		RollRight,
		RollLeft,
		StrafeLeft,
		StrafeRight,
		StrafeUp,
		StrafeDown,
		Num
	};
}


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareRCS : public UFlareEngine
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Array of possible displacements */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	TArray< TEnumAsByte<ERCSCapability::Type> > Capabilities;
	

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Can this RCS generate vertical thrust ? */
	virtual bool CanMoveVertical();

	/** Can this RCS generate horizontal thrust ? */
	virtual bool CanMoveHorizontal();


	virtual void UpdateAlpha(float DeltaTime) override;


protected:

	/*----------------------------------------------------
		Protected methods
	----------------------------------------------------*/

	/** Update the alpha based on linear command for this axis */
	void HandleLinearCommand(FVector Axis, float Command);

	/** Update the alpha based on angular command for this axis */
	void HandleAngularCommand(FVector Axis, float Command);


};
