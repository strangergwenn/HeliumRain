#pragma once

#include "GameFramework/HUD.h"
#include "FlareMenuManager.h"
#include "../UI/HUD/FlareHUDMenu.h"
#include "../UI/HUD/FlareContextMenu.h"
#include "FlareHUD.generated.h"


class SFlareHUDMenu;
class SFlareMouseMenu;

/** Target info */
USTRUCT()
struct FFlareScreenTarget
{
	GENERATED_USTRUCT_BODY()

	AFlareSpacecraft*      Spacecraft;

	float                  DistanceFromScreenCenter;

};


/** Navigation HUD */
UCLASS()
class HELIUMRAIN_API AFlareHUD : public AHUD
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

	/** Is the mouse menu open */
	bool IsWheelMenuOpen() const;

	/** Notify the HUD the played ship has changed */
	void OnTargetShipChanged();

	/** Decide if the HUD is displayed or not */
	void UpdateHUDVisibility();

	virtual void DrawHUD() override;

	virtual void Tick(float DeltaSeconds) override;


	/*----------------------------------------------------
		Cockpit
	----------------------------------------------------*/

	/** Canvas callback for the cockpit's HUD*/
	UFUNCTION()
	void DrawCockpitHUD(UCanvas* TargetCanvas, int32 Width, int32 Height);

	/** Canvas callback for the cockpit's instruments*/
	UFUNCTION()
	void DrawCockpitInstruments(UCanvas* TargetCanvas, int32 Width, int32 Height);

	/** Draw on the current canvas the ship's subsystem status */
	void DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip);

	/** Draw on the current canvas the ship's weapon or cargo status */
	void DrawCockpitEquipment(AFlareSpacecraft* PlayerShip);

	/** Draw on the current canvas the ship's target info */
	void DrawCockpitTarget(AFlareSpacecraft* PlayerShip);

	/** Draw a subsystem info line on a cockpit intrument */
	void DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position);

	/** Get the ship's current status as text */
	FText GetShipStatus(AFlareSpacecraft* PlayerShip) const;


	/*----------------------------------------------------
		HUD helpers
	----------------------------------------------------*/

	/** Get the temperature color, using custom threshold */
	static FLinearColor GetTemperatureColor(float Current, float Max);

	/** Get the health color */
	static FLinearColor GetHealthColor(float Current);


	/*----------------------------------------------------
		Internals
	----------------------------------------------------*/

protected:

	/** Should we draw the HUD ? */
	bool ShouldDrawHUD() const;

	/** Drawing back-end */
	void DrawHUDInternal();

	/** Format a distance in meter */
	FString FormatDistance(float Distance);

	/** Draw speed indicator */
	void DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed, FText Designation, bool Invert);

	/** Draw a search arrow */
	void DrawSearchArrow(FVector TargetLocation, FLinearColor Color, float MaxDistance = 10000000);

	/** Draw a designator block around a spacecraft */
	bool DrawHUDDesignator(AFlareSpacecraft*Spacecraft);

	/** Draw a designator corner */
	void DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float IconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Highlighted = false);

	/** Draw a status block for the ship */
	void DrawHUDDesignatorStatus(FVector2D Position, float IconSize, AFlareSpacecraft* Ship);

	/** Draw a status icon */
	FVector2D DrawHUDDesignatorStatusIcon(FVector2D Position, float IconSize, float Health, UTexture2D* Texture);

	/** Draw an icon */
	void DrawHUDIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, bool Center = false);

	/** Draw an icon */
	void DrawHUDIconRotated(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, float Rotation = 0);

	/** Print a text with a shadow */
	void FlareDrawText(FString Text, FVector2D Position, FLinearColor Color = FLinearColor::White, bool Center = true, bool Large = false);

	/** Draw a texture */
	void FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector);

	/** Is this position inside the viewport + border */
	bool IsInScreen(FVector2D ScreenPosition) const;

	/** Get the appropriate hostility color */
	FLinearColor GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraftPawn* Target);

	/** Convert a world location to screen-space */
	bool WorldToScreen(FVector World, FVector2D& Screen);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu reference
	UPROPERTY()
	AFlareMenuManager*                      MenuManager;

	// Settings
	float                                   CombatMouseRadius;
	float                                   FocusDistance;
	int32                                   IconSize;
	FLinearColor                            HudColorNeutral;
	FLinearColor                            HudColorFriendly;
	FLinearColor                            HudColorEnemy;
	FLinearColor                            HudColorObjective;
	
	// Spacecraft targets
	UPROPERTY()
	TArray<FFlareScreenTarget>              ScreenTargets;

	// General data
	bool                                    HUDVisible;
	bool                                    IsInteractive;
	bool                                    FoundTargetUnderMouse;
	bool                                    IsDrawingCockpit;
	FVector2D                               ViewportSize;

	// Current data
	FVector2D                               CurrentViewportSize;
	UCanvas*                                CurrentCanvas;

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
	UTexture2D*                             HUDDesignatorCornerSelectedTexture;
	UTexture2D*                             HUDDesignatorSelectionTexture;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDRCSIcon;
	UTexture2D*                             HUDHealthIcon;
	UTexture2D*                             HUDWeaponIcon;

	// Font
	UFont*                                  HUDFont;
	UFont*                                  HUDFontLarge;

	// Instruments
	FVector2D                               TopInstrument;
	FVector2D                               LeftInstrument;
	FVector2D                               RightInstrument;
	FVector2D                               InstrumentSize;
	FVector2D                               InstrumentLine;
	
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

	/** Get the spacecrafts shown on screen */
	inline TArray<FFlareScreenTarget>& GetCurrentTargets()
	{
		return ScreenTargets;
	}

	const FVector2D& GetContextMenuLocation() const
	{
		return ContextMenuPosition;
	}

	TSharedPtr<SFlareMouseMenu> GetMouseMenu() const
	{
		return MouseMenu;
	}

	FVector2D GetViewportSize() const
	{
		return ViewportSize;
	}

	UCanvas* GetCanvas() const
	{
		return Canvas;
	}

};
