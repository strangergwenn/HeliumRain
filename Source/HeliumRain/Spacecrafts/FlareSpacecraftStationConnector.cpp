
#include "FlareSpacecraftStationConnector.h"
#include "../Flare.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftStationConnector::UFlareSpacecraftStationConnector(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

FVector UFlareSpacecraftStationConnector::GetDockLocation() const
{
	FVector DockLocation;
	FRotator DockRotation;

	GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

	return DockLocation;
}

FRotator UFlareSpacecraftStationConnector::GetDockRotation() const
{
	FVector DockLocation;
	FRotator DockRotation;

	GetSocketWorldLocationAndRotation(FName("dock"), DockLocation, DockRotation);

	return DockRotation;
}
