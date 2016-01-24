#pragma once

#include "FlareCockpitManager.generated.h"

class AFlareSpacecraft;
class UFlareSpacecraftComponent;
class AFlarePlayerController;


/** Cockpit manager class */
UCLASS()
class FLARE_API AFlareCockpitManager : public AActor
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
	void EnterCockpit(AFlareSpacecraft* PlayerShip);

	/** Exit the cockpit */
	void ExitCockpit(AFlareSpacecraft* PlayerShip);

	/** Update the target info */
	void UpdateTarget(float DeltaSeconds);

	/** Update the info screen */
	void UpdateInfo(float DeltaSeconds);

	/** Update the overheat info */
	void UpdateTemperature(float DeltaSeconds);


protected:

	/*----------------------------------------------------
		Protected data
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

	// Scene capture for the ship's camera
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneCaptureComponent2D*                CockpitCapture;

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

	// Cockpit texture (camera)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitCameraTarget;

	// Cockpit texture (HUD)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitHUDTarget;

	// Cockpit texture (instruments)
	UPROPERTY(Category = Cockpit, VisibleDefaultsOnly, BlueprintReadOnly)
	UCanvasRenderTarget2D*                   CockpitInstrumentsTarget;

	// Settings
	int32                                    CockpitInstrumentsTargetSize;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline USceneCaptureComponent2D* GetCockpitCameraCapture()
	{
		return CockpitCapture;
	}

	inline UStaticMeshComponent* GetCockpitMesh()
	{
		return CockpitMesh;
	}

};