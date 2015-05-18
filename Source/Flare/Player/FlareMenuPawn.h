#pragma once

#include "../Ships/FlareShip.h"
#include "../Ships/FlareShipComponent.h"
#include "FlareMenuPawn.generated.h"


UCLASS()
class AFlareMenuPawn : public AFlareSpacecraftPawn
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		Resource loading
	----------------------------------------------------*/

	/** Load a new ship to visualize */
	void ShowShip(const FFlareShipDescription* ShipDesc, const FFlareShipSave* ShipData);

	/** Load a new station to visualize */
	//void ShowStation(const FFlareStationDescription* StationDesc, const FFlareStationSave* StationData);

	/** Load a new part to visualize */
	void ShowPart(const FFlareShipComponentDescription* PartDesc);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Update the part settings by forcing it to reload the material data */
	void UpdateCustomization();

	/** Remove all parts and go back to the original setup */
	void ResetContent(bool Unsafe = false);

	/** Move the camera on X */
	void SetHorizontalOffset(float Offset);

	/** Set the way parts slide in the browser */
	void SetSlideDirection(bool GoUp);

	/** Change the background color */
	UFUNCTION(BlueprintImplementableEvent, Category = "Flare")
	void UpdateBackgroundColor(float HueShift, float Luminosity);


protected:
	
	/*----------------------------------------------------
		Input methods
	----------------------------------------------------*/
	
	virtual void PitchInput(float Val);

	virtual void YawInput(float Val);


protected:
	
	/*----------------------------------------------------
		Component data
	----------------------------------------------------*/

	/** StaticMesh container (we will rotate this on Yaw) */
	UPROPERTY(Category = PartViewer, VisibleDefaultsOnly, BlueprintReadOnly)
	class USceneComponent* PartContainer;

	/** StaticMesh component */
	UPROPERTY(Category = PartViewer, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareShipComponent* CurrentPartA;

	/** StaticMesh component */
	UPROPERTY(Category = PartViewer, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareShipComponent* CurrentPartB;

	/** Max pitch angle on viewer */
	UPROPERTY(Category = PartViewer, EditAnywhere, BlueprintReadWrite)
	float InitialYaw;

	/** Camera distance */
	UPROPERTY(Category = ShipBase, EditAnywhere, BlueprintReadWrite)
	float DisplayDistance;
	
	/** Component size to use */
	UPROPERTY(Category = ShipBase, EditAnywhere, BlueprintReadWrite)
	float DisplaySize;

	/** Offset vector to slide in or out of the scene (Z variant)*/
	UPROPERTY(Category = PartViewer, EditAnywhere, BlueprintReadWrite)
	FVector SlideInOutUpOffset;

	/** Offset vector to slide in or out of the scene (X variant) */
	UPROPERTY(Category = PartViewer, EditAnywhere, BlueprintReadWrite)
	FVector SlideInOutSideOffset;

	/** Time to slide in or out of the scene */
	UPROPERTY(Category = PartViewer, EditAnywhere, BlueprintReadWrite)
	float SlideInOutTime;


	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	/** Ship reference */
	UPROPERTY()
	AFlareShip*                CurrentShip;

	/** Station reference */
	UPROPERTY()
	AFlareShip*             CurrentStation;

	// Part data
	FVector                    CurrentPartOffsetA;
	FVector                    CurrentPartOffsetB;
	FVector                    CurrentShipOffset;

	// Slide data
	bool                       SlideFromAToB;
	float                      SlideDirection;
	float                      SlideInOutCurrentTime;
	FVector                    SlideInOutOffset;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlarePlayerController* GetPC() const
	{
		return Cast<AFlarePlayerController>(GetController());
	}

	inline AFlareShip* GetCurrentShip()
	{
		return CurrentShip;
	}

	inline AFlareShip* GetCurrentStation()
	{
		return CurrentStation;
	}


};
