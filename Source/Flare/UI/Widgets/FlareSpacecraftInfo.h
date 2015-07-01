#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../Widgets/FlareShipStatus.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"


class SFlareSpacecraftInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSpacecraftInfo)
		: _Player(NULL)
		, _Spacecraft(NULL)
		, _NoInspect(false)
		, _Minimized(false)
		, _Visible(false)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(IFlareSpacecraftInterface*, Spacecraft)

	SLATE_ARGUMENT(bool, NoInspect)
	SLATE_ARGUMENT(bool, Minimized)
	SLATE_ARGUMENT(bool, Visible)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Set a spacecraft as content */
	void SetSpacecraft(IFlareSpacecraftInterface* Target);

	/** Set the no-inspect mode */
	void SetNoInspect(bool NewState);

	/** Set the minimized mode */
	void SetMinimized(bool NewState);

	/** SHow the menu */
	void Show();

	/** Hide the menu */
	void Hide();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the current target */
	void OnInspect();

	/** Fly the current target */
	void OnFly();

	/** Try to dock at the target station */
	void OnDockAt();

	/** Undock */
	void OnUndock();


	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get the target name */
	FText GetName() const;

	/** Get the target class name */
	FText GetDescription() const;

	/** Get the target icon */
	const FSlateBrush* GetIcon() const;

	/** Get the target class icon */
	const FSlateBrush* GetClassIcon() const;

	/** Get the company name */
	FText GetCompanyName() const;
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;
	bool                              NoInspect;
	bool                              Minimized;

	// Target data	
	IFlareSpacecraftInterface*        TargetSpacecraft;
	FFlareSpacecraftDescription*      TargetSpacecraftDesc;
	FString                           TargetName;

	// Slate data
	TSharedPtr<SFlareButton>          InspectButton;
	TSharedPtr<SFlareButton>          FlyButton;
	TSharedPtr<SFlareButton>          DockButton;
	TSharedPtr<SFlareButton>          UndockButton;
	TSharedPtr<SFlareShipStatus>      ShipStatus;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;

};
