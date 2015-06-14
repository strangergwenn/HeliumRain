
#include "../Flare.h"

#include "FlareSpacecraftStateManager.h"
#include "FlareSpacecraft.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftStateManager::UFlareSpacecraftStateManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, AngularInputDeadRatio(0.0025)
{
	// Pilot
	IsPiloted = true;
	LastWeaponType = EFlareWeaponGroupType::WG_NONE;
}

void UFlareSpacecraftStateManager::Initialize(AFlareSpacecraft* ParentSpacecraft)
{
	Spacecraft = ParentSpacecraft;
	PlayerMousePosition = FVector2D(0,0);
	PlayerMouseOffset = FVector2D(0,0);

	PlayerManualLinearVelocity = FVector::ZeroVector;
	PlayerManualAngularVelocity = FVector::ZeroVector;
	ExternalCameraPitch = 0;
	ExternalCameraYaw = 0;
	// TODO Don't copy SpacecraftPawn
	ExternalCameraDistance = 4 * Spacecraft->GetMeshScale();
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
	}

	// Player inputs
	if(LastWeaponType == EFlareWeaponGroupType::WG_NONE
			&& (CurrentWeaponType == EFlareWeaponGroupType::WG_GUN || CurrentWeaponType == EFlareWeaponGroupType::WG_BOMB ))
	{
		// Axis input mode start, reset mouse offset
		SetPlayerMouseOffset(FVector2D(0,0), false);
	}
	LastWeaponType = CurrentWeaponType;


	// Control
	switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
	{
		case EFlareWeaponGroupType::WG_NONE:
		case EFlareWeaponGroupType::WG_TURRET:
		{
			if(PlayerLeftMousePressed)
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
		case EFlareWeaponGroupType::WG_BOMB:
		case EFlareWeaponGroupType::WG_GUN:
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
	UpdateCamera();
}


void UFlareSpacecraftStateManager::UpdateCamera()
{
	if(ExternalCamera)
	{
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
				// Camera to front
				Spacecraft->SetCameraPitch(0);
				Spacecraft->SetCameraYaw(0);
			break;
			case EFlareWeaponGroupType::WG_GUN:
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

				Spacecraft->SetCameraPitch(Pitch);
				Spacecraft->SetCameraYaw(Yaw);
			break;
		}
	}
	//TODO Camera smoothing

}

void UFlareSpacecraftStateManager::EnablePilot(bool PilotEnabled)
{
	FLOGV("EnablePilot %d", PilotEnabled);
	IsPiloted = PilotEnabled;
}


void UFlareSpacecraftStateManager::SetExternalCamera(bool NewState)
{
	ExternalCamera = NewState;

	// Put the camera at the right spot
	if (ExternalCamera)
	{
		Spacecraft->SetCameraLocalPosition(FVector::ZeroVector);
	}
	else
	{
		FVector CameraOffset = Spacecraft->WorldToLocal(Spacecraft->Airframe->GetSocketLocation(FName("Camera")) - Spacecraft->GetActorLocation());
		Spacecraft->SetCameraDistance(0);
		Spacecraft->SetCameraLocalPosition(CameraOffset);
	}
}

void UFlareSpacecraftStateManager::SetPlayerMousePosition(FVector2D Val)
{
	FLOGV("SetPlayerMousePosition=%s", *Val.ToString());
	PlayerMousePosition = Val;
}

