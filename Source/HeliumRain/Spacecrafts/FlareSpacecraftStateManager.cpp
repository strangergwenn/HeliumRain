
#include "../Flare.h"

#include "FlareSpacecraftStateManager.h"
#include "../Player/FlareCockpitManager.h"
#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"
#include "../Player/FlareHUD.h"
#include "FlareSpacecraft.h"


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
	PlayerMousePosition = FVector2D(0,0);
	PlayerMouseOffset = FVector2D(0,0);
	ExternalCamera = true;

	PlayerManualLinearVelocity = FVector::ZeroVector;
	PlayerManualAngularVelocity = FVector::ZeroVector;
	float ForwardVelocity = FVector::DotProduct(ParentSpacecraft->GetLinearVelocity(), FVector(1, 0, 0));
	PlayerManualVelocityCommand = ForwardVelocity;
	PlayerManualVelocityCommandActive = true;

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

	EFlareWeaponGroupType::Type CurrentWeaponType = Spacecraft->GetWeaponsSystem()->GetActiveWeaponType();

	if (Spacecraft->GetDamageSystem()->IsAlive() && IsPiloted) // Do not tick the pilot if a player has disable the pilot
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

	// Player inputs
	if (LastWeaponType == EFlareWeaponGroupType::WG_NONE && (CurrentWeaponType == EFlareWeaponGroupType::WG_GUN || CurrentWeaponType == EFlareWeaponGroupType::WG_BOMB ))
	{
		// Axis input mode start, reset mouse offset
		SetPlayerMouseOffset(FVector2D(0,0), false);
	}
	LastWeaponType = CurrentWeaponType;

	if(Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() == 0)
	{
		// Stations can't move
		PlayerManualVelocityCommand = 0;
	}
	else if( FMath::IsNearlyZero(PlayerManualLinearVelocity.X))
	{
		// Keep current command
		if(PlayerManualVelocityCommandActive)
		{
			PlayerManualVelocityCommandActive = false;
			// Get current state
			PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
		}
	}
	else
	{
		PlayerManualVelocityCommandActive = true;

		PlayerManualVelocityCommand = FVector::DotProduct(Spacecraft->GetLinearVelocity(), Spacecraft->GetFrontVector()) / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();

		if (PlayerManualLinearVelocity.X < 0)
		{
			PlayerManualVelocityCommand -= 0.1;
		}
		else if (PlayerManualLinearVelocity.X > 0)
		{
			PlayerManualVelocityCommand += 0.1;
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

	if (!IsPiloted && !PlayerManualLinearVelocity.IsZero())
	{
		Spacecraft->ForceManual();
	}

	// Control
	switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
	{
		case EFlareWeaponGroupType::WG_TURRET:
		{
			if (PlayerLeftMousePressed && !ExternalCamera)
			{
				float DistanceToCenter = FMath::Sqrt(FMath::Square(PlayerMousePosition.X) + FMath::Square(PlayerMousePosition.Y));

				// Compensation curve
				float CompensatedDistance = FMath::Pow(FMath::Clamp((DistanceToCenter - AngularInputDeadRatio) , 0.f, 1.f), MouseSensitivityPower);
				float Angle = FMath::Atan2(PlayerMousePosition.Y, PlayerMousePosition.X);

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

		case EFlareWeaponGroupType::WG_NONE:
		case EFlareWeaponGroupType::WG_GUN:
		case EFlareWeaponGroupType::WG_BOMB:
		{
			AFlarePlayerController* PC = Spacecraft->GetPC();

			if (PC && !PC->GetNavHUD()->IsWheelMenuOpen())
			{
				float DistanceToCenter = FMath::Sqrt(FMath::Square(PlayerMouseOffset.X) + FMath::Square(PlayerMouseOffset.Y));

				// Compensation curve
				float CompensatedDistance = FMath::Pow(FMath::Clamp((DistanceToCenter - AngularInputDeadRatio), 0.f, 1.f), MouseSensitivityPower);
				float Angle = FMath::Atan2(PlayerMouseOffset.Y, PlayerMouseOffset.X);

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
	AFlarePlayerController* PC = Spacecraft->GetPC();
	if (PC)
	{
		if (PC->GetMenuManager()->IsMenuOpen())
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

		switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
		{
			case EFlareWeaponGroupType::WG_NONE:
			case EFlareWeaponGroupType::WG_TURRET:
			case EFlareWeaponGroupType::WG_BOMB:
			case EFlareWeaponGroupType::WG_GUN:
				// Camera to front
				InternalCameraYawTarget = 0;
				InternalCameraPitchTarget = 0;
				break;
		}

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
	FLOGV("EnablePilot %d", PilotEnabled);
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

void UFlareSpacecraftStateManager::SetPlayerMouseOffset(FVector2D Val, bool Relative)
{
	if (Relative)
	{
		// External camera : panning with mouse clicks
		if (ExternalCamera)
		{
			ExternalCameraYawTarget += Val.X * Spacecraft->GetCameraPanSpeed();
			ExternalCameraPitchTarget += Val.Y * Spacecraft->GetCameraPanSpeed();
			PlayerMouseOffset = FVector2D(0, 0);
		}
		
		// FP view
		else
		{
			AFlarePlayerController* PC = Cast<AFlarePlayerController>(Spacecraft->GetWorld()->GetFirstPlayerController());
			if (PC && !PC->GetNavHUD()->IsWheelMenuOpen())
			{
				float X = Val.X * MouseSensitivity;
				float Y = -Val.Y * MouseSensitivity;

				PlayerMouseOffset += FVector2D(X, Y);
				if (PlayerMouseOffset.Size() > 1)
				{
					PlayerMouseOffset /= PlayerMouseOffset.Size();
				}
			}
		}
	}
	else
	{
		PlayerMouseOffset = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerLeftMouse(bool Val)
{
	PlayerLeftMousePressed = Val;
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
	PlayerManualLinearVelocity.X = Val;
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocity(float Val)
{
	PlayerManualLinearVelocity.Y = Val;
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocity(float Val)
{
	PlayerManualLinearVelocity.Z = Val;
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocity(float Val)
{
	PlayerManualAngularVelocity.X = Val;
}

FVector UFlareSpacecraftStateManager::GetLinearTargetVelocity() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->GetLinearTargetVelocity();
	}
	else
	{
		switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
		{
			case EFlareWeaponGroupType::WG_TURRET:
			case EFlareWeaponGroupType::WG_NONE:
			case EFlareWeaponGroupType::WG_GUN:
			case EFlareWeaponGroupType::WG_BOMB:
			default:
			{
				FVector PlayerForwardVelocity;

				if(PlayerManualLockDirection && ! Spacecraft->GetLinearVelocity().IsNearlyZero())
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
	}
}

FVector UFlareSpacecraftStateManager::GetAngularTargetVelocity() const
{
	if (IsPiloted)
	{
		return Spacecraft->GetPilot()->GetAngularTargetVelocity();
	}
	else
	{
		switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
		{
			case EFlareWeaponGroupType::WG_BOMB:
			case EFlareWeaponGroupType::WG_TURRET:
			case EFlareWeaponGroupType::WG_NONE:
			case EFlareWeaponGroupType::WG_GUN:
			default:
			{
				return Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(PlayerManualAngularVelocity);
			}
		}
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
		return Spacecraft->GetDamageSystem()->GetTemperature() < Spacecraft->GetDamageSystem()->GetBurnTemperature();
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
		else
		{
			switch (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
			{
				case EFlareWeaponGroupType::WG_NONE:
				case EFlareWeaponGroupType::WG_TURRET:
					return false;
				case EFlareWeaponGroupType::WG_BOMB:
				case EFlareWeaponGroupType::WG_GUN:
					return PlayerLeftMousePressed;
				default:
					return false;
			}
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
		switch (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
		{
			case EFlareWeaponGroupType::WG_TURRET:
				return true;
			case EFlareWeaponGroupType::WG_NONE:
			case EFlareWeaponGroupType::WG_BOMB:
			case EFlareWeaponGroupType::WG_GUN:
				return false;
			default:
				return true;
		}
	}
}

bool UFlareSpacecraftStateManager::IsWantContextMenu() const
{
	if (ExternalCamera)
	{
		return true;
	}
	else
	{
		switch (Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
		{
			case EFlareWeaponGroupType::WG_TURRET:
				return true;

			case EFlareWeaponGroupType::WG_NONE:
			case EFlareWeaponGroupType::WG_BOMB:
			case EFlareWeaponGroupType::WG_GUN:
				return false;

			default:
				return true;
		}
	}
}

void UFlareSpacecraftStateManager::ResetExternalCamera()
{
	ExternalCameraPitch = 0;
	ExternalCameraYaw = 0;
	ExternalCameraPitchTarget = 0;
	ExternalCameraYawTarget = 0;
	// TODO Don't copy SpacecraftPawn
	ExternalCameraDistance = 4 * Spacecraft->GetMeshScale();
	ExternalCameraDistanceTarget = ExternalCameraDistance;
}

