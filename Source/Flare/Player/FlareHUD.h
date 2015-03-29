#pragma once

#include "SlateBasics.h"
#include "GameFramework/HUD.h"
#include "../Ships/FlareShipBase.h"
#include "../UI/Menus/FlareDashboard.h"
#include "../UI/Menus/FlareHUDMenu.h"
#include "../UI/Menus/FlareCompanyMenu.h"
#include "../UI/Menus/FlareShipMenu.h"
#include "../UI/Menus/FlareStationMenu.h"
#include "../UI/Menus/FlareSectorMenu.h"
#include "../UI/Menus/FlareContextMenu.h"
#include "FlareHUD.generated.h"


/** Possible menu targets */
UENUM()
namespace EFlareMenu
{
	enum Type
	{
		MENU_None,
		MENU_Dashboard,
		MENU_Company,
		MENU_Ship,
		MENU_ShipConfig,
		MENU_Station,
		MENU_Undock,
		MENU_Sector,
		MENU_Orbit,
		MENU_Encyclopedia,
		MENU_Help,
		MENU_Settings,
		MENU_Quit,
		MENU_Exit
	};
}


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

	/** Draw a designator block around a ship */
	void DrawHUDDesignator(AFlareShipBase* ShipBase);

	/** Draw a designator corner */
	void DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float IconSize, FVector2D MainOffset, float Rotation);

	/** Draw a status block for the ship */
	void DrawHUDDesignatorStatus(FVector2D Position, float IconSize, AFlareShip* Ship);

	/** Draw a status icon */
	FVector2D DrawHUDDesignatorStatusIcon(FVector2D Position, float IconSize, float Health, UTexture2D* Texture);

	/** Draw an icon */
	void DrawHUDIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color, bool Center = false);


	/*----------------------------------------------------
		HUD interaction
	----------------------------------------------------*/

	/** Get the current context menu position */
	FVector GetContextMenuPosition() const;


	/*----------------------------------------------------
		Menu interaction
	----------------------------------------------------*/

	/** Construct the Slate menu interface */
	virtual void SetupMenu(struct FFlarePlayerSave& PlayerData);

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
	virtual void InspectShip(IFlareShipInterface* Target = NULL, bool IsEditable = false);

	/** Show the config menu for a specific station */
	virtual void InspectStation(IFlareStationInterface* Target = NULL, bool IsEditable = false);

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

	// Menu target data
	TEnumAsByte<EFlareMenu::Type>           FadeTarget;
	void*                                   FadeTargetData;

	// Designator content
	bool                                    FoundTargetUnderMouse;
	FLinearColor                            HudColor;
	UTexture2D*                             HUDDesignatorCornerTexture;
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDNoseIcon;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDRCSIcon;
	UTexture2D*                             HUDWeaponIcon;
		     
	// HUD materials
	UPROPERTY()UMaterial*                   HUDHelpersMaterialMaster;
	UPROPERTY()	UMaterialInstanceDynamic*   HUDHelpersMaterial;
	
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
