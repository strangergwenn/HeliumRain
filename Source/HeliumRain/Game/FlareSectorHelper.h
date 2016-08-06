#pragma once
#include "../Economy/FlareResource.h"
#include "../Game/FlareTradeRoute.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

struct SectorHelper
{
	struct FlareTradeRequest
	{
		FFlareResourceDescription* Resource;
		EFlareTradeRouteOperation::Type Operation;
		int32 MaxQuantity;
		UFlareSimulatedSpacecraft *Client;
	};

	static UFlareSimulatedSpacecraft*  FindTradeStation(FlareTradeRequest Request);

	static int32 Trade(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity);
	//static bool CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius);

	//static FVector AnticollisionCorrection(AFlareSpacecraft* Ship, FVector InitialVelocity, AFlareSpacecraft* SpacecraftToIgnore = NULL);

	//static AFlareSpacecraft* GetBestTarget(AFlareSpacecraft* Ship, struct TargetPreferences Preferences);

	/** Return true if the ship is dangerous */
	//static bool IsShipDangerous(AFlareSpacecraft* ShipCandidate);

private:


	//static void CheckRelativeDangerosity(AActor* CandidateActor, FVector CurrentLocation, float CurrentSize, UStaticMeshComponent* StaticMeshComponent, FVector CurrentVelocity, AActor** MostDangerousCandidateActor, FVector*MostDangerousLocation, float* MostDangerousHitTime, float* MostDangerousInterCollisionTravelTime);

};
