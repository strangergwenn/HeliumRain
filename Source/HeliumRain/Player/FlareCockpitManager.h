#pragma once

#include "FlareCockpitManager.generated.h"

class AFlareSpacecraft;
class UFlareSpacecraftComponent;
class AFlarePlayerController;


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
	void SetupCockpitInstances(UMaterialInstanceDynamic* ScreenInstance, UMaterialInstanceDynamic* FrameInstanceh);

	/** Enter the cockpit */
	void EnterCockpit(AFlareSpacecraft* TargetPlayerShip);

	/** Exit the cockpit */
	void ExitCockpit();

	/** Update the target info */
	void UpdateTarget(float DeltaSeconds);

	/** Update the info screen */
	void UpdateInfo(float DeltaSeconds);

	/** Update the overheat info */
	void UpdateTemperature(float DeltaSeconds);

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

	// Gameplay data
	bool                                     IsInCockpit;
	int32                                    CockpitInstrumentsTargetSize;
	int32                                    CockpitFLIRTargetSize;
	float                                    CockpitHealthLightTime;
	float                                    CockpitHealthLightPeriod;
	float                                    CockpitPowerTime;
	float                                    CockpitPowerPeriod;
	float                                    CockpitLightingIntensity;


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

	// Cockpit material instance
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FreighterCockpitMaterialInstance;

	// Cockpit material instance (frame)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FreighterCockpitFrameMaterialInstance;

	// Cockpit material instance
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FighterCockpitMaterialInstance;

	// Cockpit material instance (frame)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*                FighterCockpitFrameMaterialInstance;


	/*----------------------------------------------------
		Cameras and render targets
	----------------------------------------------------*/

	// Cockpit texture (HUD)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UCanvasRenderTarget2D*                   CockpitHUDTarget;

	// Cockpit texture (instruments)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UCanvasRenderTarget2D*                   CockpitInstrumentsTarget;

	// Scene capture (target camera)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	USceneCaptureComponent2D*                CockpitFLIRCapture;

	// Cockpit texture (target camera)
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UCanvasRenderTarget2D*                   CockpitFLIRCameraTarget;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline UStaticMeshComponent* GetCockpitMesh()
	{
		return CockpitMesh;
	}
	
	UMaterialInstanceDynamic* GetCurrentScreenMaterial();

	UMaterialInstanceDynamic* GetCurrentFrameMaterial();


};
