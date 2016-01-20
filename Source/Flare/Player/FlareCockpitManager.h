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

	/*----------------------------------------------------
		Internal
	----------------------------------------------------*/

	virtual void Tick(float DeltaSeconds) override;
	

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

	// Cockpit material template
	UPROPERTY()
	UMaterial*                               CockpitMaterialTemplate;

	// Cockpit material instance
	UPROPERTY()
	UMaterialInstanceDynamic*                CockpitMaterialInstance;

	// Cockpit material template (frame)
	UPROPERTY()
	UMaterial*                               CockpitFrameMaterialTemplate;

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


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/



};