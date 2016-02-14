
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
	, AngularInputDeadRatio(0.008)
	, MouseSensitivity(0.1)
	, MouseSensitivityPower(1)
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
	PlayerManualVelocityCommand = FMath::Clamp(ForwardVelocity / ParentSpacecraft->GetNavigationSystem()->GetLinearMaxVelocity(), 0.0f, 1.0f);

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

	/*float VelocityReference = PlayerManualVelocityCommand;
	if(!IsChangingVelocity)
	{
		VelocityReference = Spacecraft->GetLinearVelocity().Size() / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
	}*/

	if( FMath::IsNearlyZero(PlayerManualLinearVelocity.Z))
	{
		PlayerManualVelocityCommand = Spacecraft->GetLinearVelocity().Size() / Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
		FLOGV("Spacecraft->GetLinearVelocity() %s", *Spacecraft->GetLinearVelocity().ToString())
		FLOGV("Spacecraft->GetLinearVelocity().Size() %f", Spacecraft->GetLinearVelocity().Size())
		FLOGV("Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() %f", Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity())
		FLOGV("PlayerManualVelocityCommand %f", PlayerManualVelocityCommand)

	}
	else
	{
		if (PlayerManualLinearVelocity.Z < 0)
		{
			//PlayerManualVelocityCommand = FMath::Clamp(VelocityReference - DeltaSeconds * 1, 0.0f, 1.0f);
			PlayerManualVelocityCommand = 0.0;
		}
		else if (PlayerManualLinearVelocity.Z > 0)
		{
			//PlayerManualVelocityCommand = FMath::Clamp(VelocityReference + DeltaSeconds * 1, 0.0f, 1.0f);
			PlayerManualVelocityCommand = 1.0;
		}
	}


	if (!IsPiloted && (PlayerManualOrbitalBoost || !PlayerManualLinearVelocity.IsZero()))
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

				// Compensation curve = 1 + (input-1)/(1-AngularInputDeadRatio)
				float CompensatedDistance = FMath::Clamp(1. + (DistanceToCenter - 1. ) / (1. - AngularInputDeadRatio) , 0., 1.);
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
			float DistanceToCenter = FMath::Sqrt(FMath::Square(PlayerMouseOffset.X) + FMath::Square(PlayerMouseOffset.Y));

			// Compensation curve = 1 + (input-1)/(1-AngularInputDeadRatio)
			float CompensatedDistance = FMath::Clamp(1. + (DistanceToCenter - 1. ) / (1. - AngularInputDeadRatio) , 0., 1.);
			float Angle = FMath::Atan2(PlayerMouseOffset.Y, PlayerMouseOffset.X);

			PlayerManualAngularVelocity.Z = CompensatedDistance * FMath::Cos(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
			PlayerManualAngularVelocity.Y = CompensatedDistance * FMath::Sin(Angle) * Spacecraft->GetNavigationSystem()->GetAngularMaxVelocity();
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
			/*case EFlareWeaponGroupType::WG_GUN:
				// Camera to bullet direction

				float AmmoVelocity = Spacecraft->GetWeaponsSystem()->GetActiveWeaponGroup()->Description->WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
				FRotator ShipAttitude = Spacecraft->GetActorRotation();
				FVector ShipVelocity = 100.f * Spacecraft->GetLinearVelocity();

				// Bullet velocity
				FVector BulletVelocity = ShipAttitude.Vector();
				BulletVelocity.Normalize();
				BulletVelocity *= 100.f * AmmoVelocity;
				
				FVector BulletDirection = (ShipVelocity + BulletVelocity).GetUnsafeNormal();
				FVector LocalBulletDirection = Spacecraft->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector(BulletDirection);
				
				float Pitch = FMath::RadiansToDegrees(FMath::Asin(LocalBulletDirection.Z));
				float Yaw = FMath::RadiansToDegrees(FMath::Asin(LocalBulletDirection.Y));

				if (Pitch > 30)
				{
					Pitch = 30;
				}
				else if (Pitch < -30)
				{
					Pitch = -30;
				}

				if (Yaw > 30)
				{
					Yaw = 30;
				}
				else if (Yaw < -30)
				{
					Yaw = -30;
				}
				InternalCameraYawTarget = Yaw;
				InternalCameraPitchTarget = Pitch;

			break;*/
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
				float X = FMath::Sign(Val.X) * FMath::Pow(FMath::Abs(Val.X), MouseSensitivityPower) * MouseSensitivity;
				float Y = -FMath::Sign(Val.Y) * FMath::Pow(FMath::Abs(Val.Y), MouseSensitivityPower) * MouseSensitivity;

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

void UFlareSpacecraftStateManager::SetPlayerOrbitalBoost(bool Val)
{
	PlayerManualOrbitalBoost = Val;
}

void UFlareSpacecraftStateManager::SetPlayerXLinearVelocity(float Val)
{
	PlayerManualLinearVelocity.X = Val;
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocity(float Val)
{
	//PlayerManualLinearVelocity.Y = Val;
	PlayerManualAngularVelocity.X = -Val;
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocity(float Val)
{
	PlayerManualLinearVelocity.Z = Val;
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocity(float Val)
{
	//PlayerManualAngularVelocity.X = Val;
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
				/*FVector LocalPlayerManualLinearVelocity = PlayerManualLinearVelocity;
				// Manual orbital boost
				if (PlayerManualOrbitalBoost)
				{
					LocalPlayerManualLinearVelocity = Spacecraft->GetNavigationSystem()->GetLinearMaxBoostingVelocity() * FVector(1, 0, 0);
				}

				// Add velocity command to current velocity
				return Spacecraft->GetLinearVelocity() + Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerManualLinearVelocity);
				*/

				if(PlayerManualOrbitalBoost && ! Spacecraft->GetLinearVelocity().IsNearlyZero())
				{

					return  Spacecraft->GetLinearVelocity().GetUnsafeNormal() * PlayerManualVelocityCommand * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity();
				}
				else
				{
					FVector LocalPlayerManualLinearVelocity = PlayerManualVelocityCommand * Spacecraft->GetNavigationSystem()->GetLinearMaxVelocity() * FVector(1, 0, 0);

					return Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerManualLinearVelocity);
				}
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
		//return PlayerManualOrbitalBoost;
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

