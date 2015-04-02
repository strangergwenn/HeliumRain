
#include "../Flare.h"

#include "FlareShipPilot.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TimeUntilNextChange = 0;
	PilotTarget = FVector::ZeroVector;
	PilotTargetShip = NULL;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
	TimeUntilNextChange -= DeltaSeconds;
	if(TimeUntilNextChange <= 0)
	{

		TimeUntilNextChange = FMath::FRandRange(10, 40);
		PilotTarget = FVector(FMath::FRandRange(-4000, 4000), FMath::FRandRange(-1000, 1000), FMath::FRandRange(-1000, 1000));
		FLOGV("Pilot change destination to %s", *PilotTarget.ToString());
		FLOGV("New change in %fs", TimeUntilNextChange);
	}
	
	FLOGV("Pilot company: %s", *Ship->GetCompany()->GetCompanyName());
		

	LinearTargetVelocity = (PilotTarget - Ship->GetActorLocation()/100);

	if(Ship->GetTemperature() < 600)
	{
		UseOrbitalBoost = true;
	}

	if(Ship->GetTemperature() > 780)
	{
		UseOrbitalBoost = false;
	}

	// If has no target
		// find target
	

	// If has ship target 
		// Turn to target
		// Allow boost
		// If near
			// If turned
				// Fire
		// else
			// Go near
	
	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction


	AngularTargetVelocity = FVector::ZeroVector;
}

void UFlareShipPilot::Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareShip* OwnerShip)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;
	
	// Setup properties
	if (Data)
	{
		ShipPilotData = *Data;
	}
}

	/*----------------------------------------------------
		Pilot Output
	----------------------------------------------------*/

FVector UFlareShipPilot::GetLinearTargetVelocity() const
{
	return LinearTargetVelocity;
}

FVector UFlareShipPilot::GetAngularTargetVelocity() const
{
	return AngularTargetVelocity;
}

bool UFlareShipPilot::IsUseOrbitalBoost() const
{
	return UseOrbitalBoost;
}
