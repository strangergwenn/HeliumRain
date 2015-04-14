#pragma once

#include "../Game/FlareCompany.h"
#include "FlareShipComponent.h"
#include "FlareShipPilot.h"
#include "FlareShipBase.generated.h"


class AFlareGame;
class AFlarePlayerController;
class UFlareInternalComponent;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship))
class AFlareShipBase : public APawn
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
		Damage status
	----------------------------------------------------*/

	/**
	 * Return true if any lifesupport system is alive
	 */
	virtual bool IsAlive() { return false; };

	/**
	 * Return true if the ship cockpit is powered
	 */
	virtual bool IsPowered() { return true; };

	/**
	 * Return true if the ship is currently on power outage
	 */
	virtual bool HasPowerOutage() { return false; };

	/**
	 * If on power outage, time until the end of the power outage. Else 0.
	 */
	virtual float GetPowerOutageDuration() { return 0.f; };

	/**
	 * Return true if the ship has weapon
	 */
	virtual bool IsArmed() { return false; };

	/**
	 * Update power status for all components
	 */
	virtual void UpdatePower();

	/**
	 * Method call if a electric component had been damaged
	 */
	virtual void OnElectricDamage(float DamageRatio) {};


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
	virtual void ReloadPart(UFlareShipComponent* Target, const FFlareShipComponentSave* Data);

	/** Update the parts settings */
	virtual void UpdateCustomization();

	/** Setup for menu display */
	virtual void StartPresentation();

	/** We are now in presentation mode */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void PresentationModeStarted();


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
	FQuat WorldToLocal(FQuat World);

	/** Get the pawn */
	AFlarePlayerController* GetPC() const;

	/** Get the game class instance */
	AFlareGame* GetGame() const;

	/** Get the ship size */
	float GetMeshScale() const;

	virtual UFlareInternalComponent* GetInternalComponentAtLocation(FVector Location) const;

	/**
	 * Return cockpit, if present
	 */
	virtual UFlareShipComponent* GetCockpit() const { return NULL; };


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

};
