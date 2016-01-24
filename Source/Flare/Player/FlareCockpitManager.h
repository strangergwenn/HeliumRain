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
	virtual void OnFlyShip(AFlareSpacecraft* NewShipPawn);

	/** Whether to use the external camera or not */
	virtual void SetExternalCamera(bool External);

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Enter the cockpit */
	void EnterCockpit(AFlareSpacecraft* ShipPawn);

	/** Exit the cockpit */
	void ExitCockpit(AFlareSpacecraft* ShipPawn);

	/** Update the target info */
	void UpdateTarget(float DeltaSeconds);

	/** Update the info screen */
	void UpdateInfo(float DeltaSeconds);

	/** Update the damage system info */
	void UpdateDamages(float DeltaSeconds);

	/** Update the overheat info */
	void UpdateTemperature(float DeltaSeconds);

	/** Update the power status */
	void UpdatePower(float DeltaSeconds);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Player controller
	UPROPERTY()
	AFlarePlayerController*                  PC;

	// Cyrrently flown ship
	UPROPERTY()
	AFlareSpacecraft*                        ShipPawn;

	// Cockpit mesh template
	UPROPERTY()
	UStaticMesh*                             CockpitMeshTemplate;

	// Cockpit
	UPROPERTY()
	UStaticMeshComponent*                    CockpitMesh;

	// Scene capture for the ship's camera
	UPROPERTY()
	USceneCaptureComponent2D*                CockpitCapture;

	// Cockpit material template
	UPROPERTY()
	UMaterial*                               CockpitMaterialTemplate;

	// Cockpit material instance
	UPROPERTY()
	UMaterialInstanceDynamic*                CockpitMaterialInstance;

	// Cockpit material template (frame)
	UPROPERTY()
	UMaterialInstanceConstant*               CockpitFrameMaterialTemplate;

	// Cockpit material instance (frame)
	UPROPERTY()
	UMaterialInstanceDynamic*                CockpitFrameMaterialInstance;

	// Cockpit texture (camera)
	UPROPERTY()
	UCanvasRenderTarget2D*                   CockpitCameraTarget;

	// Cockpit texture (HUD)
	UPROPERTY()
	UCanvasRenderTarget2D*                   CockpitHUDTarget;

	// Cockpit texture (instruments)
	UPROPERTY()
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