#pragma once

#include "FlareSpacecraftStateManager.generated.h"

/** Combat mode manager class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftStateManager : public UObject
{

public:

	GENERATED_UCLASS_BODY()													  
public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void Initialize(AFlareSpacecraft* ParentSpacecraft);

	virtual void Tick(float DeltaSeconds);

	virtual void UpdateCamera(float DeltaSeconds);

	virtual void EnablePilot(bool EnablePilot);

	/** Activate or deactivate the external camera */
	virtual void SetExternalCamera(bool NewState);

	inline bool IsExternalCamera() const
	{
		return ExternalCamera;
	}

	inline bool IsPilotMode() const
	{
		return IsPiloted;
	}

	inline FVector2D GetPlayerAim() const
	{
		return PlayerAim;
	}

	void ResetExternalCamera();

	void OnCollision();
	
	// Mouse aim and firing
	virtual void SetPlayerMousePosition(FVector2D Val);
	virtual void SetPlayerAimMouse(FVector2D Val);
	virtual void SetPlayerLeftMouse(bool Val);
	virtual void SetPlayerFiring(bool Val);

	// Joystick aim
	virtual void SetPlayerAimJoystickYaw(float Val);
	virtual void SetPlayerAimJoystickPitch(float Val);
	
	// Mouse zoom
	virtual void ExternalCameraZoom(bool ZoomIn);

	// Mouse movement
	virtual void SetPlayerLockDirection(bool Val);
	virtual void SetPlayerXLinearVelocity(float Val);
	virtual void SetPlayerYLinearVelocity(float Val);
	virtual void SetPlayerZLinearVelocity(float Val);

	// Joystick movement
	virtual void SetPlayerXLinearVelocityJoystick(float Val);
	virtual void SetPlayerYLinearVelocityJoystick(float Val);
	virtual void SetPlayerZLinearVelocityJoystick(float Val);
	
	// Roll movement
	virtual void SetPlayerRollAngularVelocityKeyboard(float Val);
	virtual void SetPlayerRollAngularVelocityJoystick(float Val);

	virtual void OnStatusChanged();

	// Output
	virtual FVector GetLinearTargetVelocity() const;
	virtual FVector GetAngularTargetVelocity() const;
	virtual bool IsUseOrbitalBoost() const;
	virtual bool IsWantFire() const;
	virtual bool IsWantCursor() const;
	virtual bool IsWantContextMenu() const;

protected:

	AFlareSpacecraft*                        Spacecraft;

	// Settings
	float                                    AngularInputDeadRatio;
	float                                    MouseSensitivity;
	float                                    MouseSensitivityPower;

	// Dynamic gameplay data
	bool                                     IsPiloted;
	bool                                     ExternalCamera;

	// External camera
	float                                    ExternalCameraPitch;
	float                                    ExternalCameraYaw;
	float                                    ExternalCameraDistance;
	float									 ExternalCameraPitchTarget;
	float									 ExternalCameraYawTarget;
	float									 ExternalCameraDistanceTarget;

	// Internal camera
	float									 InternalCameraPitch;
	float									 InternalCameraYaw;
	float									 InternalCameraPitchTarget;
	float									 InternalCameraYawTarget;
	
	EFlareWeaponGroupType::Type              LastWeaponType;
	bool                                     PlayerFiring;
	
	// Manual player pilot
	bool                                     PlayerLeftMousePressed;
	bool                                     PlayerManualLockDirection;
	FVector                                  PlayerManualAngularVelocity; // In local space
	FVector                                  FireDirectorAngularVelocity; // In local space
	FVector                                  PlayerManualLinearVelocity;
	FVector2D                                PlayerMousePosition;
	float	                                 PlayerManualVelocityCommand;
	bool	                                 PlayerManualVelocityCommandActive;
	FVector	                                 PlayerManualLockDirectionVector;

	// Linear movement state
	FVector                                  LastPlayerLinearVelocityKeyboard;
	FVector                                  LastPlayerLinearVelocityJoystick;
	bool                                     LinearVelocityIsJoystick;

	// Roll state
	float	                                 LastPlayerAngularRollKeyboard;
	float	                                 LastPlayerAngularRollJoystick;
	
	// Player aim state
	FVector2D                                PlayerAim;
	FVector2D                                LastPlayerAimJoystick;
	FVector2D                                LastPlayerAimMouse;

};
