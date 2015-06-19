#pragma once

#include "FlareSpacecraftStateManager.generated.h"

/** Combat mode manager class */
UCLASS()
class FLARE_API UFlareSpacecraftStateManager : public UObject
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

	inline FVector2D GetPlayerMouseOffset() const
	{
		return PlayerMouseOffset;
	}

	virtual void SetPlayerMousePosition(FVector2D Val);

	virtual void SetPlayerMouseOffset(FVector2D Val, bool Relative);

	virtual void SetPlayerLeftMouse(bool Val);

	virtual void ExternalCameraZoom(bool ZoomIn);

	virtual void SetPlayerOrbitalBoost(bool Val);

	virtual void SetPlayerXLinearVelocity(float Val);
	virtual void SetPlayerYLinearVelocity(float Val);
	virtual void SetPlayerZLinearVelocity(float Val);

	virtual void SetPlayerRollAngularVelocity(float Val);

	// Output
	virtual FVector GetLinearTargetVelocity() const;
	virtual FVector GetAngularTargetVelocity() const;
	virtual float GetAccelerationRatioTarget() const;
	virtual bool IsUseOrbitalBoost() const;
	virtual bool IsWantFire() const;
	virtual bool IsWantCursor() const;
	virtual bool IsWantContextMenu() const;

protected:

	AFlareSpacecraft*                        Spacecraft;
	float                                    AngularInputDeadRatio;

	// Dynamic gameplay data


	bool                                     IsPiloted;
	bool                                     ExternalCamera;

	float                                    ExternalCameraPitch;
	float                                    ExternalCameraYaw;
	float                                    ExternalCameraDistance;
	float									 ExternalCameraPitchTarget;
	float									 ExternalCameraYawTarget;
	float									 ExternalCameraDistanceTarget;

	float									 InternalCameraPitch;
	float									 InternalCameraYaw;
	float									 InternalCameraPitchTarget;
	float									 InternalCameraYawTarget;



	EFlareWeaponGroupType::Type              LastWeaponType;


	// Manual player pilot
	bool                                     PlayerLeftMousePressed;
	bool                                     PlayerManualOrbitalBoost;
	FVector                                  PlayerManualAngularVelocity; // In local space
	FVector                                  PlayerManualLinearVelocity;
	FVector2D                                PlayerMouseOffset;
	FVector2D                                PlayerMousePosition;
};
