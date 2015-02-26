#pragma once

#include "../Flare.h"
#include "FlareHUD.h"
#include "FlareMenuPawn.h"
#include "../Game/FlareGame.h"
#include "FlarePlayerController.generated.h"


/** Game save data */
USTRUCT()
struct FFlarePlayerSave
{
	GENERATED_USTRUCT_BODY()
	
	/** UObject name of the currently possessed ship */
	UPROPERTY(EditAnywhere, Category = Save)
	FString CurrentShipName;
	
	/** Paint color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPaintColorIndex;

	/** Lights color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationLightColorIndex;

	/** Engine exhaust color index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationEngineColorIndex;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};


UCLASS(MinimalAPI)
class AFlarePlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
			Gameplay methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void PlayerTick(float DeltaTime) override;

	/** Activate or deactivate the exterbal camera */
	virtual void SetExternalCamera(bool NewState, bool Force = false);

	/** Fly this ship */
	virtual void FlyShip(AFlareShip* Ship);

	/** The world is ending. Literally. */
	virtual void PrepareForExit();


	/*----------------------------------------------------
		Data management
	----------------------------------------------------*/

	/** Load the player from a save file */
	virtual void Load(const FFlarePlayerSave& Data);

	/** Save the player to a save file */
	virtual void Save(FFlarePlayerSave& Data);

	/** Take control of our current ship and remove the old pawn */
	virtual void PossessCurrentShip();


	/*----------------------------------------------------
		Menus
	----------------------------------------------------*/

	/** Spawn the menu pawn and prepare the UI */
	virtual void SetupMenu();

	/** Entering the main menu */
	virtual void OnEnterMenu();

	/** Exiting the main menu */
	virtual void OnExitMenu();

	/** Check wether we are in the menu system */
	virtual bool IsInMenu();

	/** Get the position of the mouse on the screen */
	FVector2D GetMousePosition();


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Apply customization to a component's material */
	virtual void CustomizeModuleMaterial(UMaterialInstanceDynamic* Mat);

	/** Apply customization to a component's special effect material */
	virtual void CustomizeEffectMaterial(UMaterialInstanceDynamic* Mat);

	/** Set the paint overlay color */
	virtual void SetCustomizationPaintColorIndex(int32 Index);

	/** Set the light color for the ship's headlights */
	virtual void SetCustomizationLightColorIndex(int32 Index);

	/** Set the engine color for the engine's exhausts */
	virtual void SetCustomizationEngineColorIndex(int32 Index);

	/** Set the pattern index */
	virtual void SetCustomizationPatternIndex(int32 Index);

	/** Update the pawn to account for our customization */
	void UpdateShipCustomization();

	/** Normalize a color */
	FLinearColor NormalizeColor(FLinearColor Col) const;


	/*----------------------------------------------------
		Input
	----------------------------------------------------*/

	virtual void SetupInputComponent() override;

	/** Secondary input that is used whil in external camera */
	virtual void MousePositionInput(FVector2D Val);

	/** Toggle the external view */
	virtual void ToggleCamera();

	/** Toggle the dashboard */
	virtual void ToggleMenu();

	/** Test method 1 */
	virtual void Test1();

	/** Test method 2 */
	virtual void Test2();


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	UPROPERTY()
	FFlarePlayerSave PlayerData;

	UPROPERTY()
	AFlareMenuPawn* MenuPawn;

	UPROPERTY()
	AFlareShip* ShipPawn;

	bool   ExternalCamera;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/
	
	inline AFlareMenuPawn* GetMenuPawn() const
	{
		return MenuPawn;
	}

	inline AFlareShip* GetShipPawn() const
	{
		return ShipPawn;
	}

	inline AFlareGame* GetGame() const
	{
		return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	}

	inline FLinearColor GetLightColor() const
	{
		return GetGame()->GetCustomizationCatalog()->GetColor(PlayerData.CustomizationLightColorIndex);
	}

	inline FLinearColor GetColor() const
	{
		return GetGame()->GetCustomizationCatalog()->GetColor(PlayerData.CustomizationPaintColorIndex);
	}


};

