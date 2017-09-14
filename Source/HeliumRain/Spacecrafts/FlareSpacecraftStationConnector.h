#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftStationConnector.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareSpacecraftStationConnector : public UFlareSpacecraftComponent
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Get the world-space location of this dock */
	FVector GetDockLocation() const;

	/** Get the world-space rotation of this dock */
	FRotator GetDockRotation() const;


};
