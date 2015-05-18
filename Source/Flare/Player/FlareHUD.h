#pragma once

#include "SlateBasics.h"
#include "GameFramework/HUD.h"
#include "../Spacecrafts/FlareSpacecraftPawn.h"
#include "../UI/Menus/FlareDashboard.h"
#include "../UI/Menus/FlareHUDMenu.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareStationMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Menus/FlareContextMenu.h"
#include "../UI/Widgets/FlareNotifier.h"
#include "FlareHUD.generated.h"


/** Main HUD class (container for HUD and menus) */
UCLASS()
class FLARE_API AFlareHUD : public AHUD
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Gameplay events
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void DrawHUD() override;


	/*----------------------------------------------------
		HUD library
	----------------------------------------------------*/

	/** Draw a designator block around a ship */
	bool DrawHUDDesignator(AFlareSpacecraftPawn* ShipBase);

	/** Draw a designator corner */
	void DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float IconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor);

	/** Draw a status block for the ship */
	void DrawHUDDesignatorStatus(FVector2D Position, float IconSize, AFlareSpacecraft* Ship);

	/** Draw a status icon */
	FVector2D DrawHUDDesignatorStatusIcon(FVector2D Position, float IconSize, float Health, UTexture2D* Texture);

	/** Draw an icon */
	void DrawHUDIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color, bool Center = false);

	/** Draw an icon */
	void DrawHUDIconRotated(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color, float Rotation);

	/** Get the appropriate hostility color */
	FLinearColor GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraftPawn* Target);


	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Get the current context menu position */
	FVector GetContextMenuPosition() const;

	/** Toggle the HUD's presence */
	void ToggleHUD();

	/** Decide if the HUD is displayed or not */
	void SetHUDVisibility(bool Visibility);

	/** Construct the Slate menu interface */
	virtual void SetupMenu(struct FFlarePlayerSave& PlayerData);

	/** Show a notification to the user */
	void Notify(FText Text, EFlareNotification::Type Type, EFlareMenu::Type TargetMenu, void* TargetInfo);

	/** Notify the HUD the played ship has changed */
	void OnTargetShipChanged();

	/** Open a menu asynchronously, from a target and user data */
	void OpenMenu(EFlareMenu::Type Target, void* Data = NULL);

	/** Close the current menu */
	void CloseMenu(bool HardClose = false);

	/** Show the interface on the HUD (not the flight helpers) */
	void SetInteractive(bool Status);


protected:

	/*----------------------------------------------------
		Menu commands
	----------------------------------------------------*/
	
	/** After a fading process has completed, proceed */
	virtual void ProcessFadeTarget();

	/** Open the main menu */
	virtual void OpenDashboard();

	/** Open the company menu */
	virtual void InspectCompany(UFlareCompany* Target);

	/** Show the config menu for a specific ship */
	virtual void InspectShip(IFlareSpacecraftInterface* Target = NULL, bool IsEditable = false);

	/** Show the config menu for a specific station */
	virtual void InspectStation(IFlareSpacecraftInterface* Target = NULL, bool IsEditable = false);

	/** Open the sector menu */
	virtual void OpenSector();

	/** Exit the menu */
	virtual void ExitMenu();


	/*----------------------------------------------------
		Menu management
	----------------------------------------------------*/

	/** Hide the menu */
	void ResetMenu();

	/** Fade from black */
	void FadeIn();

	/** Fade to black */
	void FadeOut();

	/** Set the menu pawn as the current pawn, or not */
	void SetMenuPawn(bool Status);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Settings
	float                                   CombatMouseRadius;
	bool                                    HUDVisible;

	// Fade-to-black system
	bool                                    MenuIsOpen;
	bool                                    IsInteractive;
	bool                                    FadeFromBlack;
	float                                   FadeDuration;
	float                                   FadeTimer;
	TSharedPtr<SBorder>                     Fader;

	// Menus
	TSharedPtr<SFlareHUDMenu>               HUDMenu;
	TSharedPtr<SOverlay>                    OverlayContainer;
	TSharedPtr<SFlareDashboard>             Dashboard;
	TSharedPtr<SFlareCompanyMenu>           CompanyMenu;
	TSharedPtr<SFlareShipMenu>              ShipMenu;
	TSharedPtr<SFlareStationMenu>           StationMenu;
	TSharedPtr<SFlareSectorMenu>            SectorMenu;
	TSharedPtr<SFlareNotifier>              Notifier;

	// Menu target data
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;
	void*                                   FadeTargetData;
	bool                                    FoundTargetUnderMouse;

	// Designator content
	FLinearColor                            HudColorNeutral;
	FLinearColor                            HudColorFriendly;
	FLinearColor                            HudColorEnemy;
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDAimHelperIcon;
	UTexture2D*                             HUDNoseIcon;
	UTexture2D*                             HUDCombatMouseIcon;
	UTexture2D*                             HUDDesignatorCornerTexture;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDRCSIcon;
	UTexture2D*                             HUDWeaponIcon;
	
	// Context menu
	TSharedPtr<SFlareContextMenu>           ContextMenu;
	FVector2D                               ContextMenuPosition;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	const FVector2D& GetContextMenuLocation() const
	{
		return ContextMenuPosition;
	}

	/*----------------------------------------------------
		Slate
	----------------------------------------------------*/

	/** Get a Slate icon from menu target */
	static const FSlateBrush* GetMenuIcon(EFlareMenu::Type MenuType);

	/** Start the loading screen */
	void ShowLoadingScreen();


};
