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

	/** Identifier of the company */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

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

	/** Set the player's company */
	virtual void SetCompany(UFlareCompany* NewCompany);
	

	/*----------------------------------------------------
		Menus
	----------------------------------------------------*/

	/** Show a notification to the user */
	void Notify(FText Text, EFlareNotification::Type Type = EFlareNotification::NT_General, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, void* TargetInfo = NULL);

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
		Input
	----------------------------------------------------*/

	virtual void SetupInputComponent() override;

	/** Secondary input that is used whil in external camera */
	virtual void MousePositionInput(FVector2D Val);

	/** Toggle the external view */
	virtual void ToggleCamera();

	/** Toggle the dashboard */
	virtual void ToggleMenu();

	/** Toggle the combat mode */
	virtual void ToggleCombat();

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

	UPROPERTY()
	UFlareCompany* Company;

	bool   CombatMode;
	bool   ExternalCamera;


public:

	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	inline UFlareCompany* GetCompany() const
	{
		return Company;
	}

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

	inline FLinearColor GetOverlayColor() const
	{
		return GetGame()->GetCustomizationCatalog()->GetColor(Company->GetOverlayColorIndex());
	}

	inline FLinearColor GetColor() const
	{
		return GetGame()->GetCustomizationCatalog()->GetColor(Company->GetPaintColorIndex());
	}


};

