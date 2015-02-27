
#include "../Flare.h"
#include "FlareRCS.h"
#include "FlareShip.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareRCS::UFlareRCS(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

bool UFlareRCS::CanMoveVertical()
{
	for (auto Iter = Capabilities.CreateIterator(); Iter; Iter++)
	{
		if (*Iter == ERCSCapability::StrafeUp || *Iter == ERCSCapability::StrafeDown)
		{
			return true;
		}
	}
	return false;
}

bool UFlareRCS::CanMoveHorizontal()
{
	for (auto Iter = Capabilities.CreateIterator(); Iter; Iter++)
	{
		if (*Iter == ERCSCapability::StrafeRight || *Iter == ERCSCapability::StrafeLeft)
		{
			return true;
		}
	}
	return false;
}
/*
void UFlareRCS::UpdateAlpha(float DeltaTime)
{
	// Setup data
	ExhaustAlpha = 0;
	float ThrustCapabilities = 0;
	float TSquared = FMath::Square(DeltaTime);
	AFlareShip* OwnerShip = Cast<AFlareShip>(Ship);

	// Add exhaust strength for every activated capability
	if (OwnerShip)
	{
		for (auto Iter = Capabilities.CreateIterator(); Iter; Iter++)
		{
			switch (*Iter)
			{
				case ERCSCapability::Thrust:      ExhaustAlpha += OwnerShip->GetAttitudeCommandThrust() / TSquared;      ThrustCapabilities++;  break;
				case ERCSCapability::Brake:	      ExhaustAlpha -= OwnerShip->GetAttitudeCommandThrust() / TSquared;      ThrustCapabilities++;  break;

				case ERCSCapability::StrafeRight: ExhaustAlpha += OwnerShip->GetAttitudeCommandHorizontal() / TSquared;  break;
				case ERCSCapability::StrafeLeft:  ExhaustAlpha -= OwnerShip->GetAttitudeCommandHorizontal() / TSquared;  break;

				case ERCSCapability::StrafeUp:    ExhaustAlpha += OwnerShip->GetAttitudeCommandVertical() / TSquared;    break;
				case ERCSCapability::StrafeDown:  ExhaustAlpha -= OwnerShip->GetAttitudeCommandVertical() / TSquared;    break;

				case ERCSCapability::TurnUp:      ExhaustAlpha += OwnerShip->GetAttitudeCommandPitch() / TSquared;       break;
				case ERCSCapability::TurnDown:    ExhaustAlpha -= OwnerShip->GetAttitudeCommandPitch() / TSquared;       break;

				case ERCSCapability::TurnRight:   ExhaustAlpha += OwnerShip->GetAttitudeCommandYaw() / TSquared;         break;
				case ERCSCapability::TurnLeft:    ExhaustAlpha -= OwnerShip->GetAttitudeCommandYaw() / TSquared;         break;

				case ERCSCapability::RollRight:   ExhaustAlpha += OwnerShip->GetAttitudeCommandRoll() / TSquared;        break;
				case ERCSCapability::RollLeft:    ExhaustAlpha -= OwnerShip->GetAttitudeCommandRoll() / TSquared;        break;

				default: break;
			}
		}

		// Thrust will always saturate commands to look better, but not rotations
		ExhaustAlpha /= (Capabilities.Num() - ThrustCapabilities);
	}
}*/
