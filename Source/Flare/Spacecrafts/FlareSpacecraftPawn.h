#pragma once

#include "../Game/FlareCompany.h"
#include "FlareSpacecraftComponent.h"
#include "FlareSpacecraftPawn.generated.h"


class AFlareGame;
class AFlarePlayerController;
class UFlareInternalComponent;

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

	/** Step the camera distance */
	void StepCameraDistance(bool TowardCenter);


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the company this object nelongs to */
	void SetCompany(UFlareCompany* NewCompany);

	/** Reload the part Target with TargetDescription */
	virtual void ReloadPart(UFlareSpacecraftComponent* Target, const FFlareSpacecraftComponentSave* Data);

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

	/** Compute a quaternion's "length" in degrees */
	static float GetRotationAmount(const FQuat& X, bool ClampToHalfTurn = false);

	/** Clamp a quaternion within a cone */
	static FQuat ClampQuaternion(const FQuat& X, float Angle);

	/** Compute a float value between -1 and 1 to regulate a value */
	static float TriStateRegulator(float Target, float Current, float Threshold);

	/** Convert a direction from world space to localspace */
	FVector WorldToLocal(FVector World);

	/** Convert a rotation from world space to local space */
	FQuat WorldToLocal(const FQuat& World);

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
	float    CameraMinDistance;
	float    CameraMaxDistance;
	float    CameraDistanceStepAmount;
	FVector  CameraLocalPosition;

	// Company reference
	UPROPERTY()
	UFlareCompany* Company;

private:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Current spherical coordinates for the camera
	float    CameraOffsetPitch;
	float    CameraOffsetYaw;
	float    CameraOffsetDistance;


	public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline bool IsPresentationMode() const
	{
		return PresentationMode;
	}

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

};
