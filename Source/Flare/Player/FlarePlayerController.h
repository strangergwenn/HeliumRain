#pragma once

#include "../Flare.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSaveGame.h"
#include "FlareMenuPawn.h"
#include "../UI/Components/FlareNotifier.h"
#include "FlarePlayerController.generated.h"


class AFlareMenuManager;
class AFlareHUD;

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

	/** Update the sound state using fading */
	virtual void UpdateSound(UAudioComponent* SoundComp, float VolumeDelta, float& CurrentVolume);

	/** Activate or deactivate the exterbal camera */
	virtual void SetExternalCamera(bool NewState);

	/** Fly this ship */
	virtual void FlyShip(AFlareSpacecraft* Ship, bool PossessNow = true);

	/** The world is ending. Literally. */
	virtual void PrepareForExit();

	virtual void SetWorldPause(bool Pause);


	/*----------------------------------------------------
		Data management
	----------------------------------------------------*/

	/** Load the company info */
	virtual void SetCompanyDescription(const FFlareCompanyDescription& SaveCompanyData);

	/** Load the player from a save file */
	virtual void Load(const FFlarePlayerSave& SavePlayerData);

	/** Get the ship pawn from the game */
	virtual void OnLoadComplete();

	/** Save the player to a save file */
	virtual void Save(FFlarePlayerSave& SavePlayerData, FFlareCompanyDescription& SaveCompanyData);

	/** Set the player's company */
	virtual void SetCompany(UFlareCompany* NewCompany);
	
	/** Call a sector is activated */
	virtual void OnSectorActivated();

	/** Call a sector is deactivated */
	virtual void OnSectorDeactivated();

	/*----------------------------------------------------
		Menus
	----------------------------------------------------*/

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, EFlareNotification::Type Type = EFlareNotification::NT_General, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, void* TargetInfo = NULL);

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

	/** Reset the mouse to the center of the screen */
	void ResetMousePosition();

	/** Signal that we are selecting weapons */
	void SetSelectingWeapon();

	/** id we select a weapon recently ? */
	bool IsSelectingWeapon() const;


	/*----------------------------------------------------
		Objectives
	----------------------------------------------------*/

	/** Start a new objective */
	void StartObjective(FText Name, FText Info);

	/** Set the current target */
	void SetObjectiveTarget(AActor* Actor);

	/** Set the current target */
	void SetObjectiveTarget(FVector Location);

	/** Set the progress from 0 to 1 */
	void SetObjectiveProgress(float Ratio);

	/** Finalize the objective */
	void CompleteObjective();

	/** Check if there is an objective yet */
	bool HasObjective() const;

	/** Get the raw objective data */
	const FFlarePlayerObjective* GetCurrentObjective() const;

	/** Get the objective location */
	FVector GetObjectiveLocation() const;


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Set the color of engine exhausts */
	inline void SetBasePaintColorIndex(int32 Index);

	/** Set the color of ship paint */
	inline void SetPaintColorIndex(int32 Index);

	/** Set the color of ship overlays */
	inline void SetOverlayColorIndex(int32 Index);

	/** Set the color of ship lights */
	inline void SetLightColorIndex(int32 Index);

	/** Set the pattern index for ship paint */
	inline void SetPatternIndex(int32 Index);


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

	/** Toggle the pilot mode */
	virtual void TogglePilot();

	/** Toggle the HUD's presence */
	virtual void ToggleHUD();

	/** Quick switch to another ship */
	virtual void QuickSwitch();

	/** Move hidden cursor */
	virtual void MouseInputX(float Val);

	/** Move hidden cursor */
	virtual void MouseInputY(float Val);
	
	/** Test method 1 */
	virtual void Test1();

	/** Test method 2 */
	virtual void Test2();


	/*----------------------------------------------------
		Wheel menu
	----------------------------------------------------*/

	/** Open the wheel menu */
	virtual void WheelPressed();

	/** Close the wheel menu */
	virtual void WheelReleased();

	/** Align to the ship's speed */
	void AlignToSpeed();

	/** Align to the ship's reverse speed */
	void AlignToReverse();

	/** Brake */
	void Brake();

	/** Get the nearest spacecraft */
	AFlareSpacecraft* GetNearestSpacecraft(bool OnScreenRequired = false);

	/** Match speed with nearest spacecraft */
	void MatchSpeedWithNearestSpacecraft();

	/** Find the nearest spacecraft */
	void LookAtNearestSpacecraft();

	/** Open the upgrade menu */
	void UpgradeShip();

	/** Undock */
	void UndockShip();


protected:

	/*----------------------------------------------------
		Sound data
	----------------------------------------------------*/

	// Engine sound node
	UPROPERTY()
	UAudioComponent*                         EngineSound;

	// RCS sound node
	UPROPERTY()
	UAudioComponent*                         RCSSound;

	// Power sound node
	UPROPERTY()
	UAudioComponent*                         PowerSound;

	/** Sound for menu openings */
	UPROPERTY()
	USoundCue*                               OnSound;

	/** Sound for menu closings */
	UPROPERTY()
	USoundCue*                               OffSound;

	// Sound data
	float                                    EngineSoundFadeSpeed;
	float                                    RCSSoundFadeSpeed;
	float                                    PowerSoundFadeSpeed;
	float                                    EngineSoundVolume;
	float                                    RCSSoundVolume;
	float                                    PowerSoundVolume;


	/*----------------------------------------------------
		Gameplay data
	----------------------------------------------------*/

	/** Dust effect template */
	UPROPERTY()
	UParticleSystem*                         DustEffectTemplate;

	/** Dust effect */
	UPROPERTY()
	UParticleSystemComponent*                DustEffect;

	/** Player save structure */
	UPROPERTY()
	FFlarePlayerSave                         PlayerData;

	/** Company data */
	UPROPERTY()
	FFlareCompanyDescription                 CompanyData;

	/** Player pawn used inside menus */
	UPROPERTY()
	AFlareMenuPawn*                          MenuPawn;

	/** Currently flown ship */
	UPROPERTY()
	AFlareSpacecraft*                        ShipPawn;

	/** Player owned company */
	UPROPERTY()
	UFlareCompany*                           Company;

	/** Menu management */
	UPROPERTY()
	AFlareMenuManager*                       MenuManager;

	/** Objective */
	UPROPERTY()
	FFlarePlayerObjective                    CurrentObjective;
	
	// Various gameplay data
	int32                                    QuickSwitchNextOffset;
	float                                    WeaponSwitchTime;
	float                                    TimeSinceWeaponSwitch;


public:

	/*----------------------------------------------------
		Getters for game classes
	----------------------------------------------------*/

	inline UFlareCompany* GetCompany() const
	{
		return Company;
	}

	inline const FFlareCompanyDescription* GetCompanyDescription() const
	{
		return &CompanyData;
	}

	inline AFlareMenuPawn* GetMenuPawn() const
	{
		return MenuPawn;
	}

	inline AFlareSpacecraft* GetShipPawn() const
	{
		return ShipPawn;
	}

	inline AFlareGame* GetGame() const
	{
		return Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
	}

	inline AFlareMenuManager* GetMenuManager() const
	{
		return MenuManager;
	}

	inline AFlareHUD* GetNavHUD() const
	{
		return Cast<AFlareHUD>(GetHUD());
	}

};

