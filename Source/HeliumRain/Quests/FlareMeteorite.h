#pragma once

#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "FlareMeteorite.generated.h"


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareMeteorite : public AActor
{
public:

   GENERATED_UCLASS_BODY()

public:

   /*----------------------------------------------------
	   Public methods
   ----------------------------------------------------*/

   virtual void BeginPlay() override;

   virtual void Tick(float DeltaSeconds) override;


   /** Properties setup */
   virtual void Load(const FFlareMeteoriteSave& Data);

   /** Save the asteroid to a save file */
   virtual FFlareMeteoriteSave* Save();

   /** Set as paused */
   virtual void SetPause(bool Paused);

   /** Setup the meteorite mesh */
  void SetupMeteoriteMesh();


public:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

   // Mesh
   UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
   UDestructibleComponent*                       Meteorite;


protected:

   // Data
   FFlareMeteoriteSave                           MeteoriteData;
   bool                                          Paused;


public:

   /*----------------------------------------------------
	   Getters
   ----------------------------------------------------*/

   UDestructibleComponent *GetAsteroidComponent()
   {
	   return Meteorite;
   }

};

