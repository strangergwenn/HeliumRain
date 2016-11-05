
#include "../Flare.h"

#include "FlareSpacecraftStateManager.h"
#include "../Player/FlareCockpitManager.h"
#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "FlareShipPilot.h"
#include "FlareSpacecraft.h"

DECLARE_CYCLE_STAT(TEXT("FlareStateManager Tick"), STAT_FlareStateManager_Tick, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareStateManager Camera"), STAT_FlareStateManager_Camera, STATGROUP_Flare);


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftStateManager::UFlareSpacecraftStateManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, AngularInputDeadRatio(0.032)
	, MouseSensitivity(0.1)
	, MouseSensitivityPower(2)
{
	// Pilot
	IsPiloted = true;
}

void UFlareSpacecraftStateManager::Initialize(AFlareSpacecraft* ParentSpacecraft)
{
	Spacecraft = ParentSpacecraft;

	PlayerAim = FVector2D::ZeroVector;
	PlayerMousePosition = FVector2D::ZeroVector;
	LastPlayerAimJoystick = FVector2D::ZeroVector;
	LastPlayerAimMouse = FVector2D::ZeroVector;

	ExternalCamera = true;
	LinearVelocityIsJoystick = false;
	PlayerManualVelocityCommandActive = true;

	LastPlayerLinearVelocityKeyboard = FVector::ZeroVector;
	LastPlayerLinearVelocityJoystick = FVector::ZeroVector;
	LastPlayerAngularRollKeyboard = 0;
	LastPlayerAngularRollJoystick = 0;

	PlayerManualLinearVelocity = FVector::ZeroVector;
	PlayerManualAngularVelocity = FVector::ZeroVector;
	float ForwardVelocity = FVector::DotProduct(ParentSpacecraft->GetLinearVelocity(), FVector(1, 0, 0));
	PlayerManualVelocityCommand = ForwardVelocity;

	InternalCameraPitch = 0;
	InternalCameraYaw = 0;
	InternalCameraPitchTarget = 0;
	InternalCameraYawTarget = 0;

	ResetExternalCamera();
	LastWeaponType = EFlareWeaponGroupType::WG_NONE;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareSpacecraftStateManager::Tick(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareStateManager_Tick);

	AFlarePlayerController* PC = Spacecraft->GetPC();
	EFlareWeaponGroupType::Type CurrentWeaponType = Spacecraft->GetWeaponsSystem()->GetActiveWeaponType();
	float MaxVelocity = Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();

	if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive() && IsPiloted) // Do not tick the pilot if a player has disable the pilot
	{
		Spacecraft->GetPilot()->TickPilot(DeltaSeconds);
		int32 PreferedWeaponGroup = Spacecraft->GetPilot()->GetPreferedWeaponGroup();
		if (PreferedWeaponGroup >= 0)
		{
			Spacecraft->GetWeaponsSystem()->ActivateWeaponGroup(PreferedWeaponGroup);
		}
		else
		{
			Spacecraft->GetWeaponsSystem()->DeactivateWeapons();
		}
	}

	// Axis input mode start, reset mouse offset
	if (LastWeaponType == EFlareWeaponGroupType::WG_NONE && (CurrentWeaponType == EFlareWeaponGroupType::WG_GUN || CurrentWeaponType == EFlareWeaponGroupType::WG_BOMB ))
	{
		PlayerAim = FVector2D::ZeroVector;
	}
	LastWeaponType = CurrentWeaponType;

	// Stations can't move
	if (Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() == 0)
	{
		PlayerManualVelocityCommand = 0;
	}

	// Keep current command if no new command has been received
	else if (FMath::IsNearlyZero(PlayerManualLinearVelocity.Size()))
	{
		if (PlayerManualVelocityCommandActive)
		{
			PlayerManualVelocityCommandActive = false;
			PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / MaxVelocity;
		}
	}

	// Manual speed
	else 
	{
		PlayerManualVelocityCommandActive = true;
		PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / MaxVelocity;

		// Joystick speed setting
		if (LinearVelocityIsJoystick)
		{
			if (PlayerManualLinearVelocity.X / MaxVelocity < PlayerManualVelocityCommand)
			{
				PlayerManualVelocityCommand -= 0.1;
			}
			else if (PlayerManualLinearVelocity.X / MaxVelocity> PlayerManualVelocityCommand)
			{
				PlayerManualVelocityCommand += 0.1;
			}
		}

		// Keyboard speed setting
		else
		{
			if (PlayerManualLinearVelocity.X < 0)
			{
				PlayerManualVelocityCommand -= 0.1;
			}
			else if (PlayerManualLinearVelocity.X > 0)
			{
				PlayerManualVelocityCommand += 0.1;
			}
		}
	}

	if (!PlayerManualLockDirection)
	{
		if(!Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			PlayerManualLockDirectionVector = Spacecraft->GetLinearVelocity().GetUnsafeNormal();
		}
		else
		{
			PlayerManualLockDirectionVector = Spacecraft->GetFrontVector();
		}
	}

	if (!IsPiloted && PlayerManualVelocityCommandActive)
	{
		Spacecraft->ForceManual();
	}

	// Control
	switch (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
	{
		case EFlareWeaponGroupType::WG_TURRET:
			PlayerManualAngularVelocity.Z = 0;
			PlayerManualAngularVelocity.Y = 0;
			break;

		case EFlareWeaponGroupType::WG_NONE:
		case EFlareWeaponGroupType::WG_GUN:
		case EFlareWeaponGroupType::WG_BOMB:
		{
			if (Spacecraft->IsFlownByPlayer() && PC && !PC->GetNavHUD()->IsWheelMenuOpen() && !PC->GetMenuManager()->IsUIOpen())
			{
				// Compensation curve
				float DistanceToCenter = FMath::Sqrt(FMath::Square(PlayerAim.X) + FMath::Square(PlayerAim.Y));
				float CompensatedDistance = FMath::Pow(FMath::Clamp((DistanceToCenter - AngularInputDeadRatio), 0.f, 1.f), MouseSensitivityPower);
				float Angle = FMath::Atan2(PlayerAim.Y, PlayerAim.X);

				PlayerManualAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
				PlayerManualAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			}
			else
			{
				PlayerManualAngularVelocity.Z = 0;
				PlayerManualAngularVelocity.Y = 0;
			}
		}
		break;
	}

	// Update Camera
	UpdateCamera(DeltaSeconds);
}

void UFlareSpacecraftStateManager::UpdateCamera(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareStateManager_Camera);

	AFlarePlayerController* PC = Spacecraft->GetPC();
	if (Spacecraft->IsFlownByPlayer() && PC)
	{
		if (PC->GetMenuManager()->IsUIOpen())
		{
			return;
		}
	}

	if (ExternalCamera)
	{
		float Speed = FMath::Clamp(DeltaSeconds * 12, 0.f, 1.f);
		ExternalCameraYaw = ExternalCameraYaw * (1 - Speed) + ExternalCameraYawTarget * Speed;

		ExternalCameraPitchTarget = FMath::Clamp(ExternalCameraPitchTarget, -Spacecraft->GetCameraMaxPitch(), Spacecraft->GetCameraMaxPitch());

		ExternalCameraPitch = ExternalCameraPitch * (1 - Speed) + ExternalCameraPitchTarget * Speed;
		ExternalCameraDistance = ExternalCameraDistance * (1 - Speed) + ExternalCameraDistanceTarget * Speed;

		Spacecraft->SetCameraPitch(ExternalCameraPitch);
		Spacecraft->SetCameraYaw(FMath::UnwindDegrees(ExternalCameraYaw));
		Spacecraft->SetCameraDistance(ExternalCameraDistance);
	}
	else
	{
		InternalCameraYawTarget = 0;
		InternalCameraPitchTarget = 0;

		float Speed = FMath::Clamp(DeltaSeconds * 8, 0.f, 1.f);
		InternalCameraYaw = InternalCameraYaw * (1 - Speed) + InternalCameraYawTarget * Speed;
		InternalCameraPitch = InternalCameraPitch * (1 - Speed) + InternalCameraPitchTarget * Speed;
		Spacecraft->SetCameraPitch(InternalCameraPitch);
		Spacecraft->SetCameraYaw(InternalCameraYaw);
	}
	// TODO Camera smoothing

}

