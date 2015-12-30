#pragma once

#include "GameFramework/HUD.h"
#include "FlareMenuManager.h"
#include "../UI/HUD/FlareHUDMenu.h"
#include "../UI/HUD/FlareContextMenu.h"
#include "FlareHUD.generated.h"


/** Navigation HUD */
UCLASS()
class FLARE_API AFlareHUD : public AHUD
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Setup
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	/** Setup the HUD */
	virtual void Setup(AFlareMenuManager* NewMenuManager);


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

	/** Decide if the HUD is displayed or not */
	void UpdateHUDVisibility();

	virtual void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		HUD drawing
	----------------------------------------------------*/

	virtual void DrawHUD() override;

protected:

	/** Format a distance in meter */
	FString FormatDistance(float Distance);

	/** Draw speed indicator */
	void DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation, bool Invert);

	/** Draw a search arrow */
	void DrawSearchArrow(FVector TargetLocation, FLinearColor Color, float MaxDistance = 10000000);

	/** Draw a designator block around a spacecraft */
	bool DrawHUDDesignator(AFlareSpacecraft*Spacecraft);

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

	/** Print a text with a shadow */
	void DrawTextShaded(FString Text, FVector2D Position, FLinearColor Color = FLinearColor::White);

	/** Is this position inside the viewport + border */
	bool IsInScreen(FVector2D ScreenPosition) const;

	/** Get the appropriate hostility color */
	FLinearColor GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraftPawn* Target);


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
	FLinearColor                            HudColorObjective;

	// General data
	AFlareMenuManager*                      MenuManager;
	bool                                    HUDVisible;
	bool                                    IsInteractive;
	bool                                    FoundTargetUnderMouse;
	FVector2D                               ViewportSize;

	// Designator content
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDBackReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDBombAimIcon;
	UTexture2D*                             HUDBombMarker;
	UTexture2D*                             HUDAimHelperIcon;
	UTexture2D*                             HUDNoseIcon;
	UTexture2D*                             HUDObjectiveIcon;
	UTexture2D*                             HUDCombatMouseIcon;
	UTexture2D*                             HUDDesignatorCornerTexture;
	UTexture2D*                             HUDDesignatorSelectionTexture;

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

	FVector2D GetViewportSize() const
	{
		return ViewportSize;
	}

};
