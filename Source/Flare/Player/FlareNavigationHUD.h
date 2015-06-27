#pragma once

#include "GameFramework/HUD.h"
#include "FlareHUD.h"
#include "../UI/HUD/FlareHUDMenu.h"
#include "../UI/HUD/FlareContextMenu.h"
#include "FlareNavigationHUD.generated.h"


/** Navigation HUD */
UCLASS()
class AFlareNavigationHUD : public AFlareHUD
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Setup
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void SetupMenu(struct FFlarePlayerSave& PlayerData) override;


	/*----------------------------------------------------
		HUD interaction
	----------------------------------------------------*/

	/** Toggle the HUD's presence */
	void ToggleHUD();

	/** Show the interface on the HUD (not the flight helpers) */
	void SetInteractive(bool Status);

	/** Set the wheel menu state */
	void SetWheelMenu(bool State);

	/** Move wheel menu cursor */
	void SetWheelCursorMove(FVector2D Move);

	/** Notify the HUD the played ship has changed */
	void OnTargetShipChanged();

	virtual void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		HUD drawing
	----------------------------------------------------*/

	virtual void DrawHUD() override;

protected:

	/** Draw speed indicator */
	void DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation, bool Invert);

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
		Internal methods
	----------------------------------------------------*/

	/** Decide if the HUD is displayed or not */
	void UpdateHUDVisibility();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Settings
	float                                   CombatMouseRadius;
	float                                   FocusDistance;
	int32                                   IconSize;
	FLinearColor                            HudColorNeutral;
	FLinearColor                            HudColorFriendly;
	FLinearColor                            HudColorEnemy;

	// General data
	bool                                    HUDVisible;
	bool                                    IsInteractive;
	bool                                    FoundTargetUnderMouse;
	FVector2D                               ViewportSize;

	// Designator content
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDBackReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDAimHelperIcon;
	UTexture2D*                             HUDNoseIcon;
	UTexture2D*                             HUDCombatMouseIcon;
	UTexture2D*                             HUDDesignatorCornerTexture;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDHealthIcon;
	UTexture2D*                             HUDWeaponIcon;

	// Font
	UFont*                                  HUDFont;
	
	// Slate menus
	TSharedPtr<SFlareHUDMenu>               HUDMenu;
	TSharedPtr<SFlareMouseMenu>             MouseMenu;
	TSharedPtr<SOverlay>                    ContextMenuContainer;
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

	bool IsMouseMenuOpen() const
	{
		return MouseMenu->IsOpen();
	}

	TSharedPtr<SFlareMouseMenu> GetMouseMenu() const
	{
		return MouseMenu;
	}

};
