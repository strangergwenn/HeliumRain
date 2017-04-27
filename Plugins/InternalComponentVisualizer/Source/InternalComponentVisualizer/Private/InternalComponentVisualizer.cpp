#include "InternalComponentVisualizer.h"
#include "InternalComponentVisualizerPCH.h"
#include "HeliumRain/Spacecrafts/FlareInternalComponent.h"
#include "UnrealEd.h"
#include "ComponentVisualizer.h"

/*----------------------------------------------------
	Visualizer
----------------------------------------------------*/
class FFlareInternalComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization( const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI ) override
	{
		if (View->Family->EngineShowFlags.LightRadius)
		{
			const UFlareInternalComponent* InternalComponent = Cast<const UFlareInternalComponent>(Component);
			if (InternalComponent != NULL)
			{
				FTransform TM = InternalComponent->ComponentToWorld;
				TM.RemoveScaling();

				// Draw component radius
				DrawWireSphereAutoSides(PDI, TM, FColor(255, 222, 200), InternalComponent->Radius * 100, SDPG_World);
			}
		}

	}
};

void InternalComponentVisualizerImpl::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Startup InternalComponentVisualizer module"));
	if (GUnrealEd != NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("Register visualizer"));
		GUnrealEd->RegisterComponentVisualizer(UFlareInternalComponent::StaticClass()->GetFName(), MakeShareable(new FFlareInternalComponentVisualizer ));
	}
}

void InternalComponentVisualizerImpl::ShutdownModule()
{
}

IMPLEMENT_MODULE(InternalComponentVisualizerImpl, Module)