void UFlareSpacecraftStateManager::EnablePilot(bool PilotEnabled)
{
	IsPiloted = PilotEnabled;
}


void UFlareSpacecraftStateManager::SetExternalCamera(bool NewState)
{
	// If nothing changed...
	if (ExternalCamera == NewState)
	{
		return;
	}

	ExternalCamera = NewState;
	AFlarePlayerController* PC = Spacecraft->GetPC();

	// Put the camera at the right spot
	if (ExternalCamera)
	{
		ResetExternalCamera();
		Spacecraft->SetCameraLocalPosition(FVector::ZeroVector);
		if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_BOMB || Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_GUN)
		{
			Spacecraft->GetWeaponsSystem()->DeactivateWeapons();
		}
	}
	else
	{
		Spacecraft->SetCameraDistance(0);

#if FLARE_USE_COCKPIT_RENDERTARGET
		if (PC && PC->UseCockpit)
		{
			Spacecraft->SetCameraLocalPosition(FVector::ZeroVector);
		}
		else
#endif
		{
			FVector CameraOffset = Spacecraft->WorldToLocal(Spacecraft->Airframe->GetSocketLocation(FName("Camera")) - Spacecraft->GetActorLocation());
			Spacecraft->SetCameraLocalPosition(CameraOffset);
		}
	}

	// Notify cockpit
	if (PC && PC->UseCockpit)
	{
		PC->GetCockpitManager()->SetExternalCamera(ExternalCamera);
	}
}

