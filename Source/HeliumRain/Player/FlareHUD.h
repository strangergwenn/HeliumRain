#pragma once

#include "GameFramework/HUD.h"
#include "FlareMenuManager.h"
#include "../UI/HUD/FlareHUDMenu.h"
#include "../UI/HUD/FlareContextMenu.h"
#include "FlareHUD.generated.h"


class SFlareHUDMenu;
class SFlareMouseMenu;
class UFlareWeapon;


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
	void SetWheelMenu(bool State, bool EnableActionOnClose = true);

	/** Move wheel menu cursor */
	void SetWheelCursorMove(FVector2D Move);

	/** Is the mouse menu open */
	bool IsWheelMenuOpen() const;

	/** Notify the HUD the played ship has changed */
	void OnTargetShipChanged();

	/** Decide if the HUD is displayed or not */
	void UpdateHUDVisibility();
	
	/** Reset target */
	void RemoveTarget(AFlareSpacecraft* Target);

	/** Reset all targets */
	void RemoveAllTargets();

	/** We just hit this spacecraft with a weapon */
	void SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType);


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
	

	/*----------------------------------------------------
		HUD helpers
	----------------------------------------------------*/

	/** Update the context menu */
	void UpdateContextMenu(AFlareSpacecraft* PlayerShip);

	/** Get the temperature color, using custom threshold */
	static FLinearColor GetTemperatureColor(float Current, float Max);

	/** Get the health color */
	static FLinearColor GetHealthColor(float Current);

	void SetDistortionGrid(uint32 Value)
	{
		DistortionGrid = Value;
	}

	/** Change a distortion value */
	void SetDistortion(uint32 Axis, uint32 X, uint32 Y, float Value);

	/** Format a distance in meter */
	static FString FormatDistance(float Distance);


	/*----------------------------------------------------
		Internals
	----------------------------------------------------*/

protected:

	/** Should we draw the HUD ? */
	bool ShouldDrawHUD() const;

	/** Drawing back-end */
	void DrawHUDInternal();

	/** Drawing debug grid*/
	void DrawDebugGrid (FLinearColor Color);

	/** Draw speed indicator */
	void DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed);

	/** Draw a search arrow */
	void DrawSearchArrow(FVector TargetLocation, FLinearColor Color, bool Highlighted, float MaxDistance = 10000000);

	/** Draw a designator block around a spacecraft */
	bool DrawHUDDesignator(AFlareSpacecraft* Spacecraft);

	/** Draw a designator corner */
	void DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float IconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Dangerous, bool Highlighted);

	/** Draw a status block for the ship */
	void DrawHUDDesignatorStatus(FVector2D Position, float IconSize, AFlareSpacecraft* Ship);

	/** Draw a docking helper around a station */
	void DrawDockingHelper(AFlareSpacecraft* Spacecraft);

	/** Draw a status icon */
	FVector2D DrawHUDDesignatorStatusIcon(FVector2D Position, float IconSize, UTexture2D* Texture);

	/** Draw an icon */
	void DrawHUDIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, bool Center = false);

	/** Draw an icon */
	void DrawHUDIconRotated(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, float Rotation = 0);

	/** Print a text with a shadow */
	void FlareDrawText(FString Text, FVector2D Position, FLinearColor Color = FLinearColor::White, bool Center = true, bool Large = false);

	/** Draw a texture */
	void FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector);
	
	/** Get an alpha fade to avoid overdrawing two objects */
	float GetFadeAlpha(FVector2D A, FVector2D B, bool UseCockpit);

	/** Is this position inside the viewport + border */
	bool IsInScreen(FVector2D ScreenPosition) const;

	/** Get the appropriate hostility color */
	FLinearColor GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraft* Target);

	/** Is the player flying a military ship */
	bool IsFlyingMilitaryShip() const;
	
	/** Get the distortion grid */
	float* GetCurrentHorizontalGrid() const;

	/** Get the distortion grid */
	float* GetCurrentVerticalGrid() const;
	
	/** Convert a world location to cockpit-space */
	bool ProjectWorldLocationToCockpit(FVector World, FVector2D& Cockpit);

	/** Convert a screen location to cockpit-space */
	bool ScreenToCockpit(FVector2D Screen, FVector2D& Cockpit);


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
	FLinearColor                            ShadowColor;
	
	// Spacecraft targets
	UPROPERTY()
	TArray<FFlareScreenTarget>              ScreenTargets;
	FName									ScreenTargetsOwner;

	// General data
	bool                                    HUDVisible;
	bool                                    IsInteractive;
	bool                                    IsDrawingCockpit;
	bool                                    IsDrawingHUD;
	FVector2D                               ViewportSize;
	AFlareSpacecraft*                       ContextMenuSpacecraft;

	// Current data
	FVector2D                               CurrentViewportSize;
	UCanvas*                                CurrentCanvas;

	// Hit target
	AFlareSpacecraft*                       PlayerHitSpacecraft;
	EFlareDamage::Type                      PlayerDamageType;
	float                                   PlayerHitTime;
	float                                   PlayerHitDisplayTime;

	// Designator content
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDCombatReticleIcon;
	UTexture2D*                             HUDBackReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDAimHitIcon;
	UTexture2D*                             HUDBombAimIcon;
	UTexture2D*                             HUDBombMarker;
	UTexture2D*                             HUDAimHelperIcon;
	UTexture2D*                             HUDNoseIcon;
	UTexture2D*                             HUDObjectiveIcon;
	UTexture2D*                             HUDCombatMouseIcon;
	UTexture2D*                             HUDSearchArrowIcon;
	UTexture2D*                             HUDHighlightSearchArrowTexture;
	UTexture2D*                             HUDDesignatorCornerTexture;
	UTexture2D*                             HUDDesignatorMilCornerTexture;
	UTexture2D*                             HUDDesignatorCornerSelectedTexture;
	UTexture2D*                             HUDDesignatorSelectionTexture;
	UTexture2D*                             HUDDockingCircleTexture;
	UTexture2D*                             HUDDockingAxisTexture;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDRCSIcon;
	UTexture2D*                             HUDHealthIcon;
	UTexture2D*                             HUDWeaponIcon;
	UTexture2D*                             HUDHarpoonedIcon;

	// Font
	UFont*                                  HUDFontSmall;
	UFont*                                  HUDFont;
	UFont*                                  HUDFontMedium;
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

	// Debug
	uint32                                  DistortionGrid;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	/** Get the spacecrafts shown on screen */
	inline TArray<FFlareScreenTarget>& GetCurrentTargets()
	{
		return ScreenTargets;
	}

	inline FName GetCurrentTargetsOwner()
	{
		return ScreenTargetsOwner;
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
