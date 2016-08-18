#pragma once

#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSpacecraftComponent.h"
#include "FlareMenuPawn.generated.h"


UCLASS()
class HELIUMRAIN_API AFlareMenuPawn : public AFlareSpacecraftPawn
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
	void ShowShip(UFlareSimulatedSpacecraft* Spacecraft);
	
	/** Load a new part to visualize */
	void ShowPart(const FFlareSpacecraftComponentDescription* PartDesc);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Update the part settings by forcing it to reload the material data */
	void UpdateCustomization();

	/** Remove all parts and go back to the original setup */
	void ResetContent(bool Unsafe = false);

	/** Move the camera */
	void SetCameraOffset(FVector2D Offset);

	/** Set the way parts slide in the browser */
	void SetSlideDirection(bool GoUp);

	/** Start using the light background setting */
	void UseLightBackground();

	/** Start using the dark background setting */
	void UseDarkBackground();

	/** Change the background color */
	UFUNCTION(BlueprintImplementableEvent, Category = "Flare")
	void UpdateBackgroundColor(float Luminosity, float Desaturation);


public:
	
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
	class UFlareSpacecraftComponent* CurrentPartA;

	/** StaticMesh component */
	UPROPERTY(Category = PartViewer, VisibleDefaultsOnly, BlueprintReadOnly)
	class UFlareSpacecraftComponent* CurrentPartB;

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
	
	/** Spacecraft reference */
	UPROPERTY()
	AFlareSpacecraft*                CurrentSpacecraft;

	// Part data
	FVector                    CurrentPartOffsetA;
	FVector                    CurrentPartOffsetB;
	FVector                    CurrentShipOffset;

	// Slide data
	bool                       SlideFromAToB;
	float                      SlideDirection;
	float                      SlideInOutCurrentTime;
	FVector                    SlideInOutOffset;

	// Camera
	float                      ExternalCameraPitch;
	float                      ExternalCameraPitchTarget;
	float                      ExternalCameraYaw;
	float                      ExternalCameraYawTarget;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlarePlayerController* GetPC() const
	{
		return Cast<AFlarePlayerController>(GetController());
	}

};