void UFlareSpacecraftStateManager::SetPlayerMousePosition(FVector2D Val)
{
	PlayerMousePosition = Val;
}

void UFlareSpacecraftStateManager::SetPlayerAimMouse(FVector2D Val)
{
	// External camera : panning with mouse clicks
	if (ExternalCamera)
	{
		ExternalCameraYawTarget += Val.X * Spacecraft->GetCameraPanSpeed();
		ExternalCameraPitchTarget += Val.Y * Spacecraft->GetCameraPanSpeed();
		PlayerAim = FVector2D::ZeroVector;
	}
		
	// FP view
	else
	{
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(Spacecraft->GetWorld()->GetFirstPlayerController());
		if (PC && !PC->GetNavHUD()->IsWheelMenuOpen())
		{
			if (Val.X != LastPlayerAimMouse.X || Val.Y != LastPlayerAimMouse.Y)
			{
				LastPlayerAimMouse = Val;
				float X = Val.X * MouseSensitivity;
				float Y = -Val.Y * MouseSensitivity;

				PlayerAim += FVector2D(X, Y);
				if (PlayerAim.Size() > 1)
				{
					PlayerAim /= PlayerAim.Size();
				}
			}
		}
	}
}

void UFlareSpacecraftStateManager::SetPlayerAimJoystickYaw(float Val)
{
	if (Val != LastPlayerAimJoystick.X)
	{
		LastPlayerAimJoystick.X = Val;
		PlayerAim.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerAimJoystickPitch(float Val)
{
	if (Val != LastPlayerAimJoystick.Y)
	{
		LastPlayerAimJoystick.Y = Val;
		PlayerAim.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerLeftMouse(bool Val)
{
	PlayerLeftMousePressed = Val;
}

void UFlareSpacecraftStateManager::SetPlayerFiring(bool Val)
{
	PlayerFiring = Val;
}

void UFlareSpacecraftStateManager::ExternalCameraZoom(bool ZoomIn)
{
	if (ExternalCamera)
	{
		// TODO don't duplicate with Spacecraft Pawn
		// Compute camera data
		float Scale = Spacecraft->GetMeshScale();
		float LimitNear = Scale * 1.5;
		float LimitFar = Scale * 4;
		float Offset = Scale * (ZoomIn ? -0.5 : 0.5);

		// Move camera
		ExternalCameraDistanceTarget = FMath::Clamp(ExternalCameraDistance + Offset, LimitNear, LimitFar);
	}
}

void UFlareSpacecraftStateManager::SetPlayerLockDirection(bool Val)
{
	PlayerManualLockDirection = Val;
}

void UFlareSpacecraftStateManager::SetPlayerXLinearVelocity(float Val)
{
	if (Val != LastPlayerLinearVelocityKeyboard.X)
	{
		LinearVelocityIsJoystick = false;
		LastPlayerLinearVelocityKeyboard.X = Val;
		PlayerManualLinearVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocity(float Val)
{
	if (Val != LastPlayerLinearVelocityKeyboard.Y)
	{
		LastPlayerLinearVelocityKeyboard.Y = Val;
		PlayerManualLinearVelocity.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocity(float Val)
{
	if (Val != LastPlayerLinearVelocityKeyboard.Z)
	{
		LastPlayerLinearVelocityKeyboard.Z = Val;
		PlayerManualLinearVelocity.Z = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerXLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.X)
	{
		LinearVelocityIsJoystick = true;
		LastPlayerLinearVelocityJoystick.X = Val;
		PlayerManualLinearVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.Y)
	{
		LastPlayerLinearVelocityJoystick.Y = Val;
		PlayerManualLinearVelocity.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.Z)
	{
		LastPlayerLinearVelocityJoystick.Z = Val;
		PlayerManualLinearVelocity.Z = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocityKeyboard(float Val)
{
	if (Val != LastPlayerAngularRollKeyboard)
	{
		LastPlayerAngularRollKeyboard = Val;
		PlayerManualAngularVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocityJoystick(float Val)
{
	if (Val != LastPlayerAngularRollJoystick)
	{
		LastPlayerAngularRollJoystick = Val;
		PlayerManualAngularVelocity.X = Val;
	}
}

FVector UFlareSpacecraftStateManager::GetLinearTargetVelocity() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->GetLinearTargetVelocity();
	}
	else
	{
		FVector PlayerForwardVelocity;

		if (PlayerManualLockDirection && ! Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			PlayerForwardVelocity =  PlayerManualLockDirectionVector * FMath::Abs(PlayerManualVelocityCommand) * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
		}
		else
		{
			FVector LocalPlayerForwardVelocity = PlayerManualVelocityCommand * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() * FVector(1, 0, 0);
			PlayerForwardVelocity = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerForwardVelocity);
		}

		FVector LocalPlayerLateralLinearVelocity = FVector(0, PlayerManualLinearVelocity.Y, PlayerManualLinearVelocity.Z);
		return PlayerForwardVelocity + Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerLateralLinearVelocity);
	}
}

void UFlareSpacecraftStateManager::OnStatusChanged()
{
	// Maybe set to manual, in this case, forgot the old command
	float ForwardVelocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), FVector(1, 0, 0));
	PlayerManualVelocityCommand = ForwardVelocity;
}

FVector UFlareSpacecraftStateManager::GetAngularTargetVelocity() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->GetAngularTargetVelocity();
	}
	else
	{
		return Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(PlayerManualAngularVelocity);
	}
}


bool UFlareSpacecraftStateManager::IsUseOrbitalBoost() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->IsUseOrbitalBoost();
	}
	else
	{
		return true;
	}
}

bool UFlareSpacecraftStateManager::IsWantFire() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->IsWantFire();
	}
	else
	{
		if (ExternalCamera)
		{
			return false;
		}
		else if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
		{
			return (PlayerLeftMousePressed || PlayerFiring) && !Spacecraft->GetPC()->GetMenuManager()->IsUIOpen();
		}
		else
		{
			return false;
		}
	}
}

bool UFlareSpacecraftStateManager::IsWantCursor() const
{
	if (ExternalCamera)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UFlareSpacecraftStateManager::IsWantContextMenu() const
{
	if (ExternalCamera)
	{
		return true;
	}
	else if (Spacecraft->GetPC()->GetMenuManager()->IsOverlayOpen())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UFlareSpacecraftStateManager::ResetExternalCamera()
{
	ExternalCameraPitch = 0;
	ExternalCameraYaw = 0;
	ExternalCameraPitchTarget = 0;
	ExternalCameraYawTarget = 0;
	// TODO Don't copy SpacecraftPawn
	ExternalCameraDistance = 2.5 * Spacecraft->GetMeshScale();
	ExternalCameraDistanceTarget = ExternalCameraDistance;
}

void UFlareSpacecraftStateManager::OnCollision()
{
	PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
}
