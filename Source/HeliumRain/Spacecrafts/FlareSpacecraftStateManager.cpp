
#include "FlareSpacecraftStateManager.h"
#include "../Flare.h"

#include "../Game/FlareGameUserSettings.h"
#include "../Player/FlareCockpitManager.h"
#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "FlareShipPilot.h"
#include "FlarePilotHelper.h"
#include "FlareSpacecraft.h"
#include "FlareShipPilot.h"

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
	PilotForced = false;
}

void UFlareSpacecraftStateManager::Initialize(AFlareSpacecraft* ParentSpacecraft)
{
	Spacecraft = ParentSpacecraft;

	PlayerAim = FVector2D::ZeroVector;
	PlayerMousePosition = FVector2D::ZeroVector;
	LastPlayerAimJoystick = FVector2D::ZeroVector;
	LastPlayerAimGamepad = FVector2D::ZeroVector;
	LastPlayerAimMouse = FVector2D::ZeroVector;

	CombatZoomEnabled = false;
	CombatZoomTimer = 0;
	CombatZoomDuration = 0.3f;

	ExternalCamera = true;
	LinearVelocitySource = EFlareInputSource::Keyboard;
	PlayerManualVelocityCommandActive = true;

	LastPlayerLinearVelocityKeyboard = FVector::ZeroVector;
	LastPlayerLinearVelocityJoystick = FVector::ZeroVector;
	LastPlayerLinearVelocityGamepad = FVector::ZeroVector;
	LastPlayerAngularRollKeyboard = 0;
	LastPlayerAngularRollJoystick = 0;

	PlayerManualLinearVelocity = FVector::ZeroVector;
	PlayerManualAngularVelocity = FVector::ZeroVector;
	FVector FrontVector = Spacecraft->Airframe-> GetComponentTransform().TransformVector(FVector(1, 0, 0));
	float ForwardVelocity = FVector::DotProduct(ParentSpacecraft->GetLinearVelocity(), FrontVector);
	PlayerManualVelocityCommand = ForwardVelocity;

	InternalCameraPitch = 0;
	InternalCameraYaw = 0;
	InternalCameraPitchTarget = 0;
	InternalCameraYawTarget = 0;

	ResetExternalCamera();
	LastWeaponType = EFlareWeaponGroupType::WG_NONE;
	IsFireDirectorInit = false;
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
	
	// Do not tick the pilot if a player has disable the pilot
	if (Spacecraft->GetParent()->GetDamageSystem()->IsAlive() && IsPiloted)
	{
		Spacecraft->GetPilot()->TickPilot(DeltaSeconds);

		// Fighters can use different weapons.
		// If this is needed for capitals too, it needs to check that the player didn't select a group already.
		if (Spacecraft->GetDescription()->Size == EFlarePartSize::S)
		{
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
	}

	// Axis input mode start, reset mouse offset
	if (LastWeaponType == EFlareWeaponGroupType::WG_NONE && (CurrentWeaponType == EFlareWeaponGroupType::WG_GUN || CurrentWeaponType == EFlareWeaponGroupType::WG_BOMB || CurrentWeaponType == EFlareWeaponGroupType::WG_MISSILE ))
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

		// Keyboard speed setting
		if (LinearVelocitySource == EFlareInputSource::Keyboard)
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

		// Joystick speed setting
		else if (LinearVelocitySource == EFlareInputSource::Joystick)
		{
			if (PlayerManualLinearVelocity.X / MaxVelocity < PlayerManualVelocityCommand)
			{
				PlayerManualVelocityCommand -= 0.1;
			}
			else if (PlayerManualLinearVelocity.X / MaxVelocity > PlayerManualVelocityCommand)
			{
				PlayerManualVelocityCommand += 0.1;
			}
		}

		// Gamepad speed setting
		else if (LinearVelocitySource == EFlareInputSource::Gamepad)
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

	if (!PlayerManualLockDirection || ExternalCamera)
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

	// Abort pilot if we're doing manual inputs, except while docking
	if (!IsPiloted && PlayerManualVelocityCommandActive
	 && !Spacecraft->GetNavigationSystem()->IsDocked()
	 && !Spacecraft->GetIsAutoDocking())
	{
		Spacecraft->GetNavigationSystem()->AbortAllCommands();
	}

	// Mouse control
	if (Spacecraft->IsFlownByPlayer() && PC && !PC->GetNavHUD()->IsWheelMenuOpen() && !PC->GetMenuManager()->IsUIOpen())
	{
		// Compensation curve
		float DistanceToCenter = FMath::Sqrt(FMath::Square(PlayerAim.X) + FMath::Square(PlayerAim.Y));
		float CompensatedDistance = FMath::Pow(FMath::Clamp((DistanceToCenter - AngularInputDeadRatio), 0.f, 1.f), MouseSensitivityPower);
		float Angle = FMath::Atan2(PlayerAim.Y, PlayerAim.X);

		// Pass data
		if (Spacecraft->GetWeaponsSystem()->IsInFireDirector())
		{
			FireDirectorAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * 2 * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			FireDirectorAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * 2 * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			PlayerManualAngularVelocity.Z = 0;
			PlayerManualAngularVelocity.Y = 0;
		}
		else if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET)
		{
			FireDirectorAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * 2 * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			FireDirectorAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * 2 * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			PlayerManualAngularVelocity.Z = 0;
			PlayerManualAngularVelocity.Y = 0;
		}
		else
		{
			FireDirectorAngularVelocity.Z = 0;
			FireDirectorAngularVelocity.Y = 0;
			PlayerManualAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			PlayerManualAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
		}
	}
	else
	{
		PlayerManualAngularVelocity.Z = 0;
		PlayerManualAngularVelocity.Y = 0;
		FireDirectorAngularVelocity.Z = 0;
		FireDirectorAngularVelocity.Y = 0;
	}

	// Update combat zoom
	if (ExternalCamera || !Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
	{
		CombatZoomEnabled = false;
	}
	CombatZoomTimer += (CombatZoomEnabled ? +1 : -1) * DeltaSeconds;
	CombatZoomTimer = FMath::Clamp(CombatZoomTimer, 0.0f, CombatZoomDuration);

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

	if (!Spacecraft->IsFlownByPlayer())
	{
		Spacecraft->ConfigureInternalFixedCamera();
		return;
	}

	if (Spacecraft->GetWeaponsSystem()->IsInFireDirector())
	{
		float YawRotation = FireDirectorAngularVelocity.Z * DeltaSeconds;
		float PitchRotation = FireDirectorAngularVelocity.Y * DeltaSeconds;


		if (!IsFireDirectorInit)
		{
			FireDirectorLookRotation =  Spacecraft->Airframe-> GetComponentTransform().GetRotation();
			IsFireDirectorInit = true;
		}

		FQuat Yaw(FVector(0,0,1), FMath::DegreesToRadians(YawRotation));
		FQuat Pitch(FVector(0,1,0), FMath::DegreesToRadians(PitchRotation));

		FireDirectorLookRotation *= Yaw;
		FireDirectorLookRotation *= Pitch;
		FireDirectorLookRotation.Normalize();


		Spacecraft->ConfigureImmersiveCamera(FireDirectorLookRotation);
	}
	else if (ExternalCamera)
	{
		float ManualAcc = 600; //°/s-2
		float Resistance = 1 / 360.f;
		float Brake = 4.f;
		float Brake2 = 2.f;

		{
			float Acc = FMath::Sign(ExternalCameraYawTarget) * ManualAcc;
			float Res = FMath::Sign(ExternalCameraYawSpeed) * (Resistance * FMath::Square(ExternalCameraYawSpeed) + (Acc == 0 ? Brake2 + Brake * FMath::Abs(ExternalCameraYawSpeed) : 0));

			float MaxResDeltaSpeed = ExternalCameraYawSpeed;


			float AccDeltaYawSpeed = Acc * DeltaSeconds;
			float ResDeltaYawSpeed = - (FMath::Abs(Res * DeltaSeconds) > FMath::Abs(MaxResDeltaSpeed) ? MaxResDeltaSpeed : Res * DeltaSeconds);
			ExternalCameraYawTarget = 0; // Consume
			ExternalCameraYawSpeed += AccDeltaYawSpeed + ResDeltaYawSpeed;
			ExternalCameraYaw += ExternalCameraYawSpeed * DeltaSeconds;
			ExternalCameraYaw = FMath::UnwindDegrees(ExternalCameraYaw);
		}

		{
			float Acc = FMath::Sign(ExternalCameraPitchTarget) * ManualAcc;
			float Res = FMath::Sign(ExternalCameraPitchSpeed) * (Resistance * FMath::Square(ExternalCameraPitchSpeed) + (Acc == 0 ? Brake2 + Brake * FMath::Abs(ExternalCameraPitchSpeed) : 0));

			float MaxResDeltaSpeed = ExternalCameraPitchSpeed;

			float AccDeltaPitchSpeed = Acc * DeltaSeconds;
			float ResDeltaPitchSpeed = - (FMath::Abs(Res * DeltaSeconds) > FMath::Abs(MaxResDeltaSpeed) ? MaxResDeltaSpeed : Res * DeltaSeconds);
			ExternalCameraPitchTarget = 0; // Consume
			ExternalCameraPitchSpeed += AccDeltaPitchSpeed + ResDeltaPitchSpeed;
			ExternalCameraPitch += ExternalCameraPitchSpeed * DeltaSeconds;
			ExternalCameraPitch = FMath::UnwindDegrees(ExternalCameraPitch);
			float ExternalCameraPitchClamped = FMath::Clamp(ExternalCameraPitch, -Spacecraft->GetCameraMaxPitch(), Spacecraft->GetCameraMaxPitch());
			if(ExternalCameraPitchClamped != ExternalCameraPitch)
			{
				ExternalCameraPitchSpeed = -ExternalCameraPitchSpeed/2;
			}
			ExternalCameraPitch = ExternalCameraPitchClamped;
		}

		float Speed = FMath::Clamp(DeltaSeconds * 12, 0.f, 1.f);
		ExternalCameraDistance = ExternalCameraDistance * (1 - Speed) + ExternalCameraDistanceTarget * Speed;

		Spacecraft->ConfigureInternalFixedCamera();
		Spacecraft->SetCameraPitch(ExternalCameraPitch);
		Spacecraft->SetCameraYaw(ExternalCameraYaw);
		Spacecraft->SetCameraDistance(ExternalCameraDistance);

		IsFireDirectorInit = false;
	}
	else if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET)
	{
		float YawRotation = FireDirectorAngularVelocity.Z * DeltaSeconds;
		float PitchRotation = FireDirectorAngularVelocity.Y * DeltaSeconds;


		if (!IsFireDirectorInit)
		{
			FireDirectorLookRotation =  Spacecraft->Airframe->GetComponentTransform().GetRotation();
			IsFireDirectorInit = true;
		}

		float Dot = FVector::DotProduct(FireDirectorLookRotation.GetForwardVector(), Spacecraft->Airframe->GetComponentTransform().GetRotation().GetForwardVector());

		float ScaledDot = (FMath::Max(Dot - 0.5f, 0.f) * 2.f);

		FQuat Yaw(FVector(0,0,1), FMath::DegreesToRadians(YawRotation * ScaledDot));
		FQuat Pitch(FVector(0,1,0), FMath::DegreesToRadians(PitchRotation * ScaledDot));




		FireDirectorLookRotation *= Yaw;
		FireDirectorLookRotation *= Pitch;


		// roll correction
		FVector CameraTop = FireDirectorLookRotation.GetAxisZ();
		FVector LocalCameraTop = Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(CameraTop);

		FVector FlatCameraTop = LocalCameraTop;
		FlatCameraTop.X = 0;
		FlatCameraTop.Normalize();

		float Angle = FMath::UnwindRadians(FMath::Atan2(FlatCameraTop.Y, FlatCameraTop.Z));

		FQuat Roll(FVector(1,0,0), Angle);

		FireDirectorLookRotation *= Roll;

		FireDirectorLookRotation.Normalize();


		Spacecraft->ConfigureInternalRotatingCamera(FireDirectorLookRotation);
	}
	else
	{
		InternalCameraYawTarget = 0;
		InternalCameraPitchTarget = 0;

		float Speed = FMath::Clamp(DeltaSeconds * 8, 0.f, 1.f);
		
		Spacecraft->ConfigureInternalFixedCamera();
		InternalCameraYaw = InternalCameraYaw * (1 - Speed) + InternalCameraYawTarget * Speed;
		InternalCameraPitch = InternalCameraPitch * (1 - Speed) + InternalCameraPitchTarget * Speed;
		Spacecraft->SetCameraPitch(InternalCameraPitch);
		Spacecraft->SetCameraYaw(InternalCameraYaw);

		IsFireDirectorInit = false;
	}
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

	// Reset state
	Spacecraft->GetWeaponsSystem()->DeactivateWeapons();

	// Put the camera at the right spot
	if (ExternalCamera)
	{
		ResetExternalCamera();
		Spacecraft->SetCameraLocalPosition(FVector::ZeroVector);
	}
	else
	{
		FVector CameraOffset = Spacecraft->WorldToLocal(Spacecraft->Airframe->GetSocketLocation(FName("Camera")) - Spacecraft->GetActorLocation());

		Spacecraft->SetCameraDistance(0);
		Spacecraft->SetCameraLocalPosition(CameraOffset);
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
	auto& App = FSlateApplication::Get();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Spacecraft->GetWorld()->GetFirstPlayerController());

	// External camera : panning with mouse clicks
	if (ExternalCamera && !PlayerManualLockDirection && GEngine->GameViewport->GetGameViewportWidget()->HasMouseCapture() && !PC->GetNavHUD()->IsWheelMenuOpen())
	{
		FVector2D CursorPos = App.GetCursorPos();

		if (IsExternalCameraMouseOffsetInit && CursorPos != LastExternalCameraMouseOffset)
		{
			FVector2D MoveDirection = (CursorPos - LastExternalCameraMouseOffset).GetSafeNormal();
			ExternalCameraYawTarget += MoveDirection.X;
			ExternalCameraPitchTarget += -MoveDirection.Y;
		}

		LastExternalCameraMouseOffset = CursorPos;
		IsExternalCameraMouseOffsetInit = true;
		PlayerAim = FVector2D::ZeroVector;
	}
		
	// FP view
	else
	{
		IsExternalCameraMouseOffsetInit = false;

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

void UFlareSpacecraftStateManager::SetPlayerAimGamepadYaw(float Val)
{
	if (Val != LastPlayerAimGamepad.X)
	{
		LastPlayerAimGamepad.X = Val;
		PlayerAim.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerAimGamepadPitch(float Val)
{
	if (Val != LastPlayerAimGamepad.Y)
	{
		LastPlayerAimGamepad.Y = Val;
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

void UFlareSpacecraftStateManager::SetCombatZoom(bool ZoomIn)
{
	if (!ExternalCamera)
	{
		CombatZoomEnabled = ZoomIn;
	}
	else
	{
		CombatZoomEnabled = false;
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
		LinearVelocitySource = EFlareInputSource::Keyboard;
		EnablePilot(false);
		LastPlayerLinearVelocityKeyboard.X = Val;
		PlayerManualLinearVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocity(float Val)
{
	if (Val != LastPlayerLinearVelocityKeyboard.Y)
	{
		LinearVelocitySource = EFlareInputSource::Keyboard;
		EnablePilot(false);
		LastPlayerLinearVelocityKeyboard.Y = Val;
		PlayerManualLinearVelocity.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocity(float Val)
{
	if (Val != LastPlayerLinearVelocityKeyboard.Z)
	{
		LinearVelocitySource = EFlareInputSource::Keyboard;
		EnablePilot(false);
		LastPlayerLinearVelocityKeyboard.Z = Val;
		PlayerManualLinearVelocity.Z = Val;
	}
}


void UFlareSpacecraftStateManager::SetPlayerXLinearVelocityGamepad(float Val)
{
	if (Val != LastPlayerLinearVelocityGamepad.X)
	{
		LinearVelocitySource = EFlareInputSource::Gamepad;
		EnablePilot(false);
		LastPlayerLinearVelocityGamepad.X = Val;
		PlayerManualLinearVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocityGamepad(float Val)
{
	if (Val != LastPlayerLinearVelocityGamepad.Y)
	{
		LinearVelocitySource = EFlareInputSource::Gamepad;
		EnablePilot(false);
		LastPlayerLinearVelocityGamepad.Y = Val;
		PlayerManualLinearVelocity.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocityGamepad(float Val)
{
	if (Val != LastPlayerLinearVelocityGamepad.Z)
	{
		LinearVelocitySource = EFlareInputSource::Gamepad;
		EnablePilot(false);
		LastPlayerLinearVelocityGamepad.Z = Val;
		PlayerManualLinearVelocity.Z = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerXLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.X)
	{
		LinearVelocitySource = EFlareInputSource::Joystick;
		EnablePilot(false);
		LastPlayerLinearVelocityJoystick.X = Val;
		PlayerManualLinearVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.Y)
	{
		LinearVelocitySource = EFlareInputSource::Joystick;
		EnablePilot(false);
		LastPlayerLinearVelocityJoystick.Y = Val;
		PlayerManualLinearVelocity.Y = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocityJoystick(float Val)
{
	if (Val != LastPlayerLinearVelocityJoystick.Z)
	{
		LinearVelocitySource = EFlareInputSource::Joystick;
		EnablePilot(false);
		LastPlayerLinearVelocityJoystick.Z = Val;
		PlayerManualLinearVelocity.Z = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocityKeyboard(float Val)
{
	if (Val != LastPlayerAngularRollKeyboard)
	{
		EnablePilot(false);
		LastPlayerAngularRollKeyboard = Val;
		PlayerManualAngularVelocity.X = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocityJoystick(float Val)
{
	if (Val != LastPlayerAngularRollJoystick)
	{
		EnablePilot(false);
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

		if ((PlayerManualLockDirection && !ExternalCamera) && ! Spacecraft->GetLinearVelocity().IsNearlyZero())
		{
			PlayerForwardVelocity =  PlayerManualLockDirectionVector * FMath::Abs(PlayerManualVelocityCommand) * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
		}
		else
		{
			FVector LocalPlayerForwardVelocity = PlayerManualVelocityCommand * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() * FVector(1, 0, 0);
			PlayerForwardVelocity = Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerForwardVelocity);
		}

		FVector LocalPlayerLateralLinearVelocity = FVector(0, PlayerManualLinearVelocity.Y, PlayerManualLinearVelocity.Z);
		FVector FinalLinearVelocity = PlayerForwardVelocity + Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerLateralLinearVelocity);
				
		// Check if we should apply anticollision to the player ship ?
		if (Spacecraft->IsPlayerShip())
		{
			UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
			if (MyGameSettings->UseAnticollision || Spacecraft->GetPC()->GetMenuManager()->IsUIOpen() || Spacecraft->GetWeaponsSystem()->IsInFireDirector())
			{
				PilotHelper::AnticollisionConfig IgnoreConfig;

				if(Spacecraft->GetIsManualDocking())
				{
					if(Spacecraft->GetVelocity().SizeSquared() < 2000 * 2000)
					{
						IgnoreConfig.IgnoreAllStations = true;
					}
					else
					{
						FText DockInfo;
						AFlareSpacecraft* DockSpacecraft = NULL;
						FFlareDockingParameters DockParameters;
						Spacecraft->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);
						IgnoreConfig.SpacecraftToIgnore = DockSpacecraft;
						IgnoreConfig.SpeedCorrectionOnly = true;
					}
				}

				FinalLinearVelocity = PilotHelper::AnticollisionCorrection(Spacecraft, FinalLinearVelocity, Spacecraft->GetPreferedAnticollisionTime(), IgnoreConfig, 0.f);
			}
		}

		return FinalLinearVelocity;
	}
}

void UFlareSpacecraftStateManager::OnStatusChanged()
{
	// Maybe set to manual, in this case, forgot the old command
	FVector FrontVector = Spacecraft->Airframe-> GetComponentTransform().TransformVector(FVector(1, 0, 0));
	float ForwardVelocity = FVector::DotProduct(Spacecraft->GetLinearVelocity(), FrontVector);
	PlayerManualVelocityCommand = ForwardVelocity / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
}

FVector UFlareSpacecraftStateManager::GetAngularTargetVelocity() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->GetAngularTargetVelocity();
	}
	else if (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET)
	{
		FVector LookAtDirection = FireDirectorLookRotation.GetForwardVector();
		FVector GlobalYawPitchTarget = Spacecraft->GetNavigationSystem()->GetAngularVelocityToAlignAxis(FVector(1.f, 0.f, 0.f), LookAtDirection, FVector::ZeroVector, 0.f);
		FVector LocalTarget = Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(GlobalYawPitchTarget);
		LocalTarget.X = PlayerManualAngularVelocity.X;
		return Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalTarget);
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
		if (Spacecraft->IsPlayerShip() && Spacecraft->GetIsManualDocking())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool UFlareSpacecraftStateManager::IsWantFire() const
{
	bool PlayerWantsToFire = (PlayerLeftMousePressed || PlayerFiring) && !Spacecraft->GetPC()->GetMenuManager()->IsUIOpen();

	if (Spacecraft->GetWeaponsSystem()->IsInFireDirector())
	{
		return PlayerWantsToFire;
	}
	else if (IsPiloted)
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
			return PlayerWantsToFire;
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

float UFlareSpacecraftStateManager::GetCombatZoomAlpha() const
{
	return FMath::InterpEaseInOut(0.0f, 1.0f, CombatZoomTimer / CombatZoomDuration, 3.0f);
}

void UFlareSpacecraftStateManager::ResetExternalCamera()
{
	ExternalCameraPitch = 0;
	ExternalCameraYaw = 0;
	ExternalCameraPitchTarget = 0;
	ExternalCameraPitchSpeed = 0;
	ExternalCameraYawTarget = 0;
	ExternalCameraYawSpeed = 0;
	// TODO Don't copy SpacecraftPawn
	ExternalCameraDistance = 2.5 * Spacecraft->GetMeshScale();
	ExternalCameraDistanceTarget = ExternalCameraDistance;
}

void UFlareSpacecraftStateManager::OnCollision()
{
	PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
}
