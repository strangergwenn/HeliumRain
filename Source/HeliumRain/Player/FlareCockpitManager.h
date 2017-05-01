#pragma once

#include "FlareCockpitManager.generated.h"

class AFlareSpacecraft;
class UFlareSpacecraftComponent;
class AFlarePlayerController;

class UCanvasRenderTarget2D;


/** Cockpit manager class */
UCLASS()
class HELIUMRAIN_API AFlareCockpitManager : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:
		
	/*----------------------------------------------------
		Player interface
	----------------------------------------------------*/
		
	/** Construct the cockpit */
	virtual void SetupCockpit(AFlarePlayerController* NewPC);

	/** Signal that a new ship is being flown */
	virtual void OnFlyShip(AFlareSpacecraft* NewPlayerShip);

	/** Signal that no ship is being flown */
	virtual void OnStopFlying();

	/** Whether to use the external camera or not */
	virtual void SetExternalCamera(bool External);


	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Setup the cockpit materials */
	void SetupCockpitInstances(UMaterialInstanceDynamic* FrameInstanceh);

	/** Enter the cockpit */
	void EnterCockpit(AFlareSpacecraft* TargetPlayerShip);

	/** Exit the cockpit */
	void ExitCockpit();

	/** Update the overheat info */
	void UpdateHealth(float DeltaSeconds);

	/** Update the target info */
	void UpdateTarget(float DeltaSeconds);

	/** Update the info screen */
	void UpdateInfo(float DeltaSeconds);

	/** Update the power level */
	void UpdatePower(float DeltaSeconds);


protected:

	/*----------------------------------------------------
		General gameplay & cockpit data
	----------------------------------------------------*/

	// Player controller
	UPROPERTY()
	AFlarePlayerController*                  PC;

	// Cyrrently flown ship
	UPROPERTY()
	AFlareSpacecraft*                        PlayerShip;

	// Cockpit
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UStaticMeshComponent*                    CockpitMesh;

	// Light
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UPointLightComponent*                    CockpitLight;

	// Light2
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UPointLightComponent*                    CockpitLight2;

	// Settings
	int32                                    CockpitInstrumentsTargetSize;
	float                                    CockpitLightingIntensity;

	// Gameplay data
	bool                                     IsInCockpit;
	float                                    CockpitHealthLightTimer;
	float                                    CockpitHealthLightPeriod;
	float                                    CockpitTargetLightTimer;
	float                                    CockpitTargetLightPeriod;
	float                                    CockpitPowerTimer;
	float                                    CockpitPowerPeriod;
	float                                    CameraSwitchTimer;
	float                                    CameraSwitchPeriod;
	float                                    LastCockpitScale;


	/*----------------------------------------------------
		Cockpit templates
	----------------------------------------------------*/

	// Freighter cockpit mesh
	UPROPERTY()
	UStaticMesh*                             FreighterCockpitMeshTemplate;

	// Fighter cockpit mesh
	UPROPERTY()
	UStaticMesh*                             FighterCockpitMeshTemplate;


	/*----------------------------------------------------
		Dynamic data
	----------------------------------------------------*/
	
	// Cockpit material instance (frame)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FreighterCockpitFrameMaterialInstance;
	
	// Cockpit material instance (frame)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FighterCockpitFrameMaterialInstance;
		
	// Cockpit texture (instruments)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UCanvasRenderTarget2D*                   CockpitInstrumentsTarget;
	

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline UStaticMeshComponent* GetCockpitMesh() const;
	
	bool PlayerShipIsPowered() const;
	
	UMaterialInstanceDynamic* GetCurrentFrameMaterial();


};
