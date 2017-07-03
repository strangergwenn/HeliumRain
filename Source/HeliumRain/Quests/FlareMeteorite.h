#pragma once

#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "FlareMeteorite.generated.h"

class UFlareSimulatedSpacecraft;
class UFlareSector;
class AFlareSpacecraft;

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
   virtual void Load(FFlareMeteoriteSave* Data, UFlareSector* ParentSector);

   /** Save the asteroid to a save file */
   virtual FFlareMeteoriteSave* Save();

   /** Set as paused */
   virtual void SetPause(bool Paused);

   /** Setup the meteorite mesh */
  void SetupMeteoriteMesh();

  void ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource, FString DamageCauser);

  virtual void OnCollision(class AActor* Other, FVector HitLocation);

public:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

   // Mesh
   UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
   UDestructibleComponent*                       Meteorite;

   UFlareSector*                                 Parent;
   AFlareSpacecraft*                    Target;

protected:

   // Data
   FFlareMeteoriteSave*                          MeteoriteData;
   bool                                          Paused;


public:

   /*----------------------------------------------------
	   Getters
   ----------------------------------------------------*/

   bool IsBroken();


   UDestructibleComponent *GetMeteoriteComponent()
   {
	   return Meteorite;
   }

};

