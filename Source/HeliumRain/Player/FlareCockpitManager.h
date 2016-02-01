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

	/** Whether to use the external camera or not */
	virtual void SetExternalCamera(bool External);


	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Enter the cockpit */
	void EnterCockpit(AFlareSpacecraft* TargetPlayerShip);

	/** Exit the cockpit */
	void ExitCockpit(AFlareSpacecraft* TargetPlayerShip);

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

	// Cockpit mesh template
	UPROPERTY()
	UStaticMesh*                             CockpitMeshTemplate;

	// Cockpit
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent*                    CockpitMesh;

	// Light
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UPointLightComponent*                    CockpitLight;

	// Light2
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UPointLightComponent*                    CockpitLight2;

	// Gameplay data
	bool                                     IsInCockpit;
	int32                                    CockpitInstrumentsTargetSize;
	int32                                    CockpitFLIRTargetSize;
	float                                    CockpitHealthLightTime;
	float                                    CockpitHealthLightPeriod;
	float                                    CockpitPowerTime;
	float                                    CockpitPowerPeriod;
	

	/*----------------------------------------------------
		Materials
	----------------------------------------------------*/

	// Cockpit material template
	UPROPERTY()
	UMaterial*                               CockpitMaterialTemplate;

	// Cockpit material instance
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UMaterialInstanceDynamic*                CockpitMaterialInstance;

	// Cockpit material template (frame)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UMaterialInstanceConstant*               CockpitFrameMaterialTemplate;

	// Cockpit material instance (frame)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UMaterialInstanceDynamic*                CockpitFrameMaterialInstance;


	/*----------------------------------------------------
		Cameras and render targets
	----------------------------------------------------*/

	// Cockpit texture (HUD)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitHUDTarget;

	// Cockpit texture (instruments)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitInstrumentsTarget;

	// Scene capture (target camera)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneCaptureComponent2D*                CockpitFLIRCapture;

	// Cockpit texture (target camera)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitFLIRCameraTarget;

#if FLARE_USE_COCKPIT_RENDERTARGET

	// Scene capture (main camera, for RT version)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneCaptureComponent2D*                CockpitCapture;

	// Cockpit texture (main camera, for RT version)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitCameraTarget;

#endif

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	
	inline UStaticMeshComponent* GetCockpitMesh()
	{
		return CockpitMesh;
	}

};