void UFlareSpacecraftStateManager::SetPlayerMouseOffset(FVector2D Val, bool Relative)
{
	FLOGV("SetPlayerMouseOffset=%s relative=%d", *Val.ToString(), Relative);
	if (Relative)
	{
		if(ExternalCamera)
		{
			ExternalCameraYaw += Val.X * Spacecraft->GetCameraPanSpeed();
			ExternalCameraPitch += Val.Y * Spacecraft->GetCameraPanSpeed();
		}
		else
		{
			FLOGV("PlayerMouseOffset before=%s", *PlayerMouseOffset.ToString());
			float X = FMath::Sign(Val.X) * FMath::Pow(FMath::Abs(Val.X),1.3) * 0.05; // TODO Config sensibility
			float Y = - FMath::Sign(Val.Y) * FMath::Pow(FMath::Abs(Val.Y),1.3) * 0.05;

			PlayerMouseOffset += FVector2D(X,Y);
			FLOGV("PlayerMouseOffset after=%s", *PlayerMouseOffset.ToString());
			if(PlayerMouseOffset.Size() > 1)
			{
				PlayerMouseOffset /= PlayerMouseOffset.Size();
			}
			FLOGV("PlayerMouseOffset clamped=%s", *PlayerMouseOffset.ToString());
		}
	}
	else
	{
		PlayerMouseOffset = Val;
	}
}

void UFlareSpacecraftStateManager::SetPlayerLeftMouse(bool Val)
{
	FLOGV("SetPlayerLeftMouse=%d", Val);
	PlayerLeftMousePressed = Val;
}

void UFlareSpacecraftStateManager::ExternalCameraZoom(bool ZoomIn)
{
	FLOGV("ExternalCameraZoom=%d", ZoomIn);
	if (ExternalCamera)
	{
	// TODO don't duplicate with Spacecraft Pawn
	// Compute camera data
	float Scale = Spacecraft->GetMeshScale();
	float LimitNear = Scale * 1.5;
	float LimitFar = Scale * 4;
	float Offset = Scale * (ZoomIn ? -0.5 : 0.5);

	// Move camera
	ExternalCameraDistance = FMath::Clamp(ExternalCameraDistance + Offset, LimitNear, LimitFar);
	}
}

void UFlareSpacecraftStateManager::SetPlayerOrbitalBoost(bool Val)
{
	FLOGV("SetPlayerOrbitalBoost=%d", Val);
	PlayerManualOrbitalBoost = Val;
}

void UFlareSpacecraftStateManager::SetPlayerXLinearVelocity(float Val)
{
	FLOGV("SetPlayerXLinearVelocity=%f", Val);
	PlayerManualLinearVelocity.X = Val;
}

void UFlareSpacecraftStateManager::SetPlayerYLinearVelocity(float Val)
{
	FLOGV("SetPlayerYLinearVelocity=%f", Val);
	PlayerManualLinearVelocity.Y = Val;
}

void UFlareSpacecraftStateManager::SetPlayerZLinearVelocity(float Val)
{
	FLOGV("SetPlayerZLinearVelocity=%f", Val);
	PlayerManualLinearVelocity.Z = Val;
}

void UFlareSpacecraftStateManager::SetPlayerRollAngularVelocity(float Val)
{
	FLOGV("SetPlayerRollAngularVelocity=%f", Val);
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
		if (ExternalCamera)
		{
			return Spacecraft->GetLinearVelocity();
		}
		else
		{
			FVector LocalPlayerManualLinearVelocity = PlayerManualLinearVelocity;
			// Manual orbital boost
			if (PlayerManualOrbitalBoost)
			{
				LocalPlayerManualLinearVelocity = Spacecraft->GetNavigationSystem()->GetLinearMaxBoostingVelocity() * FVector(1, 0, 0);
			}

			// Add velocity command to current velocity
			return Spacecraft->GetLinearVelocity() + Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalPlayerManualLinearVelocity);
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
		if (ExternalCamera)
		{
			return FVector::ZeroVector;
		}
		else
		{
			return Spacecraft->Airframe->GetComponentToWorld().GetRotation().RotateVector(PlayerManualAngularVelocity);
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
		if (ExternalCamera)
		{
			return false;
		}
		else
		{
			return PlayerManualOrbitalBoost;
		}
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
			switch(Spacecraft->GetWeaponsSystem()->GetActiveWeaponType())
			{
				case EFlareWeaponGroupType::WG_NONE:
				case EFlareWeaponGroupType::WG_TURRET:
					return false;
				case EFlareWeaponGroupType::WG_BOMB:
				case EFlareWeaponGroupType::WG_GUN:
					return PlayerLeftMousePressed;
			}
		}
	}
}
