#pragma once

#include "../Game/FlareCompany.h"
#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftPawn.generated.h"


class AFlareGame;
class AFlarePlayerController;
class UFlareInternalComponent;

/** Resource lock type values */
UENUM()
namespace EFlareCameraMode
{
	enum Type
	{
		InternalFixed,
		InternalRotating,
		Immersive,
	};
}

UCLASS(Blueprintable, ClassGroup = (Flare, Ship))
class AFlareSpacecraftPawn : public APawn
{

public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Component data
	----------------------------------------------------*/

	/** Camera container (we will rotate this on Yaw) */
	UPROPERTY(Category = Components, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneComponent* CameraContainerYaw;

	/** Camera container (we will rotate this on Pitch) */
	UPROPERTY(Category = Components, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneComponent* CameraContainerPitch;
	
	/** Camera component */
	UPROPERTY(Category = Components, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Camera;

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/*----------------------------------------------------
		Camera control
	----------------------------------------------------*/

	/** Set the camera pitch in the spherical coordinate system */
	void SetCameraPitch(float Value);

	/** Set the camera yaw in the spherical coordinate system */
	void SetCameraYaw(float Value);

	/** Set the offset for the camera containers */
	void SetCameraLocalPosition(FVector Value);

	/** Set the camera radius in the spherical coordinate system */
	void SetCameraDistance(float Value);

	void ConfigureImmersiveCamera(FQuat TargetRotation);

	void ConfigureInternalRotatingCamera(FQuat TargetRotation);

	void ConfigureInternalFixedCamera();

	void SetPhysicalVisibility(bool Visibility);

	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the company this object nelongs to */
	void SetCompany(UFlareCompany* NewCompany);

	/** Reload the part Target with TargetDescription */
	virtual void ReloadPart(UFlareSpacecraftComponent* Target, FFlareSpacecraftComponentSave* Data);

	/** Update the parts settings */
	virtual void UpdateCustomization();

	/** Setup for menu display */
	virtual void StartPresentation();

	/** We are now in presentation mode */
	UFUNCTION(BlueprintImplementableEvent)
	void PresentationModeStarted();


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Convert a direction from world space to localspace */
	FVector WorldToLocal(FVector World);

	/** Get the pawn */
	AFlarePlayerController* GetPC() const;

	/** Check if this ship is the player ship */
	bool IsFlownByPlayer() const;

	/** Get the game class instance */
	AFlareGame* GetGame() const;

	/** Get the ship size */
	float GetMeshScale() const;


protected:

	/*----------------------------------------------------
		Properties
	----------------------------------------------------*/

	// Gameplay data
	bool     PresentationMode;

	// Camera rotation properties
	float    CameraPanSpeed;
	float    CameraMaxPitch;
	float    CameraMaxYaw;

	// Camera distance properties
	FVector  CameraLocalPosition;

	// Company reference
	UPROPERTY()
	UFlareCompany* Company;

private:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Current spherical coordinates for the camera
	float                                           CameraOffsetPitch;
	float                                           CameraOffsetYaw;
	float                                           CameraOffsetDistance;
	FQuat                                           CameraTargetRotation;
	EFlareCameraMode::Type                          CameraMode;



	float                                           MeshScaleCache;

	FName                                           PreviousCameraName;
	FName                                           CurrentCameraName;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	UFUNCTION(BlueprintCallable, Category=Gameplay)
	bool IsPresentationMode() const
	{
		return PresentationMode;
	}

	bool IsPlayerShip();

	inline EFlareHostility::Type GetPlayerHostility() const
	{
		if (Company)
		{
			return Company->GetPlayerHostility();
		}
		else
		{
			return EFlareHostility::Neutral;
		}
	}

	inline EFlareHostility::Type GetHostility(UFlareCompany* TargetCompany) const
	{
		if (Company)
		{
			return Company->GetHostility(TargetCompany);
		}
		else
		{
			return EFlareHostility::Neutral;
		}
	}


	inline EFlareHostility::Type GetWarState(UFlareCompany* TargetCompany) const
	{
		if (Company)
		{
			return Company->GetWarState(TargetCompany);
		}
		else
		{
			return EFlareHostility::Neutral;
		}
	}

	UCameraComponent* GetCamera() const
	{
		return Cast<UCameraComponent>(Camera);
	}

	inline float GetCameraPanSpeed() const
	{
		return CameraPanSpeed;
	}

	inline float GetCameraMaxPitch() const
	{
		return CameraMaxPitch;
	}

	inline float GetCameraMaxYaw() const
	{
		return CameraMaxYaw;
	}

	inline EFlareCameraMode::Type GetCameraMode() const
	{
		return CameraMode;
	}

	inline bool HasFLIRCameraChanged() const
	{
		return (PreviousCameraName != CurrentCameraName);
	}
	
};
