
#include "../Flare.h"
#include "FlareInternalComponent.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareInternalComponent::UFlareInternalComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareInternalComponent::Initialize(const FFlareShipComponentSave* Data, UFlareCompany* Company, AFlareShipBase* OwnerShip, bool IsInMenu)
{
	Super::Initialize(Data, Company, OwnerShip, IsInMenu);
}

FFlareShipComponentSave* UFlareInternalComponent::Save()
{
	return Super::Save();
}

void UFlareInternalComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

/*----------------------------------------------------
	Visualizer
----------------------------------------------------*/
/*
void FFlareInternalComponentDebugVisualizer::DrawVisualization( const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI )
{
	if(View->Family->EngineShowFlags.LightRadius)
	{
		const UFlareInternalComponent* InternalComponent = Cast<const UFlareInternalComponent>(Component);
		if(InternalComponent != NULL)
		{
			FTransform TM = InternalComponent->ComponentToWorld;
			TM.RemoveScaling();

			// Draw light radius
			DrawWireSphereAutoSides(PDI, TM, FColor(200, 255, 255), InternalComponent->Radius, SDPG_World);
		}
	}

}*/