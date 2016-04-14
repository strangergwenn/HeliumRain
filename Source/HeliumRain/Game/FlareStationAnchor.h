#pragma once

#include "FlareStationAnchor.generated.h"


UCLASS()
class HELIUMRAIN_API AFlareStationAnchor : public AActor
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public interface
	----------------------------------------------------*/


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	class UArrowComponent* ArrowComponent;

#endif

};
