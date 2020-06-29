#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareDropList.h"
#include "../Components/FlareListItem.h"

#include <Widgets/Layout/SBackgroundBlur.h>


struct FFlareCompanyDescription;
struct FFlareSectorCelestialBodyDescription;
class AFlareMenuManager;


class SFlareSkirmishSetupMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSkirmishSetupMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter();

	/** Exit this menu */
	void Exit();


protected:
	
	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	FText GetAltitudeValue() const;

	FText GetAsteroidValue() const;

	FText GetDebrisValue() const;

	FText GetPlayerFleetTitle() const;

	FText GetEnemyFleetTitle() const;

	TSharedRef<ITableRow> OnGenerateSpacecraftLine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Item, const TSharedRef<STableViewBase>& OwnerTable);	

	TSharedRef<SWidget> OnGenerateCompanyComboLine(FFlareCompanyDescription Item);
	FText OnGetCurrentCompanyComboLine() const;
	
	TSharedRef<SWidget> OnGeneratePlanetComboLine(FFlareSectorCelestialBodyDescription Item);
	FText OnGetCurrentPlanetComboLine() const;

	bool IsStartDisabled() const;

	FText GetAddToPlayerFleetText() const;

	FText GetAddToEnemyFleetText() const;

	bool IsAddToPlayerFleetDisabled() const;

	bool IsAddToEnemyFleetDisabled() const;

	FText GetStartHelpText() const;


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Asteroid density */
	void OnAsteroidSliderChanged(float Value, bool ForPlayer);

	/** Debris density */
	void OnDebrisSliderChanged(float Value, bool ForPlayer);
	
	/** Clear the fleet */
	void OnClearFleet(bool ForPlayer);

	/** Sort the player fleet */
	void OnSortPlayerFleet();

	/** Do an automatic fleet for the enemy */
	void OnAutoCreateEnemyFleet();

	/** Start the process of ordering a new ship */
	void OnOrderShip(bool ForPlayer);

	/** Order a new ship */
	void OnOrderShipConfirmed(FFlareSpacecraftDescription* Spacecraft);

	/** Upgrade spacecraft */
	void OnUpgradeSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order);

	/** Remove spacecraft */
	void OnRemoveSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order);

	/** Duplicate spacecraft */
	void OnDuplicateSpacecraft(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order);

	// Upgrade callbacks
	void OnUpgradeEngine(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, FName Upgrade);
	void OnUpgradeRCS(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, FName Upgrade);
	void OnUpgradeWeapon(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order, int32 GroupIndex, FName Upgrade);

	/** Close upgrade panel */
	void OnCloseUpgradePanel();

	/** Start engagement */
	void OnStartSkirmish();

	/** Quit the menu */
	void OnMainMenu();


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	bool CanStartPlaying(FText& Reason) const;

	void SetOrderDefaults(TSharedPtr<FFlareSkirmishSpacecraftOrder> Order);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Data
	TWeakObjectPtr<class AFlareMenuManager>                  MenuManager;

	// Settings widgets
	TSharedPtr<SFlareDropList<FFlareCompanyDescription>>               CompanySelector;
	TSharedPtr<SFlareDropList<FFlareSectorCelestialBodyDescription>>   PlanetSelector;
	TSharedPtr<SSlider>                                                AltitudeSlider;
	TSharedPtr<SSlider>                                                AsteroidSlider;
	TSharedPtr<SSlider>                                                DebrisSlider;
	TSharedPtr<SFlareButton>                                           IcyButton;
	TSharedPtr<SFlareButton>                                           MetalDebrisButton;

	// Lists
	TSharedPtr<SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>>   PlayerSpacecraftList;
	TSharedPtr<SListView<TSharedPtr<FFlareSkirmishSpacecraftOrder>>>   EnemySpacecraftList;

	// List data
	UPROPERTY()
	TArray<TSharedPtr<FFlareSkirmishSpacecraftOrder>>        PlayerSpacecraftListData;
	UPROPERTY()
	TArray<TSharedPtr<FFlareSkirmishSpacecraftOrder>>        EnemySpacecraftListData;

	// Upgrade widgets
	TSharedPtr<SBackgroundBlur>                              UpgradeBox;
	TSharedPtr<SVerticalBox>                                 OrbitalEngineBox;
	TSharedPtr<SVerticalBox>                                 RCSBox;
	TSharedPtr<SHorizontalBox>                               WeaponBox;

	// State
	bool                                                     IsOrderingForPlayer;

};
