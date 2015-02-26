#pragma once

#include "FlareStationInterface.h"
#include "../Ships/FlareShipBase.h"
#include "../Ships/FlareShipModule.h"
#include "FlareStation.generated.h"


/** Station class */
UCLASS(Blueprintable, ClassGroup = (Flare, Station))
class AFlareStation : public AFlareShipBase, public IFlareStationInterface
{

public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	/** Setup this station */
	virtual void Initialize();


	/*----------------------------------------------------
		Station interface
	----------------------------------------------------*/

	virtual void Load(const FFlareStationSave& Data) override;

	virtual FFlareStationSave* Save() override;

	virtual FFlareDockingInfo RequestDock(IFlareShipInterface* Ship) override;

	virtual void ReleaseDock(IFlareShipInterface* Ship, int32 DockId) override;

	virtual void Dock(IFlareShipInterface* Ship, int32 DockId) override;

	virtual TArray<IFlareShipInterface*> GetDockedShips() override;


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Component description
	FFlareStationSave                StationData;
	FFlareStationDescription*        StationDescription;

	/** Root hull component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareShipModule*	         HullRoot;

	// Dock data
	TArray <FFlareDockingInfo>       DockingSlots;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline FFlareStationDescription* GetDescription() const
	{
		return StationDescription;
	}

};
